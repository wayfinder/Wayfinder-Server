/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PROPERTY_H
#define PROPERTY_H

#include "config.h"
#include "BucketNode.h"
#include "Utility.h"
#include "NotCopyable.h"

/**
  *   Describes one item in the property-file.
  *
  */
class Property : public BucketNode, private NotCopyable {
   public:
      /**
        *   Empty constructor that initiates the membervariables
        *   to NULL.   
        */
      Property() {
         key = NULL;
         value = NULL;
      }


      /**
        *   Constructor that initiates the membervariables
        *   to the given values.
        *   @param   key   The key-string for this property. It is 
        *                  copied into the member variable.
        *   @param   value The value-string for this property. It is 
        *                  copied into the member variable.
        */
      Property( const char *key, const char *value ) {
         this->key = new char[strlen(key)+1];
         strcpy(this->key, key);
         this->value = new char[strlen(value)+1];
         strcpy(this->value, value);
         this->hashCode = Utility::hash(key);
      }


      /**
        *   Release the memory used by this object.
        *   This method is inlined.
        *   But it is virtual so it can't be inlined!?
        */
      virtual ~Property() {
//         DEBUG8(cerr << "~Property()" << endl);
         delete [] key;
         delete [] value;
      }


      /**
        *   @return  Pointer to the key for this property.
        *   This method is inlined.
        */
      const char *getKey() const {
         return this->key;
      }


      /**
        *   @param   value The new value for this property,
        *            the value of value is copied into the member variable
        *            value.
        *   This method is inlined.
        */
      void setValue( const char *value ) {
         delete this->value;
         this->value = new char[strlen(value)+1];
         strcpy(this->value, value);
      }


      /**
        *   @return  Pointer to the value of this property.
        *   This method is inlined.
        */
      char *getValue() {
         return this->value;
      }


      /**
        *   @return  The hashcode for this property.
        *   This method is inlined.
        */
      uint16 getHashCode() const {
         return this->hashCode;
      }


   private:
      /**
        *   The key (name) of this property.
        */
      char *key;


      /**
        *   The value of this property.
        */
      char *value;


      /**
        *   The hashcode of this property.
        */
      uint16 hashCode;
};

#endif // PROPERTY_H

