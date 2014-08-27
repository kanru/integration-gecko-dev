/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ContentBridgeChild_h
#define mozilla_dom_ContentBridgeChild_h

#include "mozilla/dom/PContentBridgeChild.h"
#include "mozilla/dom/nsIContentChild.h"

#define CONTENTBRIDGECHILD_IID                                  \
  { 0x7c0a91b5, 0xe56d, 0x4489,                                 \
    { 0x8d, 0x4f, 0x7b, 0x88, 0x6a, 0x9d, 0x67, 0x4b } }

namespace mozilla {
namespace dom {

class ContentContentChild;

class ContentBridgeChild MOZ_FINAL : public PContentBridgeChild
                                   , public nsIContentChild
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(CONTENTBRIDGECHILD_IID)

  NS_DECL_ISUPPORTS

  explicit ContentBridgeChild(Transport* aTransport);

  virtual ContentContentChild* ContentContent() MOZ_OVERRIDE;

  static ContentBridgeChild*
  Create(Transport* aTransport, ProcessId aOtherProcess);

  virtual void ActorDestroy(ActorDestroyReason aWhy) MOZ_OVERRIDE;
  void DeferredDestroy();

  virtual bool RecvAsyncMessage(const nsString& aMsg,
                                const ClonedMessageData& aData,
                                const InfallibleTArray<jsipc::CpowEntry>& aCpows,
                                const IPC::Principal& aPrincipal) MOZ_OVERRIDE;

protected:
  virtual ~ContentBridgeChild();

  virtual PContentContentChild* AllocPContentContentChild() MOZ_OVERRIDE;
  virtual bool DeallocPContentContentChild(PContentContentChild* aChild) MOZ_OVERRIDE;

  DISALLOW_EVIL_CONSTRUCTORS(ContentBridgeChild);

protected: // members
  nsRefPtr<ContentBridgeChild> mSelfRef;
  Transport* mTransport; // owned
};

NS_DEFINE_STATIC_IID_ACCESSOR(ContentBridgeChild, CONTENTBRIDGECHILD_IID)

} // dom
} // mozilla

#endif // mozilla_dom_ContentBridgeChild_h
