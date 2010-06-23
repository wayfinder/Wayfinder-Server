/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavParserThreadGroup.h"
#include "NavInterfaceFactory.h"
#include "NavRouteMessage.h"
#include "RouteRequest.h"
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "NavThread.h"
#include "ClientSettings.h"
#include "RoutePacket.h"
#include "UserData.h"
#include "ItemTypes.h"
#include "MC2CRC32.h"
#include "NavClientVersionStorage.h"
#include "TopRegionMatch.h"
#include "TopRegionRequest.h"
#include "NParam.h"
#include "NParamBlock.h"
#include "NavPacket.h"
#include <algorithm>
#include "sockets.h" // write
#include "FOBC.h"
#include "NavLatestNewsData.h"
#include "NavCategoriesData.h"
#include "NamedServerLists.h"
#include "DataBuffer.h"

NavParserThreadGroup::NavParserThreadGroup(
   const char* threadGroupName ,
   uint32 minNbrThreads, uint32 maxNbrThreads,
   uint32 queueFullFactor, uint32 queueOverFullFactor ):
   InterfaceParserThreadGroup( ServerTypes::NAVIGATOR,
                               threadGroupName,
                               minNbrThreads, maxNbrThreads,
                               queueFullFactor, queueOverFullFactor )
{
   //m_categoriesData belongs to ParserThreadGroup, but cannot be set
   //there as ParserTreadGroup doesn't know what type to use.
   m_categoriesData = new NavCategoriesDataHolder();

   m_simulatedRouteFileName = NULL;
   m_threadID = 0;
   m_alternativeServersList = NULL;
   m_alternativeServersListCRC = 0;
   m_longAlternativeServersList = NULL;
   m_longAlternativeServersListCRC = 0;
   m_httpAlternativeServersList = NULL;
   m_httpAlternativeServersListCRC = 0;
   m_forceRedirect = false;
   m_clientVersions = NULL;
   m_noUserTrack = false;
   m_latestNewsSearch = new SearchNavLatestNewsData();
}


NavParserThreadGroup::~NavParserThreadGroup() {
   delete m_clientVersions;
   delete [] m_simulatedRouteFileName;
   for ( latestNewsMap::iterator it = m_latestNews.begin() ; 
         it != m_latestNews.end() ; ++it )
   {
      delete *it;
   }
   delete m_latestNewsSearch;
   delete m_categoriesData;
}


void
NavParserThreadGroup::handleDoneInterfaceRequest( 
InterfaceRequest* ireply )
{
   static_cast< NavInterfaceFactory* > ( getFirstInterfaceFactory() )
      ->handleDoneInterfaceRequest( ireply );
}


ParserThreadHandle
NavParserThreadGroup::spawnThread() {
   return new NavThread( m_threadID++, this );
}


bool
NavParserThreadGroup::simulatedRouteFileName() {
   if ( m_simulatedRouteFileName != NULL ) {
      return true;
   } else {
      return false;
   }
}

void
NavParserThreadGroup::setSimulatedRouteFileName( const char *newFileName ) 
{
   if ( m_simulatedRouteFileName ) {
      delete [] m_simulatedRouteFileName;
   }
   if ( newFileName ) {
      m_simulatedRouteFileName = StringUtility::newStrDup( newFileName );
   } else {
      m_simulatedRouteFileName = NULL;
   }
}

bool
NavParserThreadGroup::saveRouteForSimulator( RouteRequest* rr, 
                                             RouteReply* reply )
{
   if ( reply != NULL ) {
      mc2log << info << "SaveSimRoute: About to write (Nav)RouteReply "
             << "packet." << endl;
      int bufsize = 65536;
      byte buf[ bufsize ];
      if ( reply->convertToBytes( buf, bufsize ) == false ) {
         /* Failed ! */
         mc2log << error << "SaveSimRoute:  -"
                << "couldn't convert the (Nav)RouteReply to bytes" << endl;
      } else {
         int filenamelen = strlen( m_simulatedRouteFileName ) + 40;
         char* filename = new char[ filenamelen ];
         uint32 routeId = rr->getRouteID();
         uint32 routeTime = rr->getRouteCreateTime();
         sprintf( filename, "%s_routepack.%u-%u", m_simulatedRouteFileName,
                  routeTime, routeId );
         FILE* f = fopen( filename, "wb" );
         if ( f != NULL ) {
            if ( fwrite( buf, 1, reply->getLength(), f ) == 
                 reply->getLength() )
            {
               // DONE!
               mc2log << info << "SaveSimRoute: saved (Nav)RoutePacket "
                      << "as \"" << filename << "\"." << endl;
            } else {
               mc2log << error << "SaveSimRoute: -"
                      << "failed to write all (Nav)RoutePacket to "
                      << "\"" << filename << "\"." << endl;
            }

            fclose( f );
         } else {
            mc2log << error << "SaveSimRoute: -"
                   << "couldn't open \"" << filename << "\" for writing."
                   << endl;
         }

         delete [] filename;
      }
   }

   return saveMapRoute( m_simulatedRouteFileName, rr );
}


bool
NavParserThreadGroup::saveRouteForSimulator( NavReplyPacket* reply, 
                                             RouteRequest *rr )
{
   if ( reply != NULL && reply->getParamBlock().getParam( 1105 ) != NULL )
   {
      // Save data to file.
      int filenamelen = strlen( m_simulatedRouteFileName ) + 40;
      char* filename = new char[ filenamelen ];
      char* filename2 = new char[ filenamelen ];
      RouteID routeID;
      const NParam* data = reply->getParamBlock().getParam( 1105 );
      bool t = true;
      if ( reply->getParamBlock().getParam( 1100 ) ) {
         routeID = RouteID( reply->getParamBlock().getParam( 1100 )
                            ->getString(t).c_str() );
      }

      sprintf( filename, "%s_nav.%u-%u", m_simulatedRouteFileName, 
               routeID.getRouteIDNbr(), routeID.getCreationTime() );
      sprintf( filename2, "%s_nav", m_simulatedRouteFileName );
      mc2log << info << "SaveSimRoute: Will save data as \""
             << filename2 << "\" and \""
             << filename << "\"." << endl;
      int aflags = O_CREAT|O_TRUNC|O_WRONLY;
      int amode = S_IRWXU|S_IRGRP|S_IROTH;
      int fd = open( filename, aflags, amode );
      if ( fd < 0 ) {
         mc2log << warn << "SaveSimRoute: Error opening file: "
                << filename << " - "
                << strerror(errno) << endl;
      } else {
         int res = write( fd, data->getBuff(), data->getLength() );
         if ( res < 0 ) {
            mc2log << warn << "SaveSimRoute: Error writing to file: "
                   << filename << " - "
                   << strerror(errno) << endl;
         }
         if ( close( fd ) < 0 ) {
            mc2log << warn << "SaveSimRoute: Error closing file: "
                   << filename << " - "
                   << strerror(errno) << endl;
         }
      }

      fd = open( filename2, aflags, amode );
      if ( fd < 0 ) {
         mc2log << warn << "SaveSimRoute: Error opening file: " 
                << filename2 << " - "
                << strerror(errno) << endl;
      } else {
         int res = write( fd, data->getBuff(), data->getLength() );
         if ( res < 0 ) {
            mc2log << warn << "SaveSimRoute: Error writing to file: "
                   << filename2 << " - "
                   << strerror(errno) << endl;
         }
         if ( close( fd ) < 0 ) {
            mc2log << warn << "SaveSimRoute: Error closing file: "
                   << filename2 << " - "
                   << strerror(errno) << endl;
         }
      }
      
      delete [] filename;
      delete [] filename2;
   } // End if have NavReply 

   return saveMapRoute( m_simulatedRouteFileName, rr );
}


void 
NavParserThreadGroup::setLatestNews( const char* latestNews ) {
   m_latestNewsMutex.lock();

   if ( latestNews != NULL ) {
      const ClientSettingStorage* storage = getClientSettingStorage();
      if ( storage == NULL || 
           storage->getClientSettings().empty() )
      {
         mc2log << fatal << "NavParserThreadGroup::setLatestNews "
                << "no clientSettings available. ClientSetting required. "
                << "Set clientSettings before calling setLatestNews." 
                << endl;
         exit( 1 );
      }

      // client-type _ lang _ WFSubscr_t _ DaysLeft _ . ext
      // 
      // client-type is the client type identificatin string.
      // lang is the iso-639-1 string for the language.
      // WFSubscr_t is the subscription type {t|s|g}.
      // DaysLeft is the number of days left, inf|0|7|14|30.
      // ext is the file type extension for the client-type, for settings.

      const char* clientType = NULL;
      const char* langStr = NULL;
      char WFSubscr_t[ 3 ] = { 't','s', 'g' };
      char WFSubscrCh = WFSubscr_t[ 0 ];
      char daysLeftStr[ 10 ];
      daysLeftStr[ 0 ] = '\0';
      const char* ext = NULL;
      char fileName[ 4096 ];
      NavLatestNewsData* latest = NULL;

      // for all client-types
      for ( ClientSettingStorage::client_setting_set::const_iterator it = 
               getClientSettingStorage()->getClientSettings().begin() ; 
            it != getClientSettingStorage()->getClientSettings().end() ; 
            ++it )
      {
         if ( (*it)->getNoLatestNews() ) {
            continue;
         }
         // Set clientType
         clientType = (*it)->getClientType();
         // Set ext
         ext = (*it)->getImageExtension();
         bool printedError = false;

         // for all languages
         for ( uint32 lang = LangTypes::english/*0*/ ; 
               lang < LangTypes::nbrLanguages; ++lang )
         {
            if ( ItemTypes::getLanguageTypeAsLanguageCode( 
                    LangTypes::language_t( lang ) ) ==
                 StringTable::SMSISH_ENG )
            {
               // Not supported
               continue;
            }
            // Set langStr
            langStr = LangTypes::getLanguageAsISO639( 
               LangTypes::language_t( lang ) );
            // for all WfSubscription types
            for ( uint32 wft = WFSubscriptionConstants::TRIAL/*0*/ ;
                  wft <= WFSubscriptionConstants::GOLD /*end*/ ; ++wft )
            {
               // Set WFSt
               WFSubscrCh = WFSubscr_t[ wft ];

               // for all days left
               // inf
               sprintf( fileName, "%s/%s_%s_%c_%s.%s", latestNews,
                        clientType, langStr, WFSubscrCh, "inf", ext );
//               mc2dbg << "Latest news file: " << fileName << endl;
               latest = new NavLatestNewsData(
                  fileName, clientType, LangTypes::language_t( lang ),
                  WFSubscriptionConstants::subscriptionsTypes( wft ),
                  MAX_UINT16, ext, this );
               if ( ! latest->loadFile() ) {
                  if ( !printedError ) {
                     mc2log << info << "NavParserThreadGroup::"
                            << "setLatestNews failed to load \""
                            << fileName << "\" skipping." << endl;
                     printedError = true;
                  }
                  delete latest;
               } else {
                  pair< latestNewsMap::iterator, bool > insRes = 
                     m_latestNews.insert( latest );
                  if ( !insRes.second ) {
                     // Duplicate client-type or something like that
                     mc2dbg2 << "NavParserThreadGroup::"
                             << "setLatestNews duplicate \"" << fileName
                             << "\" not inserted." << endl;
                     delete latest;
                  }
               }

               // Some intervals
               for ( uint16 days = 0 ; 
                     days < NavLatestNewsData::getNbrDaysLeftValues() ; 
                     ++days ) 
               {
                  sprintf( fileName, "%s/%s_%s_%c_%u.%s", latestNews,
                           clientType, langStr, WFSubscrCh, 
                           NavLatestNewsData::getDaysLeftValue( days ), 
                           ext );

//                  mc2dbg << "Latest news file: " << fileName << endl;
                  latest = new NavLatestNewsData(
                     fileName, clientType, LangTypes::language_t( lang ),
                     WFSubscriptionConstants::subscriptionsTypes( wft ),
                     NavLatestNewsData::getDaysLeftValue( days ), ext,
                     this );
                  if ( ! latest->loadFile() ) {
                     if ( !printedError ) {
                        mc2log << info << "NavParserThreadGroup::"
                               << "setLatestNews failed to load \""
                               << fileName << "\" skipping." << endl;
                        printedError = true;
                     }
                     delete latest;
                  } else {
                     m_latestNews.insert( latest );
                  }
               }
            }

         }
      }
   } // End If not null argument

   m_latestNewsMutex.unlock();
}


bool 
NavParserThreadGroup::getLatestNews( 
   const char* clientType, 
   LangTypes::language_t lang,
   WFSubscriptionConstants::subscriptionsTypes wfSubscriptionType,
   uint16 daysLeft,
   const char* imageExtension,
   byte*& latestNews,
   uint32& latestNewsLength )
{
   m_latestNewsMutex.lock();
   bool ok = true;

   daysLeft = NavLatestNewsData::getClosestDaysLeftValue( daysLeft );
   m_latestNewsSearch->setData( clientType, lang, wfSubscriptionType,
                                daysLeft, imageExtension );
   latestNewsMap::const_iterator findIt = m_latestNews.find( 
      m_latestNewsSearch );
   m_latestNewsSearch->unSetData();

   if ( findIt != m_latestNews.end() ) {
      if ( !(*findIt)->recheckFile() ) {
         mc2log << warn << "NavParserThreadGroup::getLatestNews "
                << "recheckFile failed. Check \"" 
                << (*findIt)->getFilePatch() << "\"" << endl;
      }
      latestNews = new byte[ (*findIt)->getLastNewsLength() ];
      memcpy( latestNews, (*findIt)->getLastNews(), 
              (*findIt)->getLastNewsLength() );
      latestNewsLength = (*findIt)->getLastNewsLength();
      ok = true;
   } else {
      latestNews = NULL;
      latestNewsLength = 0;
      ok = false;
   }

   m_latestNewsMutex.unlock();
   return ok;
}


bool 
NavParserThreadGroup::getLatestNewsCRC( 
   const char* clientType, 
   LangTypes::language_t lang,
   WFSubscriptionConstants::subscriptionsTypes wfSubscriptionType,
   uint16 daysLeft,
   const char* imageExtension,
   uint32& crc )
{
   m_latestNewsMutex.lock();
   bool ok = true;

   daysLeft = NavLatestNewsData::getClosestDaysLeftValue( daysLeft );
   m_latestNewsSearch->setData( clientType, lang, wfSubscriptionType,
                                daysLeft, imageExtension );
   latestNewsMap::const_iterator findIt = m_latestNews.find( 
      m_latestNewsSearch );
   m_latestNewsSearch->unSetData();

   if ( findIt != m_latestNews.end() ) {
      ok = true;
      crc = (*findIt)->getCRC();
   } else {
      crc = 0;
      ok = false;
   }

   m_latestNewsMutex.unlock();
   return ok;
}


void 
NavParserThreadGroup::setAlternativeServersList( 
   const char* alternativeServersList ) 
{
   m_alternativeServersList = alternativeServersList;
   if ( m_alternativeServersList != NULL ) {
      // Check alternativeServersList
      if ( strlen( m_alternativeServersList ) > 255 ) {
         mc2log << fatal << "NavParserThreadGroup::"
                << "setAlternativeServersList list too long "
                << strlen( m_alternativeServersList ) << " bytes max 255."
                << endl;
         exit( 1 );
      }
      m_alternativeServersListCRC = MC2CRC32::crc32( 
         (byte*)m_alternativeServersList,
         strlen( m_alternativeServersList ) );

      if ( !replaceNamedServerList( NamedServerLists::DefaultGroup, 
                                    NamedServerLists::NavServerShortType,
                                    m_alternativeServersList ) )
      {
         mc2log << fatal << "NavParserThreadGroup::"
                << "setAlternativeServersList list not ok " << endl;
         exit( 1 );
      }
   }
}


void
NavParserThreadGroup::setLongAlternativeServersList( 
   const char* longAlternativeServersList )
{
   m_longAlternativeServersList = longAlternativeServersList;
   if ( m_longAlternativeServersList != NULL ) {
      // Check alternativeServersList
      m_longAlternativeServersListCRC = MC2CRC32::crc32( 
         (byte*)m_longAlternativeServersList,
         strlen( m_longAlternativeServersList ) );

      if ( !replaceNamedServerList( NamedServerLists::DefaultGroup, 
                                    NamedServerLists::NavServerType,
                                    m_longAlternativeServersList ) )
      {
         mc2log << fatal << "NavParserThreadGroup::"
                << "setLongAlternativeServersList list not ok " << endl;
         exit( 1 );
      }
   }
}


void
NavParserThreadGroup::setHttpAlternativeServersList( 
   const char* httpAlternativeServersList )
{
   m_httpAlternativeServersList = httpAlternativeServersList;
   if ( m_httpAlternativeServersList != NULL ) {
      // Check alternativeServersList
      m_httpAlternativeServersListCRC = MC2CRC32::crc32( 
         (byte*)m_httpAlternativeServersList,
         strlen( m_httpAlternativeServersList ) );

      if ( !replaceNamedServerList( NamedServerLists::DefaultGroup, 
                                    NamedServerLists::HttpServerType,
                                    m_httpAlternativeServersList ) )
      {
        mc2log << fatal << "NavParserThreadGroup::"
                << "setHttpAlternativeServersList list not ok " << endl;
         exit( 1 );
      }
   } 
}


void 
NavParserThreadGroup::setCurrentVersion( const char* currentVersion ) {
   delete m_clientVersions;
   m_clientVersions = new NavClientVersionStorage();

   if ( currentVersion != NULL ) {
      // Check and add to client version list

      // A client entry:
      // Program version;Client type;Client type options;Resource version
      // Program version is majorbyte.minorbyte.minibyte like 1.1.23
      // Client type and Client type options is a string
      // Resource version is like Program version
      int pos = 0;
      int length = strlen( currentVersion );
      while ( pos < length ) {
         uint32 majorV = 0;
         uint32 minorV = 0;
         uint32 miniV = 0;
         char clientType[ 256 ];
         char clientTypeOptions[ 256 ];
         uint32 majorRV = 0;
         uint32 minorRV = 0;
         uint32 miniRV = 0;
         char remainder[ 512 ];
         uint32 elementSize = 0;
         bool dataOk = false;
         bool atEnd = false;

         if ( sscanf( currentVersion + pos, 
                      "%u.%u.%u;%[^;];%[^;];%u.%u.%u" "%s", 
                      &majorV, &minorV, &miniV, clientType, 
                      clientTypeOptions, &majorRV, &minorRV, &miniRV, 
                      remainder ) == 9 )
         {
            dataOk = true;
         } else if ( sscanf( currentVersion + pos, 
                             "%u.%u.%u;%[^;];%[^;];%u.%u.%u", 
                             &majorV, &minorV, &miniV, clientType, 
                             clientTypeOptions, &majorRV, &minorRV, 
                             &miniRV ) == 8 )
         {
            dataOk = true;  
            atEnd = true;
         }
         
         if ( dataOk ) {
            // Add data to client version map/list
            m_clientVersions->addVersion( 
               new NavClientVersion( clientType, clientTypeOptions,
                                     majorV, minorV, miniV,
                                     majorRV, minorRV, miniRV ) );

            const char* commaPos = strchr( currentVersion + pos, ',' );
            if ( commaPos != NULL ) {
               elementSize = strchr( currentVersion + pos, ',' ) - 
                  (currentVersion + pos);
               pos += elementSize;
               pos++; // Skipp ','
            } else if ( atEnd ) {
               pos = length;
            } else {
              mc2log << fatal << "Client version list "
                     << "garbage after end, ',' not found! Error after "
                     << "this element: " 
                     << (currentVersion + pos) << endl;
              exit( 1 ); 
            }
         } else {
            mc2log << fatal << "client version list "
                   << "currentVersion not valid! Error at: " 
                   << (currentVersion + pos) << endl;
            exit( 1 );
         }
         
      }
      
   }
}


const NavClientVersion* 
NavParserThreadGroup::getClientVersion( const char* clientType ) const {
   m_clientVersionMutex.lock();
   const NavClientVersion* cv = m_clientVersions->getVersion( clientType );
   m_clientVersionMutex.unlock();
   return cv;
}


int 
NavParserThreadGroup::getTopRegionData( ParserThreadHandle thread, 
                                        const UserUser* user,
                                        StringTable::languageCode lang,
                                        UserEnums::URType urmask,
                                        NParam& data )
{
   int res = 0;
   // Get a top region list.
   const TopRegionRequest* topReq = getTopRegionRequest( thread );
   
   // Check status of request, could be not ok return error
   if ( topReq == NULL || topReq->getStatus() != StringTable::OK ) {
      res = 1;
      if ( topReq == NULL ) {
         res = 2;
      } else if ( topReq->getStatus() == StringTable::TIMEOUT_ERROR ) {
         res = 2;
      } else if ( topReq->getStatus() == 
                  StringTable::INTERNAL_SERVER_ERROR )
      {
         res = 1;
      }
      return res;
   }

   LangTypes::language_t language = 
      ItemTypes::getLanguageCodeAsLanguageType( lang );

   // Make sorted top region list
   ConstTopRegionMatchesVector sortedRegions;
   for ( uint32 i = 0 ; i < topReq->getNbrTopRegions() ; ++i ) {
      sortedRegions.push_back( topReq->getTopRegion( i ) );
   }
   // Sort the top regions
   std::sort( sortedRegions.begin(), sortedRegions.end(),
              topRegionMatchCompareLess( language ) );

   const TopRegionMatch* regionMatch = NULL;

   for ( uint32 i = 0 ; i < sortedRegions.size() ; ++i ) {
      regionMatch = sortedRegions[ i ];
      const char* regionName = regionMatch->getNames()->getBestName( 
         language )->getName();
      if ( thread->checkUserRegionAccess( 
              regionMatch->getID(), user, urmask ) ) 
      {
         data.addUint32( regionMatch->getID() );
         data.addUint32( regionMatch->getType() );
         data.addString( regionName, thread->clientUsesLatin1() );
      } // End if User has access to region
   }

   return res;
}


int
NavParserThreadGroup::getTopRegionCRC( 
   ParserThreadHandle thread, const UserUser* user,
   StringTable::languageCode lang, UserEnums::URType urmask, uint32& crc )
{
   NParam data( 0 );
   int res = 0;

   res = getTopRegionData( thread, user, lang, urmask, data );
   if ( res == 0 ) {
      crc = MC2CRC32::crc32( data.getBuff(), data.getLength() );
   }

   return res;
}

const ClientSetting* 
NavParserThreadGroup::getSetting( const NParamBlock& params, 
                               ParserThreadHandle thread ) const
{
   MC2String clientType = "unknown";
   MC2String clientTypeOptions = "";
   if ( params.getParam( 4 ) != NULL ) {
      clientType = params.getParam( 4 )->getString(
         thread->clientUsesLatin1());
   }
   if ( params.getParam( 5 ) != NULL ) {
      clientTypeOptions = params.getParam( 5 )->getString(
         thread->clientUsesLatin1());
   }
   
   return getSetting( clientType.c_str(), clientTypeOptions.c_str() );
}


bool
NavParserThreadGroup::saveMapRoute( const char* fileName, 
                                    RouteRequest* rr )
{
   RouteReplyPacket* rrp = NULL;
   uint32 dataLength;
   char *data;

   if ( rr != NULL && rr->getRouteReplyPacket() != NULL ) {
      rrp = const_cast< RouteReplyPacket* >( rr->getRouteReplyPacket() );
      dataLength = RouteReplyPacket::getMaxSimDataLength(rrp);

      if ( dataLength <= 0 ) {
         mc2log << warn << "NPTG::saveMapRoute: No data for simulate "
                << "route!" << endl;
         return false;
      }
      mc2log << info << "NPTG::saveMapRoute: got " << dataLength
             << " bytes of data." << endl;
      data = new char[ dataLength + 1 ];

      if ( !data ) {
         mc2log << warn << "NPTG::saveMapRoute: Failed to allocate data"
                << endl;
         return false;
      }
      mc2log << info << "NPTG::saveMapRoute: Allocated data, saving "
             << "route..." << endl;

      /* Get data for simulation. */
      RouteReplyPacket::printSimulateRouteData( rrp, data, dataLength );

      /* Save data to file. */
      int filenamelen = strlen( fileName ) + 20;
      char* idfilename = new char[ filenamelen ];
      uint32 routeId = rr->getRouteID();
      uint32 routeTime = rr->getRouteCreateTime();
      sprintf( idfilename, "%s.%u-%u", fileName, 
               routeTime, routeId );
      mc2log << info << "NPTG::saveMapRoute: Will save data as \""
             << fileName << "\" and \""
             << idfilename << "\"." << endl;
      int aflags = O_CREAT|O_TRUNC|O_WRONLY;
      int amode = S_IRWXU|S_IRGRP|S_IROTH;
      int writelen = strlen( data );
      int fd = open( idfilename, aflags, amode );
      if ( fd < 0 ) {
         mc2log << warn << "NPTG::saveMapRoute: Error opening file: "
                << idfilename << " - "
                << strerror(errno) << endl;
      } else {
         int res = write( fd, data, writelen );
         if ( res < 0 ) {
            mc2log << warn << "NPTG::saveMapRoute: Error writing to file: "
                   << idfilename << " - "
                   << strerror(errno) << endl;
         }
         if ( close( fd ) < 0 ) {
            mc2log << warn << "NPTG::saveMapRoute: Error closing file: "
                   << idfilename << " - "
                   << strerror(errno) << endl;
         }
      }

      fd = open( fileName, aflags, amode );
      if ( fd < 0 ) {
         mc2log << warn << "NPTG::saveMapRoute: Error opening file: " 
                << fileName << " - "
                << strerror(errno) << endl;
      } else {
         int res = write( fd, data, writelen );
         if ( res < 0 ) {
            mc2log << warn << "NPTG::saveMapRoute: Error writing to file: "
                   << fileName << " - "
                   << strerror(errno) << endl;
         }
         if (close (fd) < 0) {
            mc2log << warn << "NPTG::saveMapRoute: Error closing file: "
                   << fileName << " - "
                   << strerror(errno) << endl;
         }  
      }  
      
      delete[] idfilename;
      delete[] data;
   } // End if have route and reply

   return true;
}
