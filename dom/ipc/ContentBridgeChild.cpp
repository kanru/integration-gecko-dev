/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ContentBridgeChild.h"

#include "mozilla/dom/ContentChild.h"
#include "mozilla/dom/ContentContentChild.h"
#include "mozilla/dom/StructuredCloneUtils.h"
#include "mozilla/dom/TabChild.h"
#include "mozilla/ipc/InputStreamUtils.h"
#include "nsDOMFile.h"

using namespace base;
using namespace mozilla::ipc;

namespace mozilla {
namespace dom {

NS_IMPL_ISUPPORTS(ContentBridgeChild, ContentBridgeChild, nsIContentChild)

ContentBridgeChild::ContentBridgeChild(Transport* aTransport)
  : mTransport(aTransport)
{}

ContentBridgeChild::~ContentBridgeChild()
{
  XRE_GetIOMessageLoop()->PostTask(FROM_HERE, new DeleteTask<Transport>(mTransport));
}

void
ContentBridgeChild::ActorDestroy(ActorDestroyReason aWhy)
{
  MessageLoop::current()->PostTask(
    FROM_HERE,
    NewRunnableMethod(this, &ContentBridgeChild::DeferredDestroy));
}

ContentContentChild*
ContentBridgeChild::ContentContent()
{
  if (ManagedPContentContentChild().Length()) {
    return static_cast<ContentContentChild*>(
      ManagedPContentContentChild()[0]);
  }

  ContentContentChild* actor =
    static_cast<ContentContentChild*>(SendPContentContentConstructor());
  return actor;
}

/*static*/ ContentBridgeChild*
ContentBridgeChild::Create(Transport* aTransport, ProcessId aOtherProcess)
{
  nsRefPtr<ContentBridgeChild> bridge =
    new ContentBridgeChild(aTransport);
  ProcessHandle handle;
  if (!base::OpenProcessHandle(aOtherProcess, &handle)) {
    // XXX need to kill |aOtherProcess|, it's boned
    return nullptr;
  }
  bridge->mSelfRef = bridge;

  DebugOnly<bool> ok = bridge->Open(aTransport, handle, XRE_GetIOMessageLoop());
  MOZ_ASSERT(ok);
  return bridge;
}

void
ContentBridgeChild::DeferredDestroy()
{
  mSelfRef = nullptr;
  // |this| was just destroyed, hands off
}

bool
ContentBridgeChild::RecvAsyncMessage(const nsString& aMsg,
                                     const ClonedMessageData& aData,
                                     const InfallibleTArray<jsipc::CpowEntry>& aCpows,
                                     const IPC::Principal& aPrincipal)
{
  return nsIContentChild::RecvAsyncMessage(aMsg, aData, aCpows, aPrincipal);
}

bool
ContentBridgeChild::SendPBrowserConstructor(PBrowserChild* aActor,
                                            const IPCTabContext& aContext,
                                            const uint32_t& aChromeFlags,
                                            const uint64_t& aID,
                                            const bool& aIsForApp,
                                            const bool& aIsForBrowser)
{
  return PContentBridgeChild::SendPBrowserConstructor(aActor,
                                                      aContext,
                                                      aChromeFlags,
                                                      aID,
                                                      aIsForApp,
                                                      aIsForBrowser);
}

PContentContentChild*
ContentBridgeChild::AllocPContentContentChild()
{
  MOZ_ASSERT(!ManagedPContentContentChild().Length());
  PContentContentChild* child = new ContentContentChild(this);
  return child;
}

bool
ContentBridgeChild::DeallocPContentContentChild(PContentContentChild* aChild)
{
  delete aChild;
  return true;
}

PBrowserChild*
ContentBridgeChild::AllocPBrowserChild(const IPCTabContext &aContext,
                                       const uint32_t& aChromeFlags,
                                       const uint64_t& aID,
                                       const bool& aIsForApp,
                                       const bool& aIsForBrowser)
{
  return nsIContentChild::AllocPBrowserChild(aContext,
                                             aChromeFlags,
                                             aID,
                                             aIsForApp,
                                             aIsForBrowser);
}

bool
ContentBridgeChild::DeallocPBrowserChild(PBrowserChild* aChild)
{
  return nsIContentChild::DeallocPBrowserChild(aChild);
}

bool
ContentBridgeChild::RecvPBrowserConstructor(PBrowserChild* aActor,
                                            const IPCTabContext& aContext,
                                            const uint32_t& aChromeFlags,
                                            const uint64_t& aID,
                                            const bool& aIsForApp,
                                            const bool& aIsForBrowser)
{
  return ContentChild::GetSingleton()->RecvPBrowserConstructor(aActor,
                                                               aContext,
                                                               aChromeFlags,
                                                               aID,
                                                               aIsForApp,
                                                               aIsForBrowser);
}

} // namespace dom
} // namespace mozilla
