/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRAFFICHANDLER_H
#define TRAFFICHANDLER_H

#include "config.h"

#include "MC2String.h"

#include <vector>

// forward declarations
class TrafficIPC;
class PacketContainer;
class DisturbanceElement;
class TrafficSituation;

/**
 * A class for handling the TrafficSituations that are passed from the
 * TrafficThread. At startup it will ask the InfoModule for a list of currently
 * stored events for a specific provider and keep this updated so it can filter
 * out unchanged events etc.
 * It will also retrive the SSIs related to the current events.
 */
class TrafficHandler {
public:
   /// A Container containing DisturbanceElements.
   typedef std::vector< DisturbanceElement* > Disturbances;   

   /// A Container containing the TrafficSituations to process.
   typedef std::vector< TrafficSituation* > SitCont;

   /**
    * Constructor.
    *
    * @param provider The provider for this handler.
    * @param communicator Instance that communicates with the \c InfoModule.
    */
   TrafficHandler( const MC2String& provider,
                   TrafficIPC* communicator );

   /**
    * Delete the handler and release allocatede memory.
    */
   ~TrafficHandler();

   /**
    * This will ask the InfoModule for stored traffic events.
    * It will first ask the MapModule for all the underview maps and then ask
    * the InfoModule for disturbances for each map. Then it will merge the
    * disturbances with regard to situation references by adding SSIs togther.
    */
   void setup();

   /**
    * Process the traffic events and send them to the InfoModule.
    *
    * @param trafficSits A vector with the TrafficSituations we are going to
    * process.
    * @return True if all the situations are sent to the InfoModule.
    */
   bool processSituations( const SitCont& trafficSits );

private:

   /**
    * Creates a unique set of situations to be added, updated or removed
    * from the database.
    * @param parsedSitCont SitCont from parser.
    * @param removed Disturbances of elements to be removed.
    * @param newSitCont SitCont not yet in database.
    */
   void createUniqueSet( const SitCont& parsedSitCont,
                         Disturbances& removedElements,
                         SitCont& newSituations );

   /// Handles startup...
   void startup();

   /// Fetch all situations from module
   bool fetchSituationsFromModule();

   /**
    * Composes new situations from a set of situations that are not 
    * in database.
    */
   void composeNewElements( const SitCont& newSituations,
                            Disturbances& newDisturbances );

   /**
    * Send situations to module.
    * @param newElements elements to be updated.
    * @param removedElements elements to be removed.
    * @return true if the changeset was received successfully to the module.
    */
   bool sendChangeset( Disturbances& newElements,
                       Disturbances& removedElements );

   /**
    * Updates the stored elements in m_storedElements.
    * @param newElements elements to be added.
    * @param removedElements elements to be removed.
    */
   void updateStoredElements( const Disturbances& newElements,
                              const Disturbances& removedElements );

   /// The provider for this feed.
   MC2String m_provider;

   /// Elements that are stored.
   Disturbances m_storedElements;

   TrafficIPC* m_communicator;
};

#endif // TRAFFICHANDLER_H
