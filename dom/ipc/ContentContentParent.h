/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ContentContentParent_h
#define mozilla_dom_ContentContentParent_h

#include "mozilla/dom/PContentContentParent.h"

namespace mozilla {
namespace dom {

class ContentContentParent : public PContentContentParent
{
  NS_INLINE_DECL_REFCOUNTING(ContentContentParent)
public:
  ContentContentParent(Transport *aTransport);
  virtual ~ContentContentParent();

  static ContentContentParent*
  Create(Transport *aTransport, ProcessId aOtherProcess);

private:
  virtual PContentBridgeParent* AllocPContentBridgeParent() MOZ_OVERRIDE;

  virtual bool DeallocPContentBridgeParent(
    PContentBridgeParent* aActor) MOZ_OVERRIDE;

  virtual void ActorDestroy(ActorDestroyReason aWhy) MOZ_OVERRIDE;
  void DeferredDestroy();

  RefPtr<ContentContentParent> mSelfRef;
  Transport* mTransport; // owned
};

} // namespace dom
} // namespace mozilla

#endif /* mozilla_dom_ContentContentParent_h */
