/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USER_ID_KEY_H
#define USER_ID_KEY_H

#include "config.h"
#include "UserElement.h"
#include "UserConstants.h"

class UserReplyPacket;


/**
 * Class for holding a id key for a user.
 *
 *   When packed into or from a packet UserIDKey looks like this.
 *   The first TYPE_ID_KEY and Total size in bytes and the last 
 *   TYPE_ID_KEY is read by someone else when read from a packet.
 *   \begin{tabular}{lll}
 *      Pos                         & Size     & Destription \\ \hline
 *      pos                         & 4 bytes  & TYPE_ID_KEY \\
 *      +4                          & 2 bytes  & Total size in bytes \\
 *      +6                          & 4 bytes  & TYPE_ID_KEY \\
 *      +10                         & 2 bytes  & Total size in bytes \\
 *      +12                         & 4 bytes  & ID \\
 *      +16                         & 4 bytes  & Type \\
 *      +20                         & string   & ID-Key \\
 *   \end{tabular}
 */
class UserIDKey : public UserElement {
public:
   /**
    * The type of identifier.
    */
   enum idKey_t {
      /// Account identifier
      account_id_key = 0,

      /// Hardware identifier (Cellular)
      hardware_id_key = 1,

      /// Hardware identifier and time. Used by NavExternalAuth.
      hardware_id_and_time = 2,

      /// Service ID and time
      service_id_and_time = 3,

      /// Latest use of server/client type
      client_type_and_time = 4,
   };


   /**
    * Constructs an empty UserIDKey.
    */
   UserIDKey( uint32 id );

      
   /**
    * Constructs a new UserIDKey with all data set.
    */
   UserIDKey( uint32 id, const MC2String& idKey, idKey_t type );


   /// Sets all variables according to parameters in packet.
   UserIDKey( const Packet* p, int& pos );

   
   /// Copy constructor copies everything.
   UserIDKey( const UserIDKey& user );
  

   /// Deletes all allocated variables
   virtual ~UserIDKey();


   /// The maximum size the element will take packed into a Packet.
   uint32 getSize() const;


   /// Puts the UserIDKey into p
   virtual void packInto( Packet* p, int& pos );


   /// Adds all changes in the object to the packet
   virtual void addChanges( Packet* p, int& position );


   /// Reads changes from a packet.
   virtual bool readChanges( const Packet* p, int& pos, 
                             UserConstants::UserAction& action );


   /**
    * Reads a element from p and sets the values in this.
    * Sets this to be ok if read ok.
    *
    * @param p The Packet to read the element from.
    * @param pos The position in the Packet.
    */
   void readFromPacket( const Packet* p, int& pos );

      
   /**
    * Returns the id key.
    */
   const MC2String& getIDKey() const;

      
   /**
    * Sets the id key.
    */
   void setIDKey( const MC2String& idKey );

      
   /**
    * Returns the id type.
    */
   idKey_t getIDType() const;

      
   /**
    * Sets the id type.
    */
   void setIDType( idKey_t type );


   /**
    * The number of changed fields
    */
   uint32 getNbrChanged() const;


   /**
    * Prints SQL value of field.
    *
    * @param target String to print into.
    * @param field The field to print.
    * @return The number of chars written to string.
    */
   uint32 printValue( char* target, 
                      UserConstants::UserIDKeyField field ) const;


   /**
    * Field changed.
    */
   bool changed( UserConstants::UserIDKeyField field ) const;


   /**
    * Returns true if element has changes.
    */ 
   virtual bool isChanged() const;

   /**
    * Print the UserIDKey to a stream.
    *
    * @param stream The stream to print on.
    * @param idKey The key to print.
    * @return The stream.
    */
   friend ostream& operator<<( ostream& stream,
                               const UserIDKey& idkey );

   /**
    * Compares this key with other.
    */
   bool compare( const UserIDKey& other ) const;

private:
   /**
    * Reads changes from a packet.
    */
   bool readDataChanges( const Packet* p, int& pos );

   
   /**
    * The key type.
    */
   idKey_t m_idType;

   
   /**
    * The idKey.
    */
   MC2String m_idKey;


   /**
    * The changed fields.
    */
   bool m_changed[UserConstants::USER_ID_KEY_NBRFIELDS];
};


// -----------------------------------------------------------------
//  Implementation of inlined methods for UserIDKey
// -----------------------------------------------------------------


inline const MC2String&
UserIDKey::getIDKey() const {
   return m_idKey;
}


inline UserIDKey::idKey_t
UserIDKey::getIDType() const {
   return m_idType;
}


#endif // USER_ID_KEY_H

