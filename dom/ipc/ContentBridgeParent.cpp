/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ContentBridgeParent.h"

#include "JavaScriptParent.h"

namespace mozilla {
namespace dom {

jsipc::JavaScriptParent*
ContentBridgeParent::GetCPOWManager()
{
  if (ManagedPJavaScriptParent().Length()) {
    return static_cast<jsipc::JavaScriptParent*>(ManagedPJavaScriptParent()[0]);
  }
  jsipc::JavaScriptParent* actor = static_cast<jsipc::JavaScriptParent*>(SendPJavaScriptConstructor());
  return actor;
}

jsipc::PJavaScriptParent*
ContentBridgeParent::AllocPJavaScriptParent()
{
  jsipc::JavaScriptParent *parent = new jsipc::JavaScriptParent();
  if (!parent->init()) {
    delete parent;
    return nullptr;
  }
  return parent;
}

// bool
// ContentBridgeParent::RecvPJavaScriptConstructor(PJavaScriptParent* aActor)
// {
//   return PContentBridgeParent::RecvPJavaScriptConstructor(aActor);
// }

bool
ContentBridgeParent::DeallocPJavaScriptParent(PJavaScriptParent* parent)
{
  static_cast<jsipc::JavaScriptParent *>(parent)->decref();
  return true;
}


} // dom
} // mozilla
