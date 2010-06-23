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

#include "UserModule.h"
#include "UserData.h"
#include "UserReader.h"
#include "UserProcessor.h"
#include "ModulePacketSenderReceiver.h"
#include "PacketQueue.h"
#include "Properties.h"
#include "PacketReaderThread.h"
#include "JobThread.h"
#include "FilePtr.h"
#include "CommandlineOptionHandler.h"
#include "PropertyHelper.h"
#include "Utility.h"

UserModule::UserModule( int argc, char* argv[] )
   : Module( MODULE_TYPE_USER,
             argc, argv, false ),
     m_noUserCache( false ), 
     m_noSqlUpdate( false )
{
   m_cmdlineOptHandler->
      addOption( "-a", "--file",
                 CommandlineOptionHandler::stringVal,
                 1, &m_userAddFilename, "",
                 "Add users from given file, where each user is on one "
                 "line like: \n"
                 "logonID <TAB> logonPasswd <TAB> \n"
                 "firstName <TAB> initials <TAB> lastName <TAB> \n"
                 "measurementSystem <TAB> language <TAB> \n"
                 "searchType <TAB> searchSubstring <TAB> \n"
                 "searchSorting <TAB> searchObject <TAB> \n"
                 "routingCostA <TAB> routingCostB <TAB> \n"
                 "routingCostC <TAB> routingCostD <TAB> \n"
                 "editMapRights <TAB> wapService <TAB> \n"
                 "htmlService <TAB> operatorService <TAB> \n"
                 "nbrCellulars <TAB> \n"
                 "(phoneNumber <TAB> model <TAB> smsService <TAB>) "
                 "* nbrCellulars");

   m_cmdlineOptHandler->
      addOption( "-o", "--allusersok",
                 CommandlineOptionHandler::presentVal,
                 1, &m_onlyUsersAllOK, "F",
                 "Only add users if all are ok.");

   m_cmdlineOptHandler->
      addOption( "-u", "--nousercache",
                 CommandlineOptionHandler::presentVal,
                 1, &m_noUserCache, "F",
                 "If not to use user cacheing, "
                 "default using cache." );

   m_cmdlineOptHandler->
      addOption( "-n", "--nosqlupdate",
                 CommandlineOptionHandler::presentVal,
                 1, &m_noSqlUpdate, "F",
                 "If not to update sql tables, "
                 "default is to update." );
}

UserModule::~UserModule() {

}

int
UserModule::handleCommand(const char* input) 
{
   // no own commands right now, add later
   return Module::handleCommand(input);
}

void
UserModule::init()
{
   m_senderReceiver.
      reset( new ModulePacketSenderReceiver( m_preferredPort,
                                             IPnPort( m_leaderIP,
                                                      m_leaderPort ),
                                             IPnPort( m_availIP, 
                                                      m_availPort) ) );

   m_packetReader.reset( new PacketReaderThread(
       static_cast<ModulePacketSenderReceiver&>(*m_senderReceiver )  ) );

   m_queue.reset( new PacketQueue );
   m_reader = new UserReader( m_queue.get(), m_senderReceiver->getSendQueue(),
                              m_loadedMaps.get(), m_packetReader.get(), 
                              m_senderReceiver->getPort(),
                              m_rank);   
   m_processor.reset( new UserProcessor( m_loadedMaps.get(), 
                                         m_noSqlUpdate, 
                                         m_reader->getLeaderStatus() ) );
   static_cast<UserProcessor*>( m_processor.get() )->setUseUserCache( 
      !m_noUserCache );
   m_jobThread = new JobThread( m_processor.get(), m_queue.get(),
                                m_senderReceiver->getSendQueue() );
   mc2dbg2 << "UserModule::init() done, going to Module::init()" << endl;

   Module::init();
}

int main(int argc, char *argv[])
try {

   ISABThreadInitialize init;
   PropertyHelper::PropertyInit propInit;

   UserModule* UM = new UserModule( argc, argv );

   UM->parseCommandLine();

   UM->init();
   UM->start();
   UM->gotoWork(init);

   return 0;

} catch ( const ComponentException& ex ) {
   mc2log << fatal << "Could not initialize UserModule component. " 
          << ex.what() << endl;
   return 1;
}

bool
UserModule::addUsers( const char* fileName, Queue *queue, bool allok ) {
   FileUtils::FilePtr users( fopen( fileName, "r" ) );
   
   if ( users.get() == NULL ) {
      mc2dbg1 << "UserModule can't open userfile: " 
             << fileName << endl;
      return false;
   }
   
   bool allisok = true;
   // UserUserData
   char* m_logonID;
   char* m_logonPasswd;
   char* m_firstname;
   char* m_initials;
   char* m_lastname;
   UserConstants::MeasurementType m_measurementSystem;
   StringTable::languageCode m_language;
   uint8 m_searchType;
   uint8 m_searchSubstring;
   uint8 m_searchSorting;
   uint16 m_searchObject;
   uint8 m_routingCostA;
   uint8 m_routingCostB;
   uint8 m_routingCostC;
   uint8 m_routingCostD;
   uint32 m_editMapRights;
   uint32 m_editDelayRights;
   uint32 m_editUserRights;
   bool m_wapService;
   bool m_htmlService;
   bool m_operatorService;
   // UserCellularData
   char* m_phoneNumber;
   char* m_model;
   bool m_smsService = false;


   int length = 0;
   char buff[4096];
   char* tmp;
   length = 0;
   bool ok = true;
   while ( length != -1 ) {
      length = Utility::readLine( users.get(), buff, 4096, "\r\n" );
      if ( length != -1 ) {
         if ( !ok ) {
            allisok = false;
         }
         ok = false;

         // Read UserUserData
         char* pos = buff;
         char* sep = strchr( buff, '\t' );
         if ( sep != NULL ) {
            *sep = '\0';   
            m_logonID = pos;
         } else {
            mc2dbg1 << "UserModule: userfile format error! 1#"
                   << buff << "#" << endl;
            continue;
         }
         pos = sep + 1;
         sep = strchr( pos, '\t' );
         if ( sep != NULL ) {
            *sep = '\0';   
            m_logonPasswd = pos;
         } else {
            mc2dbg1 << "UserModule: userfile format error! 2#"
                   << pos << "#" << endl;
            continue;
         }
         pos = sep + 1;
         sep = strchr( pos, '\t' );
         if ( sep != NULL ) {
            *sep = '\0';   
            m_firstname = pos;
         } else {
            mc2dbg1 << "UserModule: userfile format error! 3#"
                   << pos << "#" << endl;
            continue;
         }
         pos = sep + 1;
         sep = strchr( pos, '\t' );
         if ( sep != NULL ) {
            *sep = '\0';   
            m_initials = pos;
         } else {
            mc2dbg1 << "UserModule: userfile format error! 4#"
                   << pos << "#" << endl;
            continue;
         }
         pos = sep + 1;
         sep = strchr( pos, '\t' );
         if ( sep != NULL ) {
            *sep = '\0';   
            m_lastname = pos;
         } else {
            mc2dbg1 << "UserModule: userfile format error! 5#"
                   << pos << "#" << endl;
            continue;
         }
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            if ( strcmp( tmp, "IMPERIAL" ) == 0 ) {
               m_measurementSystem = 
                  UserConstants::MEASUREMENTTYPE_IMPERIAL;
            } else {
               m_measurementSystem = UserConstants::MEASUREMENTTYPE_METRIC; 
            }
         } else {
            mc2dbg1 << "UserModule: userfile format error! 6#"
                   << pos << "#" << endl;
            continue;
         }
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_language = StringTable::getLanguageCode( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 7#"
                   << pos << "#" << endl;
            continue;
         }
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_searchType = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 8#"
                    << pos << "#" << endl;
            continue;
         }         
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_searchSubstring = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 9#"
                    << pos << "#" << endl;
            continue;
         } 
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_searchSorting = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 10#"
                    << pos << "#" << endl;
            continue;
         }
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_searchObject = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 10#"
                    << pos << "#" << endl;
            continue;
         }
         
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_routingCostA = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 11#"
                    << pos << "#" << endl;
            continue;
         }  
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_routingCostB = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 12#"
                   << pos << "#" << endl;
            continue;
         }  
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_routingCostC = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 13#"
                    << pos << "#" << endl;
            continue;
         }  
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_routingCostD = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 14#"
                    << pos << "#" << endl;
            continue;
         }  
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_editMapRights = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 15#"
                    << pos << "#" << endl;
            continue;
         }          
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_editDelayRights = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 16#"
                   << pos << "#" << endl;
            continue;
         }
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            m_editUserRights = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 17#"
                    << pos << "#" << endl;
            continue;
         }
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            if ( strcmp( tmp, "t" ) == 0 )
               m_wapService = true; 
            else 
               m_wapService = false;
         } else {
            mc2dbg1 << "UserModule: userfile format error! 18#"
                    << pos << "#" << endl;
            continue;
         }         
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            if ( strcmp( tmp, "t" ) == 0 )
               m_htmlService = true; 
            else 
               m_htmlService = false;
         } else {
            mc2dbg1 << "UserModule: userfile format error! 19#"
                    << pos << "#" << endl;
            continue;
         } 
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            if ( strcmp( tmp, "t" ) == 0 )
               m_operatorService = true; 
            else 
               m_operatorService = false;
         } else {
            mc2dbg1 << "UserModule: userfile format error! 20#"
                    << pos << "#" << endl;
            continue;
         }         

         int32 nbrCellulars;
         pos = sep + 1;
         sep = strchr( pos, '\t' ); 
         if ( sep != NULL ) {
            *sep = '\0';   
            tmp = pos;
            nbrCellulars = atoi( tmp );
         } else {
            mc2dbg1 << "UserModule: userfile format error! 21#"
                    << pos << "#" << endl;
            continue;
         }

         UserCellular* cellulars[nbrCellulars];
         // Read Cellulardata
         int32 i = 0;
         for ( i = 0; i < nbrCellulars ; i ++ ) {
            pos = sep + 1;
            sep = strchr( pos, '\t' ); 
            if ( sep != NULL ) {
               *sep = '\0';   
               tmp = pos;
               if ( atoi( tmp ) == 0 ) {
                  mc2dbg1 << "UserModule: userfile format error! 22#"
                          << tmp << "# not phonenumber!" << endl;
                  continue;
               } else {
                  m_phoneNumber = tmp;
               }
            } else {
               mc2dbg1 << "UserModule: userfile format error! 23#"
                       << pos << "#" << endl;
               continue;
            }
            pos = sep + 1;
            sep = strchr( pos, '\t' ); 
            if ( sep != NULL ) {
               *sep = '\0';   
               tmp = pos;
               m_model = tmp;
            } else {
               mc2dbg1 << "UserModule: userfile format error! 24#"
                       << pos << "#" << endl;
               continue;
            }
            pos = sep + 1;
            sep = strchr( pos, '\t' ); 
            if ( sep != NULL ) {
               *sep = '\0';   
               tmp = pos;
               if ( strcmp( tmp, "t" ) == 0 )
                  m_smsService = true; 
               else 
                  m_smsService = false;
            } else {
               mc2dbg1 << "UserModule: userfile format error! 25#"
                       << pos << "#" << endl;
               continue;
            }

            cellulars[ i ] = new UserCellular( 0 );
            cellulars[ i ]->setPhoneNumber( m_phoneNumber );
            CellularPhoneModel* model = new CellularPhoneModel( 
               m_model, "", 10, UserConstants::EOLTYPE_AlwaysCRLF, 
               2, false, 0, 0,
               CellularPhoneModel::SMSCAPABLE_YES, 
               CellularPhoneModel::SMSCONCATENATION_NO,
               CellularPhoneModel::SMSGRAPHICS_NO,
               CellularPhoneModel::WAPCAPABLE_NO,
               "", 00, "Not a real phone" );
            cellulars[ i ]->setModel( model );
            // cellulars[ i ]->setSmsService( m_smsService );
         }
         if ( i != nbrCellulars ) {
            mc2dbg1 << "Not all cellulars ok" << endl;
            exit( 1 );
         }

         // All data read
         mc2dbg1 << "ALL DATA READ!" << endl;

         UserUser* user = new UserUser( 0 );
         user->setLogonID( m_logonID );
         user->setFirstname( m_firstname );
         user->setInitials( m_initials );
         user->setLastname( m_lastname );
         user->setMeasurementSystem( m_measurementSystem );
         user->setLanguage( m_language );
         user->setSearch_type( m_searchType );
         user->setSearch_substring( m_searchSubstring );
         user->setSearch_sorting( m_searchSorting );
         user->setSearchForTypes( m_searchObject );
         user->setSearchForLocationTypes( m_searchObject );
         user->setRouting_costA( m_routingCostA );
         user->setRouting_costB( m_routingCostB );
         user->setRouting_costC( m_routingCostC );
         user->setRouting_costD( m_routingCostD );
         user->setEditMapRights( m_editMapRights );
         user->setEditDelayRights( m_editDelayRights );
         user->setEditUserRights( m_editUserRights );
         user->setWAPService( m_wapService );
         user->setHTMLService( m_htmlService );
         user->setOperatorService( m_operatorService );
         user->setSMSService( m_smsService );
         user->setMunicipal( new uint32[0], 0 );
                                        
         AddUserRequestPacket* p = 
            new AddUserRequestPacket( 0, 0, 
                                      user, 
                                      m_logonPasswd, 0/*changerUIN*/ );
         for( int32 i = 0 ; i < nbrCellulars ; i++ ) {
            p->addElement( cellulars[ i ] );
         }

         delete user;
         for( int32 i = 0 ; i < nbrCellulars ; i++ ) {
            delete cellulars[ i ];
         }

         queue->enqueue( p );

         ok = true;
      }
   }
   if ( !ok ) {
      allisok = false;
   }

   if ( allok && !allisok ) {
      mc2dbg1 << "All users not ok, quiting." << endl;
      exit( 1 );
   }

   return true;
}


