/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ContentBridgeChild.h"

#include "nsServiceManagerUtils.h"
#include "JavaScriptChild.h"
#include "nsIJSRuntimeService.h"

namespace mozilla {
namespace dom {

jsipc::JavaScriptChild *
ContentBridgeChild::GetCPOWManager()
{
  if (ManagedPJavaScriptChild().Length()) {
    return static_cast<jsipc::JavaScriptChild*>(ManagedPJavaScriptChild()[0]);
  }
  jsipc::JavaScriptChild* actor = static_cast<jsipc::JavaScriptChild*>(SendPJavaScriptConstructor());
  return actor;
}

jsipc::PJavaScriptChild*
ContentBridgeChild::AllocPJavaScriptChild()
{
  nsCOMPtr<nsIJSRuntimeService> svc = do_GetService("@mozilla.org/js/xpc/RuntimeService;1");
  NS_ENSURE_TRUE(svc, nullptr);

  JSRuntime *rt;
  svc->GetRuntime(&rt);
  NS_ENSURE_TRUE(svc, nullptr);

  jsipc::JavaScriptChild *child = new jsipc::JavaScriptChild(rt);
  if (!child->init()) {
    delete child;
    return nullptr;
  }
  return child;
}

bool
ContentBridgeChild::DeallocPJavaScriptChild(jsipc::PJavaScriptChild *child)
{
  delete child;
  return true;
}

} // namespace dom
} // namespace mozilla
