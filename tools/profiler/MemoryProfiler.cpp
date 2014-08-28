/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "MemoryProfiler.h"
#include "nsIDOMClassInfo.h"

#include "nsIGlobalObject.h"

DOMCI_DATA(MemoryEntry, MemoryEntry)

NS_INTERFACE_MAP_BEGIN(MemoryEntry)
  NS_INTERFACE_MAP_ENTRY(nsIMemoryEntry)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(MemoryEntry)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(MemoryEntry)
NS_IMPL_RELEASE(MemoryEntry)

MemoryEntry::MemoryEntry()
{
  /* member initializers and constructor code */
}

MemoryEntry::~MemoryEntry()
{
  /* destructor code */
}

NS_IMETHODIMP
MemoryEntry::GetSize(uint32_t* aSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
MemoryEntry::GetStack(JS::MutableHandleValue aStack)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
MemoryEntry::GetTimeStamp(uint32_t* aTimeStamp)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

DOMCI_DATA(MemoryProfiler, MemoryProfiler)

NS_INTERFACE_MAP_BEGIN(MemoryProfiler)
  NS_INTERFACE_MAP_ENTRY(nsIMemoryProfiler)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(MemoryProfiler)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(MemoryProfiler)
NS_IMPL_RELEASE(MemoryProfiler)

MemoryProfiler::MemoryProfiler()
{
  /* member initializers and constructor code */
}

MemoryProfiler::~MemoryProfiler()
{
  /* destructor code */
}

NS_IMETHODIMP
MemoryProfiler::StartProfiler(nsISupports* aGlobal)
{
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal);
  if (!global) {
    return NS_ERROR_INVALID_ARG;
  }
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
MemoryProfiler::StopProfiler()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
MemoryProfiler::ResetProfiler()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
MemoryProfiler::ForEachRetainingEntries(nsIMemoryEntriesHandler* aHandler)
{
  // for_each_retainingentries {
  //   nsCOMPtr<nsIMemoryEntry> entry = new MemoryEntry();
  //   aHandler->HandleEntry(entry);
  // }
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
MemoryProfiler::ForEachAllocatedEntries(nsIMemoryEntriesHandler* ahandler)
{
  // for_each_allocatedentries {
  //   nsCOMPtr<nsIMemoryEntry> entry = new MemoryEntry();
  //   aHandler->HandleEntry(entry);
  // }
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
MemoryProfiler::GetFrame(uint32_t aIndex, nsAString& aFrameString)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}
