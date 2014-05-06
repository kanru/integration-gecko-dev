/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_imagelib_ImageSource_h_
#define mozilla_imagelib_ImageSource_h_

#include "nsTArray.h"
#include "nsAutoPtr.h"
#include "mozilla/RefPtr.h"

namespace mozilla {

class VolatileBuffer;

namespace image {

class FallibleImageSource;
class VolatileImageSource;

/**
 * ImageSource
 */
class ImageSource
{
public:
  ImageSource() {}
  virtual ~ImageSource() {}

  virtual bool LockImageSourceData() { return true; }
  virtual bool UnlockImageSourceData() { return true; }

public:
  /**
   * @return The number of bytes in the image source
   */
  virtual uint32_t Bytes() const = 0;
  /**
   * @return The capacity of the image source container
   */
  virtual uint32_t Capacity() const = 0;
  /**
   * @return The memory address of the image source
   */
  virtual char *Addr() = 0;

public:
  /**
   * Set capacity of the backing buffer
   *
   * @param capacity
   * @return true if succeed
   *         false if run out of memory
   */
  virtual bool SetCapacity(uint32_t aCapacity) = 0;
  /**
   * Append image source data to the backing buffer
   *
   * @param buffer
   * @param count
   * @return The memory address of added image source data.
   *         nullptr if failed
   */
  virtual char *AppendSourceData(const char *aBuffer, uint32_t aCount) = 0;
  /**
   * Free up any extra space in the backing buffer
   */
  virtual void Compact() = 0;
  /**
   * Release the backing buffer
   */
  virtual void Clear() = 0;

public:
  virtual FallibleImageSource *AsFallible() { return nullptr; }
  virtual VolatileImageSource *AsVolatile() { return nullptr; }

public:
  virtual size_t SizeOfExcludingThis(MallocSizeOf aMallocSizeOf) const = 0;

private:
  // disable copy ctor and assignment
  ImageSource(const ImageSource&);    // not implemented
  void operator=(const ImageSource&); // not implemented
};

class FallibleImageSource : public ImageSource
{
public:
  virtual ~FallibleImageSource() {}

public:
  virtual uint32_t Bytes() const MOZ_OVERRIDE
  {
    return mSourceData.Length();
  }
  virtual uint32_t Capacity() const MOZ_OVERRIDE
  {
    return mSourceData.Capacity();
  }
  virtual char *Addr() MOZ_OVERRIDE
  {
    return mSourceData.Elements();
  }

public:
  virtual bool SetCapacity(uint32_t aCapacity) MOZ_OVERRIDE
  {
    return mSourceData.SetCapacity(aCapacity);
  }
  virtual char
  *AppendSourceData(const char *aBuffer, uint32_t aCount) MOZ_OVERRIDE
  {
    return mSourceData.AppendElements(aBuffer, aCount);
  }
  virtual void Compact() MOZ_OVERRIDE
  {
    mSourceData.Compact();
  }
  virtual void Clear() MOZ_OVERRIDE
  {
    mSourceData.Clear();
  }

public:
  virtual FallibleImageSource *AsFallible() MOZ_OVERRIDE { return this; }

public:
  virtual size_t
  SizeOfExcludingThis(MallocSizeOf aMallocSizeOf) const MOZ_OVERRIDE
  {
    return mSourceData.SizeOfExcludingThis(aMallocSizeOf);
  }

private:
  FallibleTArray<char> mSourceData;
};

class VolatileImageSource : public ImageSource
{
public:
  VolatileImageSource();
  virtual ~VolatileImageSource() {}

  virtual bool LockImageSourceData() MOZ_OVERRIDE;
  virtual bool UnlockImageSourceData() MOZ_OVERRIDE;

public:
  virtual uint32_t Bytes() const MOZ_OVERRIDE;
  virtual uint32_t Capacity() const MOZ_OVERRIDE;
  virtual char *Addr() MOZ_OVERRIDE;

public:
  virtual bool SetCapacity(uint32_t aCapacity) MOZ_OVERRIDE;
  virtual char
  *AppendSourceData(const char *aBuffer, uint32_t aCount) MOZ_OVERRIDE;
  virtual void Compact() MOZ_OVERRIDE;
  virtual void Clear() MOZ_OVERRIDE;

public:
  virtual VolatileImageSource *AsVolatile() { return this; }

public:
  virtual size_t
  SizeOfExcludingThis(MallocSizeOf aMallocSizeOf) const MOZ_OVERRIDE;

private:
  RefPtr<VolatileBuffer> mVBuf;
  nsAutoPtr<VolatileBufferPtr<char> > mVBufPtr;
  uint32_t mCapacity;
  uint32_t mDataEnd;
  int32_t mLockCount;
};

/**
 * An RAII class to ensure it's easy to balance locks and unlocks on
 * ImageSources.
 */
class AutoImageSourceLocker
{
public:
  AutoImageSourceLocker(ImageSource *aImgSrc)
    : mImageSource(aImgSrc)
    , mSucceeded(aImgSrc->LockImageSourceData())
  {}

  ~AutoImageSourceLocker()
  {
    if (mSucceeded) {
      mImageSource->UnlockImageSourceData();
    }
  }

  // Whether the lock request succeeded.
  bool Succeeded() { return mSucceeded; }

private:
  ImageSource* mImageSource;
  bool mSucceeded;
};

} // namespace image
} // namespace mozilla

#endif /* mozilla_imagelib_ImageSource_h_ */
