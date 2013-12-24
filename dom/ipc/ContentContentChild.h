/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ContentContentChild_h
#define mozilla_dom_ContentContentChild_h

#include "mozilla/dom/PContentContentChild.h"

namespace mozilla {
namespace dom {

class ContentContentChild : public PContentContentChild
{
  NS_INLINE_DECL_REFCOUNTING(ContentContentChild)
public:
  ContentContentChild(Transport *aTransport);
  virtual ~ContentContentChild();

  static ContentContentChild*
  Create(Transport *aTransport, ProcessId aOtherProcess);

private:
  virtual PContentBridgeChild* AllocPContentBridgeChild() MOZ_OVERRIDE;

  virtual bool DeallocPContentBridgeChild(
    PContentBridgeChild* aActor) MOZ_OVERRIDE;

  virtual void ActorDestroy(ActorDestroyReason aWhy) MOZ_OVERRIDE;
  void DeferredDestroy();

  RefPtr<ContentContentChild> mSelfRef;
  Transport* mTransport; // owned
};

} // namespace dom
} // namespace mozilla

#endif /* mozilla_dom_ContentContentChild_h */
