/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODULE_H
#define MODULE_H

#include "config.h"
#include "Component.h"
#include "ISABThread.h"
#include "ModuleTypes.h"
#include <memory>

class JobThread;
class PacketSenderReceiver;
class Processor;
class Reader;
class PacketReaderThread;
class PacketQueue;
class MapSafeVector;

/**
 *  Base class for an MC2 Module
 *
 */
class Module: public Component, public ISABThread
{
public:
   /**
    * Initializes the Module
    * @param moduleType The module's type
    *        used to get values from the property file.
    * @param handler the command line option handler
    * @param useMaps whether this module uses maps or not
    */
   Module( moduletype_t moduleType,
           CommandlineOptionHandler* handler,
           bool useMaps = true );

   /**
    *  Initializes the Module
    *  @param moduleType The module's type
    *         used to get values from the property file.
    *  @param argc  Number of command line arguments
    *  @param argv  Array with the command line arguments
    * @param useMaps whether this module uses maps or not
    */
   Module(moduletype_t moduleType, 
          int argc, char* argv[], bool useMaps = true );
      
   /**
        *  Cleans up
        */
      virtual ~Module();

      /**
        *  The module's initialization function
        */
      virtual void init();

      /**
        *  The module's main function
        */
      virtual void run();

      /**
        *  Handle a command from the user
        *  @param cmd Pointer to string with the command
        *  @return Integer reporting what happened; 0: error ocurred
        *          1: everything ok, -1: quit now
        */
      virtual int handleCommand(const char* input);
   /// @see Component
   virtual void parseCommandLine() throw ( ComponentException );

protected:
   MapSafeVector& getLoadedMaps() {
      return *m_loadedMaps;
   }

      /// This module's PacketQueue
      auto_ptr<PacketQueue> m_queue;
      /// This module's loaded maps
      auto_ptr<MapSafeVector> m_loadedMaps;
      /// This module's Processor
      auto_ptr<Processor> m_processor;

      auto_ptr<PacketSenderReceiver> m_senderReceiver;
      /// This module's JobThread
      JobThread* m_jobThread;
      ISABThreadHandle m_jobThreadHandle;

      /// This module's PacketReaderThread
      auto_ptr<PacketReaderThread> m_packetReader;

      /// This module's Reader
      Reader* m_reader;
      ISABThreadHandle m_readerHandle;

      /// The rank of this module
      uint32 m_rank;

      /// The type of this module
      moduletype_t m_type;

      /// The module's preferred port
      uint32 m_preferredPort; 

      /// Leader Multicast IP
      uint32 m_leaderIP;

      /// Leader Multicast Port
      uint16 m_leaderPort;

      /// Available Multicast IP
      uint32 m_availIP;

      /// Available Port
      uint16 m_availPort;

   /// Property prefix
   MC2String m_propPrefix;
   bool m_useMaps;
   /// start as leader module
   bool m_startAsLeader;
private:

   void setup();

   /**
    * Prints detailed information about the loaded maps to stdout.
    */
   void printMapStatus();
};

#endif // MODULE_H
