/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_imagelib_ImageSource_h_
#define mozilla_imagelib_ImageSource_h_

namespace mozilla {
namespace image {

class FallibleImageSource;
class VolatileImageSource;

/**
 * ImageSource
 */
class ImageSource
{
public:
  virtual ~ImageSource() {}

public:
  /**
   * @return The number of bytes in the image source
   */
  virtual uint32_t Bytes() const = 0;
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
};

} // namespace image
} // namespace mozilla

#endif /* mozilla_imagelib_ImageSource_h_ */
