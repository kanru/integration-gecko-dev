/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ContentBridgeParent.h"

#include "Blob.h"
#include "ContentContentParent.h"
#include "ContentParent.h"
#include "JavaScriptParent.h"
#include "PermissionMessageUtils.h"
#include "TabParent.h"
#include "mozilla/Preferences.h"
#include "mozilla/ipc/InputStreamUtils.h"
#include "mozilla/unused.h"
#include "nsDOMFile.h"
#include "nsIRemoteBlob.h"

namespace mozilla {
namespace dom {

ContentBridgeParent::ContentBridgeParent(mozilla::ipc::IProtocol* aManager)
  : mManager(aManager)
{
  mMessageManager = nsFrameMessageManager::NewProcessMessageManager(this);
}

BlobParent*
ContentBridgeParent::GetOrCreateActorForBlob(nsIDOMBlob* aBlob)
{
  MOZ_ASSERT(NS_IsMainThread());
  MOZ_ASSERT(aBlob);

  // If the blob represents a remote blob for this ContentParent then we can
  // simply pass its actor back here.
  if (nsCOMPtr<nsIRemoteBlob> remoteBlob = do_QueryInterface(aBlob)) {
    if (BlobParent* actor = static_cast<BlobParent*>(
          static_cast<PBlobParent*>(remoteBlob->GetPBlob()))) {
      if (static_cast<ContentBridgeParent*>(actor->Manager()) == this) {
        return actor;
      }
    }
  }

  // XXX This is only safe so long as all blob implementations in our tree
  //     inherit nsDOMFileBase. If that ever changes then this will need to grow
  //     a real interface or something.
  const nsDOMFileBase* blob = static_cast<nsDOMFileBase*>(aBlob);

  // We often pass blobs that are multipart but that only contain one sub-blob
  // (WebActivities does this a bunch). Unwrap to reduce the number of actors
  // that we have to maintain.
  const nsTArray<nsCOMPtr<nsIDOMBlob> >* subBlobs = blob->GetSubBlobs();
  if (subBlobs && subBlobs->Length() == 1) {
    const nsCOMPtr<nsIDOMBlob>& subBlob = subBlobs->ElementAt(0);
    MOZ_ASSERT(subBlob);

    // We can only take this shortcut if the multipart and the sub-blob are both
    // Blob objects or both File objects.
    nsCOMPtr<nsIDOMFile> multipartBlobAsFile = do_QueryInterface(aBlob);
    nsCOMPtr<nsIDOMFile> subBlobAsFile = do_QueryInterface(subBlob);
    if (!multipartBlobAsFile == !subBlobAsFile) {
      // The wrapping might have been unnecessary, see if we can simply pass an
      // existing remote blob for this ContentParent.
      if (nsCOMPtr<nsIRemoteBlob> remoteSubBlob = do_QueryInterface(subBlob)) {
        BlobParent* actor =
          static_cast<BlobParent*>(
            static_cast<PBlobParent*>(remoteSubBlob->GetPBlob()));
        MOZ_ASSERT(actor);

        if (static_cast<ContentBridgeParent*>(actor->Manager()) == this) {
          return actor;
        }
      }

      // No need to add a reference here since the original blob must have a
      // strong reference in the caller and it must also have a strong reference
      // to this sub-blob.
      aBlob = subBlob;
      blob = static_cast<nsDOMFileBase*>(aBlob);
      subBlobs = blob->GetSubBlobs();
    }
  }

  // All blobs shared between processes must be immutable.
  nsCOMPtr<nsIMutable> mutableBlob = do_QueryInterface(aBlob);
  if (!mutableBlob || NS_FAILED(mutableBlob->SetMutable(false))) {
    NS_WARNING("Failed to make blob immutable!");
    return nullptr;
  }

  ChildBlobConstructorParams params;

  if (blob->IsSizeUnknown() || blob->IsDateUnknown()) {
    // We don't want to call GetSize or GetLastModifiedDate
    // yet since that may stat a file on the main thread
    // here. Instead we'll learn the size lazily from the
    // other process.
    params = MysteryBlobConstructorParams();
  }
  else {
    nsString contentType;
    nsresult rv = aBlob->GetType(contentType);
    NS_ENSURE_SUCCESS(rv, nullptr);

    uint64_t length;
    rv = aBlob->GetSize(&length);
    NS_ENSURE_SUCCESS(rv, nullptr);

    nsCOMPtr<nsIDOMFile> file = do_QueryInterface(aBlob);
    if (file) {
      FileBlobConstructorParams fileParams;

      rv = file->GetMozLastModifiedDate(&fileParams.modDate());
      NS_ENSURE_SUCCESS(rv, nullptr);

      rv = file->GetName(fileParams.name());
      NS_ENSURE_SUCCESS(rv, nullptr);

      fileParams.contentType() = contentType;
      fileParams.length() = length;

      params = fileParams;
    } else {
      NormalBlobConstructorParams blobParams;
      blobParams.contentType() = contentType;
      blobParams.length() = length;
      params = blobParams;
    }
  }

  BlobParent* actor = BlobParent::Create(this, aBlob);
  NS_ENSURE_TRUE(actor, nullptr);

  return SendPBlobConstructor(actor, params) ? actor : nullptr;
}

jsipc::JavaScriptParent*
ContentBridgeParent::GetCPOWManager()
{
  if (ManagedPJavaScriptParent().Length()) {
    return static_cast<jsipc::JavaScriptParent*>(ManagedPJavaScriptParent()[0]);
  }
  jsipc::JavaScriptParent* actor = static_cast<jsipc::JavaScriptParent*>(SendPJavaScriptConstructor());
  return actor;
}

ContentParent*
ContentBridgeParent::GetContentParent()
{
  if (XRE_GetProcessType() == GeckoProcessType_Default) {
    return reinterpret_cast<ContentParent*>(mManager);
  } else {
    return nullptr;
  }
}

ContentContentParent*
ContentBridgeParent::GetContentContentParent()
{
  return nullptr;
}

bool
ContentBridgeParent::DoSendAsyncMessage(JSContext* aCx,
                                        const nsAString& aMessage,
                                        const mozilla::dom::StructuredCloneData& aData,
                                        JS::Handle<JSObject *> aCpows,
                                        nsIPrincipal* aPrincipal)
{
  ClonedMessageData data;
  if (!BuildClonedMessageDataForParent(this, aData, data)) {
    return false;
  }
  InfallibleTArray<CpowEntry> cpows;
  if (!GetCPOWManager()->Wrap(aCx, aCpows, &cpows)) {
    return false;
  }
  return SendAsyncMessage(nsString(aMessage), data, cpows, aPrincipal);
}

bool
ContentBridgeParent::CheckPermission(const nsAString& aPermission)
{
  return AssertAppProcessPermission(GetContentParent(),
                                    NS_ConvertUTF16toUTF8(aPermission).get());
}

bool
ContentBridgeParent::CheckManifestURL(const nsAString& aManifestURL)
{
  return AssertAppProcessManifestURL(GetContentParent(),
                                     NS_ConvertUTF16toUTF8(aManifestURL).get());
}

bool
ContentBridgeParent::CheckAppHasPermission(const nsAString& aPermission)
{
  return AssertAppHasPermission(GetContentParent(),
                                NS_ConvertUTF16toUTF8(aPermission).get());
}

bool
ContentBridgeParent::CheckAppHasStatus(unsigned short aStatus)
{
  return AssertAppHasStatus(GetContentParent(), aStatus);
}

PBlobParent*
ContentBridgeParent::AllocPBlobParent(const BlobConstructorParams& aParams)
{
  return BlobParent::Create(this, aParams);
}

bool
ContentBridgeParent::DeallocPBlobParent(PBlobParent* aActor)
{
  delete aActor;
  return true;
}

PBrowserParent*
ContentBridgeParent::AllocPBrowserParent(const IPCTabContext& aContext,
                                         const uint32_t &aChromeFlags)
{
    unused << aChromeFlags;

    const IPCTabAppBrowserContext& appBrowser = aContext.appBrowserContext();

    // We don't trust the IPCTabContext we receive from the child, so we'll bail
    // if we receive an IPCTabContext that's not a PopupIPCTabContext.
    // (PopupIPCTabContext lets the child process prove that it has access to
    // the app it's trying to open.)
    if (appBrowser.type() != IPCTabAppBrowserContext::TPopupIPCTabContext) {
        NS_ERROR("Unexpected IPCTabContext type.  Aborting AllocPBrowserParent.");
        return nullptr;
    }

    const PopupIPCTabContext& popupContext = appBrowser.get_PopupIPCTabContext();
    TabParent* opener = static_cast<TabParent*>(popupContext.openerParent());
    if (!opener) {
        NS_ERROR("Got null opener from child; aborting AllocPBrowserParent.");
        return nullptr;
    }

    // Popup windows of isBrowser frames must be isBrowser if the parent
    // isBrowser.  Allocating a !isBrowser frame with same app ID would allow
    // the content to access data it's not supposed to.
    if (!popupContext.isBrowserElement() && opener->IsBrowserElement()) {
        NS_ERROR("Child trying to escalate privileges!  Aborting AllocPBrowserParent.");
        return nullptr;
    }

    MaybeInvalidTabContext tc(aContext);
    if (!tc.IsValid()) {
        NS_ERROR(nsPrintfCString("Child passed us an invalid TabContext.  (%s)  "
                                 "Aborting AllocPBrowserParent.",
                                 tc.GetInvalidReason()).get());
        return nullptr;
    }

    TabParent* parent = new TabParent(this, tc.GetTabContext(), aChromeFlags);

    // We release this ref in DeallocPBrowserParent()
    NS_ADDREF(parent);
    return parent;
}

bool
ContentBridgeParent::DeallocPBrowserParent(PBrowserParent* frame)
{
    TabParent* parent = static_cast<TabParent*>(frame);
    NS_RELEASE(parent);
    return true;
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

void
ContentBridgeParent::ActorDestroy(ActorDestroyReason why)
{
    nsRefPtr<nsFrameMessageManager> ppm = mMessageManager;
    if (ppm) {
      ppm->ReceiveMessage(static_cast<nsIContentFrameMessageManager*>(ppm.get()),
                          CHILD_PROCESS_SHUTDOWN_MESSAGE, false,
                          nullptr, nullptr, nullptr, nullptr);
      ppm->Disconnect();
    }
}

bool
ContentBridgeParent::DeallocPJavaScriptParent(PJavaScriptParent* parent)
{
  static_cast<jsipc::JavaScriptParent *>(parent)->decref();
  return true;
}

bool
ContentBridgeParent::RecvSyncMessage(const nsString& aMsg,
                                     const ClonedMessageData& aData,
                                     const InfallibleTArray<CpowEntry>& aCpows,
                                     const IPC::Principal& aPrincipal,
                                     InfallibleTArray<nsString>* aRetvals)
{
  nsIPrincipal* principal = aPrincipal;
  if (!Preferences::GetBool("dom.testing.ignore_ipc_principal", false) &&
      principal && !AssertAppPrincipal(GetContentParent(), principal)) {
    return false;
  }

  nsRefPtr<nsFrameMessageManager> ppm = mMessageManager;
  if (ppm) {
    StructuredCloneData cloneData = ipc::UnpackClonedMessageDataForParent(aData);
    jsipc::CpowIdHolder cpows(GetCPOWManager(), aCpows);

    ppm->ReceiveMessage(static_cast<nsIContentFrameMessageManager*>(ppm.get()),
                        aMsg, true, &cloneData, &cpows, aPrincipal, aRetvals);
  }
  return true;
}

bool
ContentBridgeParent::AnswerRpcMessage(const nsString& aMsg,
                                      const ClonedMessageData& aData,
                                      const InfallibleTArray<CpowEntry>& aCpows,
                                      const IPC::Principal& aPrincipal,
                                      InfallibleTArray<nsString>* aRetvals)
{
  nsIPrincipal* principal = aPrincipal;
  if (!Preferences::GetBool("dom.testing.ignore_ipc_principal", false) &&
      principal && !AssertAppPrincipal(GetContentParent(), principal)) {
    return false;
  }

  nsRefPtr<nsFrameMessageManager> ppm = mMessageManager;
  if (ppm) {
    StructuredCloneData cloneData = ipc::UnpackClonedMessageDataForParent(aData);
    jsipc::CpowIdHolder cpows(GetCPOWManager(), aCpows);
    ppm->ReceiveMessage(static_cast<nsIContentFrameMessageManager*>(ppm.get()),
                        aMsg, true, &cloneData, &cpows, aPrincipal, aRetvals);
  }
  return true;
}

bool
ContentBridgeParent::RecvAsyncMessage(const nsString& aMsg,
                                      const ClonedMessageData& aData,
                                      const InfallibleTArray<CpowEntry>& aCpows,
                                      const IPC::Principal& aPrincipal)
{
  nsIPrincipal* principal = aPrincipal;
  if (!Preferences::GetBool("dom.testing.ignore_ipc_principal", false) &&
      principal && !AssertAppPrincipal(GetContentParent(), principal)) {
    return false;
  }

  nsRefPtr<nsFrameMessageManager> ppm = mMessageManager;
  if (ppm) {
    StructuredCloneData cloneData = ipc::UnpackClonedMessageDataForParent(aData);
    jsipc::CpowIdHolder cpows(GetCPOWManager(), aCpows);
    ppm->ReceiveMessage(static_cast<nsIContentFrameMessageManager*>(ppm.get()),
                        aMsg, false, &cloneData, &cpows, aPrincipal, nullptr);
  }
  return true;
}

} // dom
} // mozilla
