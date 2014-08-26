/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ContentContentParent_h
#define mozilla_dom_ContentContentParent_h

#include "mozilla/dom/PContentContentParent.h"

#include "nsISupports.h"

class nsIDOMBlob;

namespace mozilla {

namespace jsipc {
class PJavaScriptParent;
class JavaScriptParent;
class CpowEntry;
} // namespace jsipc

namespace dom {

class BlobParent;
class nsIContentParent;

class ContentContentParent : public PContentContentParent
{
public:
  explicit ContentContentParent(nsIContentParent* aManager);
  virtual ~ContentContentParent();

  BlobParent* GetOrCreateActorForBlob(nsIDOMBlob* aBlob);
  jsipc::JavaScriptParent* GetCPOWManager();

  nsIContentParent* Manager();

  virtual mozilla::ipc::IProtocol* CloneProtocol(
    Channel* aChannel, ProtocolCloneContext* aCtx) MOZ_OVERRIDE;

protected:
  virtual PBlobParent*
  AllocPBlobParent(const BlobConstructorParams&aParams) MOZ_OVERRIDE;
  virtual bool DeallocPBlobParent(PBlobParent*) MOZ_OVERRIDE;

  virtual jsipc::PJavaScriptParent* AllocPJavaScriptParent() MOZ_OVERRIDE;
  virtual bool DeallocPJavaScriptParent(jsipc::PJavaScriptParent*) MOZ_OVERRIDE;

  virtual void ActorDestroy(ActorDestroyReason why) MOZ_OVERRIDE;
private:
  nsIContentParent* mManager; // owned

  friend jsipc::JavaScriptParent;
};

} // namespace dom
} // namespace mozilla


#endif
