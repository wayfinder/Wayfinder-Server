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
#include "SupervisorDisplay.h"
#include "ReadlineHelper.h"
#include "SupervisorReceiver.h"

#include <iomanip>

SupervisorDisplay::SupervisorDisplay( SupervisorStorage* s, 
                                      SupervisorReceiver* r )
      : storage(s), m_receiver( r ), m_alertQueueSize( MAX_UINT32 ),
        m_showQueue( 0 ), m_alertOn( true ), m_collate( false ), 
        m_average( true ), m_creationTime( TimeUtility::getRealTime() ),
        m_stabilize( false )
{
}

TextDisplay::TextDisplay( SupervisorStorage* s, SupervisorReceiver* r )
      : SupervisorDisplay( s, r )
{
   m_lastUpdateTime = TimeUtility::getCurrentTime();
}


void printModInfo( const ModuleInfo* modInfo, char* tmpStr,
                   uint32 showQueue, mc2stringstream& strstr, bool avg ) 
{
   if ( modInfo->getQueueLength() >= showQueue ) {
      // If modInfo->getHostName() is nbr then make some avg.s
      MC2String statStr( modInfo->getStatisticsAsString() );
      uint32 processTime = modInfo->getProcessTime();
      float load1 = modInfo->getLoad1();
      float load5 = modInfo->getLoad5();
      float load15 = modInfo->getLoad15();
      uint32 percentMemUse = modInfo->getPercentMemUse();
      uint32 nbr = atoi( modInfo->getHostName() );
      if ( nbr > 0 && avg ) {
         // Make avgs
         processTime /= nbr;
         load1 /= nbr;
         load5 /= nbr;
         load15 /= nbr;
         percentMemUse /= nbr;
         char tmpStr[2048];
         sprintf( tmpStr, "%6.2f/%05.2f", 
                  float(modInfo->getQueueLength()) / nbr,
                  float(modInfo->getJobThreadWorking()) / nbr );
         statStr = tmpStr;
      }

      sprintf( tmpStr, "%-20.20s %-5.5s %-8.8s %12.12s %8d "
               " %5.2f, %5.2f, %5.2f "
               "%5d  ",
               modInfo->getHostName(),
               modInfo->getModuleTypeAsString(),
               modInfo->getUserName(),
               statStr.c_str(),
               processTime,
               load1, load5, load15, 
               percentMemUse );
      strstr  << tmpStr << modInfo->getLoadedMapsAsString() << endl;
   }
}


void 
TextDisplay::update()
{
   // Do not update too often.
   if ( ( TimeUtility::getCurrentTime() - m_lastUpdateTime ) < 1000 ||
        /* XXX: Or check if display is same for more than one second */
        (m_stabilize && TimeUtility::getRealTime() - m_creationTime < 3) ) {
      return;
   }
   
   m_lastUpdateTime = TimeUtility::getCurrentTime();
   
   mc2stringstream strstr;
   mc2stringstream datestr;
   bool hasInput = false;

#ifdef __unix
   // Check if we have input
   fd_set rfds;
   struct timeval tv;
   int retval;

   /* Watch stdin (fd 0) to see when it has input. */
   system( "stty cbreak </dev/tty >/dev/tty 2>&1" );
   FD_ZERO( &rfds );
   FD_SET( 0, &rfds );
   tv.tv_sec = 0;
   tv.tv_usec = 0;
   retval = select( 1, &rfds, NULL, NULL, &tv );

   if ( retval ) {
      hasInput = true;
      // Get it
      char in = 0;
      retval = read( 0, &in, 1 );
      system( "stty -cbreak </dev/tty >/dev/tty 2>&1" );

      if ( retval == 1 ) {
         if ( in == 'm' ) {
            storage->flip_set_dont_show_loaded_mapsids();
         } else if ( in == 'w' ) {
            // Turn off/on warning
            m_alertOn = !m_alertOn;
         } else if ( in == 'c' ) {
            // Flip collate
            m_collate = !m_collate;
         } else if ( in == 'a' ) {
            // Flip average
            m_average = !m_average;
         } else if ( in == 'k' || in == 's' || in == 'u' ) {
            // Move to line 3, colum 0, clear line
            cout << char(0x1b) << "[3;0f" << char(0x1b) << "[K";
            ReadlineHelper r( "", false, NULL );

            if ( in == 's' ) {
               // MapSet
               // Delete start receivers in packetReceiver from receiver
               // vector in Supervisor.cpp.
               // To be implemented
               uint32 t = r.getLong( "MapSet", "2147483647" );
               m_receiver->setAllowedMapSets( t );
            } else if ( in == 'k' || in == 'u' ) {
               // Collapse
               const char* pr = "Collate";
               if ( in == 'u' ) {
                  pr = "Uncollate";
               }
               MC2String c = r.getText( pr, "" );
               if ( !c.empty() ) {
                  if ( in == 'k' ) {
                     m_collateSet.insert( c );
                  } else {
                     if ( c == "clear" ) {
                        m_collateSet.clear();
                     } else {
                        m_collateSet.erase( c ); 
                     }
                  }
               }
            }
         } else if ( in == 'q' ) {
            kill( 0, SIGINT );
         }
      }
   }
   if ( !hasInput ) {
      system( "stty -cbreak </dev/tty >/dev/tty 2>&1" );
   }
#endif
  
   // Go through the list and print the contents.
   vector<ModuleInfo> runningModules;
   storage->getRunningModules(runningModules);
   // The longest queue
   uint32 maxQ = 0;
   
   time_t tt;
   struct tm* tm;

   ::time(&tt);
   tm = localtime(&tt);
   char timeBuf[50];
   sprintf(timeBuf, "%04d-%02d-%02d %02d:%02d:%02d ",
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
           tm->tm_hour, tm->tm_min, tm->tm_sec);
   
   datestr << "Running units " << timeBuf << endl
           << "**************" << MC2String(strlen(timeBuf), '*' );
   if ( !m_alertOn && m_alertQueueSize < MAX_UINT32 ) {
      datestr << " [Alert off]";
   }
   datestr << endl;
   strstr << endl;
   char heading[2048];
   sprintf(heading, "%-20.20s %-5.5s %-8.8s %12.12s %8.8s "    
           "%21.21s"
           "%5.5s  %s",
                    "Host",   "Type","User","Queue/Run ","Avg. PT", 
           "Load average       ",
           "%Mem", "Loaded Maps");
   strstr << heading << endl;
   strstr << "-------------------------------------"
             "-----------------------------" "--------"
          << "-----------------------" << endl;  

   
   ModuleInfo in;
   char tmpStr[ 4096 ];

   for(vector<ModuleInfo>::const_iterator it = runningModules.begin();
       it != runningModules.end();
       ++it ) {
      const ModuleInfo* modInfo = &(*it);
      if ( modInfo->getQueueLength() > maxQ ) {
         maxQ = modInfo->getQueueLength();
      }

      bool collate = false;
      bool forcePrint = false;
      if ( m_collate ) {
         if ( in == *modInfo ) {
            collate = true;
         }
         if ( modInfo->collateMatches( m_collateSet ) ) {
            collate = false; 
            forcePrint = true;
         }

         if ( collate ) {
            if ( in != ModuleInfo() ) {
               in += *modInfo;
            } else {
               in = *modInfo;
            }
         }
         if ( it == (runningModules.end() -1) ) {
            // Show it
            collate = false; 
         }
      }
      
      if ( !collate ) {
         if ( in != ModuleInfo() && it != runningModules.begin() ) {
            printModInfo( &in, tmpStr, m_showQueue, strstr, m_average );
         }
         in = *modInfo;
         if ( forcePrint ) {
            printModInfo( &in, tmpStr, m_showQueue, strstr, m_average );
            in = ModuleInfo();
         }
      }

   }
   // Print last modInfo
   if ( !runningModules.empty() && !m_collate ) {
      printModInfo( &in, tmpStr, m_showQueue, strstr, m_average );
   }

   strstr << ends;
   datestr << ends;
   if ( maxQ >= m_alertQueueSize && m_alertOn ) {
      cout << "\a";
   }
#ifdef __unix
   MC2String dateStr = datestr.str();
   MC2String dataStr = strstr.str();
   bool changeDate = m_oldDisplayTime != dateStr;
   bool changeData = m_oldDisplay != dataStr;
   MC2String toPrint;

   if ( changeData || hasInput ) {
      // Clear the screen.
      cout << char(0x1b) << "[2J";
      cout << char(0x1b) << "[0;0f";
      m_oldDisplay = dataStr;
      m_oldDisplayTime = dateStr;
      toPrint = dateStr;
      toPrint.append( dataStr );
      cout << toPrint;
   } else if ( changeDate ) {
      // Only change date
      // Save cursor pos
      cout << char(0x1b) << "[s";
      // Move to line 0, colum 0, clear line
      cout << char(0x1b) << "[0;0f" << char(0x1b) << "[K";
      // Move to line 1, colum 0, clear line
      cout << char(0x1b) << "[1;0f" << char(0x1b) << "[K";
      // Move to line 0, colum 0
      cout << char(0x1b) << "[0;0f";
      // Print date lines
      toPrint = dateStr;
      cout << toPrint;
      // Return to saved cursor pos
      cout << char(0x1b) << "[u";
//      // Print something to get there
//      cout << endl << char(0x1b) << "[2A";
      m_oldDisplayTime = dateStr;
   }
#else
#ifdef __unix
   // Clear the screen.
   cout << char(0x1b) << "[2J";
   cout << char(0x1b) << "[0;0f";
#endif
   MC2String toPrint( datestr.str() );
   toPrint.append( strstr.str() );
   if ( toPrint != m_oldDisplay ) {
      cout << toPrint;
      m_oldDisplay = toPrint;
   }
#endif
}

