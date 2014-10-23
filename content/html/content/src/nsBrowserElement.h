/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsBrowserElement_h
#define nsBrowserElement_h

#include "mozilla/dom/BindingDeclarations.h"

#include "nsCOMPtr.h"

namespace mozilla {

class ErrorResult;

namespace dom {

class BrowserElementDownloadOptions;
class BrowserElementNextPaintEventCallback;
class DOMRequest;

/**
 * A helper class for browser-element frames
 */
class nsBrowserElement
{
public:
  static bool TouchEnabled(JSContext* cx, JSObject* obj);

public:
  void SetVisible(bool aVisible, ErrorResult& aRv);
  already_AddRefed<DOMRequest> GetVisible(ErrorResult& aRv);
  void SetActive(bool aActive, ErrorResult& aRv);
  bool GetActive(ErrorResult& aRv);

  void SendMouseEvent(const nsAString& aType,
                      uint32_t aX,
                      uint32_t aY,
                      uint32_t aButton,
                      uint32_t aClickCount,
                      uint32_t aModifiers,
                      ErrorResult& aRv);
  void SendTouchEvent(const nsAString& aType,
                      const Sequence<uint32_t>& aIdentifiers,
                      const Sequence<int32_t>& aX,
                      const Sequence<int32_t>& aY,
                      const Sequence<uint32_t>& aRx,
                      const Sequence<uint32_t>& aRy,
                      const Sequence<float>& aRotationAngles,
                      const Sequence<float>& aForces,
                      uint32_t aCount,
                      uint32_t aModifiers,
                      ErrorResult& aRv);
  void GoBack(ErrorResult& aRv);
  void GoForward(ErrorResult& aRv);
  void Reload(bool aHardReload, ErrorResult& aRv);
  void Stop(ErrorResult& aRv);

  already_AddRefed<DOMRequest>
  Download(const nsAString& aUrl,
           const BrowserElementDownloadOptions& options,
           ErrorResult& aRv);

  already_AddRefed<DOMRequest> PurgeHistory(ErrorResult& aRv);

  already_AddRefed<DOMRequest>
  GetScreenshot(uint32_t aWidth,
                uint32_t aHeight,
                const Optional<nsAString>& aMimeType,
                ErrorResult& aRv);

  void Zoom(float aZoom, ErrorResult& aRv);

  already_AddRefed<DOMRequest> GetCanGoBack(ErrorResult& aRv);
  already_AddRefed<DOMRequest> GetCanGoForward(ErrorResult& aRv);
  already_AddRefed<DOMRequest> GetContentDimensions(ErrorResult& aRv);

  void AddNextPaintListener(BrowserElementNextPaintEventCallback& listener,
                            ErrorResult& aRv);
  void RemoveNextPaintListener(BrowserElementNextPaintEventCallback& listener,
                               ErrorResult& aRv);

  already_AddRefed<DOMRequest> SetInputMethodActive(bool isActive,
                                                    ErrorResult& aRv);
};

} // namespace dom
} // namespace mozilla

#endif // nsBrowserElement_h
