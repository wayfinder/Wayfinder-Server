/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDEXTRADATAUTILITY_H
#define OLDEXTRADATAUTILITY_H

#include "config.h"

#include "Vector.h"
#include "ItemTypes.h"
#include "CoordinateTransformer.h"
#include "STLStrComp.h"
#include "MC2String.h"
#include "LangTypes.h"

#include <vector>

class OldItem;
class OldStreetSegmentItem;
class OldNode;
class OldConnection;
class OldGenericMap;

/**
 *    This file contains methods that could be used when changing things
 *    on one map. They are located here (and not in e.g. OldGenericMap or 
 *    GMSMap) to make sure that they could be used by other programs, e.g.
 *    MapEditor. This also means that You should know what You are doing
 *    when/if You change or add things to this class!
 *
 */
class OldExtraDataUtility {
   public:
      /**
        *   @name The different record types.
        */
      enum record_t {
         /**
          *    This is not a real record -- only a comment!
          */
         COMMENT,

         /**
          *    Add cityparts, with lon lat and one to many names to a map
          */
         ADD_CITYPART,

         /**
          *    Update the name of an item.
          */
         UPDATE_NAME,

         /**
          *    Set the ramp-attribute of the given street-segment item
          *    to a new value (bool).
          */
         SET_RAMP,

         /**
          *    Set the roundabout-attribute of the given street-segment 
          *    item to a new value (bool).
          */
         SET_ROUNDABOUT,

         /**
          *    Set the mulit-dig.-attribute of the given street-segment 
          *    item to a new value (bool).
          */
         SET_MULTIDIGITISED,

         /**
          *    Set the entryrestrictions of the given node item to a new 
          *    value (ItemTypes::entryrestriction_t).
          */
         SET_ENTRYRESTRICTIONS,

         /**
          *    Set the speedlimit of the given node item to a new 
          *    value (int).
          */
         SET_SPEEDLIMIT,

         /**
          *    Set the level of the given node item to a new 
          *    value (int).
          */
         SET_LEVEL,

         /**
          *    Set the turndirection of the given connection to a new 
          *    value (ItemTypes::turndirection_t).
          */
         SET_TURNDIRECTION,

         /**
          *    Set the the housenumber of an OldStreetSegmentItem. Left/right,
          *    start/end and the type as an integer.
          */
         SET_HOUSENUMBER,

         /**
          *     Add one name to an item.
          */
         ADD_NAME_TO_ITEM,
            
         /**
          *    Set the vehicle restrictions of the given connection to a new 
          *    value.
          */
         SET_VEHICLERESTRICTIONS,

         /**
          *    Add a landmark to the map, calculate the landmark 
          *    description and store in the landmark table.
          */
         ADD_LANDMARK,
         
         /**
          *    Add a single landmark to the map (only to one connection),
          *    and store the description in the landmark table.
          */
         ADD_SINGLE_LANDMARK,

         /**
          *    Remove one name of an item.
          */
         REMOVE_NAME_FROM_ITEM,
         
         /**
          *    Remove one item from the map. If the item is used as
          *    a landmark, also those records in the landmark table are
          *    removed from the map.
          */
         REMOVE_ITEM,

         /**
          *    Add a signpost to a connection.
          */
         ADD_SIGNPOST,
         
         /**
          *    Remove a signpost from a connection.
          */
         REMOVE_SIGNPOST,
         
         /**
          *    Set the roundaboutish-attribute of the given street-segment 
          *    item to a new value (bool).
          */
         SET_ROUNDABOUTISH,

         /**
          *    Set the controlled access-attribute of the given street-segment 
          *    item to a new value (bool).
          */
         SET_CONTROLLEDACCESS,

         /**
          *    Set the water type of a water item to a specific value.
          */
         SET_WATER_TYPE,

         /**
          *    Set the road toll attribute of the given node
          *    to a new value (bool).
          */
         SET_TOLLROAD,

         /**
          *    The number of record-types.
          */
         NUMBER_OF_RECORD_TYPES
      };

      /**
       *    Get the record type (<tt>record_t</tt>) from a string.
       *    @param s A string with the name of the record.
       *    @return  The record_t describes by s. NUMBER_OF_RECORD_TYPES
       *             will be returned if no matching record is found.
       */
      static record_t getRecordType(MC2String& s);


      /**
       *    Describes what connections to choose when adding landmarks
       *    to the landmark table in the map. The landmark is tied to 
       *    either all connections, connections with a turndescription 
       *    that says ahead or followroad, or connections with a 
       *    description that is not ahead or followroad (turn).
       *    
       *    The connections selection is dependent on where the 
       *    coordinate for finding close street segments is located.
       *
       *    If the ssi coordinate is in a crossing, the selection 
       *    will contain those connections that lead to any node(s) 
       *    in the crossing.
       *    If the ssi coordinate is on one street segment, the 
       *    selection will contain those connections that lead to the 
       *    crossings in both nodes of the segment, and comes from 
       *    the other node of that segment, i.e. connections OVER 
       *    the segment.
       */
      enum selectConn_t {
        
         /// The landmark will be tied to any connections in the crossing(s).
         ALLCONN,
         
         /// The landmark will be tied to ahead|followroad connections.
         AHEADCONN,
         
         /// The landmark will be tied to "turn"-connections.
         TURNCONN,
         
         /**
          *    OldConnections will be chosen as above, but the offset 
          *    of the ssi coordinate is used for setting a distance on 
          *    how far from the fromNode of the connection the landmark 
          *    is located. The location is set to pass, regardless of 
          *    the turndescription for the connection(s).
          *    These three selection alternatives will not be so good
          *    to use if the ssi coordinate is located in a crossing, 
          *    but work very fine is the ssi coordinate is on a segment.
          */
//         NB: dist not (yet) implemented in the landmark table.
//         ALLCONN_DIST,
//         AHEADCONN_DIST,
//         TURNCONN_DIST,

         /// Tie to no connection.
         NOCONN
      };

      /**
       *    Find out what connections (<tt>selectConn_t</tt>) to tie 
       *    landmarks to.
       *    @param s    A string with the name of the connections selection.
       *    @return     The selectConn_t described by s. NO_CONN will 
       *                be returned if no matching record is found.
       */
      static selectConn_t getSelectConns(MC2String& s);

      /**
       *    Delimiter used in extra data files.
       */
      static const MC2String edFieldSep;
      
      /**
       *    Create a string from sub strings in a vector. The string created
       *    will be in extra data style, i.e. use this method to create
       *    strings to write in extra data files. It uses the edFieldSep 
       *    as separator. Separator will be written after the last substring.
       *    It is possible to add the EndOfRecord tag (with separator) 
       *    after the last sub string.
       *    @param   subStrings  Sub strings to merge into one string.
       *    @param   addEndOfRec If true add the EndOfRecord-tag last,
       *                         if false (default) don't add.
       */
      static MC2String createEDFileString( 
               vector<MC2String>& subStrings, bool addEndOfRec = false );
      
      

       /**
        *      Updates a misspelled or faulty name. This is the
        *      method called from ExtraDataReader and MapEditor.
        *
        *      @param theMap      The map containing the name.
        *      @param item        The selected item.
        *      @param it          The itemtype of the items that has this name.
        *      @param ename       The existing name (misspelled).
        *      @param nt          The name type of the name.
        *      @param lang        The language type of the name.
        *      @param nname       The new name (correctly spelled).
        *      @param affectsStreets Vector where to store ids of items
        *                        that affects streets (ssi + si). Used
        *                        when applying extra data to merged maps.
        *      @return  The number of items that had their names updated.
        *               (-1) upon error.
        */
       static int updateName(OldGenericMap* theMap,
                       OldItem* item,
                       ItemTypes::itemType it,
                       MC2String ename,
                       ItemTypes::name_t nt,
                       LangTypes::language_t lang,
                       MC2String nname,
                       vector<uint32>& affectsStreets);

       /**
        *      Removes a name of an item.
        *      @param theMap      The map containing the name.
        *      @param item        The selected item.
        *      @param it          The itemtype of the items that has this name.
        *      @param ename       The name to remove.
        *      @param nt          The name type of the name.
        *      @param lang        The language type of the name.
        *      @param affectsStreets Vector where to store ids of items
        *                        that affects streets (ssi + si). Used
        *                        when applying extra data to merged maps.
        *      @return  The number of items that had their names removed.
        *               (-1) upon error.
        */
       static int removeNameFromItem(OldGenericMap* theMap,
                       OldItem* item,
                       ItemTypes::itemType it,
                       MC2String& ename,
                       ItemTypes::name_t nt,
                       LangTypes::language_t lang,
                       vector<uint32>& affectsStreets);

      /**
       *    Add a name to an item either when the name is missing or
       *    when the item should have more than one name. This is the 
       *    method called from ExtraDataReader and MapEditor.
       *
       *    @param theMap    The map where the item is located.
       *    @param item      The selected item.
       *    @param it        The type of the item.
       *    @param nname     The new name that should be given the item.
       *    @param nt        The name type of the new name.
       *    @param lang      The language type of the new name.
       *    @param affectsStreets Vector where to store ids of items
       *                        that affects streets (ssi + si). Used
       *                        when applying extra data to merged maps.
       */
      static int addNameToItem(OldGenericMap* theMap,
                               OldItem* item,
                               ItemTypes::itemType it,
                               MC2String nname,
                               ItemTypes::name_t nt,
                               LangTypes::language_t lang,
                               vector<uint32>& affectsStreets);

 
      /**
       *    Get the offset in one item for one name with specified
       *    string index, name type and language.
       *
       *    @param item       The item to check.
       *    @param enameIndex The index of the string to look for.
       *    @param nt         The name-type to look for. If maxDefinedName
       *                      is given the name type will not be looked upon
       *                      when searching for the name offset. This will 
       *                      give as a result the item's first matching name, though.
       *    @param lang       The language to look for.
       *    @return The offset of the name with given stringindex, type
       *            and language. MAX_BYTE is returned upon error.
       */
      static byte getNameOffsetForItem(OldItem* item, 
                                       uint32 enameIndex,
                                       ItemTypes::name_t nt,
                                       LangTypes::language_t lang);

      /**
       *    Read a complete record and store it as strings in vector.
       *    The end-tag "EndOfRecord" is not added to the vector.
       *    @param   is                The stream to read the record from.
       *    @param   recordAsStrings   Outparameter where the record will be
       *                               stored.
       *    @return  True.
       */
      static bool readNextRecordFromFile(istream& is,
                                  vector<MC2String>& recordAsStrings);

      /**
       *    Find a node from two coordinates. 
       *    @param theMap    The map where to look for a node.
       *    @param coordType The type of the coordinates in the strings.
       *    @param coord1    A string with the first coordinate.
       *    @param coord2    A string with the second coordinate.
       *    @return The node that was found, false otherwise.
       */
      static OldNode* getNodeFromStrings(
                              OldGenericMap* theMap,
                              CoordinateTransformer::format_t coordType,
                              const MC2String& coord1, 
                              const MC2String& coord2);

      /**
       *    Get the numeric value of a coordinate in the location token 
       *    '(' <LAT> ',' <LON> ')'.
       *
       *    @param s         The string token to convert.
       *    @param lat       Latitude value.
       *    @param lon       Longitude value.
       *    @param coordType The type of the coordinate to transform.
       *
       *    @return  True upon success, false otherwise.
       */
      static bool strtolatlon(const MC2String& s, int32& lat, int32& lon,
                              CoordinateTransformer::format_t coordType);

      /**
        *   Adds a landmark to the landmark table of a map.
        *   @param theMap        The map in which to add the landmark.
        *   @param it            Type of the item representing the landmark.
        *   param pt             If a poi, the pointOfInterest type.
        *   @param importance    Importance of the landmark, 0-4.
        *   @param lat/lon       Coordinate for finding close street 
        *                        segment(s), to which this landmarks
        *                        should be added.
        *   @param radius        Search for ssi within radius meters.
        *   @param itemlat       Latitude coordinate of the item 
        *                        representing the landmark.
        *   @param itemlon       Longitude coordinate of the item
        *                        representing the landmark.
        *   @param itemName      Name of the item representing the landmark.
        *   @param selConns      Gives what connections the landmark
        *                        should be tied to, see selectConn_t.
        *
        *   @return  The number of connections the landmark is tied to.
        */
      static int addLandmark(OldGenericMap* theMap,
                             ItemTypes::itemType it,
                             ItemTypes::pointOfInterest_t pt,
                             uint32 importance,
                             int32 lat, int32 lon,
                             uint32 radius,
                             int32 itemlat, int32 itemlon,
                             const char* itemName,
                             selectConn_t selConns);

      /**
        *   Add a landmark of type railway.
        *
        *   @return  The number of connections the landmark is tied to.
        */
      static int addRailwayLandmark(OldGenericMap* theMap,
                             ItemTypes::itemType it,
                             uint32 importance,
                             int32 lat, int32 lon,
                             uint32 radius,
                             selectConn_t selConns);

      /**
        *   Add a landmark of type builtUpArea, area or poi.
        *
        *   @return  The number of connections the landmark is tied to.
        */
      static int addOtherLandmark(OldGenericMap* theMap,
                             ItemTypes::itemType it,
                             ItemTypes::pointOfInterest_t pt,
                             uint32 importance,
                             int32 lat, int32 lon,
                             uint32 radius,
                             int32 itemlat, int32 itemlon,
                             const char* itemName,
                             selectConn_t selConns);

      /**
        *   Add a single landmark to the landmark table of a map. The 
        *   landmark record is tied to the connection from fromNode 
        *   to toNode.
        *   @param theMap     The map in which to add the landmark.
        *   @param lmItem     The item representing the landmark.
        *   @param importance Importance of the landmark, 0-4.
        *   @param location   The landmark location in relation to 
        *                     the conection.
        *   @param side       The landmark side in relation to the connection.
        *   @param fromNode   The fromNode of the connection.
        *   @param toNode     The toNode of the connection.
        *
        *   @return True if the landmark is added to the landmark table 
        *           of the map, false otherwise.
        */
      static bool addSingleLandmark(OldGenericMap* theMap,
                             OldItem* lmItem,
                             uint32 importance,
                             int location,
                             int side,
                             OldNode* fromNode, OldNode* toNode);
                              

      /**
       *    Removes an item from the map. If the item is used as 
       *    a landmark, also the landmark records in the landmark 
       *    table of the map are removed.
       *    
       *    @param theMap  The map from which to remove the item.
       *    @param item    The item to remove.
       */
      static bool removeItem(OldGenericMap* theMap, OldItem* item);


      /**
       *    If a node changed by an extra data record is connected
       *    to a virtual routeable item, the change is in this method
       *    applied also to the virtual's node.
       *    @param   theMap   The map in which the node is located
       *    @param   nodeID   The id of the node that was changed by extra data.
       *    @param   val      The new value.
       *    @param   edType   The kind of extra data correction.
       *    @param   changedVirtuals   Outparam that is filled with node ids
       *                      of any virtual nodes that are changed.
       *    @return  True if any connected virtual nodes were found and 
       *             changed, false if not.
       */
      static bool transferChangeToVirtualNode(OldGenericMap* theMap,
                              uint32 nodeID, int val, record_t edType,
                              vector<uint32>& changedVirtuals);

   private:

      /**
        *   Scan for next token in a extra data record string. The tokens
        *   separated by the edFieldSep string.
        *   @param   src   Source string to scan for next token.
        *   @param   token Outparam, the token.
        *
        *   @return  True upon success = there was another token to read
        *            from source string, false otherwise = no token.
        */
      static bool nextToken( MC2String& src, MC2String& token );

      /// Get all the items in the map with a certain name string.
      static uint32 getAllItemsWithName(OldGenericMap* theMap,
                                 MC2String& str,
                                 Vector* result);

      /// Get all the items in the map with a certain name string index.
      static void getAllItemsWithName(OldGenericMap* theMap,
                                      const uint32,
                                      Vector* result);

      /**
       *    Find out if an item has a given name with nameType and 
       *    language.
       *    @param item           The item to check.
       *    @param rawStringIndex String-index of the name, including
       *                          name_t and language_t.
       *    @return True if the item has the name+name_t+language_t,
       *            false otherwise.
       */
      static bool hasItemName(OldItem* item, const uint32 rawStringIndex);


      /**
       *   @name Landmark functions
       *    Methods that are used for calculating landmark locations etc
       *    when adding landmarks from extra data.
       */
      //@{
         /**
           *   Calculates location and side for the landmark (lm) description.
           *   @param theMap     The map where the landmark is located.
           *   @param conn       The connection, to which the landmark 
           *                     should be tied. 
           *   @param toNodeID   Which node conn leads to.
           *   @param itemlat    Latitude of the item representing the lm.
           *   @param itemlon    Longitude of the item representing the lmk.
           *   @param itemID     The iD of the item representing the landmark.
           *   @param lmType     The type of the landmark.
           *   @param ssiIDs     Vector with the ssiIDs that is in the
           *                     crossing, to which the landmark is tied to.
           *   @param location   Outparam landmark location.
           *   @param side       Outparam landmark side.
           *
           *   @return True, if the outparameters are set alright. False if 
           *           the params are not set alright, e.g. if the crossing 
           *           was not a 2, 3 or 4-ways crossing. Then the landmark 
           *           should not be added.
           */
         static bool getLandmarkLocationAndSide(
               OldGenericMap* theMap, OldConnection* conn, uint32 toNodeID,
               int32 itemlat, int32 itemlon, uint32 itemID, 
               ItemTypes::landmark_t lmType, Vector& ssiIDs,
               ItemTypes::landmarklocation_t& location,
               SearchTypes::side_t& side);

         /**
           *   Get side for the landmark description.
           *   Calculates on which side of the street segment with direction 
           *   from node nodeNbr the landmark represented by a point 
           *   (itemlat,itemlon) is located.
           *
           *   @param ssi        The street segment.
           *   @param nodeNbr    From what node (0 or 1) on the ssi.
           *   @param itemlat    The lat coordinate of the point (the lm).
           *   @param itemlon    The lon coordinate of the point (the lm).
           *   @param fromSide   True if ssi is the segment you use to get TO
           *                     the crossing and false if ssi is the segment 
           *                     that leads FROM the crossing. To and from are 
           *                     related to the connection, to which this 
           *                     landmark description is tied.
           *   @param within     If the landmarkitem coordinate is "within"
           *                     the ssi. From the withinSsi-method.
           *   
           *   @return  The side, valid values are right_side, left_side, 
           *            unknown_side or side_does_not_matter. Unknown is
           *            returned if the point is located just in front of 
           *            or right behind the ssi. Side_does_not_matter is 
           *            returned if the crossing is located IN the 
           *            landmark (e.g. a park).
           */
         static SearchTypes::side_t getSide(OldStreetSegmentItem* ssi,
               uint32 nodeNbr, int32 itemlat, int32 itemlon,
               bool fromSide = true, bool within = true);

         /**
           *   Finds out if a landmark represented by a point (itemlat,itemlon)
           *   is located "within" the ssi or not. Within = if the closest 
           *   coordinate to the point on ssi is not in the crossingnode of 
           *   the ssi.
           *
           *   @return True if the point is within the ssi, false if it isn't.
           */
         static bool withinSsi(OldStreetSegmentItem* ssi, 
                                 int32 itemlat, int32 itemlon,
                                 int32 crossinglat, int32 crossinglon);

         /**
          *    Checks if a point is located on a street segment.
          *    Checks every pair of coordinates of the segment and if the
          *    distance from the point to any of the sub-segment is 0, 
          *    the point is considered to be located on the segment.
          *    
          *    @return True if the point is on the segment, false otherwise.
          */
         static bool onSsi( OldStreetSegmentItem* ssi,
                            int32 itemlat, int32 itemlon);
         
         /**
           *   Finds out if the crossing consisting of some street segments 
           *   can be considered to look like a T-crossing. Here T-crossing
           *   is defined by that one/many pair of connections (a connection 
           *   and its reverse) in the crossing have turndescription 
           *   AHEAD or FOLLOWROAD.
           *   @param   theMap   The maps that the crossing is located in.
           *   @param   ssiIDs   Vector with the itemIDs of the street 
           *                     segments that are in the crossing.
           */
         static bool crossingTlike(OldGenericMap* theMap, Vector& ssiIDs);
      //@}

      /**
       *    Initiate the dictionaries used in this class. Uses the static
       *    member m_initiated to make sure the class only is initiated
       *    once.
       */
      static void init();

      /**
       *    Is this class initiated or not. Used by the init()-method.
       */
      static bool m_initiated;

      struct ltstrcase {
         bool operator()(const char* a, const char* b ) {
            return strcasecmp( a, b ) < 0;
         }
      };

      typedef map<const char*, record_t, ltstrcase> recordMap_t;

      /**
       *    Dictionary used to look-up record types (<tt>record_t</tt>) 
       *    from strings.
       */
      static recordMap_t m_recordTypes;

      typedef map<const char*, selectConn_t, ltstrcase> selectConnMap_t;
      
      /**
       *    Dictionary used to look-up what connections that a landmark 
       *    should be tied to (<tt>selectConn_t</tt>) from strings.
       */
      static selectConnMap_t m_selectConns;

};

#endif // OLDEXTRADATAUTILITY_H

