/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Component.h"
#include "Readline.h"
#include "Properties.h"
#include "StringUtility.h"
#include "TimeUtility.h"
#include "CommandlineOptionHandler.h"
#include "FDSelectable.h"
#include "SysUtility.h"

#include <memory>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

Component::Component( const char* name,
                      CommandlineOptionHandler* handler ):
   m_cmdlineOptHandler( handler ),
   m_componentName( name ? name : "" ),
   m_shutdown( false ),
   m_noReadLine( true ),
   m_startTime( TimeUtility::getRealTime() )
{
   setup();
}

Component::Component(const char* name, int argc, char* argv[]) 
   : m_cmdlineOptHandler( new CommandlineOptionHandler( argc, argv ) ),
     m_componentName( name ? name : "" ),
     m_shutdown( false ),
     m_noReadLine( true ),
     m_startTime( TimeUtility::getRealTime() ) 
{
   setup();
}

Component::~Component() {
   mc2dbg << "[Component] dying." << endl;
   // Can't to this in modules, they don't work well with 
   // CommandlineOptionHandler's way of holding references to pointers
   // sent to it.
   if ( m_componentName.find( "Module" ) == MC2String::npos ||
        m_componentName.find( "Map" ) == MC2String::npos ) {
      m_cmdlineOptHandler->deleteTheStrings();
   }
   mc2dbg << "[Component] dead." << endl;
}

void Component::parseCommandLine() throw (ComponentException) {
   bool readLine = false;
   m_cmdlineOptHandler->addOption( "-R", "--readline",
                                   CommandlineOptionHandler::presentVal,
                                   1, &readLine, "F",
                                   "Use command prompt interface." );

   if ( ! m_cmdlineOptHandler->parse() ) {
      throw ComponentException( m_componentName + 
                                ": Error on commandline! (-h for help)" );
   }

   if ( ! Properties::setPropertyFileName( m_cmdlineOptHandler->
                                           getPropertyFileName() ) ) {
      throw ComponentException( MC2String("No such file or directory: '") +
                                m_cmdlineOptHandler->getPropertyFileName() + 
                                "'"  );
   }

   m_noReadLine = ! readLine;
}

void Component::setup() {
   // Initialize properties
   Properties::getProperty( "" );
}      


void Component::init()
{
   mc2log << m_componentName << " starting..." << endl;  
}

int Component::handleCommand(const char* input)
{
   MC2String token = input ? input : "";
   // remove everything after first space " "
   MC2String::size_type pos = token.find_first_of( " " );
   if ( pos != MC2String::npos ) {
      token = token.substr( 0, pos );
   }

   if ( token ==  "help" ) {

      cout << m_componentName << " help" << endl 
           << endl << "  help         - This help"
           << endl << "  quit         - quit immediately [exit(0)]" 
           << endl << "  shutdown     - Shutdown this component gracefully "
                                       "(also Ctrl-D)" 
           << endl << "  abort        - Abort this component (immediate "
                                       "shutdown with core dump)"
           << endl << "  config       - Configuration details" 
           << endl << "  status       - Overall status of this component" 
           << endl << "  heapstatus   - Displays information about the heap"
           << endl;
      // Let subclasses print their help commands
      helpCommands();

   } else if ( token == "quit" ) { 
      mc2log << warn << "Component exiting without clean shutdown!" << endl;
      exit(0);
   } else if ( token == "shutdown" ) { 
      mc2log << info << "Initiating component shutdown" << endl;
      return -1;
   } else if ( token == "config" ) {

      cout << m_componentName << " configuration" 
           << endl   << "  Property file path : " 
                     << Properties::getPropertyFileName() << endl;

   } else if ( token == "status" ) {

      uint32 m_uptime = TimeUtility::getRealTime() - m_startTime;
      cout << m_componentName << " status" 
           << endl << "  pid          : " << getpid()
           << endl << "  uptime       : ";
      if (m_uptime / 86400 > 0) {
           cout << m_uptime / 86400 << " day(s), ";
           m_uptime = m_uptime % 86400;
      }
      if (m_uptime / 3600 > 0) {
           cout << m_uptime / 3600 << " hour(s), ";
           m_uptime = m_uptime % 3600;
      }
      if (m_uptime / 60 > 0) {
           cout << m_uptime / 60 << " minute(s), ";
           m_uptime = m_uptime % 60;
      }
      cout << m_uptime << " second(s)"
           << endl << "  CPU usage    : " << "N/A"
           << endl;
   } else if ( token == "heapstatus" ) {
      SysUtility::printHeapStatus( cout );
   } else if ( token == "abort" ) {
      mc2log << fatal << "Component aborted by user" << endl;
      PANIC("Aborted by user!", "");
   } else  {
      cout << "Unknown command. Use the command 'help' to get help" << endl;
   }

   return 1;
}

void
Component::helpCommands() {
   // For subclasses to expand.
}

namespace {
void setBlock( bool state ) {
   struct termios ttystate;
   tcgetattr( fileno( stdin ), &ttystate );
   if ( !state ) {
      ttystate.c_lflag &= ~ICANON;
      ttystate.c_cc[VMIN] = 1;
   } else {
      ttystate.c_lflag |= ICANON;
   }
   tcsetattr( fileno( stdin ), TCSANOW, &ttystate );
}
}
void
Component::gotoWork(ISABThreadInitialize& threadInit)
{
   char* input = NULL;
   int cmdStatus = 0; // Compiler complains if uninitialized
   MC2String prompt = m_componentName + "> ";
#ifndef NO_USE_COMPONENT_UI
   auto_ptr<Readline> readl( new Readline( prompt.c_str(), true, NULL ) );

   uint32 pos = 0;

   // do non-blocking stdin
   ::setBlock( false );

   if ( fcntl( fileno( stdin ), F_SETFL, O_NONBLOCK ) == -1 ) {
      mc2log << error << "[Component] Failed to set non-blocking stdin" 
             << endl;
   }

   FDSelectable stdinSelectable( fileno( stdin ) );
   m_selector.addSelectable( &stdinSelectable,
                             true, false,
                             false ); // no notify

   if ( m_noReadLine ) {
      input = new char[2048];
      input[ 0 ] = '\0';
   } else {
      // set non-blocking io
      readl->setBlocking( false );
   }
#endif
   m_selector.addSelectable( &m_shutdownPipe,
                             true, false,
                             false ); // no notify
   
   while (!m_shutdown) {

      // select on stdin and shutdown pipe

      SelectableSelector::selSet readReady, writeReady;
      if ( m_selector.select( -1, readReady, writeReady ) != 0 ) {
         mc2log << error << "[Component] Failed to select!" << endl;
      }
      // readd to selectable
      m_selector.addSelectable( &m_shutdownPipe,
                                true, false,
                                false ); // no notify
#ifndef NO_USE_COMPONENT_UI
      m_selector.addSelectable( &stdinSelectable,
                                true, false,
                                false ); // no notify
      

      if ( readReady.find( &stdinSelectable ) != readReady.end() ) {
         if ( m_noReadLine ) {
            int readChar = fgetc( stdin );

            if ( readChar == 0x04 ) { // Control-D
               strcpy( input, "shutdown\n" );
            } else if ( readChar != '\n' ) { 
               input[pos] = static_cast<char>( readChar & 0xFF );
               if ( pos < 2046 ) {
                  pos++;
               }
               continue;
            } else {
               input[ pos ] = '\n'; input[ pos + 1 ] = 0;
               pos = 0;
            }
         } else {
            
            input = (char*)readl->getInput();
         }
      }

      if ( input == NULL ) {
         if ( m_noReadLine ) {
            cmdStatus = handleCommand( "shutdown" );
         }
      } else if (input[0] != '#') { // skip comments
         MC2String newInput = input;
         newInput[ newInput.size() - 1] = '\0'; // get rid of newline char
         cmdStatus = handleCommand( newInput.c_str() );
      }

      if ( cmdStatus < 0 ) {
         m_shutdown = true;
      }
#endif

      if ( readReady.find( &m_shutdownPipe ) != readReady.end() ) {
         cmdStatus = handleCommand("shutdown");
         if ( cmdStatus < 0 ) {
            m_shutdown = true;
            break;
         }
      }
   }

   if ( m_noReadLine && input ) {
      delete[] input;
   }
   m_shutdownPipe.notify();

   threadInit.waitTermination();
   mc2log << info << "[Component] All threads have terminated, exiting." 
          << endl;
}

int
Component::splitCommand( vector<MC2String>& res, const char* input )
{
   char curQuote = '\0';
   const char* quoteChars = "'\'\"";
   const char* curStart = input;
   MC2String curString;
   for ( const char* curChar = input;
         *curChar != '\0';
         ++curChar ) {
      if ( *curChar == ' ' && curQuote == 0 ) {
         if ( ! curString.empty() ) {
            res.push_back( curString );
         }
         curString = "";
         curStart = curChar + 1;
      } else if ( strchr( quoteChars, *curChar ) != NULL && curQuote == 0 ) {
         // Start quoting
         curQuote = *curChar;
      } else if ( *curChar == curQuote ) {
         // Stop quoting
         curQuote = '\0';
      } else {
         // Normal character or quoted space
         curString += *curChar;
      }
   }
   if ( ! curString.empty() ) {
      res.push_back( curString );
   }

#if 0
   for( vector<MC2String>::const_iterator it = res.begin();
        it != res.end();
        ++it ) {
      cout << MC2CITE( *it ) << ' ';
   }
   cout << endl;
#endif

   return res.size();
}
