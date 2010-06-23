/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDROUTEPROCESSOR_H
#define EXPANDROUTEPROCESSOR_H

#include "config.h"

#include "ItemTypes.h"
#include "Vector.h"
#include "StringTable.h"
#include "LangTypes.h"

#include <list>

class GenericMap;
class ExpandRouteLink;
class ExpandRouteHead;

class ExpandRouteRequestPacket;
class ExpandRouteReplyPacket;

class Item;
class StreetSegmentItem;
class Node;
class Connection;
class AlteredGfxList;



/**
  *   Objects of this class is used to process a ExpandRouteRequestPacket.
  *   Will create a ExpandRouteReplyPacket with the route. The format 
  *   depends on the parameters in the request packet.
  *
  */
class ExpandRouteProcessor {
   public:
      /**
       *    The differnt types of the route (in text format) that
       *    could be returned.
       *    {\it {\bf NB!} These types should be moved elsewhere when
       *    this is supported in the packet!}.
       */
      enum description_t {
         /**
          *    This indicates that all points where anything change
          *    (street name, to turn etc.) will be included in the route.
          */
         NOVICE_DESCRIPTION,

         /**
          *    The normal route description. Should be resonably short, but
          *    clearly understandable.
          */
         NORMAL_DESCRIPTION,

         /**
          *    The route description that should be used for the users that
          *    usually finds there way. The route description will be short!
          */
         EXPERT_DESCRIPTION
      };

      /**
       *    The format of the route.
       */
      enum routeformat_t {
         /**
          *    The shortest name will be printed for the streets that are
          *    included in the route descritption.
          */
         SHORT_ROUTEFORMAT,
         
         /**
          *    The official name will be printed for the streets that are
          *    included in the route descritption. If this does not 
          *    excists the alternative namewill be printed or the road 
          *    number.
          */
         NORMAL_ROUTEFORMAT,

         /**
          *    All the names of the streets in the route description will
          *    be included in the result.
          */
         LONG_ROUTEFORMAT
      };

      /**
        *   Create a new ExpandRouteProcessor.
        */
      ExpandRouteProcessor();

      /**
        *   Releasae the memory created in this object.
        */
      virtual ~ExpandRouteProcessor();

      /**
        *   Process a given ExpandRouteRequestPacket, respon with a
        *   ExpandRouteReplyPacket that contains the expanded route.
        *
        *   {\it {\bf NB!} The reply is created inside this method and 
        *   both the request and the reply {\bf must} be deleted by the 
        *   caller!}
        *
        *   @param   req         A packet containing the route to expand.
        *   @param   theMap      The map where the route is located.
        *   @return  A ExpandRouteReplyPacket that contains the expanded
        *            route.
        */
      ExpandRouteReplyPacket* processExpandRouteRequestPacket(
                                   const ExpandRouteRequestPacket* req,
                                   GenericMap* theMap );

   private:

      
      /**
       *    Exchanges UNDEFINED_TURNs with AHEAD_TURNs
       *    @return  True if succesful.
       */
      bool removeUndefinedTurns();
      
      /**
       *    Sets the street counts for route links.
       *    @return  True if succesful.
       */
      bool setStreetCounts();
      
      /**
       *    Sets the distance for the route links.
       *    @return  True if succesful.
       */
      bool setDistance();

      /**
       *  Change two or more turns separated by one or more small links into
       *  one turndescription. Ex. Turn left 90deg, drive 6m then turn left
       *  90deg becomes a UTURN, Left 90deg drive 7m then turn right 90 deg
       *  becomes AHEAD. Ex. Ahead -30 deg + Ahead -30deg + Ahead -30 deg
       *  becomes TURN_LEFT. Lower roadClass allows longer distance (7-15m).
       */
      void mergeMultiTurns();
      
      /**
       *   Tries to extend road names to unnamed segments.
       *
       */
      void expandRoadNames();
      
      /**
       *    Sets the transportation type (walking/driving) for the
       *    route links. The link containing the node id that signals
       *    changes of transportation is removed from the list.
       *    @return  True if succesful.
       */
      bool setTransportationType();

      /**
       *    Extracts and sets data from the connections of the route links.
       *    Sets the turndescriptions, traverse time, crossing kind
       *    and the signpost information.
       *    @uTurn   True if we are starting with an U-Turn.
       *    @return  True if succesful.
       */
      bool storeDataFromConnections( bool uTurn,
                                     byte routeType );
      
      /**
       *    Sets the signost belonging to a certain connection to the
       *    specified link.
       *    @param   conn  The connection containing the signposts.
       *    @param   link  The link that will get signpost added to it.
       *    @return  True if succesful.
       */
      bool setSignposts(Connection* conn, ExpandRouteLink* link);

      /**
       * Set the lanes for a link.
       *
       * @param link The ExpandRouteLink to set lanes in.
       */
      bool setLanes( ExpandRouteLink* link );
      
      /**
        *   Add landmarks to the route links.
        *   Searches for the key fromNode.toNode in the landmark table
        *   of the map, and adds all landmarks that fits to the 
        *   landmarklist of every route link. The function also checks 
        *   the bualocation of every route-item to note if a bua is
        *   entered or passed.
        *   @param   prevCountryID  The country where the previous map of 
        *                           the route (if part of multi-expand) is 
        *                           located. If this is the first part of 
        *                           the route, the countryID is MAX_UINT32.
        *                           Used for adding new country as landmark.
        *   @param   mapIDToCountryID  Table used when routing in overviewMap
        *                              to find out which country we are in.
        *                              Holds the mapIDs of the underviewMaps
        *                              and their country.
        */
      bool addLandmarks(uint32 prevCountryID,
                        const map<uint32, uint32>& mapIDToCountryID);

      /**
       *    Computes the total distance, time and standstill time.
       *    @param   totDist     Output parameter. 
       *                         Set to the total distance of the route.
       *    @param   totTime     Output parameter. 
       *                         Set to the total time of travelling the
       *                         route, including standstill time.
       *    @param   totSST      Output parameter. 
       *                         Set to the total standstill time of
       *                         travelling the route.
       *    @param   firstPacket Whether the expandroute packet is the
       *                         first one (ie. includes the start of the
       *                         route).
       *    @return True if succesful.
       */
      bool getTotals(uint32& totDist, uint32& totTime, uint32& totSST,
                     bool firstPacket);

      /**
       *    Updates the time and distance for the start and end of
       *    the route by considering the offset, ie. how far on a 
       *    street your start/end-point is. It also recomputes
       *    the standstill time based for the start and end based on
       *    the standstill time in order to get times that match the
       *    ones the RouteModule uses for its calculations.
       *    The starting angle is also computed here.
       *    
       *    XXX: Also contains the setting of turndescriptions for the
       *    start and end links. This should be moved to somewhere else.
       *
       *    @param   startOffset          The offset for the start point.
       *    @param   ignoreStartOffset    True if startoffset is to be
       *                                  ignored (doesn't start here).
       *    @param   startTowards0        
       *    @param   endOffset            The offset for the end point.
       *    @param   ignoreEndOffset      True if endoffset is to be 
       *                                  ignored (doesn't end here).
       *    @param   endTowards0 
       *    @param   lastID               The id of the last item
       *                                  on the previous map of the route 
       *                                  (only used when part of an 
       *                                  expand).
       *    @param   lastMapID            The map id of the previous map
       *                                  of the route (only used when part
       *                                  of an expand).
       *    @return  True if succesful.
       */
      bool updateStartAndEnd(uint16 startOffset, 
                             bool ignoreStartOffset, 
                             bool startTowards0,
                             uint16 endOffset, 
                             bool ignoreEndOffset, 
                             bool endTowards0, 
                             uint32 lastID,
                             uint32 lastMapID,
                             bool uTurn,
                             byte routeType);

      /**
       *  Goes through the ExpandRouteList and identifies the items
       *  that have sharp ENTER_ROUNDABOUT or EXIT_ROUNDABOUT turns.
       *  Theese are placed in the AlteredGfxList together with the
       *  the information of which coordinates should be disregarded or
       *  moved. (Only for Navigator routes.)
       */
      bool findBadGfxCoords();

      /**
       *  Get the connecting angle when passing from one RoutableItem to
       *  another.
       *  @param first The RouteLink leading in to the connection.
       *  @param second The RouteLink leading out of the connection.
       *  @return The angle of the connection.
       */
      float64 getConnectionAngle(ExpandRouteLink* first,
                                 ExpandRouteLink* second);
      
      /**
       *    Store the graphical representation of the route from the
       *    route list to the reply packet.
       *    @param   reply       The reply packet.
       *    @param   req         The request packet.
       *    @param   nodeIDs     The node ids of the route.
       *    @param   nbrItems    Nbr items in the nodeIDs list.
       *    @return  True if succesful.
       */
      bool saveRouteGfxIntoPacket(ExpandRouteReplyPacket* reply,
                                  const ExpandRouteRequestPacket* req,
                                  const list<uint32>& nodeIDs,
                                  uint32 nbrItems );
      
      /**
       *    Method yto se if two links hav ANY common name.
       *    @return True if the two items have common names.
       */
      bool commonName(ExpandRouteLink* newLink,
                   ExpandRouteLink* oldLink);
      /**
       *    Method to se if the first link has a name that an other link lacks.
       *    @return True if oldLink lacks any of newLinks names.
       */
      bool newName(ExpandRouteLink* newLink,
                   ExpandRouteLink* oldLink);

      /**
       *    Method to se if a name (given as a strinCode) is present in the
       *    item of a link.
       */
      bool namePresent(ExpandRouteLink* newLink,
                       uint32 stringCode);

      /**
       *    Sets the names of the route links as well as concatenates
       *    (removes) links that is not necessary.
       *    @param firstPacket   Whether this is the first packet of the 
       *                         expanding or not (ie. if it contains the 
       *                         start of the route).
       *    @param printAllNames Whether all names for a certain road segment 
       *                         should be used in the turndescription or not.
       *    @param removeAheadIfNameDiffer
       *                         Whether the route description should be 
       *                         included if turndescription is AHEAD and only
       *                         the name differ.
       *    @param navigator     If the packet comes from a navigator.
       *    @param noConcat      If this is set to true only follow road 
       *                         is concatinated.
       *    @param nameChangeAsWaypoint For later wayfinders. Will not conc.
       *                                a name change, but instread change the
       *                                turndesc to indicator.
       *    @return  True if succesful.
       */
      bool setNamesAndConcatenate(bool firstPacket,
                                  bool lastPacket,
                                  bool printAllNames,
                                  bool removeAheadIfNameDiffer,
                                  bool navigator,
                                  byte routeType,
                                  bool nameChangeAsWaypoint);

      /**
       *  Checks if two RouteLinks have the same name set.
       *  Only checks the choosen name of the links, not if the items share
       *  names.
       *  @param firstLink Pointer to the first RouteLink.
       *  @param secondLink Pointer to the decond RouteLink.
       *  @return True if the links have the same name or neither of them
       *          names.
       */
      bool sameName(ExpandRouteLink* firstLink, ExpandRouteLink* secondLink);
      
      /**
       *    Sets the turnnumbers of the route links 
       *    ("3:rd street to the left), and the roundabout exit numbers
       *    ("2:nd exit of roundabout / right in roundabout").
       *    @return  True if succesful.
       */
      bool setTurnNumbersAndRoundabouts();

      /**
       *    Used BEFORE  setNamesAndConcatenate()
       *    Sets the turnnumbers and passedRoadsof the route links.
       *    No previous turn setting needed.
       *    @param  passedRoadLandMarks Set to true if passed roads should
       *            be added as well.
       *    @return True if the method worked.
       */
      bool newSetTurnNumbersAndRoundabouts(bool passedRoadLandMarks);

      /**
       *    Counts the number of possible turns of the same type as a a
       *    connection and adds this to the turn count of turnLink.
       *    If usePassedRoadsLM is true the name of the passed street
       *    will be returned in passedName.
       *    @param  fromLink The link the route goes from.
       *    @param  toLink The next link in the route
       *    @param  searchedTurn The turn type to check for.
       *    @param  usePassedRoadsLM If true the method returns the name of
       *            the road we could have turned to.
       *    @return The turnCount to set in the turnLink
       */
      byte addStreetCount(ExpandRouteLink* turnLink,
                          ExpandRouteLink* fromLink,
                          ExpandRouteLink* toLink,
                          bool usePassedRoadsLM,
                          bool noSetting = false);

      /**
       *    Chooses the turndescription for the driving through the
       *    specified roundabout.
       *    @param   rbExitNumber      The number of roundabout exits 
       *                               so far.
       *    @param   restrictedRbExit  The number of restricted exits.
       *    @param   crossKind         The crossingkind of this roundabout.
       *    @param   tran              The type of transportation.
        *   @param   driveOnRightSide  If driving on right side of the road.
       *    @param   sc                Output parameter. 
       *                               The turndescription of driving
       *                               through the roundabout according
       *                               to the specified left/right
       *                               turnnumbers.
       *    @return  The turn number (exit count) of passing through the
       *             roundabout.
       */
      byte getRoundaboutInfo(byte rbExitNumber,
                             byte restrictedRbExit,
                             ItemTypes::crossingkind_t crossKind,
                             ItemTypes::transportation_t trans,
                             bool driveOnRightSide,
                             StringTable::stringCode& sc,
                             byte entryCount = 0,
                             byte exitCount  = 0);

      /**
       *    Resets (clears) m_possibleNames and updates it with the
       *    names that are present in the specified item.
       *    @param   item  The item who's names should be added to the
       *                   possible name vector.
       */
      void updatePossibleNames(Item* item);
      
      /**
       *    Adds turndescriptions due to changes of transportation type, 
       *    for example "park car".
       *    @return  True if succesful.
       */
      bool addTransportationChanges();

      /**
       *    Checks that the location for built-up areas is correct
       *    pass/into. This is done before selecting which landmarks 
       *    that should be kept in the final route.
       */
      bool updateLandmarksBeforeSelect();
      
      /**
        *   Selects which landmarks that should be included in the final 
        *   route. The selection is based on the importance of each landmark 
        *   in combinatin with how far from the next crossing it is located.
        */
      bool selectLandmarks();

      /**
        *   Performs misc updates of the landmarks in the route list.
        *   E.g. updates pass-location to at-location for landmarks that 
        *   are tied to ahead-turns (e.g. when ssi change name).
        *   Also sorts the landmarklist so that buas will appear in the 
        *   correct order compared to other landmarks in the description.
        */
      bool updateLandmarks();


      /**
        *  Creates landmarks of the m_passedStreets vectors  and adds
        *  them to the landmarks of this segment.
        *  @param nbrStreets Max number of streets included. 
        *  @return True. 
        */
      bool createPassRoadLandmarks(int nbrStreets);

       /**
        *  Creates landmarks of the traffic information sent to the expander.
        *  @return True. 
        */
     bool createTrafficLandmarks(const ExpandRouteRequestPacket* req);
      
      /**
       *    Convert region ID to country code.
       *    @param   regionID The region ID.
       *    @return  The corresponding country code if applicable.
       *             NBR_COUNTRY_CODES if no corresponding country code
       *             could be found.
       */
      StringTable::countryCode regionIDToCountryCode( uint32 regionID );
      
      /**
        *   Vector containing the string index of the possible names
        *   right now. To be able to avoid "ahead to E22; ahead to 
        *   ESSINGELEDEN; ahead to E22; etc" if there are multiple names.
        */
      Vector m_possibleNames;

      /**
        *   The map where (this part of) the route is located.
        */
      GenericMap* m_map;

      /**
       *    @name Expansion parameters.
       *    @memo Members that describes the current expansion.
       *    @doc  Members that describes the current expansion.
       */
      //@{
         /**
           *   Vector with the prefered languages of the route.
           *
           */
         Vector m_preferedLanguages;

         /**
          *    The type of route description to return. E.g. if the 
          *    points where driving ahead into a street with a new name 
          *    should be included in the route or not.
          */
         description_t m_descriptionType;

         /**
          *    The format of the route that will be returned. E.g. if all
          *    street names should be printed, only the official name or
          *    the shortest name (preferably the roadnumber).
          */
         routeformat_t m_routeFormat;
      //@}

      /**
        *   Pointer to an preallocated memoryarea that could be used to 
        *   return the roadname from the getPossibleName-method. This
        *   is mainly used to avoid reallocations and decrease the risk
        *   of memoryleaks.
        */
      char* m_currentRoadName;

      uint32 m_currentStringIndex;

      /**
        *   Indicates if a name should be printed if the names differ
        *   from what the names were on the previous link. This variable
        *   is used to avoid a changing of names just because we have
        *   crossed a mapborder.
        */
      bool m_printNamesIfTheyDiffer;
      
      /**
       *    Set to true if the street names should be abbreviated, 
       *    false otherwise. Initalized in the reset-method.
       */
      bool m_abbreviateStreetNames;

      /**
        *   Check if to print new name. If a new name should be printed
        *   then this is inserted in the m_possibleNames-Vector.
        *
        *   @param   item  The item that we are driving into.
        *   @return  True if a new item name should be written, false 
        *            otherwise.
        */
      bool toPrintNewName(Item* item);

      /**
        *   Get the number of possible names.
        *   @return  The current number of possible names.
        */
      uint32 getNbrPossibleNames();

      /**
       *  Set the names for a Link.
       */
      int setBestName(ExpandRouteLink* thisLink,
                      ExpandRouteLink* prevLink, 
                      bool manyNames);
      

      /**
        *   Get the "best" name to print.
        *   @param   manyNames    If true, several names should be chosen, 
        *                        otherwise just one name.
        *   @param   excludeBestName only if manyNames. The "best" name
        *            will not be included among the names.XS
        *   @return  Pointer to a buffer containing the "best" name(s).
        *            This buffer must NOT be deleted.
        */
      const char* getBestName(bool manyNames,
                              bool excludeBestName = false);

      /**
        *   Format the name by abbreviate it and add "väg" to roadnames
        *   only consisting of numbers -> "väg 16". Note that the adding
        *   of "väg" is only performed if swedish is the prefered language.
        *
        *   @param   roadName       The roadname to be formatted.
        *   @param   formattedName  Output parameter. 
        *                           The formatted roadname, which must be
        *                           a preallocated buffer with enough size
        *                           to fit an arbitrary name.
        */
      void  formatName(const char* roadName, char* formattedName,
                       LangTypes::language_t language,
                       bool exitNbr = false,
                       bool noNumberName = false);
      
      /**
       *  Add all the extra names to a route segment.
       */
      void expandNames();
      
      /**
       *    Get the name of a country, possibly combined with a 
       *    built-up area (e.g. "Helsingör, Danmark") that is to be used 
       *    as a landmark name. The prefered language is used.
       *    
       *    @param   country  The countryCode of the country to add as
       *                      landmark.
       *    @param   buaID    The itemID of the builtUpArea (if any).
       *    @return  The combined name of the country and built-up area.
       *             If no buaID is given (default=MAX_UINT32), only 
       *             the name of the country is returned.
       */
      const char* getCountryAndBuaName(
            StringTable::countryCode country, uint32 buaID = MAX_UINT32);
      
      /**
        *   Help-method to getBestName.
        *   {\it NB! This is copied from Item.h and MUST me moved somewhere!}
        */
      uint32 getStringWithType(ItemTypes::name_t strType, 
                               LangTypes::language_t strLang,
                               LangTypes::language_t &chosenLang,
                               uint32 &itemNameIndex);

       /**
        *   Set the possible turns. The turns that was possible to make,
        *   but not made at this place. Run this AFTER setNamesAndConc()
        *   @param thisItem The Item to set.
        *   @param prevItem The Item leading to this item.
        *   @return 
        */     
      bool setPossibleTurns();

      /**
        *   Update the number of street that leads to the right resp. to
        *   the left when passing curNode.
        *
        *   @param   curNode  The node that we are passing.
        *   @param   lastItem The previous ssi where we came from.
        *   @param   curItem  The current ssi (which consists of curNode)
        *   @param   nbrLeft  Outparam. The number of turns to the left.
        *   @param   nbrRight Outparam. The number of turns to the right.
        *   @param   nbrRbExits  Outparam. The number of roundabout exits.
        *   @param   nbrRestRbExits Outparam. The number of roundabout exits
        *            not allowed to this vehicle.
        *   @param   driveOnRightSide If driving on right side of the road.
        */
      bool getStreetCount(Node* curNode,
                          StreetSegmentItem* curItem,
                          StreetSegmentItem* nextItem,
                          byte& nbrLeft,
                          byte& nbrRight,
                          byte& nbrRbExits,
                          byte& nbrRestRbExits,
                          char* &leftStreetName,
                          char* &rightStreetName,
                          bool driveOnRightSide);
      
      /**
        *   Reset all the members so that a new route can be processed.
        *   @param   theMap   The map to reset.
        *   @param   req      The expand route request.
        */
      void reset(GenericMap* theMap, const ExpandRouteRequestPacket* req);

      /**
       *   Compare the names of two number roads. If newIndex is a
       *   more important name than oldIndex, true is returned.
       */
      bool betterNumberName(uint32 newIndex, uint32 oldIndex); 


      /**
       * Get the side of the road to drive on for a node.
       *
       * @param nodeID The node to check.
       * @return If to drive on right side, false if not(left side).
       */
      bool getDriveOnRightSide( uint32 nodeID );

      /**
       *  Returns the angle from north of a specific node.
       */
      float64 getNodeAngle(const Node* node) const; 
   
   /// Dumps landmarks to mc2log
   void dumpLandmarks();
      
   /**
    *    The list that is used when the route is expanded. Stored
    *    temporary in here to make it easier (possible) to generate
    *    good route descriptions.
    */
   ExpandRouteHead* m_routeList;
   
   /**
    *  List holding the modifications to the GfxData coordinates
    *  that the navigator needs.
    */ 
   AlteredGfxList* m_badGfxList;
   
   /**
    *  Flag that indicates that some coordinates in the route are
    *  modified.
    */
   bool m_alteredGfxData;
   
   /**
    *  Debug to include ALL(well most anyway) crossings.
    */
   bool m_noConcatinate;
   
   int m_nbrOfPassedStreets;
   
   /**
    *  Map id to country lookup table.
    */
   map<uint32, uint32> m_mapIDToCountryID;
   
   LangTypes::language_t m_requestedLanguage;
   /**
    *
    */
   //@{
   ItemTypes::routedir_nbr_t m_startDirHousenumber;
   ItemTypes::routedir_oddeven_t m_startDirOddEven;
   //@}
         
};

#endif


