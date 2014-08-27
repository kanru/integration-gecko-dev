/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsIContentChild.h"

#include "mozilla/dom/ContentContentChild.h"
#include "mozilla/dom/ContentChild.h"
#include "mozilla/dom/PermissionMessageUtils.h"
#include "mozilla/dom/StructuredCloneUtils.h"
#include "mozilla/ipc/InputStreamUtils.h"

#include "JavaScriptChild.h"
#include "nsDOMFile.h"
#include "nsIJSRuntimeService.h"
#include "nsPrintfCString.h"

using namespace mozilla::ipc;
using namespace mozilla::jsipc;

namespace mozilla {
namespace dom {

BlobChild*
nsIContentChild::GetOrCreateActorForBlob(nsIDOMBlob* aBlob)
{
  return ContentContent()->GetOrCreateActorForBlob(aBlob);
}

jsipc::JavaScriptChild*
nsIContentChild::GetCPOWManager()
{
  return ContentContent()->GetCPOWManager();
}

bool
nsIContentChild::RecvAsyncMessage(const nsString& aMsg,
                                  const ClonedMessageData& aData,
                                  const InfallibleTArray<CpowEntry>& aCpows,
                                  const IPC::Principal& aPrincipal)
{
  nsRefPtr<nsFrameMessageManager> cpm = nsFrameMessageManager::sChildProcessManager;
  if (cpm) {
    StructuredCloneData cloneData = ipc::UnpackClonedMessageDataForChild(aData);
    CpowIdHolder cpows(GetCPOWManager(), aCpows);
    cpm->ReceiveMessage(static_cast<nsIContentFrameMessageManager*>(cpm.get()),
                        aMsg, false, &cloneData, &cpows, aPrincipal, nullptr);
  }
  return true;
}

} // dom
} // mozilla
