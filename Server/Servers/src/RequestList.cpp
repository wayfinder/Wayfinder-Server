/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "RequestList.h"


RequestList::RequestList()
{
   DEBUG2(cerr << "RequestList::RequestList()" << endl);
}


RequestList::~RequestList()
{
   DEBUG2(cerr << "RequestList::RequestList()" << endl);

   clear();
}


void
RequestList::insert(RequestContainer *req)
{
   DEBUG4(cerr << "RequestList::insert()" << endl);
   req->into(this);
}


void
RequestList::remove(RequestContainer *req)
{
   DEBUG4(cerr << "RequestList::remove()" << endl);

   req->out();
}


RequestContainer *
RequestList::find(uint16 reqID)
{
   DEBUG4(cerr << "RequestList::find(" << reqID << ")" << endl);

   RequestContainer *req = (RequestContainer *)first();
   while (req && req->getRequest()->getID() != reqID)
      req = (RequestContainer *)req->suc();

   return ((req) ? req : NULL);
}


RequestContainer*
RequestList::findAndRemoveDone(){
   DEBUG4(cerr << "RequestList::findAndRemoveDone" << endl;);

   RequestContainer *req = (RequestContainer *)first();
   while ( req && !req->getRequest()->requestDone() )
      req = (RequestContainer *)req->suc();
   if ( (req != NULL) && (req->getRequest()->requestDone()) ) {
      req->out();
   } 
   return req;
}


RequestContainer *
RequestList::findAndRemove(uint16 reqID)
{
   DEBUG4(cerr << "RequestList::findAndRemove(" << reqID << ")" << endl);

   RequestContainer *req = (RequestContainer *)first();
   while (req && req->getRequest()->getID() != reqID)
      req = (RequestContainer *)req->suc();
   if ( req != NULL ) {
      req->out();
   }
   return ((req) ? req : NULL);
}


RequestContainer* 
RequestList::removeFirst() {
   RequestContainer *req = (RequestContainer *)first();
   if ( req != NULL ) {
      remove( req );
   }
   
   return req;
}


uint32 
RequestList::getCardinal() {
   return cardinal();
}
