/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CLIENTSETTINGSPRODUCTREADER_H
#define CLIENTSETTINGSPRODUCTREADER_H

#include "config.h"

#include "MC2String.h"
#include "ValuePair.h"
#include "LineParser.h"

#include <set>

/**
 * Helper class used in navclientsettings parser.  It parses
 * "Product:" lines and stores the result.  At the end of the parsing
 * function it is used to set the Product of each known client
 * type.
 * 
 */
class ClientSettingsProductReader: public LineParser {
public:

   explicit ClientSettingsProductReader( const MC2String& tag );
   /**
    * Parse a Product: line and add the client types.
    */
   bool parseLine( const Line& line );

   /**
    * Return the product name for a client type, may be empty string.
    */
   const char* getProduct( const MC2String& client_type ) const;

private:
   /// Needs to be a vector so we can make sure the patterns comes in the
   /// read order.
   typedef vector< STLUtility::ValuePair< MC2String, MC2String > > 
      productPattersCont;
   /**
    * The client type patterns for Product
    */
   productPattersCont m_productPatterns;
};

#endif // CLIENTSETTINGSPRODUCTREADER_H

