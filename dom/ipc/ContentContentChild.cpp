/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ContentContentChild.h"

namespace mozilla {
namespace dom {

ContentContentChild::ContentContentChild(Transport* aTransport)
  : mTransport(aTransport)
{}

ContentContentChild::~ContentContentChild()
{
  XRE_GetIOMessageLoop()->PostTask(FROM_HERE,
                                   new DeleteTask<Transport>(mTransport));
}

void
ContentContentChild::ActorDestroy(ActorDestroyReason aWhy)
{
   MessageLoop::current()->PostTask(
     FROM_HERE,
     NewRunnableMethod(this, &ContentContentChild::DeferredDestroy));
}

/*static*/ ContentContentChild*
ContentContentChild::Create(Transport* aTransport, ProcessId aOtherProcess)
{
  nsRefPtr<ContentContentChild> cc =
    new ContentContentChild(aTransport);
  ProcessHandle handle;
  if (!base::OpenProcessHandle(aOtherProcess, &handle)) {
    // XXX need to kill |aOtherProcess|, it's boned
    return nullptr;
  }
  cc->mSelfRef = cc;

  DebugOnly<bool> ok = cc->Open(aTransport, handle, XRE_GetIOMessageLoop());
  MOZ_ASSERT(ok);
  return cc;
}

void
ContentContentChild::DeferredDestroy()
{
  mSelfRef = nullptr;
  // |this| was just destroyed, hands off
}

PContentBridgeChild*
ContentContentChild::AllocPContentBridgeChild()
{
  ContentBridgeChild* cb = new ContentBridgeChild();
  // We release this ref in DeallocPContentBridgeChild()
  cb->AddRef();
  return cb;
}

bool
ContentContentChild::DeallocPContentBridgeChild(PContentBridgeChild* aActor)
{
  ContentBridgeChild* cb = static_cast<ContentBridgeChild*>(aActor);
  cb->Release();
  return true;
}

} // namespace dom
} // namespace mozilla
