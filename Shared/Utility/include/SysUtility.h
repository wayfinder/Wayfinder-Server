/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SYSUTILITY_H
#define SYSUTILITY_H

#include "config.h"
#include "MC2String.h"

/**
  *   Utility functions that are system/OS dependent.
  *
  */
namespace SysUtility 
{

/**
 *    Ignores pipe signals that would halt the program
 *    if the connection is closed. Now read returns -1
 *    instead. Has no effect in MSC.
 */
void ignorePipe();

/**
 *    Sets the default signal handler for SIGPIPE.
 *    Has no effect in MSC.
 */
void setPipeDefault();

/**
 * Sets if ignorePipe and setPipeDefault should do anything or not.
 */
void setChangePipeSignal( bool on );


/// ignores pipes until it goes out of scope
struct IgnorePipe {
   IgnorePipe() { 
      SysUtility::ignorePipe();
   }
   ~IgnorePipe() { 
      SysUtility::setPipeDefault();
   }
};


/**
 *   Returns process total size (code + stack + data etc).
 *   @result size in 1024 byte blocks (0 if not supported)
 */
uint32 getProcessTotalSize();

/**
 *   Returns process data size (data + stack).
 *   @result size in 1024 byte blocks (0 if not supported)
 */
uint32 getProcessDataSize();

/**
 *   Returns process resident size (data/code/etc of the process currently in memory).
 *   @result size in 1024 byte blocks (0 if not supported)
 */
uint32 getProcessResidentSize();

/**
 *   Dump process info. Dump all available process info, used
 *   when we're about to exit due to a fatal error such as failure
 *   to allocate memory, etc. Should *not* allocate anything on the 
 *   heap and *not* put anything on the stack!
 *   Print/dumps nothing if not supported.
 */
void dumpProcessInfo();


MC2String getMemoryInfo();

/**
 *    Prints backtrace. At least on linux.
 */
void printBacktrace();

/**
 *    Gets the back trace. At least on linux.
 */
void getBacktrace( vector<MC2String>& bt );
   
/**
 * Prints information about the heap. Used for
 * diagnosing memory problems such as fragmentation
 * and leaks.
 */
void printHeapStatus( std::ostream& os );

/**
 * Attempts to increase the nice value of the current process
 * to the newPrio nice value.
 * The function does not even attempt to
 * set a negative priority even if requested.
 * @param newPrio The requested new priority.
 */
void setPriority( uint32 newPrio );

};
#endif // SYSUTILITY_H
