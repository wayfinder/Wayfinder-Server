/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "mc2stringstream.h"
#include "StatisticsPacket.h"
#include "SupervisorStorage.h"
#include "SupervisorDisplay.h"
#include "DatagramSocket.h"
#include "STLStringUtility.h"
#include "PropertyHelper.h"
#include "StandardReader.h"
#include "ServerPort.h"
#include "NetUtility.h"
#include "StringSearchUtility.h"

using namespace ServerPort;

const moduletype_t SupervisorStorage::modules[] = {
   MODULE_TYPE_TRAFFIC,
   MODULE_TYPE_MAP,
   MODULE_TYPE_ROUTE,
   MODULE_TYPE_SEARCH,
   MODULE_TYPE_USER,
   MODULE_TYPE_GFX,
   MODULE_TYPE_EXTSERVICE,
   MODULE_TYPE_TILE,
   MODULE_TYPE_INVALID, // Must be last.
};

ModuleInfo::ModuleInfo(Packet *p,  DatagramReceiver* sock,
                       bool dont_show_loaded_mapsids)
      : m_originAddr(p->getOriginIP(),  p->getOriginPort()),
        m_dont_show_loaded_mapsids( dont_show_loaded_mapsids )
{
   m_lastHeartBeatTime = TimeUtility::getCurrentTime() - 5*HEARTBEAT_RATE -1;
   this->timeStamp = TimeUtility::getCurrentTime();   
   this->moduleType = getAvailablePort( sock->getPort() );
   m_createTime = TimeUtility::getRealTime();
           
   uint16 port = sock->getPort();
   uint16 mapSet = (port >> 13) & 0x7;
   // Remove mapset
   port = moduleType & 0x1FFF;
   
   // This is _wrong_! Must be changed so that it compiles.
   if ( MultiCastProperties::getTypeFromPort( port ) !=
        MODULE_TYPE_MAX_MODULE_NUMBER )
   {
      waitTime = 5*HEARTBEAT_RATE;
   } else {
      // Mabye a server?
      if ( (moduleType >= NAVIGATOR_FIRST_PORT) &&
           (moduleType < DEFAULT_FIRST_PORT+100)) {
         waitTime = 60000; // 60 seconds
      } else {
         // Unknown sender
         waitTime = 5 * HEARTBEAT_RATE;
      }
   }
   mc2stringstream strstr;
   strstr << m_originAddr << ends;
   m_hostName = strstr.str();
   bool lookup = true;
   try {
      lookup = PropertyHelper::get<bool>( "HOSTNAME_LOOKUP" ) ;
   } catch ( const PropertyException&  ) {
      // ignore
   }

   if ( lookup ) {

      MC2String nameBuf = NetUtility::getHostName( m_originAddr.getIP() );
      if ( ! nameBuf.empty() ) {
         m_hostName.clear();
         int len = nameBuf.size();
         // TODO: rewrite...
         for( int i = 0; i < len; ++i ) {
            if ( nameBuf[i] == '\0' )
               break;
            if ( nameBuf[i] == '.' )
               break;
            m_hostName += nameBuf[i];
         }
         char port[1024];
         sprintf(port, "%u", m_originAddr.getPort());
         m_hostName += MC2String(":") + port;
      }
   }



   if ( mapSet != 0 ) {
      char tmp[20];
      sprintf( tmp, "%u", mapSet );
      extraModuleType.append( tmp );
   }

   m_moduleTypeAsString = MC2String() + getModuleTypePrefix()+
      extraModuleType+" ";
}


ModuleInfo::ModuleInfo() 
      : moduleType( 0 ), m_lastHeartBeatTime( 0 ), m_originAddr( 0, 0 ),
        m_dont_show_loaded_mapsids( false )
{
}


const char*
ModuleInfo::getModuleTypeAsString() const
{
   return m_moduleTypeAsString.c_str();   
}

const char*
ModuleInfo::getModuleTypePrefix() const
{   
   // Remove mapset
   uint16 port = moduleType & 0x1FFF;

   moduletype_t type = MultiCastProperties::getTypeFromPort( port );
   if ( type != MODULE_TYPE_MAX_MODULE_NUMBER ) {
      return ModuleTypes::moduleTypeToShortString( type );
   }

   mc2dbg4 << "[ModuleInfo]: port " << port << " not found" << endl;
   
   {
      // Mabye a server?
      if ( (moduleType >= NAVIGATOR_FIRST_PORT) &&
           (moduleType < DEFAULT_FIRST_PORT+100)) {
         if (moduleType < TRISS_FIRST_PORT)
            return ("Navigator-server ");
          else if (moduleType < DEFAULT_FIRST_PORT)
            return ("TRISS-server  ");
         else
            return ("Default-server   ");
      } else {
         static char str[20];
         sprintf( str, "Unknown @ %5d ", port );
         return ( str );
      }
   }
}

const char*
ModuleInfo::getLoadedMapsAsString() const
{
   return m_loadedMaps.c_str();
}

const char*
ModuleInfo::getStatisticsAsString() const
{
   return m_statisticsValue.c_str();
}

const char*
ModuleInfo::getUserName() const {
   return m_mapStats.getUserName();
}

int
ModuleInfo::getPercentMemUse() const {
   return m_percentMemUse;
}


uint32
ModuleInfo::getQueueLength() const {
   return m_mapStats.getQueueLength();
}

uint32
ModuleInfo::getProcessTime() const {
   return m_mapStats.getProcessTime();
}


void 
ModuleInfo::updateStatStrs() {
   char statisticsValue[ 100 ];
   mc2stringstream strstr;
   strstr << "(";
   if ( m_dont_show_loaded_mapsids ) {
      if ( m_mapStats.getNbrOfMaps() > 0 ) {
         strstr << m_mapStats.getNbrOfMaps() << " maps ";
      }
   } else {
      m_mapStats.printAllWithStatus( strstr, MapElement::LOADED,
                                     "", " " );
   }
   m_mapStats.printAllWithStatus( strstr, MapElement::TOLD_TO_LOAD,
                                  "(told to ld ", ") " );
   m_mapStats.printAllWithStatus( strstr, MapElement::LOADING,
                                  "(loading ", ") " );
   m_mapStats.printAllWithStatus( strstr, MapElement::TOLD_TO_DELETE,
                                  "(told to delete ", ") " );
   m_mapStats.printAllWithStatus( strstr, MapElement::DELETING,
                                  "(deleting ", ")" );
   strstr << ")" << ends;
   m_loadedMaps = strstr.str();
   sprintf( statisticsValue, "%3i/%c", m_mapStats.getQueueLength(),
            m_mapStats.jobThreadWorking() ? 'R' : 'S' );
   m_statisticsValue = statisticsValue;
   m_percentMemUse = (uint32)(double(100.0*m_mapStats.calcUsedMem())/
                              double(m_mapStats.getOptMem()));
}


void 
ModuleInfo::setLoadedMaps(StatisticsPacket *sp)
{
   if (sp == NULL) {
      m_loadedMaps = "(?)";
      m_statisticsValue  = " ? ";
   } else {
      sp->getMapStatistics( m_mapStats );
      updateStatStrs();
   }
   // Set module name
   if ( (TimeUtility::getCurrentTime() - m_lastHeartBeatTime ) >
        HEARTBEAT_RATE * 5 ) {
      m_moduleTypeAsString = MC2String() + getModuleTypePrefix()+
         extraModuleType+" ";
   } else {
      m_moduleTypeAsString = MC2String() + getModuleTypePrefix()+
         extraModuleType+"*";
   }
   timeStamp = TimeUtility::getCurrentTime();
}

void
ModuleInfo::heartbeatRecv()
{
   mc2dbg8 << "[MI]: heartbeatRecv" << endl;
   timeStamp = 
   m_lastHeartBeatTime = TimeUtility::getCurrentTime();
}

void 
ModuleInfo::set_dont_show_loaded_mapsids( bool val ) {
   m_dont_show_loaded_mapsids = val;
}


bool 
ModuleInfo::operator == ( const ModuleInfo& o ) const {
   return strcmp( getModuleTypePrefix(), o.getModuleTypePrefix() ) == 0;
}


bool 
ModuleInfo::operator != ( const ModuleInfo& o ) const {
   return !(*this == o);
}


ModuleInfo& 
ModuleInfo::operator += ( const ModuleInfo& o ) {
   // Add to this
   char* tmpPtr = NULL;
   uint32 nbr = strtoul( m_hostName.c_str(), &tmpPtr, 0 );
   if ( tmpPtr == NULL || *tmpPtr != '\0' ) {
      nbr = 1;
   }
   m_hostName.clear();
   STLStringUtility::uint2str( nbr + 1, m_hostName );

   
   m_mapStats += o.m_mapStats;
   // Too many bytes to add up in mem-calc
   uint32 percentMemUse = m_percentMemUse + 
      (uint32)(double(100.0*o.m_mapStats.calcUsedMem())/
               double(o.m_mapStats.getOptMem()));
   updateStatStrs();
   m_percentMemUse = percentMemUse;

   return *this;
}


bool
ModuleInfo::collateMatches( const CollateSet& s ) const {
   // m_moduleTypeAsString exact almost strip * and space
   MC2String t( StringSearchUtility::stripStrangeChars( m_moduleTypeAsString ) );
   if ( s.find( t ) != s.end() ) {
      return true;
   }

   // m_hostName substr matches
   // userName substr matches
   MC2String u( m_mapStats.getUserName() );
   for ( CollateSet::const_iterator it = s.begin() ; it != s.end() ; ++it )
   {
      if ( strstr( m_hostName.c_str(), (*it).c_str() ) != NULL ||
           strstr( u.c_str(), (*it).c_str() ) != NULL ) 
      {
         return true;
      }
   }

   return false;
}


inline
bool
ModuleInfo::matchesPacket(Packet *p,  DatagramReceiver* sock)
{
   if(p == NULL) {
     cerr << "ModuleInfo::matchesPacket: p==NULL" << endl;
      return false;
   } else {
      if (m_originAddr.getIP() == p->getOriginIP() &&
          m_originAddr.getPort() == p->getOriginPort() ) {
         
         if (p->getSubType() == Packet::PACKETTYPE_STATISTICS) {
            setLoadedMaps((StatisticsPacket*) p);            
         } else if ( p->getSubType() == Packet::PACKETTYPE_HEARTBEAT) {
            heartbeatRecv();
         }
         timeStamp = TimeUtility::getCurrentTime();
         
         this->moduleType = getAvailablePort( sock->getPort() );
         return (true);
      } else {
         return (false);
      }
   }
}


uint16
ModuleInfo::getAvailablePort(uint16 port ) 
{
   // Remove mapset
   port = port & 0x1FFF;

   moduletype_t modu = MultiCastProperties::getTypeFromPort( port );
   if ( modu == MODULE_TYPE_MAX_MODULE_NUMBER ) {
      return port;
   } else {
      return MultiCastProperties::getPort( modu, false );
   }
}

bool
ModuleInfo::tooOld()
{
   uint32 currentTimeDiff = TimeUtility::getCurrentTime() - timeStamp;
   if(currentTimeDiff > waitTime ) {
      //cerr << "TimeDiff=" << currentTimeDiff << endl;
      return true;
   }
   return false;
}

time_t 
ModuleInfo::getCreateTime() const {
   return m_createTime;
}

SupervisorStorage::SupervisorStorage( bool dont_show_loaded_mapsids ) 
      :  m_dont_show_loaded_mapsids( dont_show_loaded_mapsids )
{
}

SupervisorStorage::~SupervisorStorage()
{
}


void
SupervisorStorage::addObserver(SupervisorDisplay* s)
{
   m_observers.push_back( s );
}

void
SupervisorStorage::notify()
{
   for(uint32 i = 0; i < m_observers.size(); i++) 
      m_observers[i]->update();
}


void
SupervisorStorage::handlePacket(Packet* p, DatagramReceiver* sock)
{
   if ( p != NULL && sock != NULL ) {
      // Find matching moduleInfo
      ModuleInfo* foundMod = NULL;
      for( vector<ModuleInfo>::iterator it = m_modulesRunning.begin();
           it != m_modulesRunning.end();
           ++it ) {
         if( it->matchesPacket(p, sock) ) {
            foundMod = &(*it);
            break;
         } 
      }

      // Update found moduleInfo or create a new one if not found
      if ( foundMod ) {
         foundMod->setTimeStamp();
      } else {
         if ( p->getSubType() == Packet::PACKETTYPE_STATISTICS ) {
            m_modulesRunning.push_back(
               ModuleInfo( p, sock, m_dont_show_loaded_mapsids ) );
            handlePacket(p, sock);
         }
         return;
      }
   }
   
   // Remove old modules from list
   for( vector<ModuleInfo>::iterator it = m_modulesRunning.begin();
        it != m_modulesRunning.end();
         ) {
      if ( it->tooOld() ) {
         // Remove 
         it = m_modulesRunning.erase (it);
      } else {
         // Keep
         ++it;
      }
   }

   // Tell the listeners about the wonderful new state.
   notify();
}


void 
SupervisorStorage::flip_set_dont_show_loaded_mapsids() {
   m_dont_show_loaded_mapsids = !m_dont_show_loaded_mapsids;
   // Update all ModuleInfos too
   for( vector<ModuleInfo>::iterator it = m_modulesRunning.begin() ;
        it != m_modulesRunning.end() ; ++it ) 
   {
      it->set_dont_show_loaded_mapsids( m_dont_show_loaded_mapsids );
   }
}
