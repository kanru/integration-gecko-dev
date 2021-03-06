/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsUCConstructors.h"
#include "nsCP1125ToUnicode.h"

//----------------------------------------------------------------------
// Global functions and data [declaration]

//----------------------------------------------------------------------
// Class nsCP1125ToUnicode [implementation]

nsresult
nsCP1125ToUnicodeConstructor(nsISupports* aOuter, REFNSIID aIID,
                            void **aResult) 
{
  static const uint16_t g_utMappingTable[] = {
#include "cp1125.ut"
  };

  return CreateOneByteDecoder((uMappingTable*) &g_utMappingTable,
                              aOuter, aIID, aResult);
}
