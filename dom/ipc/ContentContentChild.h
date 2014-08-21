/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ContentContentChild_h
#define mozilla_dom_ContentContentChild_h

#include "mozilla/dom/PContentContentChild.h"

#include "nsISupports.h"

namespace mozilla {
namespace dom {

class nsIContentChild;

class ContentContentChild : public PContentContentChild
{
public:
  explicit ContentContentChild(nsIContentChild* aManager);
  virtual ~ContentContentChild();

  nsIContentChild* Manager();

private:
  nsIContentChild* mManager; // owned
};

} // dom
} // mozilla


#endif
