/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsIContentParent.h"

#include "mozilla/AppProcessChecker.h"
#include "mozilla/Preferences.h"
#include "mozilla/dom/ContentContentParent.h"
#include "mozilla/dom/ContentParent.h"
#include "mozilla/dom/PTabContext.h"
#include "mozilla/dom/PermissionMessageUtils.h"
#include "mozilla/dom/StructuredCloneUtils.h"
#include "mozilla/dom/ipc/nsIRemoteBlob.h"
#include "mozilla/unused.h"

#include "nsDOMFile.h"
#include "nsFrameMessageManager.h"
#include "nsPrintfCString.h"

// XXX
using namespace mozilla::jsipc;

namespace mozilla {
namespace dom {

nsIContentParent::nsIContentParent()
{
  mMessageManager = nsFrameMessageManager::NewProcessMessageManager(this);
}

ContentParent*
nsIContentParent::AsContentParent()
{
  MOZ_ASSERT(IsContentParent());
  return static_cast<ContentParent*>(this);
}

bool
nsIContentParent::CanOpenBrowser(const IPCTabContext& aContext)
{
  const IPCTabAppBrowserContext& appBrowser = aContext.appBrowserContext();

  // We don't trust the IPCTabContext we receive from the child, so we'll bail
  // if we receive an IPCTabContext that's not a PopupIPCTabContext.
  // (PopupIPCTabContext lets the child process prove that it has access to
  // the app it's trying to open.)
  if (appBrowser.type() != IPCTabAppBrowserContext::TPopupIPCTabContext) {
    NS_ERROR("Unexpected IPCTabContext type.  Aborting AllocPBrowserParent.");
    return false;
  }

  const PopupIPCTabContext& popupContext = appBrowser.get_PopupIPCTabContext();
  TabParent* opener = static_cast<TabParent*>(popupContext.openerParent());
  if (!opener) {
    NS_ERROR("Got null opener from child; aborting AllocPBrowserParent.");
    return false;
  }

  // Popup windows of isBrowser frames must be isBrowser if the parent
  // isBrowser.  Allocating a !isBrowser frame with same app ID would allow
  // the content to access data it's not supposed to.
  if (!popupContext.isBrowserElement() && opener->IsBrowserElement()) {
    NS_ERROR("Child trying to escalate privileges!  Aborting AllocPBrowserParent.");
    return false;
  }

  MaybeInvalidTabContext tc(aContext);
  if (!tc.IsValid()) {
    NS_ERROR(nsPrintfCString("Child passed us an invalid TabContext.  (%s)  "
                             "Aborting AllocPBrowserParent.",
                             tc.GetInvalidReason()).get());
    return false;
  }

  return true;
}

BlobParent*
nsIContentParent::GetOrCreateActorForBlob(nsIDOMBlob* aBlob)
{
  return ContentContent()->GetOrCreateActorForBlob(aBlob);
}

jsipc::JavaScriptParent*
nsIContentParent::GetCPOWManager()
{
  return ContentContent()->GetCPOWManager();
}

bool
nsIContentParent::RecvSyncMessage(const nsString& aMsg,
                                  const ClonedMessageData& aData,
                                  const InfallibleTArray<CpowEntry>& aCpows,
                                  const IPC::Principal& aPrincipal,
                                  InfallibleTArray<nsString>* aRetvals)
{
  // FIXME Permission check in Content process
  nsIPrincipal* principal = aPrincipal;
  if (IsContentParent()) {
    ContentParent* parent = AsContentParent();
    if (!ContentParent::IgnoreIPCPrincipal() &&
        parent && principal && !AssertAppPrincipal(parent, principal)) {
      return false;
    }
  }

  nsRefPtr<nsFrameMessageManager> ppm = mMessageManager;
  if (ppm) {
    StructuredCloneData cloneData = ipc::UnpackClonedMessageDataForParent(aData);
    CpowIdHolder cpows(GetCPOWManager(), aCpows);

    ppm->ReceiveMessage(static_cast<nsIContentFrameMessageManager*>(ppm.get()),
                        aMsg, true, &cloneData, &cpows, aPrincipal, aRetvals);
  }
  return true;
}

bool
nsIContentParent::AnswerRpcMessage(const nsString& aMsg,
                                   const ClonedMessageData& aData,
                                   const InfallibleTArray<CpowEntry>& aCpows,
                                   const IPC::Principal& aPrincipal,
                                   InfallibleTArray<nsString>* aRetvals)
{
  // FIXME Permission check in Content process
  nsIPrincipal* principal = aPrincipal;
  if (IsContentParent()) {
    ContentParent* parent = AsContentParent();
    if (!ContentParent::IgnoreIPCPrincipal() &&
        parent && principal && !AssertAppPrincipal(parent, principal)) {
      return false;
    }
  }

  nsRefPtr<nsFrameMessageManager> ppm = mMessageManager;
  if (ppm) {
    StructuredCloneData cloneData = ipc::UnpackClonedMessageDataForParent(aData);
    CpowIdHolder cpows(GetCPOWManager(), aCpows);
    ppm->ReceiveMessage(static_cast<nsIContentFrameMessageManager*>(ppm.get()),
                        aMsg, true, &cloneData, &cpows, aPrincipal, aRetvals);
  }
  return true;
}

bool
nsIContentParent::RecvAsyncMessage(const nsString& aMsg,
                                   const ClonedMessageData& aData,
                                   const InfallibleTArray<CpowEntry>& aCpows,
                                   const IPC::Principal& aPrincipal)
{
  // FIXME Permission check in Content process
  nsIPrincipal* principal = aPrincipal;
  if (IsContentParent()) {
    ContentParent* parent = AsContentParent();
    if (!ContentParent::IgnoreIPCPrincipal() &&
        parent && principal && !AssertAppPrincipal(parent, principal)) {
      return false;
    }
  }

  nsRefPtr<nsFrameMessageManager> ppm = mMessageManager;
  if (ppm) {
    StructuredCloneData cloneData = ipc::UnpackClonedMessageDataForParent(aData);
    CpowIdHolder cpows(GetCPOWManager(), aCpows);
    ppm->ReceiveMessage(static_cast<nsIContentFrameMessageManager*>(ppm.get()),
                        aMsg, false, &cloneData, &cpows, aPrincipal, nullptr);
  }
  return true;
}

} // namespace dom
} // namespace mozilla
