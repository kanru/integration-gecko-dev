#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <unistd.h>
#include <sys/types.h>

#include "mozilla/DebugOnly.h"

#include "jsnum.h"
#include "jsprf.h"
#include "jsscript.h"
#include "js/TypeDecls.h"
#include "prmjtime.h"

#if defined(__GLIBC__)
// glibc doesn't implement gettid(2).
#include <sys/syscall.h>
static pid_t _gettid()
{
  return (pid_t) syscall(SYS_gettid);
}
#else
static pid_t _gettid()
{
  return gettid();
}
#endif

#define HEAP_NURSERY    0
#define HEAP_TENURED    1
#define HEAP_NATIVE     2

namespace std {
    namespace tr1 {
    }
    using namespace tr1;
}

std::vector<const char *> (*GetStacktrace)();

static size_t globalSwitch = 0;

static size_t nRemoved;
static size_t nMarked;
static size_t nSampled;
static size_t nRecorded;

const int32_t SAMPLE_SIZE = 4096;

class AutoMPLock
{
  public:
    explicit AutoMPLock(PRLock *lock, pid_t *owner)
    {
        MOZ_ASSERT(lock, "Parameter should not be null!");
        PR_Lock(lock);
        *owner = _gettid();
        lock_ = lock;
        owner_ = owner;
    }

    ~AutoMPLock() {
        if (owner_)
            *owner_ = 0;
        if (lock_)
            PR_Unlock(lock_);
    }

    void unlock() {
        if (owner_)
            *owner_ = 0;
        if (lock_)
            PR_Unlock(lock_);
        owner_ = nullptr;
        lock_ = nullptr;
    }

  private:
    PRLock *lock_;
    pid_t *owner_;
};

struct AllocTable {
    // Each allocation corresponds to a AllocEntry, which contains the stacktrace and size.
    struct AllocEntry {
        // Microseconds.
        int64_t mTimestamp;
        // index to a stacktrace singleton.
        uint32_t mTraceIdx;
        // Allocation size
        int mCount : 31;
        int marked : 1;

        AllocEntry(AllocTable *table, const std::vector<const char *> &aRawStacktrace, int32_t aCount, int64_t aTimestamp)
        {
            mCount = aCount;
            marked = 0;
            mTimestamp = aTimestamp;

            uint32_t parent = 0;
            for (auto i: aRawStacktrace) {
                auto name = table->frameNames.insert(std::make_pair(std::string(i), table->frameNames.size()));
                if (name.second) {
                    table->vFrameNames.push_back(&name.first->first);
                }
                uint32_t idx = table->vTraces.size();
                auto trace = table->xTraces.insert(std::make_pair(std::make_pair(parent, name.first->second), idx));
                if (trace.second) {
                    table->vTraces.push_back(TrieNode{parent, name.first->second});
                    parent = idx;
                } else {
                    parent = trace.first->second;
                }
            }
            mTraceIdx = parent;
        }
    };

    struct TrieNode {
        uint32_t parentIdx;
        uint32_t nameIdx;
    };

    struct TrieHasher {
        size_t operator()(const std::pair<uint32_t, uint32_t> &v) const noexcept {
            uint64_t k = static_cast<uint64_t>(v.first) << 32 | v.second;
            return std::hash<uint64_t>()(k);
        }
    };

    // Record allocations in a address -> AllocEntry map.
    std::unordered_map<void *, AllocEntry> nurseryEntries;
    std::unordered_map<void *, AllocEntry> retainingEntries;
    std::unordered_map<void *, AllocEntry> retainingEntriesBG;
    std::unordered_map<void *, AllocEntry> nativeEntries;
    std::vector<std::pair<void *, AllocEntry>> allocatedEntries;
    // To avoid redundent names.
    std::unordered_map<std::string, uint32_t> frameNames;
    std::vector<const std::string *> vFrameNames;
    // Tries in a vector with branches and indexes in an unordered_map.
    std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t, TrieHasher> xTraces;
    std::vector<TrieNode> vTraces;

    AllocTable() {
        init();
    }

    void init() {
        auto name = frameNames.insert(std::make_pair(std::string("(native)"), 0));
        vFrameNames.push_back(&name.first->first);
        xTraces.insert(std::pair<std::pair<uint32_t, uint32_t>, uint32_t>(std::pair<uint32_t, uint32_t>(0, 0), 0));
        vTraces.push_back(TrieNode{0, 0});
    }

    bool insert(void *aAddress, const std::vector<const char *> &aRawStacktrace, int32_t aCount, int heapKind)
    {
        AllocEntry ai(this, aRawStacktrace, aCount, PRMJ_Now());
        std::unordered_map<void *, AllocEntry> *tbl;
        switch (heapKind) {
            case HEAP_NURSERY:
                tbl = &nurseryEntries;
                break;
            case HEAP_TENURED:
                tbl = &retainingEntries;
                break;
            case HEAP_NATIVE:
                tbl = &nativeEntries;
                break;
        }
        auto res = tbl->insert(std::make_pair(aAddress, ai));
        allocatedEntries.push_back(std::make_pair(aAddress, ai));
        return res.second;
    }

    bool relocate(void *aOldAddress, void *aNewAddress)
    {
        auto iterOld = nurseryEntries.find(aOldAddress);
        if (iterOld == nurseryEntries.end())
            return false;

        auto iterNew = retainingEntries.find(aNewAddress);
        if (iterNew == retainingEntries.end())
            return false;

        AllocEntry ai = iterOld->second;
        iterNew->second.mTraceIdx = ai.mTraceIdx;
        return true;
    }

    bool remove(void *aAddress)
    {
        auto res = nativeEntries.find(aAddress);
        if (res == nativeEntries.end())
            return false;

        AllocEntry ai = res->second;
        ai.mCount = 0 - ai.mCount;
        allocatedEntries.push_back(std::make_pair(aAddress, ai));
        nativeEntries.erase(res);

        return true;
    }

    void mark_start()
    {
        std::swap(retainingEntries, retainingEntriesBG);
    }

    void mark(void *aAddress)
    {
        auto res = retainingEntriesBG.find(aAddress);
        if (res != retainingEntriesBG.end())
            res->second.marked = -1;
    }

    void sweep()
    {
        for (auto i: retainingEntriesBG) {
            if (i.second.marked != 0) {
                i.second.marked = 0;
                retainingEntries.insert(i);
            } else {
                AllocEntry ai = i.second;
                ai.mCount = 0 - ai.mCount;
                allocatedEntries.push_back(std::make_pair(i.first, ai));
            }
        }
        retainingEntriesBG.clear();
    }

    void collect()
    {
        for (auto i: nurseryEntries) {
            AllocEntry ai = i.second;
            ai.mCount = 0 - ai.mCount;
            allocatedEntries.push_back(std::make_pair(i.first, ai));
        }
        nurseryEntries.clear();
    }

    bool reset()
    {
        nurseryEntries.clear();
        retainingEntries.clear();
        retainingEntriesBG.clear();
        nativeEntries.clear();
        allocatedEntries.clear();
        frameNames.clear();
        vFrameNames.clear();
        xTraces.clear();
        vTraces.clear();
        init();
        return true;
    }
};

void MPStart(JSRuntime *runtime)
{
    js::EnableRuntimeProfilingStack(runtime, true);
    runtime->mprofiler.start();
    globalSwitch++;
}

void MPStop(JSRuntime *runtime)
{
    globalSwitch--;
    runtime->mprofiler.stop();
    js::EnableRuntimeProfilingStack(runtime, false);
}

void MPSampleNurseryHeap(void *addr, int32_t size)
{
    static int32_t cumulated = 0;

    nSampled++;
    cumulated += size;
    if (cumulated >= SAMPLE_SIZE) {
        int32_t count = cumulated / SAMPLE_SIZE;
        cumulated %= SAMPLE_SIZE;

        nRecorded++;
        JSRuntime *runtime = reinterpret_cast<js::gc::Cell *>(addr)->runtimeFromAnyThread();
        runtime->mprofiler.record(addr, count, HEAP_NURSERY);
    }
}

void MPSampleTenuredHeap(void *addr, int32_t size)
{
    static int32_t cumulated = 0;

    nSampled++;
    cumulated += size;
    if (cumulated >= SAMPLE_SIZE) {
        int32_t count = cumulated / SAMPLE_SIZE;
        cumulated %= SAMPLE_SIZE;

        nRecorded++;
        JSRuntime *runtime = reinterpret_cast<js::gc::Cell *>(addr)->runtimeFromAnyThread();
        runtime->mprofiler.record(addr, count, HEAP_TENURED);
    }
}

extern "C" void MPSampleNativeHeap(void *addr, int32_t size)
{
    if (globalSwitch == 0)
        return;
    static int32_t cumulated = 0;

    nSampled++;
    cumulated += size;
    if (cumulated >= SAMPLE_SIZE) {
        int32_t count = cumulated / SAMPLE_SIZE;
        cumulated %= SAMPLE_SIZE;

        nRecorded++;
        JSRuntime *runtime = nullptr;
        if (js::TlsPerThreadData.get())
            runtime = js::TlsPerThreadData.get()->runtimeIfOnOwnerThread();
        if (runtime)
            runtime->mprofiler.record(addr, count, HEAP_NATIVE);
    }
}

extern "C" void MPRemove(void *addr)
{
    if (globalSwitch == 0)
        return;
    nRemoved++;
    JSRuntime *runtime = nullptr;
    if (js::TlsPerThreadData.get())
        runtime = js::TlsPerThreadData.get()->runtimeIfOnOwnerThread();
    if (runtime)
        runtime->mprofiler.remove(addr);
}

void MPRemoveUnchecked(void *addr)
{
    nRemoved++;
    JSRuntime *runtime = reinterpret_cast<js::gc::Cell *>(addr)->runtimeFromAnyThread();
    runtime->mprofiler.remove_unchecked(addr);
}

void MPRelocate(void *addrOld, void *addrNew)
{
    JSRuntime *runtime = reinterpret_cast<js::gc::Cell *>(addrOld)->runtimeFromAnyThread();
    runtime->mprofiler.relocate(addrOld, addrNew);
}

void MPMarkStart(JSRuntime *runtime)
{
    runtime->mprofiler.markStart();
}

void MPSweep(JSRuntime *runtime)
{
    runtime->mprofiler.sweep();
}

void MPCollect(JSRuntime *runtime)
{
    runtime->mprofiler.collect();
}

void MPMark(void *addr)
{
    nMarked++;
    JSRuntime *runtime = reinterpret_cast<js::gc::Cell *>(addr)->runtimeFromAnyThread();
    runtime->mprofiler.mark(addr);
}

void MPReset()
{
    JSRuntime *runtime = js::TlsPerThreadData.get()->runtimeFromMainThread();

    runtime->mprofiler.clear();
}

void MPReset(JSRuntime *runtime)
{
    runtime->mprofiler.clear();
}

PRLock *MPGetLock(JSRuntime *runtime)
{
    return runtime->mprofiler.getLock();
}

bool MPIsActive(JSRuntime *runtime)
{
    return runtime->mprofiler.isActive();
}

__attribute__((visibility("default")))
void MPDebugDump()
{
    JSRuntime *runtime = js::TlsPerThreadData.get()->runtimeFromMainThread();

    char *buf = (char *)js_malloc(65536);
    runtime->mprofiler.dump2buf(buf, 65536);
    printf("%s", buf);
    js_free(buf);
}

char *MPGetResult(JSRuntime *runtime)
{
    char *buf = (char *)js_malloc(65536);
    runtime->mprofiler.dump2buf(buf, 65536);
    return buf;
}

JSObject* MPGetFrameNameTable(JSRuntime *runtime, JSContext *cx)
{
    AutoMPLock lock(runtime->mprofiler.lock_, &runtime->mprofiler.lockOwner_);
    bool active = runtime->mprofiler.active_;
    runtime->mprofiler.active_ = false;
    lock.unlock();
    auto& tbl = runtime->mprofiler.allocTable_->vFrameNames;

    JS::RootedObject array(cx, JS_NewArrayObject(cx, tbl.size()));
    for (size_t i = 0; i < tbl.size(); i++) {
        JS::RootedString name(cx, JS_NewStringCopyZ(cx, tbl[i]->c_str()));
        JS_SetElement(cx, array, i, name);
    }

    runtime->mprofiler.active_ = active;
    return array;
}

JSObject* MPGetStacktraceTable(JSRuntime *runtime, JSContext *cx)
{
    AutoMPLock lock(runtime->mprofiler.lock_, &runtime->mprofiler.lockOwner_);
    bool active = runtime->mprofiler.active_;
    runtime->mprofiler.active_ = false;
    lock.unlock();
    auto& tbl = runtime->mprofiler.allocTable_->vTraces;

    JS::RootedObject array(cx, JS_NewArrayObject(cx, tbl.size()));
    for (size_t i = 0; i < tbl.size(); i++) {
        JS::RootedObject tn(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
        JS::RootedValue nameIdx(cx, JS_NumberValue(tbl[i].nameIdx));
        JS::RootedValue parentIdx(cx, JS_NumberValue(tbl[i].parentIdx));
        JS_SetProperty(cx, tn, "nameIdx", nameIdx);
        JS_SetProperty(cx, tn, "parentIdx", parentIdx);
        JS_SetElement(cx, array, i, tn);
    }

    runtime->mprofiler.active_ = active;
    return array;
}

JSObject* MPGetRetainingEntries(JSRuntime *runtime, JSContext *cx)
{
    AutoMPLock lock(runtime->mprofiler.lock_, &runtime->mprofiler.lockOwner_);
    bool active = runtime->mprofiler.active_;
    runtime->mprofiler.active_ = false;
    lock.unlock();
    auto& tbl = runtime->mprofiler.allocTable_->retainingEntries;

    JS::RootedObject array(cx, JS_NewArrayObject(cx, tbl.size()));
    size_t i = 0;
    for (auto ent: tbl) {
        JS::RootedObject tn(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
        JS::RootedValue size(cx, JS_NumberValue(ent.second.mCount * SAMPLE_SIZE));
        JS::RootedValue traceIdx(cx, JS_NumberValue(ent.second.mTraceIdx));
        JS_SetProperty(cx, tn, "size", size);
        JS_SetProperty(cx, tn, "traceIdx", traceIdx);
        JS_SetElement(cx, array, i, tn);
        i++;
    }

    runtime->mprofiler.active_ = active;
    return array;
}

JSObject* MPGetAllocatedEntries(JSRuntime *runtime, JSContext *cx)
{
    AutoMPLock lock(runtime->mprofiler.lock_, &runtime->mprofiler.lockOwner_);
    bool active = runtime->mprofiler.active_;
    runtime->mprofiler.active_ = false;
    lock.unlock();
    auto& tbl = runtime->mprofiler.allocTable_->allocatedEntries;

    JS::RootedObject array(cx, JS_NewArrayObject(cx, tbl.size()));
    size_t i = 0;
    for (auto ent: tbl) {
        if (ent.second.mCount == 0)
            continue;
        JS::RootedObject tn(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
        JS::RootedValue size(cx, JS_NumberValue(ent.second.mCount * SAMPLE_SIZE));
        JS::RootedValue traceIdx(cx, JS_NumberValue(ent.second.mTraceIdx));
        JS::RootedValue timestamp(cx, JS_NumberValue(ent.second.mTimestamp));
        JS_SetProperty(cx, tn, "size", size);
        JS_SetProperty(cx, tn, "traceIdx", traceIdx);
        JS_SetProperty(cx, tn, "timestamp", timestamp);
        JS_SetElement(cx, array, i, tn);
        i++;
    }

    runtime->mprofiler.active_ = active;
    return array;
}

MProfiler::MProfiler()
{
    active_ = false;
    lock_ = PR_NewLock();
    allocTable_ = new AllocTable();
    marking_ = false;
    lockOwner_ = 0;
}

MProfiler::~MProfiler()
{
    PR_Lock(lock_);
    active_ = false;
    PR_Unlock(lock_);
    delete allocTable_;
    allocTable_ = nullptr;
    PR_DestroyLock(lock_);
}

void MProfiler::clear()
{
    AutoMPLock lock(lock_, &lockOwner_);
    //MOZ_ASSERT(!active_);
    allocTable_->reset();
    marking_ = false;
}

static std::vector<std::string> getStacktrace()
{
    std::vector<std::string> trace;

    PerThreadData *ptd = TlsPerThreadData.get();
    if (!ptd)
        return trace;

    js::Activation *activation = ptd->activation();
    if (!activation) {
        JSRuntime* runtime = ptd->runtimeIfOnOwnerThread();
        if (!runtime)
            return trace;

        activation = runtime->mainThread.activation();
        if (!activation)
            return trace;
    }

    if (activation->isJit() && (ptd->jitTop == (uint8_t *)0xba1 || ptd->jitTop == nullptr))
        return trace;

    ThreadSafeContext *tscx = activation->cx();
    if (!tscx || !tscx->isJSContext())
        return trace;

    JSContext* cx = tscx->asJSContext();
    for (FrameIter i(cx, FrameIter::ALL_CONTEXTS, FrameIter::GO_THROUGH_SAVED); !i.done(); ++i) {
        const char *filename;
        unsigned line;
        uint32_t column;

        if (!i.hasScript()) {
            filename = i.scriptFilename();
            line = i.computeLine(&column);
        } else {
            RootedScript script(cx, i.script());
            filename = script->filename();
#if 0
            line = PCToLineNumber(script, i.pc(), &column);
#else
            line = script->lineno();
            column = script->column();
#endif
        }
        if (!filename)
            filename = "";

        RootedAtom name(cx, i.isNonEvalFunctionFrame() ? i.functionDisplayAtom() : nullptr);
        std::string ss;
        JSAutoByteString bytes;
        if (name) {
            const char *cstr;
            cstr = bytes.encodeLatin1(cx, name);
            ss = std::string(cstr);
        } else {
            ss = "(unknown)";
        }
        char strbuf[16];
        snprintf(strbuf, 16, "%u", line);
        trace.push_back(ss + " @ " + filename + ": " + strbuf);
    }
    return trace;
}

void MPSetStacktracer(JSRuntime *runtime, std::vector<const char *>(*get)())
{
    GetStacktrace = get;
}

void MProfiler::record(void *addr, int32_t size, int heapKind)
{
    if (lockOwner_ == _gettid())
        return;
    AutoMPLock lock(lock_, &lockOwner_);
    if (!active_)
        return;

#if 0
    std::vector<std::string> strace = getStacktrace();
    std::vector<const char *> trace;
    for (int i = strace.size() - 1; i >= 0; i--)
        trace.push_back(strace[i].c_str());
#else
    if (!GetStacktrace)
        return;
    std::vector<const char *> trace = GetStacktrace();
#endif
    allocTable_->insert(addr, trace, size, heapKind);
}

void MProfiler::remove(void *addr)
{
    if (lockOwner_ == _gettid())
        return;
    AutoMPLock lock(lock_, &lockOwner_);
    if (!active_)
        return;
    allocTable_->remove(addr);
}

void MProfiler::remove_unchecked(void *addr)
{
    MOZ_ASSERT(pthread_mutex_trylock(reinterpret_cast<pthread_mutex_t *>(lock_)) != 0);
    allocTable_->remove(addr);
}

void MProfiler::relocate(void *old_addr, void *new_addr)
{
    AutoMPLock lock(lock_, &lockOwner_);
    if (!active_)
        return;

    allocTable_->relocate(old_addr, new_addr);
}

void MProfiler::start()
{
    AutoMPLock lock(lock_, &lockOwner_);
    MOZ_ASSERT(!active_);
    active_ = true;
}

void MProfiler::stop()
{
    AutoMPLock lock(lock_, &lockOwner_);
    MOZ_ASSERT(active_);
    active_ = false;
}

void MProfiler::markStart()
{
    AutoMPLock lock(lock_, &lockOwner_);
    if (!active_ || marking_)
        return;
    marking_ = true;
    allocTable_->mark_start();
}

void MProfiler::sweep()
{
    AutoMPLock lock(lock_, &lockOwner_);
    if (!active_ || !marking_)
        return;
    allocTable_->sweep();
    marking_ = false;
}

void MProfiler::collect()
{
    AutoMPLock lock(lock_, &lockOwner_);
    if (!active_)
        return;
    allocTable_->collect();
}

void MProfiler::mark(void *addr)
{
    // Because there's no change to the table itself and
    // only writes to its entries, no lock is needed.
    if (!marking_)
        return;
    allocTable_->mark(addr);
}

struct StatEnt {
    const char *name = nullptr;
    size_t selfSize = 0;
    size_t selfCount = 0;
    size_t totalSize = 0;
    size_t totalCount = 0;
    StatEnt(const char *aName) : name(aName) {};
};

using namespace std;
std::string& stringappendf(std::string& s, const char *fmt, ...)
{
    char line[256];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);

    s.append(line).append("\n");
    return s;
}

void MProfiler::dump2buf(char *buf, size_t size)
{
    AutoMPLock lock(lock_, &lockOwner_);

    std::string ss;
    stringappendf(ss, "%s: %zu", "RetainingEntries", allocTable_->retainingEntries.size());
    stringappendf(ss, "%s: %zu", "AllocatedEntries", allocTable_->allocatedEntries.size());
    stringappendf(ss, "%s: %zu", "Names", allocTable_->frameNames.size());
    stringappendf(ss, "%s: %zu", "TraceNodes", allocTable_->vTraces.size());
    stringappendf(ss, "%s: %zu", "nRemoved", nRemoved);
    stringappendf(ss, "%s: %zu", "nMarked", nMarked);
    stringappendf(ss, "%s: %zu", "nSampled", nSampled);
    stringappendf(ss, "%s: %zu", "nRecorded", nRecorded);

    std::vector<StatEnt> hist;

    // Dump allocation history.
    for (auto &i: allocTable_->vFrameNames) {
        hist.push_back(StatEnt(i->c_str()));
    }

    for (auto i: allocTable_->allocatedEntries) {
        for (auto j = i.second.mTraceIdx; j != 0; j = allocTable_->vTraces[j].parentIdx) {
            hist[allocTable_->vTraces[j].nameIdx].totalSize += i.second.mCount;
            if (i.second.mCount > 0)
                hist[allocTable_->vTraces[j].nameIdx].totalCount++;
            else
                hist[allocTable_->vTraces[j].nameIdx].totalCount--;
        }
        hist[allocTable_->vTraces[i.second.mTraceIdx].nameIdx].selfSize += i.second.mCount;
        if (i.second.mCount > 0)
            hist[allocTable_->vTraces[i.second.mTraceIdx].nameIdx].selfCount++;
        else
            hist[allocTable_->vTraces[i.second.mTraceIdx].nameIdx].selfCount--;
    }

    sort(hist.begin(), hist.end(),
         [](const StatEnt& a, const StatEnt& b){return a.selfSize > b.selfSize;});

    stringappendf(ss, "    SizeSelf   SizeTotal   CountSelf  CountTotal  Name");
    for (size_t i = 0; i < 20 && i < hist.size(); i++) {
        auto &st = hist[i];
        stringappendf(ss, "%12zu%12zu%12zu%12zu  %s", st.selfSize * SAMPLE_SIZE, st.totalSize * SAMPLE_SIZE,
                                             st.selfCount, st.totalCount,
                                             st.name);
    }
    stringappendf(ss, "");

    // Dump allocation history.
    hist.clear();
    for (auto &i: allocTable_->vFrameNames) {
        hist.push_back(StatEnt(i->c_str()));
    }

    for (auto i: allocTable_->allocatedEntries) {
        if (i.second.mCount > 0) {
            for (auto j = i.second.mTraceIdx; j != 0; j = allocTable_->vTraces[j].parentIdx) {
                hist[allocTable_->vTraces[j].nameIdx].totalSize += i.second.mCount;
                hist[allocTable_->vTraces[j].nameIdx].totalCount++;
            }
            hist[allocTable_->vTraces[i.second.mTraceIdx].nameIdx].selfSize += i.second.mCount;
            hist[allocTable_->vTraces[i.second.mTraceIdx].nameIdx].selfCount++;
        }
    }

    sort(hist.begin(), hist.end(),
         [](const StatEnt& a, const StatEnt& b){return a.selfSize > b.selfSize;});

    stringappendf(ss, "    SizeSelf   SizeTotal   CountSelf  CountTotal  Name");
    for (size_t i = 0; i < 20 && i < hist.size(); i++) {
        auto &st = hist[i];
        stringappendf(ss, "%12zu%12zu%12zu%12zu  %s", st.selfSize * SAMPLE_SIZE, st.totalSize * SAMPLE_SIZE,
                                             st.selfCount, st.totalCount,
                                             st.name);
    }
    stringappendf(ss, "");

    strncpy(buf, ss.c_str(), size);
}
