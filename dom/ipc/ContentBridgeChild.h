/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ContentBridgeChild_h
#define mozilla_dom_ContentBridgeChild_h

#include "mozilla/dom/PContentBridgeChild.h"

namespace mozilla {

namespace jsipc {
class JavaScriptChild;
} // namespace jsipc

namespace dom {

class ContentChild;

class ContentBridgeChild : public PContentBridgeChild
{
  friend ContentChild;

public:
  ContentBridgeChild() {}
  virtual ~ContentBridgeChild() {}

  jsipc::JavaScriptChild *GetCPOWManager();

// IPDL methods
public:
  virtual jsipc::PJavaScriptChild* AllocPJavaScriptChild();
  virtual bool DeallocPJavaScriptChild(jsipc::PJavaScriptChild*);
// end IPDL methods
};

} // namesapce dom
} // namespace mozilla

#endif /* mozilla_dom_ContentBridgeChild_h */
