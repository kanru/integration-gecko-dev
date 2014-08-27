/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_nsIContentChild_h
#define mozilla_dom_nsIContentChild_h

#include "mozilla/dom/ipc/Blob.h"

#include "nsISupports.h"

#define NS_ICONTENTCHILD_IID                                    \
  { 0x4eed2e73, 0x94ba, 0x48a8,                                 \
    { 0xa2, 0xd1, 0xa5, 0xed, 0x86, 0xd7, 0xbb, 0xe4 } }

namespace IPC {
class Principal;
} // IPC

namespace mozilla {

namespace jsipc {
class JavaScriptChild;
class CpowEntry;
} // jsipc

namespace dom {
class IPCTabContext;
class ContentContentChild;

class nsIContentChild : public nsISupports
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ICONTENTCHILD_IID)

  virtual ContentContentChild* ContentContent() = 0;

  BlobChild* GetOrCreateActorForBlob(nsIDOMBlob* aBlob);
  jsipc::JavaScriptChild* GetCPOWManager();

protected:
  virtual bool RecvAsyncMessage(const nsString& aMsg,
                                const ClonedMessageData& aData,
                                const InfallibleTArray<jsipc::CpowEntry>& aCpows,
                                const IPC::Principal& aPrincipal);
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIContentChild, NS_ICONTENTCHILD_IID)

} // dom
} // mozilla
#endif /* mozilla_dom_nsIContentChild_h */
