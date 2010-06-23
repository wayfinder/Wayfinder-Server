/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERRIGHT_H
#define USERRIGHT_H

#include "config.h"
#include "UserElement.h"
#include "UserEnums.h"
#include "UserConstants.h"

class UserReplyPacket;
class ClientSetting;

/**
 * Class for holding information about user's access to a region.
 *
 *   When packed into or from a packet UserRight looks like this.
 *   The first TYPE_REGION_ACCESS and Total size in bytes and the last 
 *   TYPE_REGION_ACCESS is read by someone else when read from a packet.
 *   \begin{tabular}{lll}
 *      Pos                         & Size     & Destription \\ \hline
 *      pos                         & 4 bytes  & TYPE_RIGHT \\
 *      +4                          & 2 bytes  & Total size in bytes \\
 *      +6                          & 4 bytes  & TYPE_RIGHT \\
 *      +10                         & 2 bytes  & Total size in bytes \\
 *      +12                         & 4 bytes  & UserRight ID \\
 *      +16                         & 4 bytes  & Add time \\
 *      +20                         & 4 bytes  & UserRight Type \\
 *      +24                         & 4 bytes  & Regon ID \\
 *      +28                         & 4 bytes  & Start time \\
 *      +32                         & 4 bytes  & End time \\
 *      +36                         & 1 bytes  & Deleted \\
 *      +37                         & string   & Origin \\
 *   \end{tabular}
 *
 */
class UserRight : public UserElement {
   public:
      /**
       * Constructs a UserRight.
       *
       * @param id The id of the UserRight, use 0 if new UserRight.
       * @param addTime The time when right was added, in UTC.
       * @param type The type of right.
       * @param regionID The id of the region that this right is for.
       * @param startTime The time when the right starts, in UTC.
       * @param endTime The time when the right ends, in UTC.
       * @param deleted If the right is deleted.
       * @param origin String describing from where the right comes.
       */
      UserRight( uint32 id, uint32 addTime, UserEnums::URType type,
                 uint32 regionID, uint32 startTime, uint32 endTime,
                 bool deleted, const char* origin );


      /**
       * Constructs an empty UserRight, must call readFromPacket 
       * before using the UserRight.
       */
      UserRight( uint32 id );


      /**
       * Sets all variables according to parameters in packet.
       */
      UserRight( const Packet* p, int& pos );

   
      /**
       * Copy constructor copies everything.
       */
      UserRight( const UserRight& user );


      /**
       * Destructor.
       */
      virtual ~UserRight();


      /**
       * The maximum size the UserRight will take packed into a 
       * Packet.
       *
       * @return The size in a Packet.
       */
      uint32 getSize() const;


      /**
       * Pack the UserRight into a Packet.
       * May resize the Packet if nessesary.
       *
       * @param p The Packet to pack the UserRight into, may be 
       *          resized to make the UserRight fit.
       * @param pos The position in the Packet.
       */
      virtual void packInto( Packet* p, int& pos );


      /**
       * Adds all changes in the object to the packet.
       *
       */
      virtual void addChanges( Packet* p, int& position );


      /**
       * Reads changes from a packet.
       */
      virtual bool readChanges( const Packet* p, int& pos, 
                                UserConstants::UserAction& action );


      /**
       * Reads a UserRight from p and sets the values in this.
       * Sets this to be ok if read ok.
       *
       * @param p The Packet to read the UserRight from.
       * @param pos The position in the Packet.
       */
      void readFromPacket( const Packet* p, int& pos );


      /**
       * Get the region's add time.
       *
       * @return The region's add time.
       */
      uint32 getAddTime() const;


      /**
       * Set the region's add time.
       *
       * @param val The region's add time.
       */
      void setAddTime( uint32 val );


      /**
       * Get the region's UserRightType.
       *
       * @return The region's UserRightType.
       */
      UserEnums::URType getUserRightType() const;


      /**
       * Set the region's UserRightType.
       *
       * @param val The region's UserRightType.
       */
      void setUserRightType( UserEnums::URType val );


      /**
       * Get the region's ID.
       *
       * @return The region's ID.
       */
      uint32 getRegionID() const;


      /**
       * Set the region's ID.
       *
       * @param val The region's ID.
       */
      void setRegionID( uint32 val );

   /**
    * Get the version lock value if this UserRight object is a version
    * lock user right, i.e. have the UserRightType UR_VERSION_LOCK. If
    * it is not a version lock user right it will return a default
    * value specified by the caller.
    * @param defval The value that will be returned if this is not a
    *               version lock user right.
    * @return The version lock value or <code>defval</code> if this
    *         object is not a version lock right.
    */
   uint32 getVersionLock( uint32 defval ) const;

   /**
    * Set the version lock value if this UserRight object is a version
    * lock user right, i.e. have the UserRitghType UR_VERSION_LOCK. 
    * @param versionLock The new version lock value.
    * @return True if this is a version lock right and the verison
    *         lock value was set, flase if this is not a version lock
    *         right.
    */
   bool setVersionLock( uint32 versionLock );

   
      /**
       * Get the time that the access starts.
       *
       * @return The time that the access starts.
       */
      uint32 getStartTime() const;

   
      /**
       * Set the time that the access starts.
       *
       * @param val The time that the access starts.
       */
      void setStartTime( uint32 val );


      /**
       * Get the time that the access ends.
       *
       * @return The time that the access end.
       */
      uint32 getEndTime() const;


      /**
       * Set the time that the access ends.
       *
       * @param val The time that the access end.
       */
      void setEndTime( uint32 val );


      /**
       * Check if this right has access at a specific time.
       *
       * @param t The time to check.
       */
      bool checkAccessAt( uint32 t ) const;

      /**
       * Get the length of the right, in seconds. 
       *
       * @return The time between start and end in seconds, 
       *         MAX_UINT32 if lifetime right.
       */
      uint32 getRightTimeLength() const;

      /**
       * Check if this right has access at a specific time AND is not
       * deleted.
       *
       * @param t The time to check.
       * @return <code>checkAccesAt( t ) && ! isDeleted()</code>
       */
      bool notDeletedAndValidAt( uint32 t ) const;

      /**
       * Get if the right is deleted.
       */
      bool isDeleted() const;


      /**
       * Set if the right is deleted.
       */
      void setDeleted( bool val );


      /**
       * Get the string describing how the right was created.
       */ 
      const char* getOrigin() const;


      /**
       * Set the string describing how the right was created.
       */ 
      void setOrigin( const char* val );


      /**
       * The number of changed fields.
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
                         UserConstants::UserRightField field ) const;


      /**
       * Get the id of the UserRight element.
       * 
       * @return The id of the UserRight element.
       */
      uint32 getID() const;


      /**
       * Field changed.
       */
      bool changed( UserConstants::UserRightField field ) const;


      /**
       * Returns true if element has changes.
       */ 
      virtual bool isChanged() const;


   private:
      /**
       * Reads changes from a packet.
       */
      bool readDataChanges( const Packet* p, int& pos );


      /**
       * The add time.
       */
      uint32 m_addTime;


      /**
       * The UserRightType.
       */
      UserEnums::URType m_userRightType;


      /**
       * The region id.
       */
      uint32 m_regionID;


      /**
       * The start time.
       */
      uint32 m_startTime;


      /**
       * The end time.
       */
      uint32 m_endTime;


      /**
       * If deleted.
       */
      bool m_deleted;


      /**
       * The origin string.
       */
      MC2String m_origin;


      /**
       * The changed fields.
       */
      bool m_changed[ UserConstants::USER_RIGHT_NBRFIELDS ];
};


// -----------------------------------------------------------------
//  Implementation of inlined methods for UserRight
// -----------------------------------------------------------------


inline uint32
UserRight::getAddTime() const {
   return m_addTime;
}


inline void
UserRight::setAddTime( uint32 val ) {
   m_changed[ UserConstants::USER_RIGHT_ADD_TIME ] = true;
   m_addTime = val;
}


inline UserEnums::URType
UserRight::getUserRightType() const {
   return m_userRightType;
}


inline void
UserRight::setUserRightType( UserEnums::URType val ) {
   m_changed[ UserConstants::USER_RIGHT_TYPE ] = true;
   m_userRightType = val;
}


inline uint32
UserRight::getRegionID() const {
   return m_regionID;
}


inline void
UserRight::setRegionID( uint32 val ) {
   m_changed[ UserConstants::USER_RIGHT_REGION_ID ] = true;
   m_regionID = val;
}

inline uint32 
UserRight::getVersionLock(uint32 value) const
{
   return ((m_userRightType.second == UserEnums::UR_VERSION_LOCK) ?
           getRegionID() : value);
}

inline bool 
UserRight::setVersionLock(uint32 versionLock)
{
   bool is_vl = ( m_userRightType.second == UserEnums::UR_VERSION_LOCK );
   if( is_vl ) {
      setRegionID( versionLock );
   } 
   return is_vl;
}

inline uint32
UserRight::getStartTime() const {
   return m_startTime;
}


inline void
UserRight::setStartTime( uint32 val ) {
   m_changed[ UserConstants::USER_RIGHT_START_TIME ] = true;
   m_startTime = val;
}


inline uint32
UserRight::getEndTime() const {
   return m_endTime;
}


inline void
UserRight::setEndTime( uint32 val ) {
   m_changed[ UserConstants::USER_RIGHT_END_TIME ] = true;
   m_endTime = val;
}

inline bool 
UserRight::notDeletedAndValidAt( uint32 t ) const
{
   return checkAccessAt( t ) && ! isDeleted();
}

inline bool
UserRight::isDeleted() const {
   return m_deleted;
}


inline void
UserRight::setDeleted( bool val ) {
   m_changed[ UserConstants::USER_RIGHT_DELETED ] = true;
   m_deleted = val;
}


inline const char*
UserRight::getOrigin() const {
   return m_origin.c_str();
}


inline void
UserRight::setOrigin( const char* val ) {
   m_changed[ UserConstants::USER_RIGHT_ORIGIN ] = true;
   m_origin = val;
}


inline uint32 
UserRight::getID() const {
   return m_id;
}


#endif // USERRIGHT_H

