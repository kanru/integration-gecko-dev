/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined(nsHTMLAudioElement_h__)
#define nsHTMLAudioElement_h__

#include "nsITimer.h"
#include "nsIDOMHTMLAudioElement.h"
#include "nsIJSNativeInitializer.h"
#include "nsHTMLMediaElement.h"

typedef uint16_t nsMediaNetworkState;
typedef uint16_t nsMediaReadyState;

class nsHTMLAudioElement : public nsITimerCallback,
                           public nsHTMLMediaElement,
                           public nsIDOMHTMLAudioElement,
                           public nsIJSNativeInitializer
{
public:
  nsHTMLAudioElement(already_AddRefed<nsINodeInfo> aNodeInfo);
  virtual ~nsHTMLAudioElement();

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // nsIDOMNode
  NS_FORWARD_NSIDOMNODE(nsHTMLMediaElement::)

  // nsIDOMElement
  NS_FORWARD_NSIDOMELEMENT(nsHTMLMediaElement::)

  // nsIDOMHTMLElement
  NS_FORWARD_NSIDOMHTMLELEMENT(nsHTMLMediaElement::)

  // nsIDOMHTMLMediaElement
  NS_FORWARD_NSIDOMHTMLMEDIAELEMENT(nsHTMLMediaElement::)

  // nsIAudioChannelAgentCallback
  NS_DECL_NSIAUDIOCHANNELAGENTCALLBACK

  // NS_DECL_NSITIMERCALLBACK
  NS_DECL_NSITIMERCALLBACK

  // nsIDOMHTMLAudioElement
  NS_DECL_NSIDOMHTMLAUDIOELEMENT

  // nsIJSNativeInitializer
  NS_IMETHOD Initialize(nsISupports* aOwner, JSContext* aContext,
                        JSObject* aObj, uint32_t argc, jsval* argv);

  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;
  virtual nsresult SetAcceptHeader(nsIHttpChannel* aChannel);

  virtual nsXPCClassInfo* GetClassInfo();

  virtual nsIDOMNode* AsDOMNode() { return this; }

protected:
  virtual void GetItemValueText(nsAString& text);
  virtual void SetItemValueText(const nsAString& text);

  // Update the audio channel playing state
  virtual void UpdateAudioChannelPlayingState() MOZ_OVERRIDE;

  // Due to that audio data API doesn't indicate the timing of pause or end,
  // the timer is used to defer the timing of pause/stop after writing data.
  nsCOMPtr<nsITimer> mDeferStopPlayTimer;
  // To indicate mDeferStopPlayTimer is on fire or not.
  bool mTimerActivated;
};

#endif
