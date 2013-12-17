/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ContentBridgeParent_h
#define mozilla_dom_ContentBridgeParent_h

#include "mozilla/dom/PContentBridgeParent.h"

namespace mozilla {

namespace jsipc {
class JavaScriptParent;
class PJavaScriptParent;
} // namespace jsipc

namespace dom {

class ContentParent;

class ContentBridgeParent : public PContentBridgeParent
{
  friend ContentParent;

public:
  ContentBridgeParent() {}
  virtual ~ContentBridgeParent() {}

  jsipc::JavaScriptParent *GetCPOWManager();

// IPDL methods
public:
  virtual PJavaScriptParent*
  AllocPJavaScriptParent() MOZ_OVERRIDE;
  virtual bool
  RecvPJavaScriptConstructor(PJavaScriptParent* aActor) MOZ_OVERRIDE {
    return PContentBridgeParent::RecvPJavaScriptConstructor(aActor);
  }
private:
  virtual bool DeallocPJavaScriptParent(mozilla::jsipc::PJavaScriptParent*);
// end IPDL methods
};

} // namespace dom
} // namespace mozilla

#endif /* mozilla_dom_ContentBridgeParent_h */
