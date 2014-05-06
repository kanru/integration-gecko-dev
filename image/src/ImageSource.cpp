/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/VolatileBuffer.h"

#include "ImageSource.h"

namespace mozilla {
namespace image {

VolatileImageSource::VolatileImageSource()
  : mVBuf(nullptr)
  , mVBufPtr(nullptr)
  , mCapacity(0)
  , mDataEnd(0)
  , mLockCount(0)
{
}

bool
VolatileImageSource::LockImageSourceData()
{
  NS_ABORT_IF_FALSE(mLockCount >= 0, "Unbalanced locks and unlocks");
  if (mLockCount <0) {
    return false;
  }
  mLockCount++;
  if (mLockCount != 1) {
    return true;
  }
  if (mVBuf) {
    mVBufPtr = new VolatileBufferPtr<char>(mVBuf);
    if (mVBufPtr->WasBufferPurged()) {
      return false;
    }
  }
  return true;
}

bool
VolatileImageSource::UnlockImageSourceData()
{
  NS_ABORT_IF_FALSE(mLockCount != 0, "Unlocking and unlocked image!");
  if (mLockCount == 0) {
    return false;
  }
  mLockCount--;
  NS_ABORT_IF_FALSE(mLockCount >= 0, "Unbalanced locks and unlocks");
  if (mLockCount < 0) {
    return false;
  }
  if (mLockCount != 0) {
    return true;
  }
  mVBufPtr = nullptr;
  return true;
}

uint32_t
VolatileImageSource::Bytes() const
{
  return mDataEnd;
}

uint32_t
VolatileImageSource::Capacity() const
{
  return mCapacity;
}

char*
VolatileImageSource::Addr()
{
  VolatileBufferPtr<char> ref(mVBuf);
  MOZ_ASSERT(!ref.WasBufferPurged(), "Expected image source data!");
  return ref;
}

bool
VolatileImageSource::SetCapacity(uint32_t aCapacity)
{
  mVBuf = new VolatileBuffer();
  if (mVBuf->Init(aCapacity)) {
    mCapacity = aCapacity;
    return true;
  }
  return false;
}

char*
VolatileImageSource::AppendSourceData(const char *aBuffer, uint32_t aCount)
{
  if (mDataEnd + aCount > mCapacity) {
    MOZ_ASSERT(false, "mCapacity must bigger than needed");
    return nullptr;
  }

  char *base = Addr();
  memcpy(base + mDataEnd, aBuffer, aCount);
  mDataEnd += aCount;
  return base + mDataEnd;
}

void
VolatileImageSource::Compact()
{
}

void
VolatileImageSource::Clear()
{
  // XXX force purge
  mVBuf = nullptr;
}

size_t
VolatileImageSource::SizeOfExcludingThis(MallocSizeOf aMallocSizeOf) const
{
  // XXX think about this
  return mVBuf->HeapSizeOfExcludingThis(aMallocSizeOf);
}

} // namespace image
} // namespace mozilla
