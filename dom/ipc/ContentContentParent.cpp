/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ContentContentParent.h"

#include "mozilla/dom/ContentBridgeParent.h"
#include "mozilla/dom/ContentParent.h"

namespace mozilla {
namespace dom {

ContentContentParent::ContentContentParent(nsIContentParent* aManager)
  : mManager(aManager)
{
}

ContentContentParent::~ContentContentParent()
{
}

nsIContentParent*
ContentContentParent::Manager()
{
  return mManager;
}

void
ContentContentParent::ActorDestroy(ActorDestroyReason aWhy)
{
}

} // dom
} // mozilla
