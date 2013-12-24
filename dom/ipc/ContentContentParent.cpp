/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ContentContentParent.h"

namespace mozilla {
namespace dom {

ContentContentParent::ContentContentParent(Transport *aTransport)
  : mTransport(aTransport)
{}

ContentContentParent::~ContentContentParent()
{
  XRE_GetIOMessageLoop()->PostTask(FROM_HERE, new DeleteTask<Transport>(mTransport));
}

void
ContentContentParent::ActorDestroy(ActorDestroyReason aWhy)
{
  MessageLoop::current()->PostTask(
    FROM_HERE,
    NewRunnableMethod(this, &ContentContentParent::DeferredDestroy));
}

/*static*/ ContentContentParent*
ContentContentParent::Create(Transport* aTransport, ProcessId aOtherProcess)
{
  nsRefPtr<ContentContentParent> cc =
    new ContentContentParent(aTransport);
  ProcessHandle handle;
  if (!base::OpenProcessHandle(aOtherProcess, &handle)) {
    // XXX need to kill |aOtherProcess|, it's boned
    return nullptr;
  }
  cc->mSelfRef = cc;

  DebugOnly<bool> ok = cc->Open(aTransport, handle, XRE_GetIOMessageLoop());
  MOZ_ASSERT(ok);
  return cc.get();
}

void
ContentContentParent::DeferredDestroy()
{
  mSelfRef = nullptr;
  // |this| was just destroyed, hands off
}

PContentBridgeParent*
ContentContentParent::AllocPContentBridgeParent()
{
  ContentBridgeParent* cb = new ContentBridgeParent(this);
  // We release this ref in DeallocPContentBridgeParent()
  cb->AddRef();
  return cb;
}

bool
ContentContentParent::DeallocPContentBridgeParent(PContentBridgeParent* aActor)
{
  ContentBridgeParent* cb = static_cast<ContentBridgeParent*>(aActor);
  cb->Release();
  return true;
}

} // namespace dom
} // namespace mozilla
