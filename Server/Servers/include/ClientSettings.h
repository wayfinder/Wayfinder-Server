/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CLIENTSETTINGS_H
#define CLIENTSETTINGS_H

#include "config.h"
#include "StringUtility.h"
#include "MC2String.h"
#include "WFSubscriptionConstants.h"
#include "StringTable.h"
#include "ImageTable.h"
#include "CategoryRegionID.h"
#include "NotCopyable.h"

#include <set>
#include <map>

class CategoriesData;
class ParserThreadGroup;
class ClientVersion;


/**
 * Class holding a specific client type's settings.
 *
 */
class ClientSetting: private NotCopyable {
public: 

      /**
       * Constructor.
       *
       * @param clientType The clientType, is copied.
       * @param clientTypeOptions The clientTypeOptions, is copied.
       */
      ClientSetting( ParserThreadGroup* group,
                     const char* clientType, 
                     const char* clientTypeOptions,
                     uint32 matrixOfDoomLevel,
                     bool notAutoUpgradeToSilver,
                     uint32 silverRegionID,
                     int32 silverTimeYear,
                     int32 silverTimeMonth,
                     int32 silverTimeDay,
                     uint32 explicitSilverTime,
                     uint32 blockDate,
                     bool notCreateWLUser,
                     WFSubscriptionConstants::subscriptionsTypes
                     createLevel,
                     uint32 createRegionID,
                     int32 createRegionTimeYear,
                     int32 createRegionTimeMonth,
                     int32 createRegionTimeDay,
                     uint32 explicitCreateRegionTime,
                     int32 createTransactionDays,
                     const char* phoneModel,
                     const char* imageExtension,
                     bool noLatestNews,
                     const char* callCenterList,
                     const char* brand,
                     const char* categoryPrefix,
                     ImageTable::ImageSet imageSet,
                     const char* version,
                     const char* lockedVersion,
                     /* TransactionType none,days,transactions */
                     /* May downgrade to X from Y */
                     const char* serverListName,
                     const MC2String& extraRights,
                     const MC2String& upgradeId );


      /**
       * Destructor.
       */
      ~ClientSetting();


      /**
       * Get the clientType.
       * 
       * @return The clientType.
       */
      const char* getClientType() const;


      /**
       * Get the clientTypeOptions.
       * 
       * @return The clientTypeOptions.
       */
      const char* getClientTypeOptions() const;


      /**
       * Get the Matrix Of Doom level. Current valid values are 1 and 2.
       *
       * @return The Matrix Of Doom level.
       */
      uint32 getMatrixOfDoomLevel() const;


      /**
       * Get the notAutoUpgradeToSilver.
       *
       * @return The notAutoUpgradeToSilver.
       */
      bool getNotAutoUpgradeToSilver() const;


      /**
       * Get the regionID to set for Silver upgrade.
       *
       * @return The regionID to set on creation.
       */
      uint32 getSilverRegionID() const;


      /**
       * Get Silver year upgrade time.
       * 
       * @return The Silver year upgrade time.
       */
      int32 getSilverTimeYear() const;


      /**
       * Get Silver month upgrade time.
       * 
       * @return The Silver month upgrade time.
       */
      int32 getSilverTimeMonth() const;


      /**
       * Get Silver day upgrade time.
       * 
       * @return The Silver day upgrade time.
       */
      int32 getSilverTimeDay() const;


      /**
       * Get Silver upgrade explicit time.
       * 
       * @return The Silver upgrade explicit time.
       */
      uint32 getExplicitSilverTime() const;


      /**
       * The date to start blocking client.
       *
       * @return The date to start blocking client.
       */
      uint32 getBlockDate() const;


      /**
       * If not create WL-user even if the Matrix Of Doom says so.
       *
       * @return If not to create WL-user.
       */
      bool getNotCreateWLUser() const;


      /**
       * Get the wfsubscriptionlevel to set on creation.
       *
       * @return The wfsubscriptionlevel to set on creation.
       */
      WFSubscriptionConstants::subscriptionsTypes getCreateLevel() const;


      /**
       * Get the regionID to set on creation.
       *
       * @return The regionID to set on creation.
       */
      uint32 getCreateRegionID() const;


      /**
       * Get the create WL-user year time.
       *
       * @return The create WL-user year time.
       */
      int32 getCreateRegionTimeYear() const;


      /**
       * Get the create WL-user month time.
       *
       * @return The create WL-user month time.
       */
      int32 getCreateRegionTimeMonth() const;


      /**
       * Get the create WL-user day time.
       *
       * @return The create WL-user day time.
       */
      int32 getCreateRegionTimeDay() const;


      /**
       * Get the explicit create WL-user time.
       *
       * @return The explicit create WL-user time.
       */
      uint32 getExplicitCreateRegionTime() const;


      /**
       * Get the number of transactions days for create.
       *
       * @return The number of transactions days for create.
       */
      int32 getCreateTransactionDays() const;


      /**
       * Get the phoneModel.
       *
       * @return The phoneModel.
       */
      const char* getPhoneModel() const;


      /**
       * Get the imageExtension.
       *
       * @return The imageExtension.
       */
      const char* getImageExtension() const;


      /**
       * Get the noLatestNews.
       *
       * @return The noLatestNews.
       */
      bool getNoLatestNews() const;


      /**
       * Get the callCenterList.
       *
       * @return The callCenterList.
       */
      const char* getCallCenterList() const;


      /**
       * Get the CallCenter list crc.
       */
      uint32 getCallCenterListCRC() const;


      /**
       * Get the brand.
       *
       * @return The brand.
       */
      const char* getBrand() const;

      /**
       * Get the category prefix.
       */
      const char* getCategoryPrefix() const;


      /**
       *    @name Get categories for sending to client.
       *    @memo Methods for handling categories.
       *    @doc  Methods for handling categories.
       */
      //@{
         /**
          * Get the categories data for a language.
          *
          * @param language The language to get categories for.
          * @param usesLatin1 If client uses utf-8 or iso-8859-1.
          * @param regionID The geographical region to get categories for.
          * @return CategoriesData, NULL if no categories.
          */
         auto_ptr<CategoriesData> getCategories( 
            StringTable::languageCode language, 
            bool usesLatin1,
            CategoryRegionID regionID ) const;


         /**
          * Get the crc for the categories in language language.
          *
          * @param language The language to get categories for.
          * @param usesLatin1 If client uses utf-8 or iso-8859-1.
          * @param regionID The geographical region to get categories for.
          * @param categoriesCrc Set to the crc of the categories for 
          *                      language.
          * @return True if have categories false if not.
          */
         bool getCategoriesCRC( StringTable::languageCode language, 
                                bool usesLatin1,
                                CategoryRegionID regionID,
                                uint32& categoriesCrc ) const;
      //@}

      /**
       * Get the version string.
       */
      const char* getVersion() const;

      /**
       * Get if upgrade should be forced
       */
      bool getForceUpgrade() const;

     /**
      * Get the upgrade id string
      */
     const MC2String& getUpgradeId() const;

      /**
       * Get the version as object.
       */
      const ClientVersion* getVersionObj() const;

      /**
       * Get the locked version string.
       *
       * @param majorV The major version of the client.
       */
      const char* getLockedVersion( uint32 majorV, uint32 minorV,
                                    uint32 miniV ) const;

      /**
       * Get the serverlist name.
       */
      const MC2String& getServerListName() const;

   const MC2String& getExtraRights() const;

   uint32 getVersionLock() const;
   void setVersionLock( uint32 versionLock );
   friend ostream& operator<<(ostream& stream, const ClientSetting& setting);

      /**
       * If client is using WFID startup.
       */
      bool getWFID() const;

      /**
       * If client is using WFID startup.
       */
      void setWFID( bool val );

      /**
       * If client is using services but not WFID.
       */
      bool getNotWFIDButUsesServices() const;

      /**
       * If client is using services but not WFID.
       */
      void setNotWFIDButUsesServices( bool val );

      /**
       * Get the product.
       */
      const MC2String& getProduct() const;

      /**
       * Set the product.
       */
      void setProduct( const MC2String& product );
      /// @return draw version that should be used
      inline uint32 getDrawVersion() const;
      /// Set draw version.
      /// @param drawVersion the draw setting version to be used
      inline void setDrawVersion( uint32 drawVersion );

   /// Gets the server prefix
   uint32 getServerPrefix() const;

   /// Sets the server prefix
   void setServerPrefix( uint32 serverPrefix );

      /**
       * Get the special layers for this client type.
       *
       * @param layers Set if method return true.
       * @return True if specific layers should be used, false if not.
       */
      bool getSpecificTMapLayers( uint32& layers ) const;

      /**
       * If lifetime GOLD rights is prohibited for client type.
       */
      bool noGoldLifeTime() const;

      /**
       * If lifetime GOLD rights is prohibited for client type.
       */
      void setNoGoldLifeTime( bool val );

      /**
       * Check if the client uses rights.
       *
       * @return True if the client uses rigths, false otherwise.
       */
      bool usesRights() const;
      
      /**
       * Check if the client is an app store billing client.
       *
       * @return True if the client is an iPhone, false otherwise.
       */
      bool isAppStoreClient() const;
      
      /// @return which image set to use.
      ImageTable::ImageSet getImageSet() const;


private:

   void parseLockedVersion(const MC2String& lvs);


      /**
       * The ParserThreadGroup.
       */
      ParserThreadGroup* m_group;


      /**
       * The clientType.
       */
      char* m_clientType;


      /**
       * The clientTypeOptions
       */
      char* m_clientTypeOptions;


      /**
       * The matrixOfDoomLevel.
       */
      uint32 m_matrixOfDoomLevel;


      /**
       * The notAutoUpgradeToSilver.
       */
      bool m_notAutoUpgradeToSilver;

      /**
       * The silverRegionID.
       */
      uint32 m_silverRegionID;


      /**
       * The silverTimeYear.
       */
      int32 m_silverTimeYear;


      /**
       * The silverTimeMonth.
       */
      int32 m_silverTimeMonth;


      /**
       * The silverTimeDay.
       */
      int32 m_silverTimeDay;


      /**
       * The explicitSilverTime.
       */
      uint32 m_explicitSilverTime;


      /**
       * The block date.
       */
      uint32 m_blockDate;


      /**
       * The notCreateWLUser.
       */
      bool m_notCreateWLUser;


      /**
       * The createLevel.
       */
      WFSubscriptionConstants::subscriptionsTypes m_createLevel;


      /**
       * The createRegionID.
       */
      uint32 m_createRegionID;


      /**
       * The createRegionTimeYear.
       */
      int32 m_createRegionTimeYear;


      /**
       * The createRegionTimeMonth.
       */
      int32 m_createRegionTimeMonth;


      /**
       * The createRegionTimeDay.
       */
      int32 m_createRegionTimeDay;


      /**
       * The explicit createRegionTime.
       */
      uint32 m_explicitCreateRegionTime;


      /**
       * The number of transactions days for create.
       */
      int32 m_createTransactionDays;


      /**
       * The phoneModel.
       */
      char* m_phoneModel;


      /**
       * The imageExtension.
       */
      char* m_imageExtension;


      /**
       * The noLatestNews.
       */
      bool m_noLatestNews;


      /**
       * The callCenterList.
       */
      char* m_callCenterList;


      /**
       * The callCenterListCRC.
       */
      uint32 m_callCenterListCRC;


      /**
       * The brand.
       */
      MC2String m_brand;


      /**
       * The category prefix string.
       */
      MC2String m_categoryPrefix;

      /**
       * The image set.
       */
      ImageTable::ImageSet m_imageSet;

      /**
       * The version string.
       */
      MC2String m_version;

      /**
       * If force upgrade.
       */
      bool m_forceUpgrade;

      /**
       * The version as object.
       */
      ClientVersion* m_verObj;

      class LockedV {
         public:
         LockedV( uint32 majV, uint32 mioV, uint32 minV ) 
            : majorV( majV ), minorV( mioV ), miniV( minV ) {}


         bool operator < ( const LockedV& o ) const {
            if ( majorV != o.majorV ) {
               return majorV < o.majorV;
            } else if ( minorV != o.minorV ) {
               return minorV < o.minorV;
            } else {
               return miniV < o.miniV;
            }
         }


         uint32 majorV;
         uint32 minorV;
         uint32 miniV;
      };


      typedef map< LockedV, MC2String > lockedVersionMap;
      

      /**
       * The locked version strings map.
       */
      lockedVersionMap m_lockedVersions;


      typedef map< pair<StringTable::languageCode, bool>, 
         const CategoriesData* > categoriesMap;


      /**
       * The name of the serverlist this client should use.
       */
      MC2String m_serverListName;

   /**
    * The VersionLock value required to use this client. 
    */
   uint32 m_versionLock;

      /**
       * ClientSettingStorage is a friend.
       */
      friend class ClientSettingStorage;


   MC2String m_extraRights;
   
      /**
       * If WFID.
       */
      bool m_wfid;

      /**
       * If not using WFID but services.
       */
      bool m_notWFIDButUsesServices;

      /**
       * The product.
       */
      MC2String m_product;

      /// draw setting version in server tile map format desc
      uint32 m_drawVersion;

      /// The server prefix used by this client type in vector maps
      uint32 m_serverPrefix;

      /**
       * If no Gold life time rights.
       */
      bool m_noGoldLifeTime;

      /**
       * Platform market identifier ( to identify new client 
       * version for download )
       */
      MC2String m_upgradeId;
};


// =======================================================================
//                                     Implementation of inlined methods =

inline 
uint32 ClientSetting::getDrawVersion() const {
   return m_drawVersion;
}

inline
void ClientSetting::setDrawVersion( uint32 drawVersion ) {
   m_drawVersion = drawVersion;
}

inline const char* 
ClientSetting::getClientType() const {
   return m_clientType;
}


inline const char* 
ClientSetting::getClientTypeOptions() const {
   return m_clientTypeOptions;
}


inline uint32 
ClientSetting::getMatrixOfDoomLevel() const {
   return m_matrixOfDoomLevel;
}


inline bool 
ClientSetting::getNotAutoUpgradeToSilver() const {
   return m_notAutoUpgradeToSilver;
}


inline uint32 
ClientSetting::getSilverRegionID() const {
   return m_silverRegionID;
}


inline int32 
ClientSetting::getSilverTimeYear() const {
   return m_silverTimeYear;
}


inline int32 
ClientSetting::getSilverTimeMonth() const {
   return m_silverTimeMonth;
}


inline int32 
ClientSetting::getSilverTimeDay() const {
   return m_silverTimeDay;
}


inline uint32 
ClientSetting::getExplicitSilverTime() const {
   return m_explicitSilverTime;
}


inline uint32
ClientSetting::getBlockDate() const {
   return m_blockDate;
}


inline bool 
ClientSetting::getNotCreateWLUser() const {
   return m_notCreateWLUser;
}


inline uint32 
ClientSetting::getCreateRegionID() const {
   return m_createRegionID;
}


inline WFSubscriptionConstants::subscriptionsTypes
ClientSetting::getCreateLevel() const {
   return m_createLevel;
}


inline int32 
ClientSetting::getCreateRegionTimeYear() const {
   return m_createRegionTimeYear;
}


inline int32 
ClientSetting::getCreateRegionTimeMonth() const {
   return m_createRegionTimeMonth;
}


inline int32 
ClientSetting::getCreateRegionTimeDay() const {
   return m_createRegionTimeDay;
}


inline uint32 
ClientSetting::getExplicitCreateRegionTime() const {
   return m_explicitCreateRegionTime;
}


inline int32 
ClientSetting::getCreateTransactionDays() const {
   return m_createTransactionDays;
}


inline const char* 
ClientSetting::getPhoneModel() const {
   return m_phoneModel;
}


inline const char* 
ClientSetting::getImageExtension() const {
   return m_imageExtension;
}


inline bool 
ClientSetting::getNoLatestNews() const {
   return m_noLatestNews;
}


inline const char* 
ClientSetting::getCallCenterList() const {
   return m_callCenterList;
}


inline uint32
ClientSetting::getCallCenterListCRC() const {
   return m_callCenterListCRC;
}


inline const char* 
ClientSetting::getBrand() const {
   return m_brand.c_str();
}


inline const char*
ClientSetting::getCategoryPrefix() const {
   return m_categoryPrefix.c_str();
}

inline const char*
ClientSetting::getVersion() const {
   return m_version.c_str();
}

inline bool
ClientSetting::getForceUpgrade() const {
   return m_forceUpgrade;
}

inline const MC2String&
ClientSetting::getUpgradeId() const {
   return m_upgradeId;
}

inline const ClientVersion*
ClientSetting::getVersionObj() const {
   return m_verObj;
}

inline const MC2String&
ClientSetting::getServerListName() const {
   return m_serverListName;
}

inline const MC2String&
ClientSetting::getExtraRights() const {
   return m_extraRights;
}

inline bool
ClientSetting::getWFID() const {
   return m_wfid;
}

inline void
ClientSetting::setWFID( bool val ) {
   m_wfid = val;
}

inline bool
ClientSetting::getNotWFIDButUsesServices() const {
   return m_notWFIDButUsesServices;
}

inline void
ClientSetting::setNotWFIDButUsesServices( bool val ) {
   m_notWFIDButUsesServices = val;
}

inline const MC2String&
ClientSetting::getProduct() const {
   return m_product;
}

inline void
ClientSetting::setProduct( const MC2String& product ) {
   m_product = product;
}


inline uint32 ClientSetting::getVersionLock() const
{
   return m_versionLock;
}

inline void ClientSetting::setVersionLock( uint32 versionLock )
{
   m_versionLock = versionLock;
}

inline bool
ClientSetting::noGoldLifeTime() const {
   return m_noGoldLifeTime;
}

inline void
ClientSetting::setNoGoldLifeTime( bool val ) {
   m_noGoldLifeTime = val;
}


/**
 * Class for comparing two ClientSetting.
 *
 */
class ClientSettingLessComp {
   public:
      /**
       * Less than compare two ClientSetting.
       */
      bool operator() ( const ClientSetting* a,
                        const ClientSetting* b ) const
      {
         return StringUtility::strcmp( a->getClientType(), 
                                       b->getClientType() ) < 0;
      }


      /**
       * Less than compare two ClientSetting.
       */
      bool operator() ( const ClientSetting& a,
                        const ClientSetting& b ) const
      {
         return StringUtility::strcmp( a.getClientType(), 
                                       b.getClientType() ) < 0;
      }
};

/**
 * Class for holding a number of settings for clients.
 * 
 */
class ClientSettingStorage {
   public:
      /**
       * Constructor.
       */
      ClientSettingStorage( ParserThreadGroup* group );

      /**
       * Destructor.
       */
      virtual ~ClientSettingStorage();


      /**
       * Get the program and resource versions for a specific client type.
       * 
       * @param clientType The client type.
       * @param clientOptions The client type options.
       * @return Returns default if no match.
       */
      const ClientSetting* getSetting( 
         const char* clientType, const char* clientOptions ) const;
      

      /**
       * Add a ClientSetting.
       *
       * @param ver The ClientSetting to add, is owned by this class 
       *            now.
       */
      void addSetting( ClientSetting* ver );


      /**
       * Empties the storage.
       */
      void clear();


      /**
       * Parses and creates ClientSettings.
       *
       * @return True if all is ok, false if not.
       */
      bool parseSettingsFile( const char* data );

      /**
       * A set of ClientSettings.
       */
      typedef multiset< ClientSetting*, ClientSettingLessComp >
         client_setting_set;


      /**
       * Returns the set of clientsettings.
       */
      const client_setting_set& getClientSettings() const;

   private:
      /**
       * Checks if str is a boolean and converts if so.
       *
       * @param str The string to check.
       * @param value Set to the boolean value of str, not changed 
       *              if str isn't a boolean.
       * @return True if str is a boolean and false if not.
       */
      static bool checkBoolean( const char* str, bool& value );


      /**
       * The string value of str, removes " at start and end of string.
       *
       * @param str The string to fixup.
       * @param res The string to output.
       * @return True if string is ok, false if not.
       */
      static bool checkString( const char* str, MC2String& res );


      /**
       * The ParserThreadGroup.
       */
      ParserThreadGroup* m_group;


      /**
       * The client Settings.
       */
      client_setting_set m_clientSettings;


      /**
       * The ClientSetting used in searches.
       */
      ClientSetting m_searchClientSettings;


      /**
       * The default client Settings.
       */
      ClientSetting m_defaltClientSettings;
};

ostream& operator<<(ostream& stream, const ClientSettingStorage& store);



// =======================================================================
//                                     Implementation of inlined methods =


inline const ClientSettingStorage::client_setting_set& 
ClientSettingStorage::getClientSettings() const {
   return m_clientSettings;
}



#endif // CLIENTSETTINGS_H

