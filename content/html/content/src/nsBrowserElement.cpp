/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsBrowserElement.h"

#include "mozilla/dom/BrowserElementBinding.h"
#include "mozilla/dom/DOMRequest.h"
#include "mozilla/Preferences.h"

namespace mozilla {
namespace dom {

static const char kTouchEnabled[] = "dom.w3c_touch_events.enabled";

bool
nsBrowserElement::TouchEnabled(JSContext* cx, JSObject* obj)
{
  return Preferences::GetInt(kTouchEnabled, 0) != 0;
}

void
nsBrowserElement::SetVisible(bool aVisible, ErrorResult& aRv)
{
}

already_AddRefed<DOMRequest>
nsBrowserElement::GetVisible(ErrorResult& aRv)
{
  return nullptr;
}

void
nsBrowserElement::SetActive(bool aVisible, ErrorResult& aRv)
{
}

bool
nsBrowserElement::GetActive(ErrorResult& aRv)
{
  return false;
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
}

void
nsBrowserElement::GoBack(ErrorResult& aRv)
{
}

void
nsBrowserElement::GoForward(ErrorResult& aRv)
{
}

void
nsBrowserElement::Reload(bool aHardReload, ErrorResult& aRv)
{
}

void
nsBrowserElement::Stop(ErrorResult& aRv)
{
}

already_AddRefed<DOMRequest>
nsBrowserElement::Download(const nsAString& aUrl,
                           const BrowserElementDownloadOptions& aOptions,
                           ErrorResult& aRv)
{
  return nullptr;
}

already_AddRefed<DOMRequest>
nsBrowserElement::PurgeHistory(ErrorResult& aRv)
{
  return nullptr;
}

already_AddRefed<DOMRequest>
nsBrowserElement::GetScreenshot(uint32_t aWidth,
                                uint32_t aHeight,
                                const Optional<nsAString>& aMimeType,
                                ErrorResult& aRv)
{
  return nullptr;
}

void
nsBrowserElement::Zoom(float aZoom, ErrorResult& aRv)
{
}

already_AddRefed<DOMRequest>
nsBrowserElement::GetCanGoBack(ErrorResult& aRv)
{
  return nullptr;
}

already_AddRefed<DOMRequest>
nsBrowserElement::GetCanGoForward(ErrorResult& aRv)
{
  return nullptr;
}

already_AddRefed<DOMRequest>
nsBrowserElement::GetContentDimensions(ErrorResult& aRv)
{
  return nullptr;
}

void
nsBrowserElement::AddNextPaintListener(BrowserElementNextPaintEventCallback& aListener,
                                       ErrorResult& aRv)
{
}

void
nsBrowserElement::RemoveNextPaintListener(BrowserElementNextPaintEventCallback& aListener,
                                          ErrorResult& aRv)
{
}

already_AddRefed<DOMRequest>
nsBrowserElement::SetInputMethodActive(bool aIsActive,
                                       ErrorResult& aRv)
{
  return nullptr;
}

} // namespace dom
} // namespace mozilla
