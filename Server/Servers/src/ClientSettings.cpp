/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ClientSettings.h"
#include <time.h>
#include <iostream>
#include <iomanip>
#include <iterator>
#include "MC2String.h"
#include "MC2CRC32.h"
#include "LangTypes.h"
#include "ItemTypes.h"
#include "ParserThreadGroup.h"
#include "CategoriesData.h"
#include "ClientVersion.h"
#include "ScopedArray.h"
#include "StringUtility.h"
#include "STLUtility.h"
#include "BitUtility.h"
#include "TileMapTypes.h"
#include "VersionLockParser.h"
#include "ClientSettingsWFIDReader.h"
#include "ClientSettingsProductReader.h"
#include "UserData.h"
#include "ParserActivationHandler.h"
#include "boost/lexical_cast.hpp"
#include "ServerTileMapFormatDesc.h"

namespace {
void printTime( time_t ttime ) 
{
   struct tm result;
   struct tm* tm = gmtime_r( &ttime, &result );
   mc2dbg8  << setfill('0') << (tm->tm_year + 1900) 
            << "-" << setw(2) << (tm->tm_mon + 1) << "-"
            << setw(2) << (tm->tm_mday)
//                        << " "
//                        << setw(2) << tm->tm_hour << ":"
//                        << setw(2) << tm->tm_min << ":"
//                        << setw(2) << tm->tm_sec
            << ", ";   
}
}

ClientSetting::ClientSetting( ParserThreadGroup* group,
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
                              const char* serverListName,
                              const MC2String& extraRights,
                              const MC2String& upgradeId )
      : m_group( group ),
        m_clientType( StringUtility::newStrDup( clientType ) ),
        m_clientTypeOptions( StringUtility::newStrDup( 
                                clientTypeOptions ) ),
        m_matrixOfDoomLevel( matrixOfDoomLevel ),
        m_notAutoUpgradeToSilver( notAutoUpgradeToSilver ),
        m_silverRegionID( silverRegionID ),
        m_silverTimeYear( silverTimeYear ),
        m_silverTimeMonth( silverTimeMonth ),
        m_silverTimeDay( silverTimeDay ),
        m_explicitSilverTime( explicitSilverTime ),
        m_blockDate( blockDate ),
        m_notCreateWLUser( notCreateWLUser ),
        m_createLevel( createLevel ),
        m_createRegionID( createRegionID ),
        m_createRegionTimeYear( createRegionTimeYear ),
        m_createRegionTimeMonth( createRegionTimeMonth ),
        m_createRegionTimeDay( createRegionTimeDay ),
        m_explicitCreateRegionTime( explicitCreateRegionTime ),
        m_createTransactionDays( createTransactionDays ),
        m_phoneModel( StringUtility::newStrDup( phoneModel ) ),
        m_imageExtension( StringUtility::newStrDup( imageExtension ) ),
        m_noLatestNews( noLatestNews ),
        m_callCenterList( StringUtility::newStrDup( callCenterList ) ),
        m_callCenterListCRC( 
           callCenterList[ 0 ] == '\0' ? 0 : MC2CRC32::crc32( 
              (byte*)callCenterList, strlen( callCenterList ) ) ),
        m_brand( brand ), 
        m_categoryPrefix( categoryPrefix ),
        m_imageSet( imageSet ),
        m_version( version ),
        m_forceUpgrade( false ),
        m_verObj( NULL ),
        m_serverListName( serverListName ),
        m_versionLock( 0 ),
        m_extraRights( extraRights ),
        m_wfid( false ),
        m_notWFIDButUsesServices( false ),
        m_product( "" ),
        m_drawVersion( 0 ),
        m_serverPrefix( STMFDParams::DEFAULT_SERVER_PREFIX ),
        m_noGoldLifeTime( false ),
        m_upgradeId( upgradeId )
{
   // Check version
   if ( m_version[ 0 ] == '!' ) {
      m_forceUpgrade = true;
      m_version.erase( 0, 1 );
   }
   m_verObj = new ClientVersion( m_version );

   // Parse lockedVersion
   parseLockedVersion(lockedVersion);
}


ClientSetting::~ClientSetting() {
   delete [] m_clientType;
   delete [] m_clientTypeOptions;
   delete [] m_phoneModel;
   delete [] m_imageExtension;
   delete [] m_callCenterList;
   delete m_verObj;
}


auto_ptr<CategoriesData>
ClientSetting::getCategories( StringTable::languageCode language,
                              bool usesLatin1,
                              CategoryRegionID regionID ) const
{
   if ( m_group == NULL ||
        m_group->getCategories() == NULL ) {
      return auto_ptr<CategoriesData>();
   }

   const char* catprefix = "wfcat";
   if ( getCategoryPrefix()[ 0 ] != '\0' ) {
      catprefix = getCategoryPrefix();
   }

   LangType::language_t langType = 
      ItemTypes::getLanguageCodeAsLanguageType( language );

   auto_ptr<CategoriesData> categories( 
      m_group->getCategories()->makeCategory( catprefix,  
                                              getImageSet(),
                                              getClientType(), 
                                              langType, 
                                              usesLatin1,
                                              regionID ) );
   return categories;
}


bool 
ClientSetting::getCategoriesCRC( StringTable::languageCode language,
                                 bool usesLatin1,
                                 CategoryRegionID regionID,
                                 uint32& categoriesCrc ) const
{
   auto_ptr<CategoriesData> categories( getCategories( language,
                                                       usesLatin1,
                                                       regionID ) );
   
   if ( categories.get() == NULL ) {
      categoriesCrc = 0;
      return false;
   }

   categoriesCrc = categories->getCRC();
   return true;
}


bool
ClientSetting::getSpecificTMapLayers( uint32& layers ) const {
   bool isSpecial = false;

   // Power search uses only map layer nothing else, no pois.
   if ( StringUtility::regexp( ".*-ps$", getClientType() ) ) {
      layers = 0; // clear all layers
      BitUtility::setBit( layers, TileMapTypes::c_mapLayer, true );
      BitUtility::setBit( layers, TileMapTypes::c_routeLayer, true );
      isSpecial = true;
   }

   return isSpecial;
}

const char* 
ClientSetting::getLockedVersion( uint32 majorV, uint32 minorV,
                                    uint32 miniV ) const 
{
   LockedV v( majorV, minorV, miniV );
   lockedVersionMap::const_iterator findIt = m_lockedVersions.find( v );

   mc2dbg << "Found end " << (findIt == m_lockedVersions.end()) << endl;

#if 0
m_lockedVersions.lower_bound( 
      v );
   
   // v is Finds the first element whose key is not less than k.
   // we wants the prevoius one (4.64.1052 -> 4.50.0)
   if ( findIt != m_lockedVersions.begin() && 
        findIt != m_lockedVersions.end()) 
   {
      mc2dbg << "getLockedVersion found match " << findIt->first.majorV
             << endl;
      --findIt;
      if ( findIt->first.majorV != majorV ) {
         findIt = m_lockedVersions.end();
      }
   } else {
      findIt = m_lockedVersions.end();
   }
#endif

   if ( findIt != m_lockedVersions.end() ) {
      return findIt->second.c_str();
   } else {
      return "";
   }
}

void 
ClientSetting::parseLockedVersion(const MC2String& lvs) {
   MC2String::size_type pos = 0;
   do {
      MC2String::size_type findPos = lvs.find( ';', pos );
      if ( findPos != MC2String::npos || pos < lvs.size() ) {
         MC2String s =
            lvs.substr(pos, 
                       ( (findPos != MC2String::npos) ? (findPos - pos) : 
                         MC2String::npos) );
         uint32 majorV = 0;
         uint32 minorV = 0;
         uint32 miniV = 0;
         char restStr[ s.size() ];
         
         if ( sscanf( s.c_str(), "%u.%u.%u|%[^;]", 
                      &majorV, &minorV, &miniV, restStr ) == 4 ) 
         {
            // Check restStr 5.01.0:4.17.0:0
            uint32 d = 0;
            if ( sscanf( restStr, "%u.%u.%u:%u.%u.%u:%u", 
                         &d, &d, &d, &d, &d, &d, &d ) != 7 ) 
            {
               mc2log << fatal << "ClientSetting bad lockedVersion "
                      << " \"" << restStr << "\" in lockedVersion " 
                      << MC2CITE( lvs ) << " for "
                      << m_clientType << " " << m_clientTypeOptions
                      << endl;
               exit( 1 );
            }

            pair<lockedVersionMap::iterator, bool> res = 
               m_lockedVersions.insert( 
               make_pair( LockedV( majorV, minorV, miniV ), 
                          MC2String( restStr ) ) );
            if ( !res.second ) {
               mc2log << error << "ClientSetting Failed to insert "
                      << "locked version " 
                      << majorV << "." << minorV << "." << miniV << "|" 
                      << restStr << " for " << m_clientType << " " 
                      << m_clientTypeOptions << endl;
               exit( 1 );
            }
         } else {
            mc2log << fatal << "ClientSetting bad lockedVersion str \""
                   << s << "\" in lockedVersion " 
                   << MC2CITE( lvs ) << " for " << m_clientType
                   << " " << m_clientTypeOptions << endl;
            exit( 1 );
         }
      }

      if ( findPos != MC2String::npos ) {
         pos = findPos + 1;
      } else {
         pos = findPos;
      }

   } while ( pos != MC2String::npos && pos <= lvs.size() );
}

bool
ClientSetting::usesRights() const {
   // For now this only applies for App store clients.
   return m_brand == "AppStore";
}

bool
ClientSetting::isAppStoreClient() const {
   return StringUtility::regexp( ".*-apps$", getClientType() );
}

ImageTable::ImageSet ClientSetting::getImageSet() const {
   return m_imageSet;
}

uint32 ClientSetting::getServerPrefix() const {
   return m_serverPrefix;
}

void ClientSetting::setServerPrefix( uint32 serverPrefix ) {
   m_serverPrefix = serverPrefix;
}



// ----------------  ClientSettingStorage -----------------


ClientSettingStorage::ClientSettingStorage(
   ParserThreadGroup* group ) 
   : m_group( group ),    
     m_searchClientSettings( group, "","",2, false,1,0,0,0,0, MAX_INT32,
                             true, WFSubscriptionConstants::TRIAL, 
                             1,0,0,0,0,0, "6600", "gif", 
                             false, "", "", "", ImageTable::DEFAULT,"", "", 
                             "", "", "" ),
     m_defaltClientSettings( group, "default","", 2, false,1,0,6,0,0, 
                             2145916800,
                             true, WFSubscriptionConstants::TRIAL, 
                             2097173,0,0,5,MAX_UINT32,0, "unknown", "gif", 
                             true, "", "", "", ImageTable::DEFAULT, "", "", 
                             "", "", "" )
{
   // Clear m_navClientSettings
   delete [] m_searchClientSettings.m_clientType;
   m_searchClientSettings.m_clientType = NULL;
   delete [] m_searchClientSettings.m_clientTypeOptions;
   m_searchClientSettings.m_clientTypeOptions = NULL;
}


ClientSettingStorage::~ClientSettingStorage() {
   clear();
}


const ClientSetting*
ClientSettingStorage::getSetting( 
   const char* clientType, const char* clientOptions ) const
{
   const_cast<ClientSetting&>( m_searchClientSettings ).m_clientType = 
      const_cast< char* > ( clientType );
   const_cast<ClientSetting&>( m_searchClientSettings ).m_clientTypeOptions
      = const_cast< char* > ( clientOptions );

   const ClientSetting* match = NULL;
   pair< client_setting_set::iterator, client_setting_set::iterator > range =
      m_clientSettings.equal_range( &const_cast<ClientSetting&>( 
                                       m_searchClientSettings ) );

   const_cast<ClientSetting&>( m_searchClientSettings ).m_clientType 
      = NULL;
   const_cast<ClientSetting&>( m_searchClientSettings ).m_clientTypeOptions
      = NULL;

   if ( range.first != range.second ) {
      // Find the one with same options
      client_setting_set::iterator exactMatch = range.second;
      for ( client_setting_set::iterator it = range.first; it != range.second ;
            ++it )
      {
         if ( StringUtility::strcmp( (*it)->getClientTypeOptions(),
                                     clientOptions ) == 0 )
         {
            exactMatch = it;
            break;
         }
      }
      if ( exactMatch != range.second ) {
         match = *exactMatch;
      } else {
         // Use the one with empty options
         for ( client_setting_set::iterator it = range.first ; 
               it != range.second ; ++it )
         {
            if ( (*it)->getClientTypeOptions()[ 0 ] == '\0' ) {
               match = *it;
               break;
            }
         }
         if ( match == NULL ) {
            // Use first
            match = * range.first;
         }
      }
      
   } else {
      // No match use default
      match = &m_defaltClientSettings;
   }

   return match;
}


void
ClientSettingStorage::addSetting( ClientSetting* ver ) {
   m_clientSettings.insert( ver );
}


void
ClientSettingStorage::clear() {
   // Delete all and clear set.
   for ( client_setting_set::iterator it = m_clientSettings.begin() ;
         it != m_clientSettings.end() ; ++it )
   {
      delete *it;
   }
   m_clientSettings.clear();
}

namespace {

void parseTag( ClientSettingsProductReader& tagParser, 
               const LineParser::Line& line ) {
   // Product line
   if ( ! tagParser.parseLine( line ) ) {
      mc2log << fatal << tagParser.getTag() << " line not ok at line " << line 
             << endl;
      exit( 1 );
   }
}

void parseTag( ClientSettingsWFIDReader& tagParser, 
               const LineParser::Line& line ) {
   // WFID line
   if ( ! tagParser.parseLine( line ) ) {
      mc2log << fatal << tagParser.getTag() << " line not ok at line " << line 
             << endl;
      exit( 1 );
   }
}

}

bool
ClientSettingStorage::parseSettingsFile( const char* data ) {
   if ( data == NULL ) {
      mc2dbg << "[CSS]: data == NULL" << endl;
      return false;
   }

   // Lines starting with # are comments
   // Strings are in utf-8 (ascii is valid utf-8).
   //
   // Client-Type , Client-Type-Options , MatrixOfDoom
   // , notAutoUpgradeToSilver , silverRegionID
   // , SilverTimeYear , SilverTimeMonth , SilverTimeDay 
   // , explicitSilverTime
   // , BlockDate , notCreateWLUser , createLevel , createRegionID 
   // , createRegionTimeYear , createRegionTimeMonth , createRegionTimeDay
   // , explicitCreateRegionTime, createTransactionDays
   // , phoneModel, imageExtension, noLatestNews, callCenterList, brand
   // , categoryPrefix
   // % CommentText NL
   //
   // Client-Type         String identifing type of client
   // Client-Type-Options String with additional info about client
   // MatrixOfDoom        Int    Level of Doom, 1 or 2.
   // notAutoUpgradeToSilver Bool No autoupgrade to silver even if 
   //                             Matrix I.
   // silverRegionID      Uint32 Region ID to set when autoupdating to
   //                            silver.
   // SilverTimeYear      Int    The number of years to set for silver 
   //                            region.
   // SilverTimeMonth     Int    The number of months to set for silver 
   //                            region
   // SilverTimeDay       Int    The number of days to set for silver
   //                            region
   // explicitSilverTime  Date   The explicit date when silver expires.
   //                            Empty string if not explicit date. In UTC.
   // BlockDate           Date   The date to start blocking client.
   // notCreateWLUser     Bool   If not to create WL-user even if Matrix I.
   // createLevel         Uint32 The WFSubscription level to create.
   //                            0 = Trial, 1 = Silver, 2 = Gold, 3 = Iron
   // createRegionID      Uint32 Region ID to set when autocreating WL-user
   // createRegionTimeYear Int   The number of years to set for WL-user's
   //                            region.
   // createRegionTimeMonth Int  The number of months to set for WL-user's
   //                            region.
   // createRegionTimeDay   Int  The number of days to set for WL-user's
   //                            region.
   // explicitCreateRegionTime Date The explicit date when WL-user's 
   //                               expire. Empty string if no explicit
   //                               date. In UTC.
   // createTransactionDays Int  The number of transactions days for 
   //                            create. User set to be transaction days
   //                            based if createTransactionDays > 0.
   // phoneModel          String The default phonemodel for this client 
   //                            type.
   // imageExtension      String The image type extension, gif.
   // noLatestNews        Bool   If no latest news for client type.
   // callCenterList      String The call center list.
   // brand               String The brand type, empty if not branded.
   // categoryPrefix      String The prefix of category files. If empty
   //                            string then default "wfcat" is used.
   // imageSet            String The image set to use. If empty string
   //                            then ImageTable::DEFAULT is used.
   // version             String The version we recommend to use. Example:
   //                            4.64.0:4.9.0:0 If ! is first version the
   //                            clients are forced to upgrade to this version.
   // lockedVersion       String The latest version for clients locked to
   //                            major version. Example:
   //                            4.50.0|4.64.0:4.9.0:0;5.1.0|5.01.0:4.9.0:0
   // serverlistname      String The name of the serverlist that should be
   //                            used with this client. Serverlists
   //                            are defined in the serverlist
   //                            file. The empty string is interpreted
   //                            as "default" which is specified as a
   //                            command line argument.
   // upgradeId            String Identifier in the platform market for 
   //                            downloading new client version
   // extraRights         String additional rights for the user.
   //                            Example: "GOLD(1m,2097152)"
   //                            Will give the user GOLD rights for 1 month

   //
   // When finding client setting first all rows with same Client-Type are
   // searched for one with the same Client-Type-Options. If no row
   // with same Client-Type-Options the row with empty Client-Type-Options
   // is selected. If no empty Client-Type-Options then first best row is 
   // used.
   // If no rows with same Client-Type exists a default setting is used:
   // "default","", 2, false,1,0,0,0,"", "2038-01-01",true, 1,0,0,0,"",0,
   // "7650", "gif", false, "", "", "", ""
   //
   // If TimeYear,TimeMonth and TimeDay all are zero(0) it means inf time.
   // If explicitTime is set it is used in favour of Time(Year,Month,Day)
   
   int len = strlen( data );
   char clientType[ len ];
   char clientTypeOptions[ len ];
   uint32 matrixOfDoomLevel = 0;
   bool notAutoUpgradeToSilver = false;
   char notAutoUpgradeToSilverStr[ len ];
   uint32 silverRegionID = 0;
   int32 silverTimeYear = 0;
   int32 silverTimeMonth = 0;
   int32 silverTimeDay = 0;
   uint32 explicitSilverTime = 0;
   char explicitSilverTimeStr[ len ];
   uint32 blockDate = 0;
   char blockDateStr[ len ];
   bool notCreateWLUser = false;
   char notCreateWLUserStr[ len ];
   uint32 createLevel = 0;
   uint32 createRegionID = 0;
   int32 createRegionTimeYear = 0;
   int32 createRegionTimeMonth = 0;
   int32 createRegionTimeDay = 0;
   uint32 explicitCreateRegionTime = 0;
   int32 createTransactionDays = 0;
   char explicitCreateRegionTimeStr[ len ];
   char phoneModel[ len ];
   char imageExtension[ len ];
   bool noLatestNews = false;
   char noLatestNewsStr[ len ];
   char callCenterList[ len ];
   char brand[ len ];
   char categoryPrefix[ len ];
   char imageSet[ len ];
   char version[ len ];
   char lockedVersion[ len ];
   char serverList[ len ];
   char extraRights[ len ];
   char restStr[ len ];
   char upgradeId[ len ];
   int pos = 0;
   int line = 0;

   // Used to test extra rights, NULL thread to avoid sending anything to
   // modules.
   ParserActivationHandler rightshandler( NULL, m_group );

   //store versionlock data
   VersionLockParser vl_parser;
   ClientSettingsWFIDReader wfid( "WFID:" );
   ClientSettingsWFIDReader wfid_exception( "WFID_EXCEPTION:" );
   ClientSettingsWFIDReader notWFIDButUsesServices( 
      "NotWFIDButUsesServices:" );
   ClientSettingsProductReader product( "Product:" );
   ClientSettingsProductReader drawSettingVersion( "DrawVersion:" );
   ClientSettingsProductReader serverPrefix( "ServerPrefix:" );
   ClientSettingsWFIDReader noGoldLifeTime( "NoGoldLifeTime:" );
   ClientSettingsWFIDReader noGoldLifeTime_Exception( 
      "NoGoldLifeTime_Exception:" );

   while ( StringUtility::trimStart( data + pos )[ 0 ] != '\0' ) {
      // Some may not be set so clear them before using.
      serverList[ 0 ] = '\0';
      extraRights[ 0 ] = '\0';
      restStr[ 0 ] = '\0';      
      ++line;
      LineParser::Line realLine( data + pos, line );
      if ( StringUtility::trimStart( data + pos )[ 0 ] == '#' ) {
         // Comment
      } else if ( data[ pos ] == '\0' ) {
         // Eof
      } else if ( data[ pos ] == '\n' || 
                  (data[ pos ] == '\r' && data[ pos+1 ] == '\n' ) )
      {
         // Empty line
      } else if ( 0 == 
                  strncmp("VersionLock:", data + pos, sizeof("VersionLock:") - 1 ) ) {
         //VersionLock line
         vl_parser.parseVersionLock( data + pos );
      } else if ( wfid.isTagLine( realLine ) ) {
         ::parseTag( wfid, realLine );
      } else if ( wfid_exception.isTagLine( realLine ) ) {
         ::parseTag( wfid_exception, realLine );
      } else if ( notWFIDButUsesServices.isTagLine( realLine ) ) {
         ::parseTag( notWFIDButUsesServices, realLine );
      } else if ( product.isTagLine( realLine ) ) {
         ::parseTag( product, realLine );
      } else if ( drawSettingVersion.isTagLine( realLine ) ) {
         ::parseTag( drawSettingVersion, realLine );
      } else if ( serverPrefix.isTagLine( realLine ) ) {
         ::parseTag( serverPrefix, realLine );
      } else if ( noGoldLifeTime.isTagLine( realLine ) ) {
         ::parseTag( noGoldLifeTime, realLine );
      } else if ( noGoldLifeTime_Exception.isTagLine( realLine ) ) {
         ::parseTag( noGoldLifeTime_Exception, realLine );
      } else {
         // Get line
         //some format line hints:
         // "%u ," ignores any amount of whitespace, reads one
         // unsigned, ignores any amount of whitespace and then reads
         // one ','.
         // " %[^,\n]," ignores any amount of whitespace, reads a
         // string containing no ',' or '\n', and consumes one
         // ','. Any whitespace before the ',' is read into the
         // string. '"' will be stripped by checkString. Note that ','
         // is not protected by "".
         int res = 
            sscanf( data + pos, 
                    " %[^,\n], %[^,\n]," //clientType, clienttypeOptions
                    "%u , %[^,\n]," //doomlevel, notAutoUpgradeToSilverStr
                    "%u ,%d ," //silverregionid, silvertimeyear
                    "%d ,%d ," //silvertimemonth, silvertimeday
                    " %[^,\n]," // explicitSilverTimeStr
                    " %[^,\n], %[^,\n],%u ," //blockdatestr, notCreateWLUserStr, createlevel
                    "%u ,%d ," //createRegionID, creatRegionTimeYear
                    "%d ,%d ," //createRegiontimeMonth, createregiontimeDay
                    " %[^,\n]," // explicitCreateRegionTimeStr
                    "%d , %[^,\n]," //createtransactiondays, phonemodel
                    " %[^,\n], %[^,\n]," //imageExtension, noLatestNewsStr
                    " %[^,\n], %[^,\n], %[^,\n], %[^,\n],"//callCenterList, brand, categoryPrefix, imageSet
                    " %[^,\n], %[^,\n], %[^%,\n]"//version, lockedVersion, serverList,
                    ", %[^,\n]" // market id
                    ", %[^%\n]" //  extra rights
                    "%% %[^\n]\n",
                    clientType, clientTypeOptions, 
                    &matrixOfDoomLevel, notAutoUpgradeToSilverStr,
                    &silverRegionID, &silverTimeYear, 
                    &silverTimeMonth,&silverTimeDay, 
                    explicitSilverTimeStr,
                    blockDateStr, notCreateWLUserStr, &createLevel,
                    &createRegionID, &createRegionTimeYear,
                    &createRegionTimeMonth, &createRegionTimeDay,
                    explicitCreateRegionTimeStr, 
                    &createTransactionDays, phoneModel, 
                    imageExtension, noLatestNewsStr, 
                    callCenterList, brand, categoryPrefix, imageSet, 
                    version, lockedVersion, serverList,
                    upgradeId, extraRights,
                    restStr );

         if ( res == 31 ) {
            // Got it, check bools and other values
            // Booleans
            StringUtility::trimEnd( notAutoUpgradeToSilverStr );
            if ( !checkBoolean( StringUtility::trimStart( 
                                   notAutoUpgradeToSilverStr ), 
                                notAutoUpgradeToSilver ) ) 
            {
               mc2log << fatal << " notAutoUpgradeToSilver value " 
                      << MC2CITE( notAutoUpgradeToSilverStr )
                      << " not a boolean, error at line " << line << endl;
               exit( 1 );
            }
            StringUtility::trimEnd( notCreateWLUserStr );
            if ( !checkBoolean( StringUtility::trimStart( 
                                   notCreateWLUserStr ), 
                                notCreateWLUser ) ) 
            {
               mc2log << fatal << "notCreateWLUser value " 
                      << MC2CITE( notCreateWLUserStr )
                      << " not a boolean, error at line " << line << endl;
               exit( 1 );
            }
            StringUtility::trimEnd( noLatestNewsStr );
            if ( !checkBoolean( StringUtility::trimStart( 
                                   noLatestNewsStr ), noLatestNews ) ) 
            {
               mc2log << fatal << "noLatestNews value " 
                      << MC2CITE( noLatestNewsStr )
                      << " not a boolean, error at line " << line << endl;
               exit( 1 );
            }
            // Strings
            MC2String clientTypeStr;
            if ( !checkString( clientType, clientTypeStr ) ) {
               mc2log << fatal << "clientType value " 
                      << MC2CITE( clientType )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            MC2String clientTypeOptionsStr;
            if ( !checkString( clientTypeOptions, clientTypeOptionsStr ) )
            {
               mc2log << fatal << "clientTypeOptions value " 
                      << MC2CITE( clientTypeOptions )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            MC2String phoneModelStr;
            if ( !checkString( phoneModel, phoneModelStr ) )
            {
               mc2log << fatal << "phoneModel value " 
                      << MC2CITE( phoneModel )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            MC2String imageExtensionStr;
            if ( !checkString( imageExtension, imageExtensionStr ) )
            {
               mc2log << fatal << "imageExtension value " 
                      << MC2CITE( imageExtension )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            MC2String callCenterListStr;
            if ( !checkString( callCenterList, callCenterListStr ) )
            {
               mc2log << fatal << "callCenterList value " 
                      << MC2CITE( callCenterList )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            MC2String brandStr;
            if ( !checkString( brand, brandStr ) )
            {
               mc2log << fatal << "brand value " 
                      << MC2CITE( brand )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            MC2String categoryPrefixStr;
            if ( !checkString( categoryPrefix, categoryPrefixStr ) )
            {
               mc2log << fatal << "categoryPrefix value " 
                      << MC2CITE( categoryPrefix )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            MC2String imageSetStr;
            if ( !checkString( imageSet, imageSetStr ) )
            {
               mc2log << fatal << "imageSet value " 
                      << MC2CITE( imageSet )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            MC2String versionStr;
            if ( !checkString( version, versionStr ) ) {
               mc2log << fatal << "version value " 
                      << MC2CITE( version )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            MC2String lockedVersionStr;
            if ( !checkString( lockedVersion, lockedVersionStr ) )
            {
               mc2log << fatal << "lockedVersion value " 
                      << MC2CITE( lockedVersion )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            MC2String upgradeIdStr;
            if ( !checkString( upgradeId, upgradeIdStr ) )
            {
               mc2log << fatal << "upgradeId value " 
                      << MC2CITE( upgradeId )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            // Integers
            if ( matrixOfDoomLevel != 1 && matrixOfDoomLevel != 2 ) {
               mc2log << fatal << "matrixOfDoomLevel not in valid range "
                      << matrixOfDoomLevel << " at line " << line << endl;
               exit( 1 );
            }
            // Dates
            // blockDateStr
            MC2String blockDateString;
            if ( !checkString( blockDateStr, blockDateString ) ) {
               mc2log << fatal << "blockDate value " 
                      << MC2CITE( blockDateStr )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            if ( blockDateString.empty() ) {
               blockDate = 0;
            } else {
               blockDate = StringUtility::makeDate( 
                  blockDateString.c_str() );
               if ( blockDate == 0 ) {
                  mc2log << fatal << "blockDate not valid "
                         << "date " << MC2CITE( blockDateStr )
                         << " at line " << line << endl;
                  exit( 1 );
               }
            }
            // explicitSilverTimeStr
            MC2String explicitSilverTimeString;
            if ( !checkString( explicitSilverTimeStr, 
                               explicitSilverTimeString ) )
            {
               mc2log << fatal << "explicitSilverTime value " 
                      << MC2CITE( explicitSilverTimeStr )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            if ( explicitSilverTimeString.empty() ) {
               explicitSilverTime = MAX_UINT32;
            } else {
               explicitSilverTime = StringUtility::makeDate( 
                  explicitSilverTimeString.c_str() );
               if ( explicitSilverTime == 0 ) {
                  mc2log << fatal << "explicitSilverTime not valid "
                         << "date " << MC2CITE( explicitSilverTimeStr )
                         << " at line " << line << endl;
                  exit( 1 );
               }
            }
            // explicitCreateRegionTimeStr
            MC2String explicitCreateRegionTimeString;
            if ( !checkString( explicitCreateRegionTimeStr, 
                               explicitCreateRegionTimeString ) )
            {
               mc2log << fatal << "explicitCreateRegionTime value " 
                      << MC2CITE( explicitCreateRegionTimeStr )
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }
            if ( explicitCreateRegionTimeString.empty() ) {
               explicitCreateRegionTime = MAX_UINT32;
            } else {
               explicitCreateRegionTime = StringUtility::makeDate( 
                  explicitCreateRegionTimeString.c_str() );
               if ( explicitCreateRegionTime == 0 ) {
                  mc2log << fatal << "explicitCreateRegionTime not valid "
                         << "date " << MC2CITE( explicitCreateRegionTime )
                         << " at line " << line << endl;
                  exit( 1 );
               }
            }
            //serverlistname
            MC2String serverListNameString;
            if ( ! checkString( serverList, serverListNameString ) ) {
               mc2log << fatal << "namedServerList value " 
                      << MC2CITE( serverList ) 
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            } 
            MC2String extraRightsString;
            if ( ! checkString( extraRights, extraRightsString ) ) {
               mc2log << fatal << "extraRights value " 
                      << MC2CITE( extraRights ) 
                      << " not ok, error at line " << line << endl;
               exit( 1 );
            }

            if ( !extraRightsString.empty() ) {
               // Test ExtraRights
               UserUser rightsUser( 0 );
               if ( rightshandler.addUserRights( 
                       &rightsUser, "", "", MAX_INT32, extraRightsString,
                       "", TimeUtility::getRealTime() ) != 0 ) {
                  mc2log << fatal << "extraRights does not activate: " 
                         << MC2CITE( extraRightsString ) 
                         << " is not ok, error at line " << line << endl;
                  exit( 1 );
               }
            }

            ClientSetting* cs = new ClientSetting(
               m_group, 
               clientTypeStr.c_str(), clientTypeOptionsStr.c_str(),
               matrixOfDoomLevel, notAutoUpgradeToSilver, silverRegionID, 
               silverTimeYear, silverTimeMonth, silverTimeDay,
               explicitSilverTime, blockDate, notCreateWLUser, 
               WFSubscriptionConstants::subscriptionsTypes( createLevel ),
               createRegionID, createRegionTimeYear, createRegionTimeMonth,
               createRegionTimeDay, explicitCreateRegionTime,
               createTransactionDays, phoneModelStr.c_str(), 
               imageExtensionStr.c_str(), noLatestNews, 
               callCenterListStr.c_str(), brandStr.c_str(),
               categoryPrefixStr.c_str(), ImageTable::getImageSetFromString(imageSetStr),
               versionStr.c_str(), lockedVersionStr.c_str(), 
               serverListNameString.c_str(), extraRightsString, upgradeIdStr );
            addSetting( cs ) ;

            mc2dbg8 << "Line: " << MC2CITE( clientTypeStr.c_str() ) << ", "
                   << MC2CITE( clientTypeOptionsStr.c_str() )
                   << ", " << matrixOfDoomLevel << ", " 
                   << StringUtility::booleanAsString( 
                      notAutoUpgradeToSilver ) << ", " << silverRegionID
                   << ", " << silverTimeYear << ", " << silverTimeMonth 
                   << ", " << silverTimeDay << ", ";
            if ( explicitSilverTime == MAX_UINT32 ) {
               mc2dbg8 << MC2CITE( "" ) << ", ";
            } else {

               ::printTime( explicitSilverTime );

            }
            if ( blockDate == 0 ) {
               mc2dbg8 << MC2CITE( "" ) << ", ";
            } else {
               ::printTime( blockDate );
            }
            mc2dbg8 << StringUtility::booleanAsString( notCreateWLUser ) 
                   << ", " << createRegionID
                   << ", " << createRegionTimeYear 
                   << ", " << createRegionTimeMonth 
                   << ", " << createRegionTimeDay;
            if ( explicitCreateRegionTime == MAX_UINT32 ) {
               mc2dbg8 << ", " << MC2CITE( "" );
            } else {
               ::printTime( explicitCreateRegionTime );
            }
            mc2dbg8 << ", " << createTransactionDays
                   << ", " << MC2CITE( phoneModelStr.c_str() ) 
                   << ", " << MC2CITE( imageExtensionStr.c_str() ) 
                   << ", " 
                   << StringUtility::booleanAsString( noLatestNews )
                   << ", " << MC2CITE( callCenterListStr.c_str() )
                   << ", " << MC2CITE( brandStr.c_str() )
                   << ", " << MC2CITE( categoryPrefixStr.c_str() )
                   << ", " << MC2CITE( imageSetStr.c_str() )
                   << ", " << MC2CITE( versionStr.c_str() )
                   << ", " << MC2CITE( lockedVersionStr.c_str() )
                   << ", " << MC2CITE( serverListNameString.c_str() )
                   << ", " << MC2CITE( extraRightsString.c_str() )
                   << endl;
            
         } else {
            mc2log << fatal << "Bad line starting at "
                   << MC2CITE( data + pos ) << " res " << res 
                   << " at line " << line << endl;
            exit( 1 );
         }
         
      }

      char* eolPos = StringUtility::strchr( data + pos, '\n' );
      if ( eolPos != NULL ) {
         pos = eolPos - data + 1;
      } else {
         mc2log << fatal << "No eol at line " << MC2CITE( data + pos ) 
                << " at line " << line << ". Pos " << pos << endl;
         exit( 1 );
      }
   } // End while not null-byte

   //set version lock valuse for all clientsettings objects.
   for(client_setting_set::iterator it = m_clientSettings.begin();
       it != m_clientSettings.end(); ++it){
      const char* client_type = (*it)->getClientType();
      uint32 versionLock = vl_parser.findVersionLock( client_type );
      if( versionLock != 0 ){
         (*it)->setVersionLock( versionLock );
      }
      // And WFID
      (*it)->setWFID( wfid.isSet( client_type ) );
      // WFID exceptions
      if ( (*it)->getWFID() && wfid_exception.isSet( client_type ) ) {
         (*it)->setWFID( false );
      }
      // NotWFIDButUsesServices
      (*it)->setNotWFIDButUsesServices( notWFIDButUsesServices.isSet( 
                                           client_type ) );
      // AND Product name
      (*it)->setProduct( product.getProduct( client_type ) );
      // get draw setting version string and convert it to integer
      MC2String versionStr = drawSettingVersion.getProduct( client_type );
      uint32 version = 0;
      if ( !versionStr.empty() ) {
         try {
            version = boost::lexical_cast<uint32>( versionStr );
         } catch ( const boost::bad_lexical_cast& ex ) {
            mc2log << warn << "[ClientSettings] Failed to convert string: "
                   << "\"" << versionStr << "\" " << ex.what() << endl;
         }
      }
      (*it)->setDrawVersion( version );

      // get server prefix and convert it to integer
      MC2String serverPrefixStr = serverPrefix.getProduct( client_type );
      if ( !serverPrefixStr.empty() ) {
         try {
            (*it)->setServerPrefix( 
               boost::lexical_cast<uint32>( serverPrefixStr ) );
         } catch ( const boost::bad_lexical_cast& ex ) {
            mc2log << warn << "[ClientSettings] Failed to convert string: "
                   << "\"" << serverPrefixStr << "\" " << ex.what() << endl;
         }
      }

      // And noGoldLifeTime
      (*it)->setNoGoldLifeTime( noGoldLifeTime.isSet( client_type ) );
      if ( (*it)->noGoldLifeTime() && 
           noGoldLifeTime_Exception.isSet( client_type ) ) {
         (*it)->setNoGoldLifeTime( false );
      }
   }
   return true;
}

bool 
ClientSettingStorage::checkBoolean( const char* str, bool& value ) {
   if ( StringUtility::strcasecmp( str, "false" ) == 0 ) {
      value = false;
      return true;
   } else if ( StringUtility::strcasecmp( str, "true" ) == 0 ) {
      value = true;
      return true;
   } else {
      return false;
   }
}


bool 
ClientSettingStorage::checkString( const char* str, MC2String& res ) {
   bool ok = true;
   res = StringUtility::trimStartEnd( str );
   // Check for " at start
   char startCitation = '\0';
   if ( res[ 0 ] == '"' || res[ 0 ] == '\'' ) {
      startCitation = res[ 0 ];
      res.erase( 0, 1 );
   }
   // Remove whitespace at end
   while ( res.size() > 0 && isspace( res[ res.size() -1 ] ) ) {
      res.erase( res.size() -1, 1 );
   }
   // Check for " at end
   if ( res.size() > 0 ) {
      if ( res[ res.size() -1 ] == '"' || res[ res.size() -1 ] == '\'' ) {
         if ( startCitation != res[ res.size() -1 ] ) {
            mc2log << error << "Citation error: " 
                   << MC2CITE( startCitation )
                   << " not matched by ending " 
                   << MC2CITE( res[ res.size() -1 ] ) << " string "
                   << MC2CITE( str ) << endl;
            ok = false;
         }
         res.erase( res.size() -1, 1 );
      } else if ( startCitation != '\0' ) {
         mc2log << error << "Citation error: " << MC2CITE( startCitation )
                << " not matched at end of string " 
                << MC2CITE( res[ res.size() -1 ] ) << " string "
                << MC2CITE( str ) << endl;
         ok = false;
      }
   } // End if Empty string
   // Done
   return ok;
}

ostream& operator<<(ostream& stream, const ClientSettingStorage& store)
{
   const ClientSettingStorage::client_setting_set& settings = 
      store.getClientSettings();
   stream << settings.size() << " settings" << endl;
   for(ClientSettingStorage::client_setting_set::const_iterator it = 
          settings.begin(); it != settings.end(); ++it){
      stream << **it << endl;
   }
   return stream;
}

ostream& operator<<(ostream& stream, const ClientSetting& setting) {
   stream << setting.m_clientType
          << ", " << setting.m_clientTypeOptions
          << ", " << setting.m_matrixOfDoomLevel
          << ", " << setting.m_notAutoUpgradeToSilver
          << ", " << setting.m_silverRegionID
          << ", " << setting.m_silverTimeYear
          << ", " << setting.m_silverTimeMonth
          << ", " << setting.m_silverTimeDay
          << ", " << setting.m_explicitSilverTime
          << ", " << setting.m_blockDate
          << ", " << setting.m_notCreateWLUser
          << ", " << setting.m_createLevel
          << ", " << setting.m_createRegionID
          << ", " << setting.m_createRegionTimeYear
          << ", " << setting.m_createRegionTimeMonth
          << ", " << setting.m_createRegionTimeDay
          << ", " << setting.m_explicitCreateRegionTime
          << ", " << setting.m_createTransactionDays 
          << ", " << setting.m_phoneModel
          << ", " << setting.m_imageExtension
          << ", " << setting.m_noLatestNews
          << ", " << setting.m_callCenterList
          << ", " << setting.m_callCenterListCRC
          << ", " << setting.m_brand
          << ", " << setting.m_categoryPrefix
          << ", " << ImageTable::imageSetToString( setting.m_imageSet )
          << ", " << setting.m_version
          << ", " << setting.m_forceUpgrade
      
      //           << ", " << *(setting.m_verObj)      
//    lockedVersionMap                            setting.m_lockedVersions
      
          << ", " << setting.m_serverListName
          << ", " << setting.m_versionLock
          << ", " << setting.m_upgradeId
          << ", " << setting.m_extraRights;
   
   return stream << endl;
}
