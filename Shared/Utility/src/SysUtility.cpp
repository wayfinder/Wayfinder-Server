/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Setup some stuff
#ifdef ARCH_OS_LINUX
#define CAN_BACKTRACE
#else
#undef CAN_BACKTRACE
#endif

#ifdef CAN_BACKTRACE
// This is probably the wrong way to do this
#  ifndef _GNU_SOURCE
#     define _GNU_SOURCE
#  endif
#  include <features.h>
#  ifndef __USE_GNU
#    define __USE_GNU
#  endif
#  include <ucontext.h>
#  include <string.h>
#  include <execinfo.h>
#endif

#include "SysUtility.h"
#include "MC2String.h"

#include <stdlib.h>
#include <malloc.h>
#include <signal.h>

#ifdef ARCH_OS_LINUX

#include <unistd.h>
#include <fcntl.h>

#include <sys/resource.h>

namespace {

uint32
getLinuxProcessSizes(int sizeType)
{
   /*   From proc(5): (this is wrong, drs and lrs should be swapped
    *   (linux/fs/proc/array.c)
    statm  Provides information about memory status in pages.
    The columns are:
    size       total program size
    resident   resident set size
    share      shared pages
    trs        text (code)
    drs        data/stack
    lrs        library
    dt         dirty pages
   */

   /*
     real example data
     PID USER     PRI  NI  SIZE  RSS SHARE STAT %CPU %MEM   TIME COMMAND
     14087 mc2        9   0 1935M 1.3G  2120 S     0.0 33.8 105:23 RouteModule
     12225 mc2        9   0 2066M 781M  3972 S     0.0 19.4   4:01 MapModule

     [root@cheops-4 root]# cat /proc/14087/statm
     495442 348164 530 253 193064 154847 332514
     size, resident, share, trs, lrs, drs, dt

     [root@cheops-4 root]# cat /proc/14087/status
     Name:   RouteModule
     VmSize:  2384516 kB
     VmLck:         0 kB
     VmRSS:   1392660 kB
     VmData:  2379072 kB
     VmStk:        24 kB
     VmExe:      2012 kB
     VmLib:      2740 kB

     [root@cheops-4 root]# cat /proc/12225/statm
     529150 200039 993 493 102445 97101 80657

     [root@cheops-4 root]# cat /proc/12225/status
     Name:   MapModule
     VmSize:  2129104 kB
     VmLck:         0 kB
     VmRSS:    800156 kB
     VmData:  2119776 kB
     VmStk:        28 kB
     VmExe:      2608 kB
     VmLib:      5244 kB

     statm seems to be *wrong* !?
   */

   const char* fileName = "/proc/self/status";

   MC2String lookFor;

   switch( sizeType ) {
   case 0:
      lookFor = "VmSize:";
      break;
   case 1:
      lookFor = "VmData:";
      break;
   case 2:
      lookFor = "VmRSS:";
      break;
   default:
      mc2log << warn << "SysUtil::getLinuxProcessSizes(): unknown sizeType!" << endl;
      break;
   }

   char buf[100];
   uint32 aSize = 0;
   FILE* file = fopen( fileName, "r" );
   if ( file != NULL ) {
      while ( fgets( buf, sizeof( buf ) - 1, file ) != NULL ) {
         if ( strstr( buf, lookFor.c_str() ) != NULL ) {
            if ( sscanf( buf, "%*s\t %u",  &aSize ) != 1 ) {
               mc2log << error << "SysUtil::getLinuxProcessSizes(): parse error ("
                      << fileName << ")" << endl;
               fclose( file );
               return 0;
            }
            buf[ strlen( buf ) - 1 ] = '\0';
         }
      }
      fclose( file );
   } else {
      mc2log << error << "SysUtil::getLinuxProcessSizes(): failed to open " 
             << fileName << "for reading" << endl;
      return 0;
   }
   return aSize;
}

void
dumpLinuxProcessInfo()
{
   static const char* fileName = "/proc/self/status";
   static char buf[10*1024];
   static int lastRead, totRead;
   int fd = open(fileName, O_RDONLY);
   if (fd < 0) {
      mc2log << error << "SysUtil::dumpLinuxProcessInfo(): failed to open " 
             << fileName << "for reading" << endl;
      return;
   }
   lastRead = 1;
   totRead = 0;
   while (lastRead != 0)
   {
      lastRead = read(fd, &buf[totRead], sizeof(buf) - totRead);
      if ((lastRead < 0) && (errno == EINTR))
         lastRead = 1;
      else
         totRead += lastRead;
   }
   cerr << "Process info follows: " << endl;
   buf[totRead + 1] = '\0';
   cerr << buf << endl;

   close( fd );

}

}

#endif

namespace SysUtility {

// Way to turn off calling the signal function from all threads
// as signal "Use of this function is unspecified in a multi-threaded process."
static bool setPipeSignal = true;

void ignorePipe()
{
#ifndef _MSC_VER
   // If the socket is closed by the other side during transmission, we
   // receve the SIGPIPE-signal that normaly leads to program exit.
   // But now this signal is ignord during read  --> read() returns
   // -1 if this happens.
   if ( setPipeSignal && signal( SIGPIPE, SIG_IGN ) == SIG_ERR ) {
      mc2log << warn << "[TCPSocket] Failed to ignore pipe!" << endl;
   }
#endif
}


void
setPipeDefault() {
#ifndef _MSC_VER
   // Let the default handler take care of the SIGPIPE-signal now when
   // data read.
   
   if ( setPipeSignal && signal( SIGPIPE, SIG_DFL ) == SIG_ERR ) {
      mc2log << warn << "[TCPSocket] Failed to restore pipe signal!" << endl;
   }
   
#endif   
}

void
setChangePipeSignal( bool on ) {
   setPipeSignal = on;
}

uint32
getProcessTotalSize()
{
#ifdef ARCH_OS_LINUX
   return getLinuxProcessSizes(0);
#else
   return 0;
#endif
}

uint32
getProcessDataSize()
{
#ifdef ARCH_OS_LINUX
   return getLinuxProcessSizes(1);
#else
   return 0;
#endif
}

uint32
getProcessResidentSize()
{
#ifdef ARCH_OS_LINUX
   return getLinuxProcessSizes(2);
#else
   return 0;
#endif
}

MC2String 
getMemoryInfo() {
   char buff[1024];
   snprintf( buff, 1024, 
             "Total: %d Data: %d Resident: %d",
             getProcessTotalSize(),
             getProcessDataSize(),
             getProcessResidentSize() );
   return buff;
}

void
dumpProcessInfo()
{
#ifdef ARCH_OS_LINUX
   dumpLinuxProcessInfo();
#else
   return;
#endif
}

void 
getBacktrace( vector<MC2String>& bt )
{
#ifndef CAN_BACKTRACE
   cerr <<  "SysUtility::getBacktrace not implemented" << endl;
   return;
#else
   // Trace pointers are put here
   void *trace[512];
   int trace_size = backtrace(trace, sizeof(trace) / sizeof(trace[0]) );
   // overwrite sigaction with caller's address if it exists
   
   char** messages = backtrace_symbols(trace, trace_size);
   /* skip first stack frame (points here) */
   for (int i = 1; i < trace_size; ++i) {
      bt.push_back( messages[i] );
   }
   free(messages);
#endif
}

void
printBacktrace()
{
#ifndef CAN_BACKTRACE
   cerr <<  "SysUtility::printBacktrace not implemented" << endl;
   return;
#else
   // Trace pointers are put here
   void *trace[512];
   int trace_size = backtrace(trace, sizeof(trace) / sizeof(trace[0]) );
   // overwrite sigaction with caller's address if it exists
   
   char** messages = backtrace_symbols(trace, trace_size);
   /* skip first stack frame (points here) */
   fprintf(stderr, "[bt] Execution path: \n");
   for (int i = 1; i < trace_size; ++i) {
      fprintf(stderr, "[bt] %s\n", messages[i]);
   }
   fprintf(stderr, "\n");
   free(messages);
#endif
}

void printHeapStatus( std::ostream& os ) {
   struct mallinfo info = mallinfo();
   
   const int columnWidth = 20;
   
   // usage in percent, don't divide by zero:
   double utilization = 0.0;
   if ( info.arena != 0 ) {
      utilization = 100*( static_cast<double>( info.uordblks )/info.arena );
   }

   os << "Normal heap: " << endl;
   os << "\tMemory in use:   " << setw( columnWidth ) 
      << info.uordblks << " bytes" << std::endl;
   os << "\tTotal heap size: " << setw( columnWidth ) 
      << info.arena << " bytes" << std::endl;
   os << "\tHeap utilization:" 
      << setprecision( 2 ) << fixed << setw( columnWidth )
      << utilization << " %" << std::endl;

   os << "mmap'ed memory: " << endl;
   os << "\tTotal size of memory allocated with mmap: " 
      << info.hblkhd << std::endl;
}      

void setPriority( uint32 newPrio ) {
#ifdef __linux
   if (newPrio < 0) {
      newPrio = 0; // we don't want negative nice value
   }
   if (newPrio <= 20) {
      if (setpriority(PRIO_PROCESS, 0, newPrio) != 0) {
         perror("setpriority");
         DEBUG1(cerr << "newPrio = " << newPrio << endl);
      }
   }
#endif // __linue
}

} // SysUtility
