/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLPARSERERRORREPORTER_H
#define XMLPARSERERRORREPORTER_H

#ifdef USE_XML

#include <util/XercesDefs.hpp>
#include <sax/ErrorHandler.hpp>
#include <iostream>

#include <sax/SAXParseException.hpp>
#include <stdlib.h>
#include "MC2String.h"
#include <parsers/XercesDOMParser.hpp>

#if (XERCES_VERSION_MAJOR == 2 && XERCES_VERSION_MINOR >= 4) || (XERCES_VERSION_MAJOR > 2)
using namespace xercesc;
#endif

/**
 *    Prints errors from the DOMParser.
 *
 */
class XMLParserErrorReporter : public ErrorHandler
{
   public:
      /**
       *    Constructor.
       */
      XMLParserErrorReporter();
      
      /**
       *    Destructor.
       */
      virtual ~XMLParserErrorReporter();

      /**
       *    @name Error-handling.
       *    @memo Implementation of the error handler interface.
       *    @doc  Implementation of the error handler interface.
       */
      //@{
         /**
          *    Handles warnings, ignores them.
          *    @param toCatch The exception that is thrown from the 
          *                   XML-parser.
          */
         void warning(const SAXParseException& toCatch);


         /**
          *    Handles errors. Will print them to standard out.
          *    @param toCatch The exception that is thrown from the 
          *                   XML-parser.
          */
         void error(const SAXParseException& toCatch);


         /**
          *    Handles fatal errors. Will print them to standard out.
          *    @param toCatch The exception that is thrown from the 
          *                   XML-parser.
          */
         void fatalError(const SAXParseException& toCatch);

         
         /**
          *    Resests internal error state, does nothing.
          */
         void resetErrors();
      //@}
};

#endif // USE_XML

#endif // XMLPARSERERRORREPORTER_H

