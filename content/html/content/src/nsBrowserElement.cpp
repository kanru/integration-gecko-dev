/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsBrowserElement.h"

#include "mozilla/Preferences.h"
#include "mozilla/Services.h"
#include "mozilla/dom/BrowserElementBinding.h"
#include "mozilla/dom/DOMRequest.h"
#include "mozilla/dom/ToJSValue.h"

#include "nsComponentManagerUtils.h"
#include "nsIDOMDOMRequest.h"
#include "nsIObserver.h"
#include "nsIObserverService.h"
#include "nsWeakReference.h"

namespace mozilla {
namespace dom {

static const char kRemoteBrowserPending[] = "remote-browser-pending";
static const char kInprocessBrowserShown[] = "inprocess-browser-shown";

class nsBrowserElement::BrowserShownObserver : public nsIObserver
                                             , public nsSupportsWeakReference
{
public:
  BrowserShownObserver(nsBrowserElement* aBrowserElement);
  NS_DECL_ISUPPORTS
  NS_DECL_NSIOBSERVER
  void AddObserver();
  void RemoveObserver();
private:
  virtual ~BrowserShownObserver();
  nsBrowserElement* mBrowserElement; // owned
};

NS_IMPL_ISUPPORTS(nsBrowserElement::BrowserShownObserver, nsIObserver, nsISupportsWeakReference)

nsBrowserElement::BrowserShownObserver::BrowserShownObserver(nsBrowserElement* aBrowserElement)
  : mBrowserElement(aBrowserElement)
{
}

nsBrowserElement::BrowserShownObserver::~BrowserShownObserver()
{
  RemoveObserver();
}

NS_IMETHODIMP
nsBrowserElement::BrowserShownObserver::Observe(nsISupports* aSubject,
                                                const char* aTopic,
                                                const char16_t* aData)
{
  if (!strcmp(aTopic, kRemoteBrowserPending) ||
      !strcmp(aTopic, kInprocessBrowserShown)) {
    nsCOMPtr<nsIFrameLoader> frameLoader = do_QueryInterface(aSubject);
    if (frameLoader && mBrowserElement->IsOwnFrameLoader(frameLoader)) {
      mBrowserElement->SetFrameLoader(frameLoader);
    }
  }
  return NS_OK;
}

void
nsBrowserElement::BrowserShownObserver::AddObserver()
{
  nsCOMPtr<nsIObserverService> obs = services::GetObserverService();
  if (obs) {
    obs->AddObserver(this, kRemoteBrowserPending, true);
    obs->AddObserver(this, kInprocessBrowserShown, true);
  }
}

void
nsBrowserElement::BrowserShownObserver::RemoveObserver()
{
  nsCOMPtr<nsIObserverService> obs = services::GetObserverService();
  if (obs) {
    obs->RemoveObserver(this, kRemoteBrowserPending);
    obs->RemoveObserver(this, kInprocessBrowserShown);
  }
}

static const char kTouchEnabled[] = "dom.w3c_touch_events.enabled";

bool
nsBrowserElement::TouchEnabled(JSContext* cx, JSObject* obj)
{
  return Preferences::GetInt(kTouchEnabled, 0) != 0;
}

bool
nsBrowserElement::IsBrowserElementOrThrow(ErrorResult& aRv)
{
  if (mBrowserElementAPI) {
    return true;
  }
  aRv.Throw(NS_ERROR_DOM_INVALID_NODE_TYPE_ERR);
  return false;
}

void
nsBrowserElement::SetFrameLoader(nsIFrameLoader* aFrameLoader)
{
  NS_ENSURE_TRUE_VOID(aFrameLoader);

  bool isBrowserOrApp;
  nsresult rv = aFrameLoader->GetOwnerIsBrowserOrAppFrame(&isBrowserOrApp);
  NS_ENSURE_SUCCESS_VOID(rv);

  if (!isBrowserOrApp) {
    return;
  }

  if (mFrameLoaderPtr == aFrameLoader) {
    return;
  }

  mBrowserElementAPI = do_CreateInstance("@mozilla.org/dom/browser-element-api;1");
  if (mBrowserElementAPI) {
    mBrowserElementAPI->SetFrameLoader(aFrameLoader);
    mFrameLoaderPtr = aFrameLoader;
  }
}

nsBrowserElement::nsBrowserElement()
{
  nsRefPtr<BrowserShownObserver> observer = new BrowserShownObserver(this);
  observer->AddObserver();
  mObserver = do_QueryObject(observer);
}

nsBrowserElement::~nsBrowserElement()
{
}

void
nsBrowserElement::SetVisible(bool aVisible, ErrorResult& aRv)
{
  NS_ENSURE_TRUE_VOID(IsBrowserElementOrThrow(aRv));
  mBrowserElementAPI->SetVisible(aVisible);
}

already_AddRefed<DOMRequest>
nsBrowserElement::GetVisible(ErrorResult& aRv)
{
  NS_ENSURE_TRUE(IsBrowserElementOrThrow(aRv), nullptr);

  nsCOMPtr<nsIDOMDOMRequest> req;
  mBrowserElementAPI->GetVisible(getter_AddRefs(req));

  return req.forget().downcast<DOMRequest>();
}

void
nsBrowserElement::SetActive(bool aVisible, ErrorResult& aRv)
{
  NS_ENSURE_TRUE_VOID(IsBrowserElementOrThrow(aRv));
  mBrowserElementAPI->SetActive(aVisible);
}

bool
nsBrowserElement::GetActive(ErrorResult& aRv)
{
  NS_ENSURE_TRUE(IsBrowserElementOrThrow(aRv), false);

  bool isActive;
  mBrowserElementAPI->GetActive(&isActive);

  return isActive;
}

void
nsBrowserElement::SendMouseEvent(const nsAString& aType,
                                 uint32_t aX,
                                 uint32_t aY,
                                 uint32_t aButton,
                                 uint32_t aClickCount,
                                 uint32_t aModifiers,
                                 ErrorResult& aRv)
{
  NS_ENSURE_TRUE_VOID(IsBrowserElementOrThrow(aRv));
  mBrowserElementAPI->SendMouseEvent(aType,
                                     aX,
                                     aY,
                                     aButton,
                                     aClickCount,
                                     aModifiers);
}

void
nsBrowserElement::SendTouchEvent(const nsAString& aType,
                                 const Sequence<uint32_t>& aIdentifiers,
                                 const Sequence<int32_t>& aXs,
                                 const Sequence<int32_t>& aYs,
                                 const Sequence<uint32_t>& aRxs,
                                 const Sequence<uint32_t>& aRys,
                                 const Sequence<float>& aRotationAngles,
                                 const Sequence<float>& aForces,
                                 uint32_t aCount,
                                 uint32_t aModifiers,
                                 ErrorResult& aRv)
{
  NS_ENSURE_TRUE_VOID(IsBrowserElementOrThrow(aRv));
  mBrowserElementAPI->SendTouchEvent(aType,
                                     aIdentifiers.Elements(),
                                     aXs.Elements(),
                                     aYs.Elements(),
                                     aRxs.Elements(),
                                     aRys.Elements(),
                                     aRotationAngles.Elements(),
                                     aForces.Elements(),
                                     aCount,
                                     aModifiers);
}

void
nsBrowserElement::GoBack(ErrorResult& aRv)
{
  NS_ENSURE_TRUE_VOID(IsBrowserElementOrThrow(aRv));
  mBrowserElementAPI->GoBack();
}

void
nsBrowserElement::GoForward(ErrorResult& aRv)
{
  NS_ENSURE_TRUE_VOID(IsBrowserElementOrThrow(aRv));
  mBrowserElementAPI->GoForward();
}

void
nsBrowserElement::Reload(bool aHardReload, ErrorResult& aRv)
{
  NS_ENSURE_TRUE_VOID(IsBrowserElementOrThrow(aRv));
  mBrowserElementAPI->Reload(aHardReload);
}

void
nsBrowserElement::Stop(ErrorResult& aRv)
{
  NS_ENSURE_TRUE_VOID(IsBrowserElementOrThrow(aRv));
  mBrowserElementAPI->Stop();
}

already_AddRefed<DOMRequest>
nsBrowserElement::Download(JSContext* aCx,
                           const nsAString& aUrl,
                           const BrowserElementDownloadOptions& aOptions,
                           ErrorResult& aRv)
{
  NS_ENSURE_TRUE(IsBrowserElementOrThrow(aRv), nullptr);

  nsCOMPtr<nsIDOMDOMRequest> req;
  JS::Rooted<JS::Value> options(aCx);
  if (!ToJSValue(aCx, aOptions, &options)) {
    aRv.Throw(NS_ERROR_OUT_OF_MEMORY);
  }
  mBrowserElementAPI->Download(aUrl, options, getter_AddRefs(req));

  return req.forget().downcast<DOMRequest>();
}

already_AddRefed<DOMRequest>
nsBrowserElement::PurgeHistory(ErrorResult& aRv)
{
  NS_ENSURE_TRUE(IsBrowserElementOrThrow(aRv), nullptr);

  nsCOMPtr<nsIDOMDOMRequest> req;
  mBrowserElementAPI->PurgeHistory(getter_AddRefs(req));

  return req.forget().downcast<DOMRequest>();
}

already_AddRefed<DOMRequest>
nsBrowserElement::GetScreenshot(uint32_t aWidth,
                                uint32_t aHeight,
                                const Optional<nsAString>& aMimeType,
                                ErrorResult& aRv)
{
  NS_ENSURE_TRUE(IsBrowserElementOrThrow(aRv), nullptr);

  nsCOMPtr<nsIDOMDOMRequest> req;
  nsresult rv = mBrowserElementAPI->GetScreenshot(
    aWidth, aHeight, aMimeType.WasPassed() ? aMimeType.Value() : EmptyString(),
    getter_AddRefs(req));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }

  return req.forget().downcast<DOMRequest>();
}

void
nsBrowserElement::Zoom(float aZoom, ErrorResult& aRv)
{
  NS_ENSURE_TRUE_VOID(IsBrowserElementOrThrow(aRv));
  mBrowserElementAPI->Zoom(aZoom);
}

already_AddRefed<DOMRequest>
nsBrowserElement::GetCanGoBack(ErrorResult& aRv)
{
  NS_ENSURE_TRUE(IsBrowserElementOrThrow(aRv), nullptr);

  nsCOMPtr<nsIDOMDOMRequest> req;
  mBrowserElementAPI->GetCanGoBack(getter_AddRefs(req));

  return req.forget().downcast<DOMRequest>();
}

already_AddRefed<DOMRequest>
nsBrowserElement::GetCanGoForward(ErrorResult& aRv)
{
  NS_ENSURE_TRUE(IsBrowserElementOrThrow(aRv), nullptr);

  nsCOMPtr<nsIDOMDOMRequest> req;
  mBrowserElementAPI->GetCanGoForward(getter_AddRefs(req));

  return req.forget().downcast<DOMRequest>();
}

already_AddRefed<DOMRequest>
nsBrowserElement::GetContentDimensions(ErrorResult& aRv)
{
  NS_ENSURE_TRUE(IsBrowserElementOrThrow(aRv), nullptr);

  nsCOMPtr<nsIDOMDOMRequest> req;
  mBrowserElementAPI->GetContentDimensions(getter_AddRefs(req));

  return req.forget().downcast<DOMRequest>();
}

void
nsBrowserElement::AddNextPaintListener(BrowserElementNextPaintEventCallback& aListener,
                                       ErrorResult& aRv)
{
  NS_ENSURE_TRUE_VOID(IsBrowserElementOrThrow(aRv));

  CallbackObjectHolder<BrowserElementNextPaintEventCallback,
                       nsIBrowserElementNextPaintListener> holder(&aListener);
  nsCOMPtr<nsIBrowserElementNextPaintListener> listener = holder.ToXPCOMCallback();

  mBrowserElementAPI->AddNextPaintListener(listener);
}

void
nsBrowserElement::RemoveNextPaintListener(BrowserElementNextPaintEventCallback& aListener,
                                          ErrorResult& aRv)
{
  NS_ENSURE_TRUE_VOID(IsBrowserElementOrThrow(aRv));

  CallbackObjectHolder<BrowserElementNextPaintEventCallback,
                       nsIBrowserElementNextPaintListener> holder(&aListener);
  nsCOMPtr<nsIBrowserElementNextPaintListener> listener = holder.ToXPCOMCallback();

  mBrowserElementAPI->RemoveNextPaintListener(listener);
}

already_AddRefed<DOMRequest>
nsBrowserElement::SetInputMethodActive(bool aIsActive,
                                       ErrorResult& aRv)
{
  NS_ENSURE_TRUE(IsBrowserElementOrThrow(aRv), nullptr);

  nsCOMPtr<nsIDOMDOMRequest> req;
  nsresult rv = mBrowserElementAPI->SetInputMethodActive(aIsActive,
                                                         getter_AddRefs(req));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }

  return req.forget().downcast<DOMRequest>();
}

} // namespace dom
} // namespace mozilla
