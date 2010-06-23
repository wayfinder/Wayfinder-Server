/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVSESSION_H
#define NAVSESSION_H

#include "config.h"
#include <map>


/**
 *   Class that handles session management when communicating with 
 *   some kind of navigation client.
 *
 *   It provides a storage place for state that needs to be 
 *   persistant during a session.
 *
 */
class NavSession {

public:

   /**
    *  Creates a new isabBoxNavComm nav session.
    */
   NavSession();
   
   /** 
    *   Destructor. 
    */
   virtual ~NavSession();

   /**
    *   Are we conected?
    *   @return bool connected.
    */
   virtual bool connected();

   /**
    *   Set binary data.
    *   This object will handle the destruction of the submitted
    *   byte array. Any previous data stored in this object will be
    *   overwritten.
    *
    *   @param data  Binary data.
    *   @param size  Size in bytes of the binary data.
    */
   void setData( byte* data, uint32 size );

   /**
    *   Adds data to the data vector of the session. Does not remove
    *   any earlier stored data.
    *
    *   The data is _not_ copied into the session, deletion of the
    *   data is taken care of by the session object.
    *
    *   @param data Pointer to the data to store.
    *   @param size Size of the data to store.
    */
   void addData( byte* data, uint32 size );

   /**
    *   Deletes all data stored in the session.
    */
   void clearData();

   /**
    *   Returnes a pointer to the byte array with the offset higher than 
    *   the offset, the parameter offset is set to. The pair keeps the 
    *   size of the byte array in the first element and the pointer to 
    *   the byte array in the second element.
    *
    *   @param offset The offset in bytes from the beginnig of the 
    *                 stored data.
    *   @param maxSize The maximum size of the returned data.
    *   @return  The returned pair keeps the size of the byte array in 
    *            the first element and the pointer to the byte array in
    *            the second element.
    */
   pair<int32, byte*> getData(int32 offset) const;
   
   /**
    *   Get the size in bytes of the binary data.
    *   @return The size in bytes of the binary data.
    */
   int32 getSizeOfData() const;

   /**
    *  
    *  @param 
    */
   void setCallDone(int callDone) { m_callDone = callDone; }
   /**
    *  
    *  @param 
    */
   int getCallDone() { return m_callDone; }

   /**
    *   Get max data buffer length to send to client.
    */
   uint32 getMaxBufferLength() const;

   /**
    *   Get the minimum data buffer length to send to client.
    */
   uint32 getMinBufferLength() const;

   /**
    *   Set the maximum packet length to send to the client.
    */
   void setMaxBufferLength( uint32 val );

   /**
    * Dumps the session data to the ostream.
    */
   virtual void dump( ostream& out );

protected:


   int m_callDone;

   /**
    *   Max length of the data buffer sent to the client.
    */
   uint32 m_maxLengthToSendToClient;

   /** 
    *   Can't send packets smaller than this to the client.
    *   Calculated to contain header and some data.
    */
   static const uint32 m_minLengthToSendToClient = 14;

   /**
    *   This variable is used to indicate if a route should
    *   be sent as binary data (poll data reply).
    *   The value decides which format the route data should have.
    */
   uint8 m_downloadAdditionalRouteData;

   /**
    *   Clears all stored state and cached data.
    */
   virtual void clearSessionState();

   /**
    *   Size in bytes of the data.
    */
/*    uint32 m_sizeOfData; */
   
   /**
    *   The data. A map with the pointers to data buffers. The key,
    *   i.e. the int32 containes the size of the data buffer plus
    *   the size of all data buffers stored earlier in the vector.
    *
    *   Example: Sizes of data buffers: 3, 5,  2.
    *            Values of the keys:    3, 8, 10. 
    *
    */
   map<int32, byte*> m_dataBytePtrs;

};



#endif
