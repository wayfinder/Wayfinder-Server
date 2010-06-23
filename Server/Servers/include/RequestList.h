/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REQUESTLIST_H
#define REQUESTLIST_H

#include "config.h"
#include "CCSet.h"
#include "RequestContainer.h"



/**
 *    A list of pending requests.
 *
 */
class RequestList : private Head {
   public:
      /**
       * Constructs a new empty RequestList.
       */
      RequestList();

      
      /**
       * Destructor.
       */
      virtual ~RequestList();


      /**
       * Insert a requestcontainer first in the list.
       */
      void insert( RequestContainer* req );


      /**
       * Remove a requestcontainer from the list.
       */
      void remove( RequestContainer* req );


      /**
       * Find a particular request in the list.
       *
       * @param   reqID The identification of the request to lookup.
       *
       * @return  the requestcontainer, NULL if not found.
       */
      RequestContainer* find( uint16 reqID );


      /**
       * Find a done request and removes it.
       *
       * @return  the requestcontainer, NULL if not found.
       */
      RequestContainer* findAndRemoveDone();


      /**
       * Find a particular request in the list and removes it.
       *
       * @param   reqID The identification of the request to lookup.
       *
       * @return  the requestcontainer, NULL if not found.
       */
      RequestContainer *findAndRemove(uint16 reqID);


      /**
       * Removes and returns the first request in the list. NULL if
       * no request in list.
       *
       * @return First request in list or NULL it empty list.
       */
      RequestContainer* removeFirst();

      
      /**
       * Get number of requests in list.
       */
      uint32 getCardinal();

   private:
};

#endif // RequestList

