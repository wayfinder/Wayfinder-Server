/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERPACKET_H
#define USERPACKET_H

#include "config.h"
#include "Types.h"
#include "Packet.h"
#include "StringTable.h"
#include <vector>

#define USER_REPLY_PRIO   DEFAULT_PACKET_PRIO
#define USER_REQUEST_PRIO DEFAULT_PACKET_PRIO


// Forward declaration
class UserItem;
class UserElement;
class UserUser;
class CellularPhoneModels;
class CellularPhoneModel;
class DBUserNavDestination;
class DebitElement;


/**
 *    Abstract superclass of all packets sent from the UserModule.
 *    In addition to the normal Packet header, this packet contans 
 *    an UIN (4bytes).
 *
 */
class UserRequestPacket : public RequestPacket {
   public:
      /** 
        *   The length of a UserRequestPackets header.
        */
      #define USER_REQUEST_HEADER_SIZE (REQUEST_HEADER_SIZE + 4)

      /**
       * Constructor, makes a UserRequestPacket with subType subType.
       * @param bufLength is the size of the packet.
       * @param subType is the type of userpacket. See Packet.h
       * @param packetID is the ID of the packet.
       * @param requestID is the requestID  of the packet.
       * @param UIN is the User Identification Number.
       */
      UserRequestPacket(uint32 bufLength,
                        uint16 subType,
                        uint16 packetID,
                        uint32 requestID,
                        uint32 UIN);

      /**
       * Constructor, makes a UserRequestPacket with subType subType.
       * @param bufLength is the size of the packet.
       * @param subType is the type of userpacket. See Packet.h
       * @param UIN is the User Identification Number.
       */
      UserRequestPacket(uint32 bufLength,
                        uint16 subType,
                        uint32 UIN);

      /**
       *  Get the UIN of the requested user.
       *  @return The UIN of the requested user.
       */
      uint32 getUIN() const;

  private:
      /**
       *    Default constructor. Declaired privat so that it not can 
       *    be used from outside this class.
       */
      UserRequestPacket();
};


/**
 *    Abstract superclass of all packets sent from the UserModule.
 *    In addition to the normal header, this packet contans UIN (4bytes).
 *
 */
class UserReplyPacket : public ReplyPacket
{
   public:
      /** 
        *   The length of a UserReplyPackets header.
        */
      #define USER_REPLY_HEADER_SIZE (REPLY_HEADER_SIZE + 4)

      /**
       *    Constructor.
       *    @param bufLength  The length of the packet.
       *    @param subType    The type of userreplypacket.
       *    @param inpacket   The UserRequestPacket that this is a reply to.
       */
      UserReplyPacket(uint32 bufLength,
                      uint16 subType,
                      const UserRequestPacket*  inpacket);
      
      /**
       *    Constructor.
       *    @param bufLength  The length of the packet.
       *    @param subType    The type of userreplypacket.
       */
      UserReplyPacket( uint32 bufLength,
                       uint16 subType );
      
      /**
       *  Get the UIN of the requested user.
       *  @return The UIN of the requested user.
       */
      uint32 getUIN() const;

   private:
      /**
       *    Default constructor. Declaired privat so that it not can 
       *    be used from outside this class.
       */
      UserReplyPacket();
};


/**
 *    RequestPacket for adding a user to the database.
 *    After the normal header the request packet contains:
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & 2 bytes  & Number elements \\
 *       +2                          & 2 bytes  & Strlen passwd \\
 *       +4                          & 4 bytes  & changerUIN \\
 *       +8                          & string   & passwd \\
 *       for Number elements         & data     & Elements \\
 *    \end{tabular}
 *
 */
class AddUserRequestPacket : public UserRequestPacket
{
   public:
      /**
       * Constructor
       * @param packetID is the id of the packet.
       * @param requestID is the id of the request sending the packet.
       * @param user is the user to add. NB: UIN must be 0!.
       * @param passwd is the password which the user logs on with
       * @param changerUIN The UIN for the user adding the user.
       */
      AddUserRequestPacket( uint16 packetID, uint16 requestID,
                            UserUser* user, const char* passwd,
                            uint32 changerUIN );
      
      /**
       *    Get number of elements in packet.
       *    @return  The number of elements in this packet.
       */
      uint16 getNbrElements() const;
      

      /**
       * Get the changerUIN.
       */
      uint32 getChangerUIN() const;


      /**
       *    Additional data about a user. 
       *    @return True if data was put into packet.
       */
      bool addElement( UserElement* elem );

      /**
       *    Get the position for the first element.
       *    @return The position of the first element.
       */
      int getFirstElementPosition() const;

      /**
       *    Extracts the next element from packet.
       *    @param   pos   Parameter that is used internal to maintain the
       *                   current position.
       *    @return  The next user. Must be deleted by the caller!
       */
      UserElement* getNextElement( int& pos ) const;

      /**
       *    Get the password of the user.
       *    @return A pointer to the password.
       */
      const char* getPassword() const;

   protected:
      /** 
        *   Set the number of elements in the packet.
        *   @param   nbrElements The new value of the number of elements
        *                        in this packet.
        */
      void setNbrElements( uint16 nbrElements );


      /** 
        *   Get the length of the password string, not including the 
        *   terminat.
        *   @return  The length of the password, the stringterminator 
        *            ('\0') in not included in the length.
        */
      uint16 getPasswordLength() const;
};


/**
 *    Reply to a AddUserRequestPacket with status of the operation.
 *    After the normal header the request packet contains:
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REPLY_HEADER_SIZE      & 4 bytes  & UIN \\
 *    \end{tabular}
 *
 */
class AddUserReplyPacket : public UserReplyPacket
{
   public:
      /**
       *    Create a AddUserReplyPacket from a request packet. This
       *    reply includes the status of the operation.
       *    @param   packet
       *    @param   UIN   The identification number of the user. If 
       *                   UIN == 0 then not ok else ok.
       */
      AddUserReplyPacket( const AddUserRequestPacket* packet, uint32 UIN);

      /**
       *    Get the identification of the added user.
       *    @return  The UIN of the added user.
       */
      uint32 getUIN() const;
};


/**
 *    Packet for removing an User from the database.
 *
 */
class DeleteUserRequestPacket : public UserRequestPacket
{
   public:
      /**
       *    Create a delete user request with given packet and request 
       *    IDs, and the identification number of the user.
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that whants the
       *                         reply of this request packet.
       *    @param   UIN         The identification of the user.
       */
      DeleteUserRequestPacket( uint16 packetID, 
                               uint16 requestID,
                               uint32 UIN );
};


/**
 *    ReplyPacket to a DeleteUserRequestPacket with status of the 
 *    operation.
 *
 */
class DeleteUserReplyPacket : public UserReplyPacket
{
   public:
      /**
       *    Create a reply to a DeleteUserRequestPacket with status 
       *    of the operation.
       *    @param   packet   The packet that this is a reply to.
       *    @param   ok       The status of the performed operation.
       */
      DeleteUserReplyPacket( const DeleteUserRequestPacket* packet,
                             bool ok );
      
      /**
       *    Get the status of the operation.
       *    @return True if the operation was a success, false otherwise.
       */
      bool operationOK() const;
};


/**
 *    Packet for altering data about a User.
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & 2 bytes  & nbrElements \\
 *       +2                          & 4 bytes  & changerUIN  \\
 *       for each element            & data     & UserElemtChanges \\
 *    \end{tabular}
 *
 */
class ChangeUserDataRequestPacket : public UserRequestPacket
{
   public:
      /**
       *    Create a change user request packet.
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that whants the
       *                         reply of this request packet.
       *    @param   UIN         The identification of the user.
       *    @param changerUIN    The UIN for the user changing the user.
       */
      ChangeUserDataRequestPacket( uint16 packetID, 
                                   uint16 requestID,
                                   uint32 UIN,
                                   uint32 changerUIN );


      /**
       *    Add the changes of elem to the packet.
       *    @param   elem  The user to add.
       *    @return  True if the new user is added to the packet, false
       *             otherwise.
       */
      bool addChangedElement( UserElement* elem );


      /**
       *    The number of changed elements in packet
       */
      uint16 getNbrElements() const;


      /**
       * Get the changerUIN.
       */
      uint32 getChangerUIN() const;


      /**
       *    Get the position of the first element in the packet. This
       *    method should be used in getNextElement.
       *    @return  The position of the first element in this packet.
       */
      int getFirstElementPosition() const;


      /**
       *    Get the next user in this packet. This method should be used
       *    together with getFirstElementPosition().
       *    @param   pos   The current position in the packet. This should
       *                   be initiated by using the 
       *                   getFirstElementPosition-method.
       *    @return  The next UserElement in the packet.
       */
      UserElement* getNextElement( int& pos ) const;

   
   private:
      /**
        *   Set the number of elements in the packet.
        *   @param   nbrElements The new number of elements in this 
        *            packet.
        */
      void setNbrElements( uint16 nbrElements );
};


/**
 *    Reply to a ChangeUserDataRequestPacket with status of the 
 *    operation.
 *
 */
class ChangeUserDataReplyPacket : public UserReplyPacket
{
   public:
      /**
       *    Create a change user data reply, from the corresponding
       *    request packet.
       *    Create a replypacket from a change password request packet.
       *    @param   packet   The corresponding request packet.
       *    @param   ok       The status of the performed operation.
       */
      ChangeUserDataReplyPacket( const ChangeUserDataRequestPacket* packet,
                                 bool ok );

      /**
       *    Get the status of the performed operation.
       *    @return True if the operation was a success.
       */
      bool operationOK() const;
};


/**
 *    Get data about a user. After the normal header the request 
 *    packet contains:
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & 4 bytes  & Group type \\
 *    \end{tabular}
 *
 */
class GetUserDataRequestPacket : public UserRequestPacket
{
   public:
      /**
       *    Create a packet to get data about one user.
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that whants the
       *                         reply of this request packet.
       *    @param   UID         The identification of the user.
       *    @param   elementTypes The types of userelements 
       *                         to return, use operator | and 
       *                         UserContants::UserItemType fields.
       *    @param   notFromCache If to get clean user not from cache.
       */
      GetUserDataRequestPacket( uint16 packetID,
                                uint16 requestID,
                                uint32 UID,
                                uint32 elementTypes,
                                bool notFromCache = false );

      /**
       *    Find out what kind of information that is requested.
       *    @return  The group of information requested.
       */
      uint32 getElementType() const;


      /**
       * Get wipeFromCache.
       */
      bool getNotFromCache() const;
};


/**
 *    Reply to a GetUserDataRequestPacket with data if status is OK.
 *    If there is data in the packet then it is stored like this.
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & 4 bytes  & elementTypes \\
 *       +4                          & 2 bytes  & nbrFields \\
 *       nbrFields                   & data     & Fields (UserUser, 
 *                                   &          &         UserCellular)\\
 *       last                        & 2 byte   & Group type \\
 *    \end{tabular}
 *
 */
class GetUserDataReplyPacket : public UserReplyPacket
{
   public:
      /**
       *    Create a replypacket from a the corresponding request packet.
       *    @param   inPacket    The corresponding request packet.
       *    @param   bufLength   The size of the packet, 
       *                         default MAX_PACKET_SIZE.
       */
      GetUserDataReplyPacket( const GetUserDataRequestPacket* inPacket,
                              uint16 bufLength = MAX_PACKET_SIZE );


      /**
       *    Create a replypacket from a nothing.
       *    @param   bufLength   The size of the packet, 
       *                         default MAX_PACKET_SIZE.
       */
      GetUserDataReplyPacket( uint16 bufLength = MAX_PACKET_SIZE );

      /**
       *    Add an userelement to packet and returns if it really was 
       *    stored.
       *    @param   elem  The user to add to this packet.
       *    @return  True if the user is added, false otherwise.
       */
      bool addUserElement( UserElement* elem );

      /**
       *    Get the number of datafields in the packet.
       *    @return The amount of dataFields in packet. 
       */
      uint16 getNbrFields() const;

      /**
       *    Get the position of the first field in this packet. Used 
       *    together with getNextElement.
       *    @return  The position of the first field position in the packet. 
       */
      int getFirstElementPosition() const;

      /**
       *    Extract data from the packet. The position is initialized by
       *    using the getFirstElementPosition-method.
       *    @param   pos   The position of the next element.
       *    @return  The next user element.
       */
      UserElement* getNextElement( int& pos ) const;


   protected:
      /**
       *    Set the amount of dataFields in packet.
       *    @param   nbrFields   The new number of fields in this packet.
       */
      void setNbrFields( uint16 nbrFields );
};


/**
 *    User login packet.
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & 2 bytes  & Login strlen \\
 *       +2                          & 2 bytes  & Password strlen \\
 *       +4                          & 1 bytes  & checkExpired \\
 *                                   & string   & logonID \\ 
 *                                   & string   & password \\
 *    \end{tabular}
 *
 */
class CheckUserPasswordRequestPacket : public UserRequestPacket
{
   public:
      /**
       *    Create a requestpacket to check the password of a user.
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that whants the
       *                         reply of this request packet.
       *    @param   logonID     The login string of the user.
       *    @param   password    The password string of the user.
       *    @param   checkExpired If to check if user is expired, default
       *                          false.
       */
      CheckUserPasswordRequestPacket( uint16 packetID,
                                      uint16 requestID,
                                      const char* logonID,
                                      const char* password,
                                      bool checkExpired = false );

      /**
       *    Get the username.
       *    @return  Logon of the user.
       */
      const char* getLogonID() const;

      /**
       *    Get the password.
       *    @return  Get the pasword of the user.
       */
      const char* getPassword() const;

      /**
       * Get if checkExpired.
       */
      bool getExpired() const;
};


/**
 *    Reply to a CheckUserPasswordRequestPacket. If the password and 
 *    logon in the request was correct the UIN of the user is 
 *    returned in this packet.
 *
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REPLY_HEADER_SIZE      & 4 bytes  & UIN \\
 *       +4                          & 2 bytes  & sessionID strlen \\
 *       +6                          & 2 bytes  & sessionKey strlen \\
 *                                   & string   & sessionID \\ 
 *                                   & string   & sessionKey \\
 *    \end{tabular}
 *
 */
class CheckUserPasswordReplyPacket : public UserReplyPacket
{
   public:
      /**
       *    Create the reply packet from the corresponding request packet.
       *    Create a replypacket from a check password request packet.
       *    @param   packet   The corresponding request packet.
       *    @param   UIN      The identification of the user that have the
       *                      given logon and password. 0 if that user not
       *                      OK.
       */
      CheckUserPasswordReplyPacket(const CheckUserPasswordRequestPacket* packet,
                                   uint32 UIN, 
                                   const char* sessionID,
                                   const char* sessionKey );

      
      /**
       *    Get the identification of the user, 0 if login failed.
       *    @return  The UIN of the user, 0 if login failed and 
       *             MAX_UINT32 -1 if user is expired.
       */
      uint32 getUIN() const;

      
      /**
       *    Get the SessionID of this login.
       *    @return The sessionID associated with this login. Is empty 
       *            string if login falied.
       */
      const char* getSessionID() const;


      /**
       *    Get the SessionKey of this login.
       *    @return The sessionKey associated with this login. Is empty 
       *            string if login falied.
       */
      const char* getSessionKey() const;
};


/**
 *    Verify user packet. Verifies a session.
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & 2 bytes  & sessionID strlen \\
 *       +2                          & 2 bytes  & sessionKey strlen \\
 *       +4                          & 1 bytes  & checkExpired \\
 *                                   & string   & sessionID \\ 
 *                                   & string   & sessionKey \\
 *    \end{tabular}
 *
 */
class VerifyUserRequestPacket : public UserRequestPacket
{
   public:
      /**
       * Creates a new UserVerifyRequestPacket for checking a session.
       *
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that whants the
       *                         reply of this request packet.
       *    @param   sessionID   The sessionID string.
       *    @param   password    The sessionKey string.
       *    @param   checkExpired If to check if user is expired, default
       *                          false.
       */
      VerifyUserRequestPacket( uint16 packetID,
                               uint16 requestID,
                               const char* sessionID,
                               const char* sessionKey,
                               bool checkExpired = false );
      
     
      /**
       *    Get the sessionID.
       *    @return  sessionID string.
       */
      const char* getSessionID() const;


      /**
       *    Get the sessionKey.
       *    @return  Get the sessionKey string.
       */
      const char* getSessionKey() const;

      /**
       * Get if checkExpired.
       */
      bool getExpired() const;
};


/**
 *    Reply to a VerifyUserRequestPacket. If the sessionID and 
 *    sessionKey in the request was correct the UIN of the user 
 *    returned in this packet.
 *
 */
class VerifyUserReplyPacket : public UserReplyPacket
{
   public:
      /**
       *    Create the reply packet from the corresponding request packet.
       *    Create a replypacket from a verifyuserrequestpacket.
       *    @param   packet   The corresponding request packet.
       *    @param   UIN      The identification of the user that have the
       *                      given session.
       */
      VerifyUserReplyPacket( const VerifyUserRequestPacket* packet, 
                             uint32 UIN  );

      
      /**
       *    Get the identification numberof the user. 
       *    @returm  The UIN of the user. 0 if no such session, MAX_UINT32
       *             if session has expired and MAX_UINT32-1 if expired 
       *             user.
       */
      uint32 getUIN() const;
};


/**
 *    Logout user packet. Removes a session.
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & 2 bytes  & sessionID strlen \\
 *       +2                          & 2 bytes  & sessionKey strlen \\
 *                                   & string   & sessionID \\ 
 *                                   & string   & sessionKey \\
 *    \end{tabular}
 *
 */
class LogoutUserRequestPacket : public UserRequestPacket
{
   public:
      /**
       * Creates a new LogoutUserRequestPacket for obsoleting a session.
       *
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that whants the
       *                         reply of this request packet.
       *    @param   sessionID   The sessionID string.
       *    @param   password    The sessionKey string.
       */
      LogoutUserRequestPacket( uint16 packetID,
                               uint16 requestID,
                               const char* sessionID,
                               const char* sessionKey );
      
     
      /**
       *    Get the sessionID.
       *    @return  sessionID string.
       */
      const char* getSessionID() const;


      /**
       *    Get the sessionKey.
       *    @return  Get the sessionKey string.
       */
      const char* getSessionKey() const;
};


/**
 *    Reply to a LogoutUserRequestPacket. If the sessionID and 
 *    sessionKey in the request was correct the UIN of the user 
 *    returned in this packet.
 *
 */
class LogoutUserReplyPacket : public UserReplyPacket
{
   public:
      /**
       *    Create the reply packet from the corresponding request packet.
       *    Create a replypacket from a LogoutUserRequestPacket.
       *    @param   packet   The corresponding request packet.
       *    @param   UIN      The identification of the user that have the
       *                      given session. 0 if that session doesn't 
       *                      exist..
       */
      LogoutUserReplyPacket( const LogoutUserRequestPacket* packet, 
                             uint32 UIN  );

      
      /**
       *    Get the identification numberof the user. 
       *    @returm  The UIN of the user. 0 if no such session.
       */
      uint32 getUIN() const;
};


/**
 *    Request for starting a session cleanup. Inactive sessions are
 *    moved to history, or removed. 
 *    WARNING: All old sessions are moved, this can take a while...
 *
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & 2 bytes  & sessionID strlen \\
 *       +2                          & 2 bytes  & sessionKey strlen \\
 *                                   & string   & sessionID \\ 
 *                                   & string   & sessionKey \\
 *    \end{tabular}
 *
 */
class SessionCleanUpRequestPacket : public UserRequestPacket
{
   public:
      /**
       * Creates a new SessionCleanUpRequestPacket for cleaning up
       * sessions by moving old to history.
       *
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that whants the
       *                         reply of this request packet.
       *    @param   sessionID   The sessionID string.
       *    @param   password    The sessionKey string.
       */
      SessionCleanUpRequestPacket( uint16 packetID,
                                   uint16 requestID,
                                   const char* sessionID,
                                   const char* sessionKey );
      
     
      /**
       *    Get the sessionID.
       *    @return  sessionID string.
       */
      const char* getSessionID() const;


      /**
       *    Get the sessionKey.
       *    @return  Get the sessionKey string.
       */
      const char* getSessionKey() const;
};


/**
 *    Reply to a SessionCleanUpRequestPacket. If the sessionID and 
 *    sessionKey in the request was correct the UIN of the user 
 *    returned in this packet.
 *
 */
class SessionCleanUpReplyPacket : public UserReplyPacket
{
   public:
      /**
       *    Create the reply packet from the corresponding request packet.
       *    Create a replypacket from a SessionCleanUpRequestPacket.
       *    @param   packet   The corresponding request packet.
       *    @param   status   The stringCode of the return message.
       */
      SessionCleanUpReplyPacket( const SessionCleanUpRequestPacket* packet, 
                                 uint32 status  );
};


/**
 *    Authenticate User  packet.
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & 2 bytes  & Login strlen \\
 *       +2                          & 2 bytes  & Password strlen \\
 *       +4                          & 1 bytes  & checkExpired \\
 *                                   & string   & logonID \\ 
 *                                   & string   & password \\
 *    \end{tabular}
 *
 */
class AuthUserRequestPacket : public UserRequestPacket
{
   public:
      /**
       *    Create a requestpacket to check the password of a user.
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that whants the
       *                         reply of this request packet.
       *    @param   logonID     The login string of the user.
       *    @param   password    The password string of the user.
       *    @param   checkExpired If to check if user is expired, default
       *                          false.
       */
      AuthUserRequestPacket( uint16 packetID,
                             uint16 requestID,
                             const char* logonID,
                             const char* password,
                             bool checkExpired = false );

      /**
       *    Get the username.
       *    @return  Logon of the user.
       */
      const char* getLogonID() const;

      /**
       *    Get the password.
       *    @return  Get the pasword of the user.
       */
      const char* getPassword() const;

      /**
       * Get if checkExpired.
       */
      bool getExpired() const;
};


/**
 *    Reply to an AuthUserRequestPacket. If the password and 
 *    logon in the request was correct the UIN of the user is 
 *    returned in this packet.
 *
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REPLY_HEADER_SIZE      & 4 bytes  & UIN
 *    \end{tabular}
 *
 */
class AuthUserReplyPacket : public UserReplyPacket
{
   public:
      /**
       *    Create the reply packet from the corresponding request packet.
       *    Create a replypacket from a check password request packet.
       *    @param   packet   The corresponding request packet.
       *    @param   UIN      The identification of the user that have the
       *                      given logon and password.
       */
      AuthUserReplyPacket( const AuthUserRequestPacket* packet, uint32 UIN );

      
      /**
       *    Get the identification of the user, 0 if login failed.
       *    @return  The UIN of the user, 0 if login failed, MAX_UINT32 -1
       *             if user is expired.
       */
      uint32 getUIN() const;
};


/**
 *    Create session for user packet.
 *    Contains only UIN
 *
 */
class CreateSessionRequestPacket : public UserRequestPacket
{
   public:
      /**
       *    Create a requestpacket to create a session for a user.
       *
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that whants the
       *                         reply of this request packet.
       *    @param   UIN         The users ID.
       */
      CreateSessionRequestPacket( uint16 packetID,
                                  uint16 requestID,
                                  uint32 UIN );
};


/**
 *    Reply to an CreateSessionRequestPacket. 
 *
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REPLY_HEADER_SIZE      & 2 bytes  & sessionID strlen \\
 *       +2                          & 2 bytes  & sessionKey strlen \\
 *                                   & string   & sessionID \\ 
 *                                   & string   & sessionKey \\
 *    \end{tabular}
 *
 */
class CreateSessionReplyPacket : public UserReplyPacket
{
   public:
      /**
       *    Create the reply packet from the corresponding request packet.
       *    Create a replypacket from a check password request packet.
       *    @param   packet   The corresponding request packet.
       *    @param   status   The status of the reply.
       *    @param   sessionID The new sessionID. 
       *             Is empty string if request falied.
       *    @param   sessionKey The new sessionKey.
       *             Is empty string if request falied.
       */
      CreateSessionReplyPacket( const CreateSessionRequestPacket* packet, 
                                StringTable::stringCode status,
                                const char* sessionID,
                                const char* sessionKey );


      /**
       * Get the SessionID of this login.
       * @return The sessionID associated with this login. Is empty 
       *         string if request falied.
       */
      const char* getSessionID() const;


      /**
       * Get the SessionKey of this login.
       * @return The sessionKey associated with this login. Is empty 
       *         string if request falied.
       */
      const char* getSessionKey() const;
};


/**
 *    Find a user from data about a user.
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & data     & elem \\
 *    \end{tabular}
 *
 */
class FindUserRequestPacket : public UserRequestPacket
{
   public:
      /**
       *    Create the request packet.
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that whants the
       *                         reply of this request packet.
       *    @param   elem        The element with the changes describing 
       *                         the user, if UserUser, UserCellular UIN, 
       *                         ID must be MAX_UINT32.
       */
      FindUserRequestPacket( uint16 packetID,
                             uint16 requestID,
                             UserElement* elem );
      
      /**
       *    Get the element with the changes describing the user.
       *    @return  The element with the changes describing the user.
       */
      UserElement* getElement() const;
};


/**
 *    Reply to a FindUserRequestPacket.
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REPLY_HEADER_SIZE      & 4 bytes  & nbrUsers \\
 *       for each user               & 4 bytes  & UIN \\
 *       for each user               & string   & logonID \\
 *    \end{tabular}
 *
 */
class FindUserReplyPacket : public UserReplyPacket
{
   public:
      /**
       *    Create a replypacket from the corresponding request packet.
       *    @param   p        The corresponding request packet.
       *    @param   nbrUsers The number of matching users.
       *    @param   UINs     The identification number of the users.
       *    @param   logonIDs The logonIDs of the users.
       */
      FindUserReplyPacket( const FindUserRequestPacket* p,
                           uint32 nbrUsers,
                           uint32 UINs[],
                           const vector<MC2String>* logonIDs );
      
      /**
       *    Get the number of Users fiting the fields.
       *    @return  The number of users that fits the given fields.
       */
      uint32 getNbrUsers() const;

      /**
       *    Get the user UINs.
       *    @return  The UINs of all the users. The returned array has
       *             getNbrUsers elements. 
       *             Q: Must the returned array
       *                be deleted by the caller or not? 
       *             A: Yes, IT IS A PACKET and it can't store it for you!
       */
      uint32* getUINs() const;

      /**
       * Get the users logonIDs.
       * 
       * @return An array with strings pointing into the packet. The
       *         array must be deleted by caller but not the strings.
       *         The strings are in the Pakcet.
       */
      const_char_p* getLogonIDs() const;
};


/**
 *    Get cellular phone model data. 
 * 
 */
class GetCellularPhoneModelDataRequestPacket : public RequestPacket 
{

   public:
   /**
    *    Create a packet to get the data about a celluar phone model.
    *
    *    @param   packetID    The ID of this packet.
    *    @param   requestID   The ID of the reuest that whants the
    *                         reply of this request packet.
    */
   GetCellularPhoneModelDataRequestPacket( uint16 packetId,
                                           uint16 requestID );     

   /**
    * Constructor
    * Not limited to single phonemodel
    */
   GetCellularPhoneModelDataRequestPacket( uint16 packetID,
                                           uint16 requestID,
                                           CellularPhoneModel* cellular);
   

   /**
    * Set how many different models
    */
   void setNbrModels(uint32 nbrModels);

   /**
    * Get how many different models
    */
   uint32 getNbrModels() const;

   /**
    * Returns a single CellularPhoneModel
    */
   CellularPhoneModel* getModel() const;

};


/**
 *    Reply to a GetCellularPhoneModelData with the phone models.
 *
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REPLY_HEADER_SIZE      & 4 bytes  & nbrModels \\
 *       for each model              & date     & CellularPhoneModel \\
 *    \end{tabular}
 *
 */
class GetCellularPhoneModelDataReplyPacket : public ReplyPacket
{
   public:
      /**
       *    Constructor, adds all models in list to packet.
       *    @param   p     The corresponding request packet.
       *    @param   list  The list of cellular models.
       */
      GetCellularPhoneModelDataReplyPacket( 
            const GetCellularPhoneModelDataRequestPacket* p, 
            CellularPhoneModels* list );
      
      /**
       *    Extract all phone models from the packet.
       *    @return  All the phone models in thos packet. 
       *    Q: Where should the returned array/object be deleted?
       *    A: By caller.
       */
      CellularPhoneModels* getAllPhoneModels() const;

      /**
       *    Get the number of phone models in the reply.
       *    @return  The number of phone models in this reply packet.
       */
      uint32 getNbrCellularPhoneModels() const;

   private:
      /** 
        *   Set the number of phone models in packet.
        *   @param   nbrModels   The new number of models in this packet.
        */
      void setNbrCellularPhoneModels( uint32 nbrModels );
};



/**
 *    Request packet for adding cellularphonemodel.
 *
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & data     & CellularPhoneModel \\
 *    \end{tabular}
 *
 */
class AddCellularPhoneModelRequestPacket : public RequestPacket
{
   public:
     /**
       *    Create a packet to add a new celluar phone model.
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that wants the
       *                         reply of this request packet.
       *    @param   model       The new model to add to the database.
       */
      AddCellularPhoneModelRequestPacket( uint16 packetId,
                                          uint16 requestID,
                                          CellularPhoneModel* model);

      /**
       *    Retrive the cellularmodel in this packet.
       *    @return  The new celluar model that should be added to the
       *             database.
       */
      CellularPhoneModel* getCellularPhoneModel() const;
}; 
 

/**
 *    Reply to AddCellularPhoneModellRequestPacket. Contains status of 
 *    the performed operation.
 * 
 */
class AddCellularPhoneModelReplyPacket : public ReplyPacket
{
   public:
      /**
       *    Create a replypacket from the corresponding request packet.
       *    @param   packet   The corresponding request packet.
       *    @param   status   The status of the performed operation.
       */
      AddCellularPhoneModelReplyPacket(
                      const AddCellularPhoneModelRequestPacket* packet,
                     uint32 status);
};


/**
 *    Request packet for changing password.
 *
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & data     & UserPasswordRequest \\
 *    \end{tabular}
 *
 */
class ChangeUserPasswordRequestPacket : public UserRequestPacket
{
   public:
      /**
       *    Create a request packet for changing the password of an user.
       *    @param   packetID    The ID of this packet.
       *    @param   requestID   The ID of the reuest that whants the
       *                         reply of this request packet.
       *    @param   passeord    The new password of the user.
       *    @param   UIN         The identification of the user.
       *    @param changerUIN The UIN for the user changing the user.
       */
      ChangeUserPasswordRequestPacket( uint16 packetId,
                                       uint16 requestID,
                                       const char* password,
                                       uint32 UIN,
                                       uint32 changerUIN );


      /**
       *    Retrive the userpassword.
       *    @return  The new password of the user.
       */
      const char* getUserPassword() const;



      /**
       * Get the changerUIN.
       */
      uint32 getChangerUIN() const;
}; 

/**
 *    Reply to ChangeUserPasswordRequestPacket. Contains status that tells
 *    if the password have been succesfully changed.
 * 
 */
class ChangeUserPasswordReplyPacket: public UserReplyPacket
{
   public:
      /**
       *    Create a replypacket from a change password request packet.
       *    @param   packet   The corresponding request packet.
       *    @param   status   The status of the performed operation.
       */
      ChangeUserPasswordReplyPacket( const ChangeUserPasswordRequestPacket* packet,
                                    uint32 status);
};



/**
 *    Request packet for changing cellularphonemodel.
 *
 *    \begin{tabular}{lll}
 *       Pos                         & Size     & Destription \\ \hline
 *       USER_REQUEST_HEADER_SIZE    & data     & CellularPhoneModel \\
 *    \end{tabular}
 *
 */
class ChangeCellularPhoneModelRequestPacket : public RequestPacket
{
  public:
  /**
    * Constructor.
    */
   ChangeCellularPhoneModelRequestPacket( uint16 packetId,
                                          uint16 requestID,
                                          CellularPhoneModel* model,
                                          const char* name);
   
   /**
    * Function for retrive the cellularmodell
    */
   CellularPhoneModel* getCellularPhoneModel() const;


   /**
    * Function for retrieving cellularphonemodel name
    */
   const char* getName() const;

}; 


/**
 *    Reply to ChangeCellularPhoneModellRequestPacket.
 *    Contains status if succesfully added.
 *
 */
class ChangeCellularPhoneModelReplyPacket : public ReplyPacket
{
  public:
   /**
    * Constructor.
    */
   ChangeCellularPhoneModelReplyPacket( const ChangeCellularPhoneModelRequestPacket*
                                        packet, uint32 status );
};


/**
 * Packets of this class are used to get informtion from the debit 
 * table in the UserModule. The rows that will be returned might be
 * specified by using date and UIN.
 *
 * After the normal RequestPacket header the request packet contains:
 * \begin{tabular}{lll}
 *    Pos                         & Size     & Destription \\ \hline
 *    USER_REQUEST_HEADER_SIZE    & 4 bytes  & startTime \\
 *    +4                          & 4 bytes  & endTime \\
 *    +8                          & 4 bytes  & startIndex \\
 *    +12                         & 4 bytes  & endIndex \\
 * \end{tabular}
 *
 */
class ListDebitRequestPacket : public UserRequestPacket {
   public:
      /**
       * Create a new packet with the values of the dates and
       * the UIN set.
       * 
       * @param UIN The user to get debit records for.
       * @param startTime The minium time for the records to return. 
       *                  0 for no limit.
       * @param endTime The maxium time  for the records to return. 
       *                MAX_UINT32 for no limit.
       * @param startIndex The index of the first debit record to return.
       * @param endIndex The index of the last debit record to return.
       */
      ListDebitRequestPacket( uint32 UIN,
                              uint32 startTime,
                              uint32 endTime,
                              uint32 startIndex = 0,
                              uint32 endIndex = 99 );


      /**
       * @name Get time.
       * Get the start and end time for the list to return.
       */
      //@{
         /**
          * Get the start time.
          * @return Start date for the intervall.
          */
         uint32 getStartTime() const;


         /**
          * Get the end time.
          * @return End date for the intervall.
          */
         uint32 getEndTime() const;
      //@}


      /**
       * @name Get the indexes.
       * Get the start and end indexes for the list to return.
       */
      //@{
         /**
          * Get the start index.
          * @return Start index.
          */
         uint32 getStartIndex() const;


         /**
          * Get the end index.
          * @return End index.
          */
         uint32 getEndIndex() const;
      //@}


   private:
      /**
       * The positions of the things with static locations in the packet.
       */
      enum positions {
         startTime_POS           = USER_REQUEST_HEADER_SIZE,
         endTime_POS             = startTime_POS + 4,
         startIndex_POS          = endTime_POS + 4,
         endIndex_POS            = startIndex_POS + 4,
         
         endStatic_POS           = endIndex_POS + 4
      };
};


/**
 *    The reply of the ListDebitRequestPacket. Contains the records in the
 *    specifid time intervall. This
 *    answer also contains information about if all records that matched
 *    the criteria fitted into this packet.
 *
 */
class ListDebitReplyPacket : public UserReplyPacket {
   public:
      /**
       * Create the reply packet from the request packet.
       */
      ListDebitReplyPacket( const ListDebitRequestPacket* p,
                            StringTable::stringCode status );


      /**
       * Add one DebitElement to this packet.
       *
       * @param el The DebitElement to add to this packet.
       */
      void addDebitElement( const DebitElement* el );


      /**
       * Get the number of DebitElement in this reply packet.
       * @return  The number of DebitElement in this packet.
       */
      uint32 getNbrDebitElements() const;


      /**
       * @name Get records.
       * These methods are to be used when getting the debit elements from
       * this packet.
       */
      //@{
         /**
          * Get the first record.
          *
          * Example of usage (@c p is a ListDebitReplyPacket):
          * @code 
            int pos = 0;
            DebitElement* el = p->getFirstElement( pos );
            while ( el != NULL ) {
               [Use el]
               delete el;
               el = p->getNextElement( pos );
            }
          @endcode
          *
          * @param pos Outparameter that is set to the position
          *            of the next DebitElement.
          * @return The first DebitElement, caller must delete it.
          */
         DebitElement* getFirstElement( int& pos ) const;


         /**
          * Get the next DebitElement.
          *
          * @param pos The position to read the DebitElement that will
          *            be returned. This is set to a new value after
          *            that DebitElement is read.
          * @return The next DebitElement, caller must delete it.
          */
         DebitElement* getNextElement( int& pos ) const;
      //@}


      /**
       * @name Get returned indexes.
       * These methods are for the indexes.
       */
      //@{
         /**
          * Get the start index.
          * @return Start index.
          */
         uint32 getStartIndex() const;


         /**
          * Get the end index.
          * @return End index.
          */
         uint32 getEndIndex() const;         


         /**
          * Get the total number of rows, might be larger than end index 
          * and NbrDebitElements.
          *
          * @return Total number of rows.
          */
         uint32 getTotalNbrDebits() const;
      //@}


      /**
       * @name Set returned indexes.
       * These methods are for the indexes.
       */
      //@{
         /**
          * Set the start index.
          * @param startIndex The start index.
          */
         void setStartIndex( uint32 startIndex );


         /**
          * Set the end index.
          * @param endIndex The end index.
          */
         void setEndIndex( uint32 endIndex );


         /**
          * Set the total number of rows, might be larger than end index 
          * and NbrDebitElements.
          *
          * @param nbr The total number of rows.
          */
         void setTotalNbrDebits( int32 nbr );
      //@}


   private:
      /**
       * The positions of the things with static locations in the packet.
       */
      enum positions {
         nbrElements_POS         = USER_REPLY_HEADER_SIZE,
         startIndex_POS          = nbrElements_POS + 4,
         endIndex_POS            = startIndex_POS + 4,
         totalNbr_POS            = endIndex_POS + 4,
         
         endStatic_POS           = totalNbr_POS + 4
      };


      /**
       * Increase the number of elements in this packet with one.
       */
      void incNbrElements();
};



//**********************************************************************
// UserNavDestination Packets
//**********************************************************************


/**
 * Packet for adding a new UserNavDestination.
 *
 */
class AddUserNavDestinationRequestPacket : public RequestPacket {
   public:
      /**
       * Constructs a new AddUserNavDestinationRequestPacket for adding
       * a new UserNavDestination.
       * @param nav The UserNavDestination to add.
       */
      AddUserNavDestinationRequestPacket( DBUserNavDestination* nav );


      /**
       * The UserNavDestination to be added.
       */
      DBUserNavDestination* getUserNavDestination() const;
};


/**
 * Reply to a addUserNavDestinationRequestPacket with status of 
 * the request.
 *
 */
class AddUserNavDestinationReplyPacket : public ReplyPacket {
   public:
      /**
       * Reply to a addUserNavDestinationRequestPacket with status of 
       * the request.
       * @param reqPack The RequestPacket that this ReplyPacket is a 
       *                reply to.
       * @param status The status of the request.
       */
      AddUserNavDestinationReplyPacket( const RequestPacket* reqPack,
                                        StringTable::stringCode status );
};


/**
 * Packet for deleting a UserNavDestination.
 *
 */
class DeleteUserNavDestinationRequestPacket : public RequestPacket {
   public:
      /**
       * Delete a UserNavDestination.
       * @param ID The UserNavDestinations to delete's id.
       */
      DeleteUserNavDestinationRequestPacket( uint32 ID );

      
      /**
       * The UserNavDestinations to delete's id.
       */
      uint32 getDeleteID() const;
};


/**
 * Reply to a DeleteUserNavDestinationRequestPacket with status of 
 * the request.
 *
 */
class DeleteUserNavDestinationReplyPacket : public ReplyPacket {
   public:
      /**
       * Reply to a DeleteUserNavDestinationRequestPacket with status of 
       * the request.
       * @param reqPack The RequestPacket that this ReplyPacket is a 
       *                reply to.
       * @param status The status of the request.
       */
      DeleteUserNavDestinationReplyPacket( 
         const RequestPacket* reqPack,
         StringTable::stringCode status );
};


/**
 * Packet for retreiving all UserNavDestinations with some limitations.
 *
 */
class GetUserNavDestinationRequestPacket : public RequestPacket {
   public:
     /**
      * Get all UserNavDestinations with some limitations.
      *
      * @param onlyUnSentNavDestinations Get only unsent 
      *                                  UserNavDestinations.
      * @param onlyNavigatorLastContact Get only UserNavDestinations for
      *                                 UserNavigators where
      *                                 last contact was successfull.
      * @param navID Get only UserNavDestinations for a specific 
      *              UserNavigator, 0 means any UserNavigator.
      * @param navAddress Get only UserNavDestinations with a specific
      *                   address, "" means any UserNavigator.
      */
      GetUserNavDestinationRequestPacket( bool onlyUnSentNavDestinations,
                                          bool onlyNavigatorLastContact,
                                          uint32 navID,
                                          const char* navAddress );

      
      /**
       * If get only unsent UserNavDestinations.
       */
      bool getOnlyUnSentNavDestinations() const;

      
      /**
       * If get only UserNavDestinations for
       * UserNavigators where
       * last contact was successfull.
       */
      bool getOnlyNavigatorLastContact() const;

      
      /**
       * The navID to serach for, 0 means any UserNavigator.
       * @return navID.
       */
      uint32 getNavID() const;

      
      /**
       * The navAddress to serach for, "" means any UserNavigator.
       * @return navAddress.
       */
      const char* getNavAddress() const;
};


/**
 * Reply to a GetUserNavDestinationRequestPacket with status of 
 * the request.
 *
 */
class GetUserNavDestinationReplyPacket : public ReplyPacket {
   public:
      /**
       * Reply to a GetUserNavDestinationRequestPacket with status of
       * the request.
       * @param reqPack The RequestPacket that this ReplyPacket is a 
       *                reply to.
       * @param status The status of the request.
       */
      GetUserNavDestinationReplyPacket( const RequestPacket* reqPack,
                                        StringTable::stringCode status );


      
      /**
       * Add a UserNavDestination to the packet.
       * @return False if not enough room for the UserNavDestination.
       */
      bool addUserNavDestination( DBUserNavDestination* nav );


      /**
       * The number of UserNavDestinations in the packet.
       */
      uint32 getNbrUserNavDestination() const;


      /**
       * The initial position to send into getUserNavDestination.
       */
      int getFirstUserNavDestinationPos() const;

      
      /**
       * Extract a UserNavDestinations at pos. Get initial pos from
       * getFirstUserNavDestinationPos.
       * @param pos The position in the packet, updated to be after
       * the UserNavDestinations.
       */
      DBUserNavDestination* getUserNavDestination( int& pos ) const;

      
      /**
       * The maximum size of a DBUserNavDestination.
       */
      static const uint32 MAX_DBUSERNAVIGATION_SIZE;

   private:
      /**
       * Set the number of UserNavDestinations in the packet.
       */
      void setNbrUserNavDestination( uint32 nbr );
};


/**
 * Packet for changing a UserNavDestination.
 *
 */
class ChangeUserNavDestinationRequestPacket : public RequestPacket {
   public:
      /**
       * Constructs a new ChangeUserNavDestinationRequestPacket for 
       * changing a UserNavDestination.
       * @param nav The UserNavDestination to change.
       * @param navID The UserNavDestination to change's ID.
       */
      ChangeUserNavDestinationRequestPacket( DBUserNavDestination* nav,
                                             uint32 navID );

      
      /**
       * The UserNavDestination to be changed.
       */
      DBUserNavDestination* getUserNavDestination() const;


      /**
       * The navID of the UserNavDestination to be changed.
       */
      uint32 getNavID() const;
};


/**
 * Reply to a ChangeUserNavDestinationReplyPacket with status of 
 * the request.
 *
 */
class ChangeUserNavDestinationReplyPacket : public ReplyPacket {
   public:
      /**
       * Reply to a ChangeUserNavDestinationRequestPacket with status of 
       * the request.
       * @param reqPack The RequestPacket that this ReplyPacket is a 
       *                reply to.
       * @param status The status of the request.
       */
      ChangeUserNavDestinationReplyPacket( 
         const RequestPacket* reqPack,
         StringTable::stringCode status );
};


#endif // USERPACKET_H
