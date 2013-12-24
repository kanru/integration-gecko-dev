/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ContentBridgeParent_h
#define mozilla_dom_ContentBridgeParent_h

#include "mozilla/dom/PContentBridgeParent.h"

#include "mozilla/dom/ipc/Blob.h"
#include "nsFrameMessageManager.h"

#define CHILD_PROCESS_SHUTDOWN_MESSAGE NS_LITERAL_STRING("child-process-shutdown")

namespace mozilla {

namespace jsipc {
class JavaScriptParent;
class PJavaScriptParent;
} // namespace jsipc

namespace ipc {
class IProtocol;
} // namespace ipc

namespace dom {

class ContentParent;
class ContentContentParent;

class ContentBridgeParent : public PContentBridgeParent
                          , public mozilla::dom::ipc::MessageManagerCallback
{
  NS_INLINE_DECL_REFCOUNTING(ContentBridgeParent)

  friend ContentParent;

public:
  ContentBridgeParent(mozilla::ipc::IProtocol* aManager);
  virtual ~ContentBridgeParent() {}

  BlobParent* GetOrCreateActorForBlob(nsIDOMBlob* aBlob);
  jsipc::JavaScriptParent *GetCPOWManager();

  /**
   * MessageManagerCallback methods that we override.
   */
  virtual bool DoSendAsyncMessage(JSContext* aCx,
                                  const nsAString& aMessage,
                                  const mozilla::dom::StructuredCloneData& aData,
                                  JS::Handle<JSObject *> aCpows,
                                  nsIPrincipal* aPrincipal) MOZ_OVERRIDE;
  virtual bool CheckPermission(const nsAString& aPermission) MOZ_OVERRIDE;
  virtual bool CheckManifestURL(const nsAString& aManifestURL) MOZ_OVERRIDE;
  virtual bool CheckAppHasPermission(const nsAString& aPermission) MOZ_OVERRIDE;
  virtual bool CheckAppHasStatus(unsigned short aStatus) MOZ_OVERRIDE;

  ContentParent* GetContentParent();
  ContentContentParent* GetContentContentParent();

// IPDL methods
public:
  virtual PJavaScriptParent*
  AllocPJavaScriptParent() MOZ_OVERRIDE;
  virtual bool
  RecvPJavaScriptConstructor(PJavaScriptParent* aActor) MOZ_OVERRIDE {
    return PContentBridgeParent::RecvPJavaScriptConstructor(aActor);
  }
protected:
  virtual void ActorDestroy(ActorDestroyReason why) MOZ_OVERRIDE;
private:
  // Hide the raw constructor methods since we don't want client code
  // using them.
  using PContentBridgeParent::SendPBrowserConstructor;

  virtual PBlobParent* AllocPBlobParent(const BlobConstructorParams& aParams);
  virtual bool DeallocPBlobParent(PBlobParent*);

  virtual PBrowserParent* AllocPBrowserParent(const IPCTabContext& aContext,
                                              const uint32_t& aChromeFlags);
  virtual bool DeallocPBrowserParent(PBrowserParent* frame);

  virtual bool DeallocPJavaScriptParent(mozilla::jsipc::PJavaScriptParent*);

  virtual bool RecvSyncMessage(const nsString& aMsg,
                               const ClonedMessageData& aData,
                               const InfallibleTArray<CpowEntry>& aCpows,
                               const IPC::Principal& aPrincipal,
                               InfallibleTArray<nsString>* aRetvals);
  virtual bool AnswerRpcMessage(const nsString& aMsg,
                                const ClonedMessageData& aData,
                                const InfallibleTArray<CpowEntry>& aCpows,
                                const IPC::Principal& aPrincipal,
                                InfallibleTArray<nsString>* aRetvals);
  virtual bool RecvAsyncMessage(const nsString& aMsg,
                                const ClonedMessageData& aData,
                                const InfallibleTArray<CpowEntry>& aCpows,
                                const IPC::Principal& aPrincipal);
// end IPDL methods

  nsRefPtr<nsFrameMessageManager> mMessageManager;
  mozilla::ipc::IProtocol* mManager;
};

} // namespace dom
} // namespace mozilla

#endif /* mozilla_dom_ContentBridgeParent_h */
