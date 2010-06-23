/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERENUMS_H
#define USERENUMS_H

#include "config.h"
#include "WFSubscriptionConstants.h"
#include "LangTypes.h"
#include "MC2String.h"

class Packet;  //Forward decl.

/**
 * Class holding constans for user data.
 */
class UserEnums {
   public:
      /**
       * The user right level.
       */
      enum userRightLevel {
         UR_TRIAL                         = (0x1 <<31),
         UR_SILVER                        = (0x1 <<30),
         UR_GOLD                          = (0x1 <<29),
         UR_DEMO                          = (0x1 <<28),
         UR_IRON                          = (0x1 <<27),
         UR_LITHIUM                       = (0x1 <<26),
         UR_CESIUM                        = (0x1 <<25),
         UR_WOLFRAM                       = (0x1 <<24),

         // For convenience a all levels mask.
         ALL_LEVEL_MASK                   = (UR_TRIAL|UR_SILVER|UR_GOLD|
                                             UR_IRON|UR_LITHIUM|UR_WOLFRAM),
         // For convenience a TrialSilverGold mask.
         TSG_MASK                         = (UR_TRIAL|UR_SILVER|UR_GOLD),
         // For convenience a SilverGold mask.
         SG_MASK                          = (UR_SILVER|UR_GOLD),
         // For convenience a TrialSilver mask.
         TS_MASK                          = (UR_TRIAL|UR_SILVER),
         // For convenience a TrialGold mask.
         TG_MASK                          = (UR_TRIAL|UR_GOLD),
         // For convenience a TrialLithium mask.
         TL_MASK                          = (UR_TRIAL|UR_LITHIUM),
         // For convenience a TrialWolfram mask.
         TW_MASK                          = (UR_TRIAL|UR_WOLFRAM),

         // No level
         UR_NO_LEVEL                      = 0,

         /// All flags
         ALL_FLAGS                        = (UR_DEMO),
         ALL_FLAGS_INV                    = ~(ALL_FLAGS),
      };


      /**
       *   The types of service rights. Remember to update the array
       *   of names if there is a need to display it to the user, e.g.
       *   for POI:s only available to some users.
       */
      enum userRightService {
         UR_WF                              = 0x1,
         UR_RESERVED_BIT_2                  = 0x2,
         UR_MYWAYFINDER                     = 0x4,
         UR_MAPDL_PREGEN                    = 0x8,
         UR_MAPDL_CUSTOM                    = 0x10,
         UR_XML                             = 0x20,
         UR_RESERVED_BIT_6                  = 0x40,
         UR_TRAFFIC                         = 0x80,
         UR_SPEEDCAM                        = 0x100,
         UR_RESERVED_BIT_9                  = 0x200,
         UR_RESERVED_BIT_10                 = 0x400,
         UR_RESERVED_BIT_11                 = 0x800,
         UR_FREE_TRAFFIC                    = 0x1000,
         UR_POSITIONING                     = 0x2000,
         UR_RESERVED_BIT_14                 = 0x4000,
         UR_RESERVED_BIT_15                 = 0x8000,
         UR_FLEET                           = 0x10000,
         UR_VERSION_LOCK                    = 0x20000,
         UR_RESERVED_BIT_18                 = 0x40000,
         UR_RESERVED_BIT_19                 = 0x80000,
         UR_RESERVED_BIT_20                 = 0x100000,
         UR_RESERVED_BIT_21                 = 0x200000,
         UR_RESERVED_BIT_22                 = 0x400000,
         UR_WF_TRAFFIC_INFO_TEST            = 0x800000,
         UR_RESERVED_BIT_24                 = 0x1000000,

         UR_RESERVED_BIT_25                 = 0x2000000,
         UR_RESERVED_BIT_26                 = 0x4000000,
         UR_RESERVED_BIT_27                 = 0x8000000,
         UR_RESERVED_BIT_28                 = 0x10000000,
         UR_RESERVED_BIT_29                 = 0x20000000,
         UR_RESERVED_BIT_30                 = 0x40000000,
         UR_RESERVED_BIT_31                 = 0x80000000,

         UR_USE_GPS                         = 0x5,
         UR_ROUTE                           = 0x6,
         UR_SEARCH                          = 0x7,
         UR_POI                             = 0x9,
         // Unused right here, 0xa
         UR_PRO_WEATHER                     = 0xb,

         /// If user may add rights that doesn't start with "SUP: "
         UR_ADD_NON_SUP_RIGHTS              = 0xc,
         // Unused rights here, 0xd-0xf 
         
         /// Speed Camera DataBase http://www.scdb.info/
         UR_SCDB_SPEEDCAM                   = 0x11,
         /// disable poi layer
         UR_DISABLE_POI_LAYER               = 0x12,

         /// App Store Navigation
         UR_APP_STORE_NAVIGATION            = 0x13,

         /// When adding a POI right don't forget to add to POISetProperties

         /// The traffic bits 
         // userRightService is no longer a bitfield use new method
         UR_ALL_TRAFFIC_MASK = (UR_FREE_TRAFFIC | UR_WF_TRAFFIC_INFO_TEST ),
      };


      /**
       * Checks if userRightService is a one bit only right.
       */
      static bool isBitRight( userRightService r );


      /**
       * Check if a userRightService is a traffic right.
       */
      static bool isTrafficRight( userRightService r );


      /// The type of right with both level and service.
      class URType : public pair<userRightLevel,userRightService> {
         public:
            /**
             * Constructor.
             *
             * @param level The userRightLevel.
             * @param service The userRightService.
             */
            URType( userRightLevel level, userRightService service )
               : pair<userRightLevel,userRightService>( level, service ) {}


            /**
             * Empty Constructor.
             */
            URType() 
               : pair<userRightLevel,userRightService>( 
                  userRightLevel(0), userRightService(0) ) {}

            /**
             *   Returns true if the URType is empty.
             */
            inline bool empty() const {
               return first == 0 && second == 0;
            }

            /**
             *   Returns true if the URType is not empty.
             */
            inline operator const void*() const {
               return empty() ? NULL : this;
            }

            /**
             * Get the level.
             */
            userRightLevel level() const { return first; }


            /**
             * Set the level.
             */
            void setLevel( userRightLevel l ) { first = l; }


            /**
             * Get the service.
             */
            userRightService service() const { return second; }

            /**
             *   Prints on ostream.
             */
            friend ostream& operator<<( ostream& ost, const URType& ur ) {
               return ost << MC2HEX(ur.getAsInt());
            }

            /**
             *   Put something in the string for php-page use.
             */
            MC2String& phpPrint( MC2String& tmp ) const;

            /**
             * Set the service.
             */
            void setService( userRightService s ) { second = s; }

            /**
             *    Returns a mask that matches all types.
             */
            static URType getAllMask() { return URType(MAX_UINT64); }
         
            /**
             * Get the level as WFST, MAX_BYTE if no WFST.
             */
            WFSubscriptionConstants::subscriptionsTypes getLevelAsWFST()
               const;

            /**
             *   Returns true if the service matches
             *   as well as at least one bit in the level.
             */
            bool levelAndServiceMatch( const URType& other ) const {
               bool serviceMath = false;
               bool thisOneBit = isBitRight( service() );
               bool otherOneBit = isBitRight( other.service() );
               if ( thisOneBit ) {
                  if ( otherOneBit ) {
                     serviceMath = (service() & other.service());
                  }
               } else if ( !otherOneBit ) {
                  serviceMath = (service() == other.service());
               }
               return ( ( level() & other.level() ) || 
                        ((level()&ALL_FLAGS_INV) == UR_NO_LEVEL && 
                         (other.level()&ALL_FLAGS_INV) ==
                         UR_NO_LEVEL ) ) && serviceMath;
            }


            /**
             *   Returns true if at least one bit in the mask matches
             *   as well as at least one bit in the level.
             */
            bool levelAndServiceMatchMask( const URType& mask ) const {
               bool serviceMath = false;
               bool thisOneBit = isBitRight( service() );
               if ( thisOneBit ) {
                  serviceMath = (service() & mask.service());
               }
               return ( ( (level()&ALL_FLAGS_INV) & 
                          (mask.level()&ALL_FLAGS_INV) ) || 
                        ((level()&ALL_FLAGS_INV) == UR_NO_LEVEL && 
                         (mask.level()&ALL_FLAGS_INV) ==
                         UR_NO_LEVEL ) ) && serviceMath;
            }
            

            /**
             *   Bitwise operator &=
             */
            URType& operator &= ( const URType& other ) {
               first  = userRightLevel( level() & other.level() );
               second = userRightService( service() & other.service() );
               return *this;
            }
            
            /**
             *   Bitwise operator |=
             */
            URType& operator|=( const userRightService& inservice ) {
               second = userRightService( second | inservice );
               return *this;
            }
            
            /**
             *   Bitwise operator |=
             */
            URType& operator|=( const userRightLevel& inlevel ) {
               first = userRightLevel( first | inlevel );
               return *this;
            }

            /**
             *   Bitwise operator |=
             */
            URType& operator|=( const URType& other ) {
               *this |= other.first;
               *this |= other.second;
               return *this;
            }

            /**
             *    Bitwise operator ~
             */
            URType operator~() const {
               return URType ( userRightLevel(~uint32(first)),
                               userRightService(~(second)) );
            }

            /**
             *    Loads the URType from the packet.
             *    @param p   Packet to load from.
             *    @param pos Position to start at. Will be updated.
             */
            bool load(const Packet* p, int& pos);
            
            /**
             *    Saves the URType to the packet.
             *    @param p Packet to save the collection into.
             *    @param pos Position to start at. Will be updated.
             */
            bool save(Packet* p, int &pos) const;
            
            /**
             *    Returns the number of bytes that the URType
             *    will use in the packet.
             */
            int getSizeInPacket() const;
           
            
        private:
            /**
             * Bitwise operator &.
             * Private to avoid use, use levelAndServiceMatch or
             * levelAndServiceMatchMask.
             */
            URType operator &( const URType& other ) const {
               return URType( *this ) &= other;
            }
            
            /// Look out in the UserProcessor. It creates from uint64.
            friend class UserProcessor;
            /// Look out in the UserData. It converts to uint64.
            friend class UserRight;
            /// Look out in the XMLParserThread. It converts from admin ui.
            friend class XMLParserThread;            
            
            /**
             * Constructor from uint64.
             *
             * @param type The type as uint64.
             */
            URType( uint64 type )
               : pair<userRightLevel,userRightService>( 
                  userRightLevel(type >> 32), 
                  userRightService(type&MAX_UINT32) ) {}
            
            /**
             * Get as uint64.
             */
            uint64 getAsInt() const { 
               return (uint64(level())<<32 | service()); }

      };

      /**
       *    Returns a name string to use with items
       *    that have a certain user right bit set.
       *    Returns empty string for UR:s without names
       *    the copyright for the first one with a name if
       *    more than one bit is set.
       */
      static MC2String getName( const UserEnums::URType& type,
                                LangTypes::language_t lang );

      /// The default URType mask
      static const URType defaultMask;

      /// All URTypes set
      static const URType allMask;

      /// Static empty URType
      static const URType emptyMask;
      
      /**
       * Turn an WFST into level.
       */
      static userRightLevel wfstAsLevel( 
         WFSubscriptionConstants::subscriptionsTypes wfst );

   
   private:
      /**
       * Private constructor.
       */
      UserEnums();
};


#endif // USERENUMS_H

