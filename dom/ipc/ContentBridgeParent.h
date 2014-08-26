/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ContentBridgeParent_h
#define mozilla_dom_ContentBridgeParent_h

#include "mozilla/dom/PContentBridgeParent.h"
#include "mozilla/dom/nsIContentParent.h"

#define CONTENTBRIDGEPARENT_IID                                 \
  { 0x1cf11502, 0xdaa0, 0x4db1,                                 \
    { 0xb2, 0x28, 0x55, 0xfd, 0x9a, 0x97, 0xf6, 0xf7 } }

namespace mozilla {
namespace dom {

class ContentContentParent;

class ContentBridgeParent : public PContentBridgeParent
                          , public nsIContentParent
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(CONTENTBRIDGEPARENT_IID)

  NS_DECL_ISUPPORTS

  explicit ContentBridgeParent(Transport* aTransport);

  virtual ContentContentParent* ContentContent() MOZ_OVERRIDE;

  virtual void ActorDestroy(ActorDestroyReason aWhy) MOZ_OVERRIDE;
  void DeferredDestroy();

  static ContentBridgeParent*
  Create(Transport* aTransport, ProcessId aOtherProcess);

  virtual PBrowserParent*
  SendPBrowserConstructor(PBrowserParent* aActor,
                          const IPCTabContext& aContext,
                          const uint32_t& aChromeFlags,
                          const uint64_t& aID,
                          const bool& aIsForApp,
                          const bool& aIsForBrowser) MOZ_OVERRIDE;

  virtual uint64_t ChildID() MOZ_OVERRIDE
  {
    return mChildID;
  }
  virtual bool IsForApp() MOZ_OVERRIDE
  {
    return mIsForApp;
  }
  virtual bool IsForBrowser() MOZ_OVERRIDE
  {
    return mIsForBrowser;
  }

protected:
  virtual ~ContentBridgeParent();

  void SetChildID(uint64_t aId)
  {
    mChildID = aId;
  }
  void SetIsForApp(bool aIsForApp)
  {
    mIsForApp = aIsForApp;
  }
  void SetIsForBrowser(bool aIsForBrowser)
  {
    mIsForBrowser = aIsForBrowser;
  }

protected:
  virtual PContentContentParent* AllocPContentContentParent() MOZ_OVERRIDE;
  virtual bool DeallocPContentContentParent(PContentContentParent* aParent) MOZ_OVERRIDE;

  virtual bool RecvSyncMessage(const nsString& aMsg,
                               const ClonedMessageData& aData,
                               const InfallibleTArray<jsipc::CpowEntry>& aCpows,
                               const IPC::Principal& aPrincipal,
                               InfallibleTArray<nsString>* aRetvals) MOZ_OVERRIDE;
  virtual bool RecvAsyncMessage(const nsString& aMsg,
                                const ClonedMessageData& aData,
                                const InfallibleTArray<jsipc::CpowEntry>& aCpows,
                                const IPC::Principal& aPrincipal) MOZ_OVERRIDE;

  virtual PBrowserParent*
  AllocPBrowserParent(const IPCTabContext &aContext,
                      const uint32_t& aChromeFlags,
                      const uint64_t& aID,
                      const bool& aIsForApp,
                      const bool& aIsForBrowser) MOZ_OVERRIDE;
  virtual bool DeallocPBrowserParent(PBrowserParent*) MOZ_OVERRIDE;

  DISALLOW_EVIL_CONSTRUCTORS(ContentBridgeParent);

protected: // members
  nsRefPtr<ContentBridgeParent> mSelfRef;
  Transport* mTransport; // owned
  uint64_t mChildID;
  bool mIsForApp;
  bool mIsForBrowser;

private:
  friend class ContentParent;
};

NS_DEFINE_STATIC_IID_ACCESSOR(ContentBridgeParent, CONTENTBRIDGEPARENT_IID)

} // dom
} // mozilla

#endif // mozilla_dom_ContentBridgeParent_h
