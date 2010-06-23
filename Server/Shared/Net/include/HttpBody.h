/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPBODY_H
#define HTTPBODY_H

// Includes
#include "config.h"
#include "MC2String.h"

class SharedBuffer;


/** 
 *    Http Body, represents a HttpBody.
 * 
 */
class HttpBody {
   public:
      /** 
       * Creates a new Body with the content of Body. 
       *
       * @param Body is the raw Body.
       */
      HttpBody( const MC2String& Body );
   
   
      /**
       * Constructs a empty body
       */
      HttpBody();
   
   
      /**
       * Removes the allocated data.
       */
      virtual ~HttpBody();
   
   
      /** 
       * Adds a string to the body
       * @param Str is the string to add to the body
       */
      void addString(const MC2String& Str);
   
   
      /**
       * Adds a string to the body.
       * @param Str is the string to add to the body.
       */
      void addString(const char* Str);


      /**
       * Adds a char to the body.
       * @param ch is the char to add to the body.
       */ 
      void addString(const char ch);
      
      
      /**
       * Adds a string to the body and makes the first char capital.
       * @param str s the string to add to the body.
       */ 
      void addAndSetCapital( const MC2String& str );
      
      
      /**
       * Adds a string to the body and makes the first char capital.
       * @param str s the string to add to the body.
       */
      void addAndSetCapital( const char* str );
      
      
      /** 
       * Clears the body of any content. Resets the size to allocationsize
       */
      void clear();
      
      
      /**
       * Sets the contents of the body.
       * @param newBody is the new bodycontent. Is owned and handled by the
       *        body.
       */
      void setBody(MC2String* newBody);


      /**
       * Sets the contents of the body to the binary content in newBody.
       * Sets the binary flag to true.
       * @param newBody is the new bodycontent. Is owned and handled by the
       *        body.
       * @param length The length of newBody.
       */
      void setBody( const byte* newBody, uint32 length );

      /**
       *   Sets the contents of the body to the binary content in buf.
       *   Sets the binary flag to true.
       *   @param buf is the new bodycontent, copied into the HttpBody.
       *   Same as setBody buf.getBufferAddress(), buf.getBufferSize()
       */
      void setBody( const SharedBuffer& buf );

      /**
       * Sets the binary flag, is default false.
       * @param flag The new value of the binary flag.
       */
      void setBinary( bool flag );


      /**
       * Returns the binary flag.
       */
      inline bool getBinary() const;
      

      /**
       *   Returns the CharSet for the body.
       *   NULL means do not use.
       */
      const char* getCharSet() const;

   
      /**
       *   Sets the CharSet for the body.
       */
      void setCharSet( const char* charSet, bool mayBeConverted = false  );
   
      /**
       *   Returns true if the body may be recoded to a different
       *   character set.
       */
      bool charSetMayBeChanged() const;

   
      /**
       * Returns the content of the body.
       * @return returns the body. May not be deleted.
       */
      const char* getBody();
      
      
      /**
       * Returns the length of the body, 0 if no body.
       * @return the length of the body.
       */
      uint32 getBodyLength() const;
      
      
      /**
       * Sets the allocation size for the body. Never less than 1.
       */
      void setAllocationSize(uint32 newSize);


      /**
       * Default allocation size in chars.
       */
      static const uint32 defaultAllocSize = 4096;

   
  private:
      /**
       * Make sure size chars can be added to the Body.
       * Reallocates if nessesary
       */
      void checkSize( uint32 size );
      
      
      /** The Body */
      char* m_body;
      

      /// The allocation size
      uint32 m_allocSize;
      
      
      /// The current size of the body
      uint32 m_size;
      

      /** The current position in the Body */
      uint32 m_pos;

      
      /** The binary flag */
      bool m_binary;

     /**
      *   True if the character set of the body may be changed
      *   by the HttpParserThread.
      */
      bool m_charSetMayBeChanged;

      /**
       *   Character set of the body. Used if charSetMayBeChanged is
       *   false. (By HttpParserThread).
       */
      MC2String m_charSet;

      /**
       * HttpInterfaceRequest is a friend.
       */
      friend class HttpInterfaceRequest;
};


// ========================================================================
//                                  Implementation of the inlined methods =


bool 
HttpBody::getBinary() const {
   return m_binary;
}


#endif // HTTPBODY_H
