/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SIMPLEBALANCER_H
#define SIMPLEBALANCER_H

#include "config.h"

#include "MC2String.h"
#include "Balancer.h"
#include "IPnPort.h"
#include "MapBits.h"
#include <memory>
#include <set>

class ModuleNotice;
class RequestPacket;
class StatisticsPacket;
class ModuleList;
class LoadMapReplyPacket;

/**
 *   The SimpleBalancer will stop loading maps when all modules
 *   are loading, just like the old system. This is to avoid
 *   problems with slow modules that can get a queue of many
 *   LoadMapRequestPackets and delay all the jobs. It is also
 *   more difficult to determine how much memory is used, since
 *   the sizes of the maps are not known beforehand.
 */
class SimpleBalancer : public Balancer {
  public:
   /**
    *    Creates a new SimpleBalancer
    *    @param ownAddr The address for this module.
    *    @param moduleName The name of this module (for logging purposes).
    *    @param ownStatistics Initial statistics packet for this module
    *    @param usesMaps Whether the module uses maps.
    *    @param  multipleMaps True for those modules the can have multiple
    *            instances for each map. (Eg not InfoModule)
    */   
   SimpleBalancer(const IPnPort& ownAddr,
                  const MC2String& moduleName,
                  StatisticsPacket* ownStatistics,
                  bool usesMaps,
                  bool multipleMaps = true);

   ~SimpleBalancer();

   /**
    *    @see Balancer.
    */
   virtual bool updateStats(StatisticsPacket* packet,
                            PacketSendList& packetList);

   /**
    *    @see Balancer.
    */
   int getNbrModules() const;

   /**
    *    @see Balancer.
    */
   int timeout( StatisticsPacket* ownStatistics, 
                PacketSendList& packets );

   /**
    *    @see Balancer.
    */
   virtual bool getModulePackets(PacketSendList& packets,
                                 RequestPacket* req);

   
   /**
    *    @see Balancer
    */
   virtual void becomeLeader();

   /**
    *    @see Balancer
    */
   void becomeAvailable();

   /**
    *   To avoid "All modules are loading"
    */
   void reactToMapLoaded(PacketSendList& packets,
                         LoadMapReplyPacket* replyPacket);

   /**
    * @return Whether this module uses maps.
    */
   bool usesMaps() const;

   /**
    *  Lets the balancer know about all available maps.
    *  @param allMaps The maps.
    */
   void setAllMaps( const set<MapID>& allMaps );
   
protected:
   
   /**
    *   Make sure that there are no double instances of a map.
    *   (Only for modules that don't allow doubles, ie InfoModule.)
    */
   void checkNoMultipleMaps(PacketSendList& packets);   

   /**
    *   Returns the module best suited for map loading,
    *   i.e. the one using approx least memory and has the
    *   oldest map.
    *   @return ModuleNotice or NULL.
    */
   ModuleNotice* getModuleUsingLeastMemPrefNotLoading( uint32 mapID );
   
   /**
    *   Checks if there are modules that have loaded too
    *   many (much) maps and adds DeleteMapRequestPackets
    *   to the packetList if there are.
    *   @return Number of deletpackets added.
    */
   int checkMemOverUse(PacketSendList& packetList);
   
   /**
    *  Acknowledge or refuse a packet to the sender.
    *  NB! Does not delete the packet if refusing.
    *  Virtual to enable different behaviour for subclasses.
    *  @param pack The packet to reply to.
    *  @param eta The expected time of the response.
    *  @param refuse If set to true the packet was refused.
    */
   virtual void sendAcknowledge(PacketSendList& packetList,
                                RequestPacket* pack, uint32 eta,
                                bool refuse = false);
   
   /**
    *    Creates a moduleList.
    */
   virtual void createModuleList( StatisticsPacket* ownStatistics );
   
   /**
    *   Checks if any maps have been unused for a long
    *   time and starts deleting them.
    */
   int checkOldAge( PacketSendList& packetList );
   
   /**
    *   Tell a module to delete a map.
    *   @param mapID   The ID of the map that should be deleted.
    *   @param mn      The notice for the module that should be told
    *                  to delete the map.
    *   
    */
   void moduleDeleteMap( PacketSendList& packetList,
                         uint32 mapID, ModuleNotice* mn );

   /**
    *   Tell a module to load a map.
    *   @param request The request that causes the map to be loaded.
    *   @param mapID   The ID of the map that should be loaded.
    *   @param mn      The notice for the module that should be told
    *                  to load the map.
    *   
    */
   void moduleLoadMap( const RequestPacket* request,
                       PacketSendList& packetList,
                       uint32 mapID, ModuleNotice* mn );

    /**
     *    Checks if the map id of the request packet exists in the
     *    allmaps vector and if it doesn't, it sends an ack packet
     *    to the sender. If the map ID is <code>MAX_UINT32</code>
     *    there is no such map, but ok will be true.
     *    The following combinations can be returned:
     *    <table><tr>
     *            <td><b>Return value</b></td>
     *            <td><b>Value of ok</b></td>
     *            <td><b>Comment</b></td>
     *           </tr><tr>
     *            <td>Valid notice</td>
     *            <td>true</td>
     *            <td>Map was found</td>
     *           </tr><tr>
     *            <td>NULL</td>
     *            <td>true</td>
     *            <td>Map id was MAX_UINT32 - map does not matter</td>
     *           </tr><tr>
     *            <td>NULL</td>
     *            <td>false</td>
     *            <td>MapID was not found in allmaps.</td>
     *           </tr>
     *    </table>
     *    @param ok Outparameter which will be set to true if
     *              the request was ok, i.e. if the map exists
     *              or the map id of the request was MAX_UINT32.
     */
   void getNoticeOrSendAck(PacketSendList& packetList,
                           const RequestPacket* request,
                           bool& ok);

   void sendMapLoadedAcknowledge(PacketSendList& packetList,
                                 RequestPacket* pack);

   /**
    *   @return The name of this module.
    */
   const MC2String& getModuleName() const;

   /// All known modules if leader, else NULL.
   auto_ptr< ModuleList > m_moduleList;
   
protected:

   /**
    *   Reads values from the properties
    *   If e.g. MODULE_MAX_MEM for RouteModule is about to be read
    *   and the map set is set to 0, the following order will be used:
    *   <ol>
    *    <li>MODULE_MAX_MEM</li>
    *    <li>MODULE_MAX_MEM_0</li>
    *    <li>ROUTE_MODULE_MAX_MEM</li>
    *    <li>ROUTE_MODULE_MAX_MEM_0</li>
    *   </ol>
    */
   bool readPropValues(const MC2String& prefix = "",
                       const MC2String& suffix = "" );
   
   
private:   
   /// If this is true the module uses maps.
   bool m_moduleUsesMaps;

   /**
    * If this is true several modules may load the same map.
    * (False for InfoModule.)
    */
   bool m_multipleMaps;
         
   /// Copies of tasks that were refused due to maploading
   vector<RequestPacket*> m_refusedDueToLoading;

   /// The address of the Reader.
   IPnPort m_ownAddr;

   /** 
    *   All available maps. Retreived from the MapModule when 
    *   become leader.
    */
   set<MapID> m_allMaps;

   /// The number of heartbeats sent so far
   uint32 m_nbrHeartBeat;
   
   /// The maximum age of a map before it is deleted
   uint32 m_maxMapAgeMinutes;

   /// Always keep one instance of the maps in this set.
   set<uint32> m_mapsToKeepOneOf;

   /// True if the (experimental) map balancing should be used
   bool m_useMapBalancing;

   /// Used for logging
   MC2String m_moduleName;
};

/**
 *    Enqueues all packets into the leader.
 */
class InfoModuleSimpleBalancer : public SimpleBalancer {
public:
   /**
    *    Creates a new InfoModuleSimpleBalancer.
    */
   InfoModuleSimpleBalancer( const IPnPort& ownAddr,
                             const MC2String& moduleName,
                             StatisticsPacket* ownStatistics );
};

#endif

