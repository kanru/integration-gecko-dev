/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ContentContentChild.h"

#include "mozilla/dom/ContentBridgeChild.h"
#include "mozilla/dom/ContentChild.h"
#include "mozilla/dom/ipc/Blob.h"
#include "mozilla/dom/ipc/nsIRemoteBlob.h"

#include "JavaScriptChild.h"
#include "nsIJSRuntimeService.h"

namespace mozilla {
namespace dom {

ContentContentChild::ContentContentChild(nsIContentChild* aManager)
  : mManager(aManager)
{
}

ContentContentChild::~ContentContentChild()
{
}

BlobChild*
ContentContentChild::GetOrCreateActorForBlob(nsIDOMBlob* aBlob)
{
  MOZ_ASSERT(NS_IsMainThread());
  MOZ_ASSERT(aBlob);

  // If the blob represents a remote blob then we can simply pass its actor back
  // here.
  const auto* domFile = static_cast<DOMFile*>(aBlob);
  nsCOMPtr<nsIRemoteBlob> remoteBlob = do_QueryInterface(domFile->Impl());
  if (remoteBlob) {
    BlobChild* actor =
      static_cast<BlobChild*>(
        static_cast<PBlobChild*>(remoteBlob->GetPBlob()));
    MOZ_ASSERT(actor);
    if (actor->Manager() == this) {
      return actor;
    }
  }

  // All blobs shared between processes must be immutable.
  nsCOMPtr<nsIMutable> mutableBlob = do_QueryInterface(aBlob);
  if (!mutableBlob || NS_FAILED(mutableBlob->SetMutable(false))) {
    NS_WARNING("Failed to make blob immutable!");
    return nullptr;
  }

#ifdef DEBUG
  {
    // XXX This is only safe so long as all blob implementations in our tree
    //     inherit DOMFileImplBase. If that ever changes then this will need to
    //     grow a real interface or something.
    const auto* blob = static_cast<DOMFileImplBase*>(domFile->Impl());

    MOZ_ASSERT(!blob->IsSizeUnknown());
    MOZ_ASSERT(!blob->IsDateUnknown());
  }
#endif

  ParentBlobConstructorParams params;

  nsString contentType;
  nsresult rv = aBlob->GetType(contentType);
  NS_ENSURE_SUCCESS(rv, nullptr);

  uint64_t length;
  rv = aBlob->GetSize(&length);
  NS_ENSURE_SUCCESS(rv, nullptr);

  nsCOMPtr<nsIInputStream> stream;
  rv = aBlob->GetInternalStream(getter_AddRefs(stream));
  NS_ENSURE_SUCCESS(rv, nullptr);

  InputStreamParams inputStreamParams;
  nsTArray<mozilla::ipc::FileDescriptor> fds;
  SerializeInputStream(stream, inputStreamParams, fds);

  MOZ_ASSERT(fds.IsEmpty());

  params.optionalInputStreamParams() = inputStreamParams;

  nsCOMPtr<nsIDOMFile> file = do_QueryInterface(aBlob);
  if (file) {
    FileBlobConstructorParams fileParams;

    rv = file->GetName(fileParams.name());
    NS_ENSURE_SUCCESS(rv, nullptr);

    rv = file->GetMozLastModifiedDate(&fileParams.modDate());
    NS_ENSURE_SUCCESS(rv, nullptr);

    fileParams.contentType() = contentType;
    fileParams.length() = length;

    params.blobParams() = fileParams;
  } else {
    NormalBlobConstructorParams blobParams;
    blobParams.contentType() = contentType;
    blobParams.length() = length;
    params.blobParams() = blobParams;
  }

  BlobChild* actor = BlobChild::Create(aBlob);
  NS_ENSURE_TRUE(actor, nullptr);

  return SendPBlobConstructor(actor, params) ? actor : nullptr;
}

jsipc::JavaScriptChild *
ContentContentChild::GetCPOWManager()
{
  if (ManagedPJavaScriptChild().Length()) {
    return static_cast<jsipc::JavaScriptChild*>(ManagedPJavaScriptChild()[0]);
  }
  jsipc::JavaScriptChild* actor =
    static_cast<jsipc::JavaScriptChild*>(SendPJavaScriptConstructor());
  return actor;
}

nsIContentChild*
ContentContentChild::Manager()
{
  return mManager;
}

PBlobChild*
ContentContentChild::AllocPBlobChild(const BlobConstructorParams& aParams)
{
  return BlobChild::Create(aParams);
}

bool
ContentContentChild::DeallocPBlobChild(PBlobChild* aActor)
{
  delete aActor;
  return true;
}

jsipc::PJavaScriptChild*
ContentContentChild::AllocPJavaScriptChild()
{
  nsCOMPtr<nsIJSRuntimeService> svc =
    do_GetService("@mozilla.org/js/xpc/RuntimeService;1");
  NS_ENSURE_TRUE(svc, nullptr);

  JSRuntime *rt;
  svc->GetRuntime(&rt);
  NS_ENSURE_TRUE(svc, nullptr);

  nsAutoPtr<jsipc::JavaScriptChild> child(new jsipc::JavaScriptChild(rt));
  if (!child->init()) {
    return nullptr;
  }
  return child.forget();
}

bool
ContentContentChild::DeallocPJavaScriptChild(jsipc::PJavaScriptChild* aChild)
{
  static_cast<jsipc::JavaScriptChild*>(aChild)->decref();
  return true;
}

} // dom
} // mozilla
