/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERLICENCEKEY_H
#define USERLICENCEKEY_H

#include "config.h"
#include "UserElement.h"
#include "UserConstants.h"

class UserReplyPacket;

/**
 * Class for holding a licence key sent to user.
 *
 *   When packed into or from a packet UserLicenceKey looks like this.
 *   The first TYPE_LICENCE_KEY and Total size in bytes and the last 
 *   TYPE_LICENCE_KEY is read by someone else when read from a packet.
 *   \begin{tabular}{lll}
 *      Pos                         & Size     & Destription \\ \hline
 *      pos                         & 4 bytes  & TYPE_LICENCE_KEY \\
 *      +4                          & 2 bytes  & Total size in bytes \\
 *      +6                          & 4 bytes  & TYPE_LICENCE_KEY \\
 *      +10                         & 2 bytes  & Total size in bytes \\
 *      +12                         & 4 bytes  & Licence ID \\
 *      +16                         & 2 bytes  & Licence length \\
 *      +18                         & 2 bytes  & Product length \\
 *      +20                         & 2 bytes  & Key type length \\
 *      +22                         & Licence length bytes & Licence data\\
 *       X                          & String & Product string \\
 *       X                          & String & Key type \\
 *   \end{tabular}
 *
 */
class UserLicenceKey : public UserElement {
   public:
      /**
       * Constructs an empty UserLicenceKey.
       */
      UserLicenceKey( uint32 id = MAX_UINT32 );
      
      /**
       * Constructs a new UserLicenceKey with all data set.
       */
      UserLicenceKey( uint32 id, const byte* licenceKey, 
                      uint32 licenceLength, const MC2String& product,
                      const MC2String& keyType );

      /**
       * Constructs a new UserLicenceKey with all data set.
       */
      UserLicenceKey( uint32 id, const MC2String& licenceKey, 
                      const MC2String& product, const MC2String& keyType );

      /// Sets all variables according to parameters in packet.
      UserLicenceKey( const Packet* p, int& pos );
   
      /// Copy constructor copies everything.
      UserLicenceKey( const UserLicenceKey& user );

      /// Copy assignment
      UserLicenceKey& operator = ( const UserLicenceKey& o );

      /// Deletes all allocated variables
      virtual ~UserLicenceKey();

      /// Returns the size allocated by the variables
      uint32 getSize() const;

      /// Puts the UserLicenceKey into p
      virtual void packInto( Packet* p, int& pos );

      /// Adds all changes in the object to the packet
      virtual void addChanges( Packet* p, int& position );

      /// Reads changes from a packet.
      virtual bool readChanges( const Packet* p, int& pos, 
                                UserConstants::UserAction& action );

      /// Returns the LicenceKey.
      const byte* getLicenceKey() const;

      /// Return the LicenceLength .
      uint32 getLicenceLength() const;

      /**
       * Sets the licence.
       */
      void setLicence( const byte* licenceKey, uint32 licenceLength );

      /**
       * Sets ths licence from a string. 
       * Essentialy the same as 
       * <code>
       * setLicence((const byte*)licenceKey.c_str(), licenceKey.length())
       * </code>
       */
      void setLicence( const MC2String& licenceKey );

      /**
       * Get the product name.
       */
      const MC2String& getProduct() const;

      /**
       * Set the product name.
       */
      void setProduct( const MC2String& name );

      /**
       * Get the licence type.
       */
      const MC2String& getKeyType() const;

      /**
       * Set the licence type.
       */
      void setKeyType( const MC2String& type );

      /**
       * Returns the LicenceKey as a string.
       */
      MC2String getLicenceKeyStr() const;

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
                         UserConstants::UserLicenceKeyField field ) const;

      /**
       * Prints SQL value of field but for the key special care is taken 
       * for key types imei as only 15 digits are printed.
       *
       * @param target String to print into.
       * @param field The field to print.
       * @return The number of chars written to string.
       */
      uint32 printValueIMEIFix( 
         char* target, UserConstants::UserLicenceKeyField field ) const;

      /**
       * Prints like old licence key SQL table value.
       */
      uint32 printOldKeyValue( char* target ) const;

      /**
       * Field changed.
       */
      bool changed( UserConstants::UserLicenceKeyField field ) const;

      /**
       * Returns true if element has changes.
       */ 
      virtual bool isChanged() const;

      /**
       * Compares licenceKey, keyType and product.
       */
      bool compare( const UserLicenceKey& o ) const;

      /**
       * Extract the first 14 digits from the licence key and adds the
       * 15th Luhn check digit to target.
       * Used to get the significant digits for an imei.
       *
       * @param target The digits are added to this string.
       * @return True if 14 digits was found and only '-'s was skipped,
       *         the 15th check digits is added to target too.
       */
      bool extract15Digits( MC2String& target ) const;

      /**
       * Returns true if KeyType is imei.
       */
      bool isIMEIKey() const;

      /**
       * Checks if the licence key is of a format such that we think it
       * contains both IMEI & SV, in which case we have some special handling
       * which removes the SV part before storing the key in the database.
       */
      bool isIMEISV() const;

   private:
      /**
       * Reads changes from a packet.
       */
      bool readDataChanges( const Packet* p, int& pos );
   
      /**
       * The licenceKey.
       */
      byte* m_licenceKey;

      /**
       * The licenceLength.
       */
      uint32 m_licenceLength;

      /**
       * The product name.
       */
      MC2String m_product;

      /**
       * The key type.
       */
      MC2String m_keyType;

      /**
       * The changed fields.
       */
      bool m_changed[UserConstants::USER_LICENCE_KEY_NBRFIELDS];
};


typedef vector< UserLicenceKey > UserLicenceKeyVect;
typedef vector< UserLicenceKey* > UserLicenceKeyPVect;
typedef vector< const UserLicenceKey* > ConstUserLicenceKeyPVect;

// Types from UserData, move to own file.
typedef vector< UserElement* > UserElementVect;
      
/// Return type of getElementRange
typedef pair< UserElementVect::iterator, UserElementVect::iterator > 
   UserElementRange;
   
/// Return type of getElementRange
typedef pair< UserElementVect::const_iterator,
              UserElementVect::const_iterator > ConstUserElementRange;


/**
 * Count the number of keys for a product.
 * Get the keys with a certain product.
 *
 */
class UserLicenceKeyProductCounter {
public:
   /**
    * Constructor.
    *
    * @param product The product to count.
    */
   UserLicenceKeyProductCounter( const MC2String& prod );

   /**
    * Count the nunber of keys with product.
    */
   int getNbrKeys( const UserLicenceKeyPVect& keys ) const;

   /**
    * Count the nunber of keys with product.
    */
   int getNbrKeys( const UserLicenceKeyVect& keys ) const;

   /**
    * Count the nunber of keys with product.
    * Call only with UserLicenceKey elements.
    */
   int getNbrKeys( ConstUserElementRange keys ) const;

   /**
    * Add the pointers to keys with product in keys to prodKeys.
    */
   void getProductKeys( const UserLicenceKeyPVect& keys, 
                        UserLicenceKeyPVect& prodKeys ) const;

private:
   /// The product to count.
   MC2String m_product;
};

#endif // USERLICENCEKEY_H

