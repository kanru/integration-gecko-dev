/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ContentContentParent.h"

#include "mozilla/dom/ContentBridgeParent.h"
#include "mozilla/dom/ContentParent.h"
#include "mozilla/dom/ipc/Blob.h"
#include "mozilla/dom/ipc/nsIRemoteBlob.h"

namespace mozilla {
namespace dom {

ContentContentParent::ContentContentParent(nsIContentParent* aManager)
  : mManager(aManager)
{
}

ContentContentParent::~ContentContentParent()
{
}

BlobParent*
ContentContentParent::GetOrCreateActorForBlob(nsIDOMBlob* aBlob)
{
  MOZ_ASSERT(NS_IsMainThread());
  MOZ_ASSERT(aBlob);

  // If the blob represents a remote blob for this ContentParent then we can
  // simply pass its actor back here.
  const auto* domFile = static_cast<DOMFile*>(aBlob);
  nsCOMPtr<nsIRemoteBlob> remoteBlob = do_QueryInterface(domFile->Impl());
  if (remoteBlob) {
    if (BlobParent* actor = static_cast<BlobParent*>(
          static_cast<PBlobParent*>(remoteBlob->GetPBlob()))) {
      MOZ_ASSERT(actor);

      if (actor->Manager() == this) {
        return actor;
      }
    }
  }

  // All blobs shared between processes must be immutable.
  nsCOMPtr<nsIMutable> mutableBlob = do_QueryInterface(aBlob);
  if (!mutableBlob || NS_FAILED(mutableBlob->SetMutable(false))) {
    NS_WARNING("Failed to make blob immutable!");
    return nullptr;
  }

  // XXX This is only safe so long as all blob implementations in our tree
  //     inherit DOMFileImplBase. If that ever changes then this will need to grow
  //     a real interface or something.
  const auto* blob = static_cast<DOMFileImplBase*>(domFile->Impl());

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

  BlobParent* actor = BlobParent::Create(aBlob);
  NS_ENSURE_TRUE(actor, nullptr);

  return SendPBlobConstructor(actor, params) ? actor : nullptr;
}

nsIContentParent*
ContentContentParent::Manager()
{
  return mManager;
}

PBlobParent*
ContentContentParent::AllocPBlobParent(const BlobConstructorParams& aParams)
{
  return BlobParent::Create(aParams);
}

bool
ContentContentParent::DeallocPBlobParent(PBlobParent* aActor)
{
  delete aActor;
  return true;
}

void
ContentContentParent::ActorDestroy(ActorDestroyReason aWhy)
{
}

} // dom
} // mozilla
