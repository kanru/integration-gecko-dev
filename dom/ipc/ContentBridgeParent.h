/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ContentBridgeParent_h
#define mozilla_dom_ContentBridgeParent_h

#include "mozilla/dom/PContentBridgeParent.h"
#include "mozilla/dom/ContentContentParent.h"

namespace mozilla {
namespace dom {

class ContentBridgeParent : public PContentBridgeParent
{
public:
  explicit ContentBridgeParent(Transport* aTransport);

  NS_DECL_ISUPPORTS

  virtual void ActorDestroy(ActorDestroyReason aWhy);
  void DeferredDestroy();

  static ContentBridgeParent*
  Create(Transport* aTransport, ProcessId aOtherProcess);

  virtual PBlobParent*
  SendPBlobConstructor(PBlobParent* actor,
                       const BlobConstructorParams& params);

  virtual PBrowserParent*
  SendPBrowserConstructor(PBrowserParent* aActor,
                          const IPCTabContext& aContext,
                          const uint32_t& aChromeFlags,
                          const uint64_t& aID,
                          const bool& aIsForApp,
                          const bool& aIsForBrowser);

  jsipc::JavaScriptParent* GetCPOWManager();

  virtual uint64_t ChildID()
  {
    return mChildID;
  }
  virtual bool IsForApp()
  {
    return mIsForApp;
  }
  virtual bool IsForBrowser()
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
  virtual bool RecvSyncMessage(const nsString& aMsg,
                               const ClonedMessageData& aData,
                               const InfallibleTArray<jsipc::CpowEntry>& aCpows,
                               const IPC::Principal& aPrincipal,
                               InfallibleTArray<nsString>* aRetvals);
  virtual bool RecvAsyncMessage(const nsString& aMsg,
                                const ClonedMessageData& aData,
                                const InfallibleTArray<jsipc::CpowEntry>& aCpows,
                                const IPC::Principal& aPrincipal);

  virtual jsipc::PJavaScriptParent* AllocPJavaScriptParent();
  virtual bool
  DeallocPJavaScriptParent(jsipc::PJavaScriptParent*);

  virtual PBrowserParent*
  AllocPBrowserParent(const IPCTabContext &aContext,
                      const uint32_t& aChromeFlags,
                      const uint64_t& aID,
                      const bool& aIsForApp,
                      const bool& aIsForBrowser);
  virtual bool DeallocPBrowserParent(PBrowserParent*);

  virtual PBlobParent*
  AllocPBlobParent(const BlobConstructorParams& aParams);

  virtual bool DeallocPBlobParent(PBlobParent*);

  DISALLOW_EVIL_CONSTRUCTORS(ContentBridgeParent);

protected: // members
  nsRefPtr<ContentBridgeParent> mSelfRef;
  nsRefPtr<ContentContentParent> mContentContent;
  Transport* mTransport; // owned
  uint64_t mChildID;
  bool mIsForApp;
  bool mIsForBrowser;

private:
  friend class ContentParent;
};

} // dom
} // mozilla

#endif // mozilla_dom_ContentBridgeParent_h
