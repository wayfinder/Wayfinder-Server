/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPFUNCTIONHANDLER_H
#define HTTPFUNCTIONHANDLER_H


// Includes

#include "config.h"

#include "StringUtility.h"
#include "HttpParserThreadConfig.h"

#include "ImageDraw.h"
#include "ServerProximityPackets.h"
#include "StringTableUtility.h"
#include "NotCopyable.h"

// Forward declatations
class ThreadRequestContainer;
class UserSearchRequestPacket;
class RouteRequest;
class SearchRequest;
class UserItem;
class UserStorage;
class SearchMatchLink;
class PacketContainer;
class ExpandItemID;
class ExpandStringItem;

class HttpHeader;
class HttpBody;
class HttpFunctionNote; // See below
class HttpVariableContainer; // See below
class HttpParserThread; // See HttpParserThread.h

//////////////////////////////////////////////////////////////
// HttpFunctionHandler
//////////////////////////////////////////////////////////////

/**
 * Contains the functions used to make dynamic pages.
 *
 */
class HttpFunctionHandler: private NotCopyable {
   public:
      /** 
       * Definition of a Html function. 
       * \begin{itemize}
       *   \item The stringVector* is the vector containing the parameters.
       *   \item The int is the acctual number of parameters in 
       *         the stringVector*.
       *   \item The ParametersMap* contains all the parameters to the 
       *         page.
       *   \item The first HttpHead* is the Header of the request.
       *   \item The second HttpHead* is the Header of the responce.
       *   \item The first HttpBody* is the body of the request.
       *   \item The secong HttpBody* is the body of the responce.
       *   \item The HttpThread* is the caller of the function. 
       *         Used to send requests.
       *   \item The HttpVariableContainer contains the variables passed 
       *         between the functions, while parsing the page.
       * \end{itemize}
       */
      typedef bool (htmlFunction)( stringVector* params,
                                   int paramc, 
                                   stringMap* paramsMap,
                                   HttpHeader* inHead, 
                                   HttpHeader* outHead,
                                   HttpBody* inBody,
                                   HttpBody* outBody,
                                   HttpParserThread* myThread,
                                   HttpVariableContainer* myVar );


      /**
       * Constructs a new HttpFunctionHandler
       */
      HttpFunctionHandler();


      /**
       * Removes allocated memory.
       */
      ~HttpFunctionHandler();


      /**
       * Returns a notice if the string func corresponds to a function.
       * @param func the string with the name of the function
       * @return a notice pointer. NULL if no such function.
       */
      HttpFunctionNote* getFunctionNoteOf(const MC2String& func);


      /**
       * Requrns the variables for the htmlfunctions.
       * @return pointer to the HttpVariableContainer.
       */
      HttpVariableContainer* getVariableContainer();
      

   private:
      typedef map<MC2String, HttpFunctionNote*> FunctionsMap;
      /// Maps between function name and function poiner.
      FunctionsMap m_funcMap;


      /// The variables used while parsing a page.
      HttpVariableContainer* variableContainer;

      /**
       * Add a method.
       */
      void addMethod( const MC2String& name, HttpFunctionNote* n );

      /**
       * A testfunction.
       */
      static bool htmlTest( stringVector* params,
                            int paramc, 
                            stringMap* paramsMap,
                            HttpHeader* inHead, 
                            HttpHeader* outHead,
                            HttpBody* inBody,
                            HttpBody* outBody,
                            HttpParserThread* myThread,
                            HttpVariableContainer* myVar );
};


//////////////////////////////////////////////////////////////
// HttpVariableContainer
//////////////////////////////////////////////////////////////


/**
 * Contains variables for the HttpFunctions.
 */
class HttpVariableContainer {
   public:
      /**
       * Constructs a new container with undefined values.
       */
      HttpVariableContainer();


      /**
       * Destructs the variablecontainer. Does nothing.
       */
      ~HttpVariableContainer();


      // Selected language
      StringTable::languageCode currentLanguage;

      // Sockets and Crypt
      bool https;

      
   private:
};


//////////////////////////////////////////////////////////////
// HttpFunctionNote
//////////////////////////////////////////////////////////////


/**
 * A notice to hold information about and a httpFunction
 *
 */
class HttpFunctionNote {
   public:
      /**
       * A notice to a html-function.
       * @param theFunction is the html generating function.
       * @param minArguments is the mininum number of arguments needed
       *        in order to run the function.
       */
      HttpFunctionNote( HttpFunctionHandler::htmlFunction* theFunction, 
                        uint32 minArguments );
   

      /** The mininum allowed number of parameters to the function.
       * @returns the minargc.
       */  
      uint32 getMinArguments();
   

      /**
       * Return the function of this notice
       * @return the function.
       */
      HttpFunctionHandler::htmlFunction* getFunction();
      
      
   private:
      /// The function of this notice
      HttpFunctionHandler::htmlFunction* m_Function; 

      
      /// The minimum number of arguments
      uint32 m_minArgc;
};

#endif // HTTPFUNCTIONHANDLER_H
