/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERELEMENT_H
#define USERELEMENT_H

#include "config.h"
#include "UserConstants.h"
#include "VectorElement.h"

class Packet;

/**
 * Superclass holding a type of information about a user.
 *
 */
class UserElement : public VectorElement
{
  public: 
   /**
    * Constructs a new UserElement of the type \texttt{type}
    */
   UserElement( UserConstants::UserItemType type );

   
   /**
    * Copy constructor
    */
   UserElement( const UserElement& el );
   

   /**
    * Destructor, does nothing.
    */
   virtual ~UserElement();


   /**
    *  Returns the type of the object
    */
   UserConstants::UserItemType getType();


   /**
    *  Returns the size of the allocated variables and elements.
    *  Used to make sure that the element fits in a packet.
    *  @return size of the element packed into a packet. If the precise 
    *          size isn't known the maximum possible size is used.
    */
   virtual uint32 getSize() const;


   /**
    * Packs the Element into a packet.
    */
   virtual void packInto( Packet* p, int& pos );


   /**
    * Add the changes made to the element into a packet.
    */
   virtual void addChanges( Packet* p, int& pos ) = 0;


   /**
    * Reads changes from a packet and returns true if reading was ok.
    */
   virtual bool readChanges( const Packet* p, int& pos, 
                             UserConstants::UserAction& action ) = 0;


   /**
    *  Removes the element. Marks the element for deletion. 
    */
   void remove();


   /**
    *  Checks if the item is removed. If its marked for deletetion.
    */
   bool removed() const;


   /**
    * If the element was loaded ok.
    */
   bool isOk() { return m_ok; }


   /**
    * Inserts the string str as a valid SQL string into target.
    * @return nbr chars written to target.
    */
   static uint32 sqlString( char* target, const char* str );

   /**
    * Inserts the string str as a valid SQL string into target.
    * @return nbr chars written to target.
    */
   static uint32 sqlString( char* target, const MC2String& str );


   /**
    * Turns an unix date 1 Jan 1970 into a
    * SQL DATE and TIME. Eg. '2000-01-21'
    * @param time is the unix time.
    * @param date is where to print the date, at least 12 chars long.
    */
   static void makeSQLOnlyDate( uint32 time, char* dateStr );

   
   /**
    * Returns true if element has changes.
    */ 
   virtual bool isChanged() const = 0;


   /**
    * Get the ID of this UserElement.
    *
    * @return The ID of this UserElement.
    */
   virtual inline uint32 getID() const;


   /**
    * Set the ID of this UserElement.
    *
    * @param id The new ID of this UserElement.
    */
   virtual inline void setID( uint32 id );


  protected:
   /**
    * Sets the Ok flag
    */
   void setOk(bool ok) { m_ok = ok; }


   /// The id of this UserElement.
   uint32 m_id;


  private:
   /// If the Element is to be removed from the UserDatabase.
   bool m_removed;


   /// If the element was loaded correctly
   bool m_ok;

   /// Type of Element
   UserConstants::UserItemType m_type;
};


// -----------------------------------------------------------------
//  Implementation of inlined methods for UserElement
// -----------------------------------------------------------------


uint32
UserElement::getID() const {
   return m_id;
}

void
UserElement::setID( uint32 id ) {
   m_id = id;
}


#endif // USERELEMENT_H
