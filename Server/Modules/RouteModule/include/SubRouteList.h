/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SUBROUTELIST_H
#define SUBROUTELIST_H

#include "config.h"
#include "RMSubRoute.h"
#include "RMDriverPref.h"
#include "SubRouteListTypes.h"

/**
 * Contains list of sub routes that might be a part of the route. 
 *
 */
class SubRouteList : public VectorElement
{
   public:
      /**
       * Create an empty sub route list.
       */
      SubRouteList();
   
      /**
       *   Delete this sub route list. 
       *   NB! Doesn't delete all the sub routes in the list.
       */
      virtual ~SubRouteList();

      /**
       * @name Operators.
       * @memo Implementation of operators for searching and sorting.
       * @doc  Making it possible for the subroutecontainer to search 
       *       for SubRouteLists in that nice binary way.
       */
      //@{
         // equal
         bool virtual operator == (const VectorElement& elm) const;

         // not equal
         bool virtual operator != (const VectorElement& elm) const;

         // greater
         bool virtual operator > (const VectorElement& elm) const;

         // less
         bool virtual operator < (const VectorElement& elm) const;
      //@}

      /**
       * Delete all sub routes in the list.
       */
      void deleteAllSubRoutes();
      
      /**
       * Add a new <code>SubRoute</code> to this list. If the route could be
       * added, the subroute is owned by the list and should not be used
       * anywhere else. I.e., if this function returns something else than
       * MAX_UINT32, the subroute should be deleted.
       * 
       * @param  subRoute The SubRoute that should be added to 
       *                  this list. Should never be used after
       *                  a succesful adding to the list.
       * @return          The vector index of the added subroute, 
       *                  MAX_UINT32 upon failure.
       */
      virtual uint32 addSubRoute( RMSubRoute* subRoute );

      /**
       * Get the number of sub routes in this list.
       * 
       * @return  The number of subroutes currently in the list.
       */
      inline uint32 getNbrSubRoutes() const;

      /**
       * Get one of the subroutes in the list.
       * 
       * @param  index The index of the subroute to return. Valid
       *               values are 0 <= index < getNbrSubRoutes().
       * @return       Subroute number index, NULL is returned upon error.
       */
      inline RMSubRoute* getSubRoute( uint32 index );

      /**
       *  Sets the subroute at position <code>index</code> to the value
       *  <code>val</code>.
       *  @param index The position to change.
       *  @param val   The value to set.
       *  @return The value previously at position <code>index</code>.
       */
      inline RMSubRoute* setSubRouteAt( uint32 index,
                                        RMSubRoute* val);
      
      
      /**
       * Copies list header data from subRouteList.
       *
       * @param subRouteList The list to copy from.
       */
      void copyListHeader(const SubRouteList& subRouteList);

      /**
       *    Set the ID of the route that the sub routes in this list
       *    is part of.
       *
       *    @param   id The new ID of the route.
       */
      inline void setRouteID( uint32 id );

      /**
       *    Returns the id of the route.
       *    
       *    @return the id of the route.
       */
      inline uint32 getRouteID() const;

      /**
       *    Get the cut-off cost for this route.
       *
       *    @return  The cut-off for this route.
       */
      inline uint32 getCutOff() const;

      /**
       *    Set the cut-off cost for this route. 
       *
       *    @param   cutOff   The new cut-off for this route. 
       */
      inline void setCutOff( uint32 cutOff );

      /** 
       * Gets the type of this list.
       * @see setListType
       * 
       * @return  The type of this list.
       */
      inline uint16 getListType() const;

      /**
       * Set the type of list. Valid lists types are found in list_type.
       * 
       * @param listType The new type for this list.
       */
      inline void setListType( uint16 listType );

      /**
       *    Get the total size of all the subroutes in this list (in 
       *    bytes).
       *    @return  The total size of the sub routes in this list.
       */
      inline uint32 getTotalSize() const;

      /**
       * Gets the value of m_routeOnHigherLevel.
       * 
       * @return True if this route is part of a higher level routing.
       */
      inline bool getRouteOnHigherLevel() const;

      /**
       * Get the routing direction.
       *
       * @return True if routing forward, false if routing backward.
       */
      inline bool isForwardRouting() const;

      /**
       *    Search for an RMSubRoute with the supplied external node.
       *    @param externalID The item id of the external node.
       *    @return The RMSubRoute with the supplied node or NULL.
       */
      RMSubRoute* findExternalNodeSubRoute( uint32 subRouteID );

      /**
       *    Get one of the external nodex.
       *    @param externalID is the item id of the external node.
       *    @return the subroute ID if found otherwise MAX_UINT32.
       */
      uint32 findExternalNode( uint32 externalID );


      /**
       * Checks if the prevSubRouteIDs of the given SubRoute is nested in
       * a loop
       *
       * @param  subRouteID The ID of the SubRoute to check for loops.
       * @return            True if the SubRoute chain loops.
       */
      inline bool checkForLoops(uint32 subRouteID);

      /**
       *    Writes the content of this list to standard out.
       */
      virtual void dump();

private:      
      /**
       * Function that tells if a SubRoute with lower cost exists 
       * in this SubRouteList.
       *
       * @param  subRoute The SubRoute to search for in the list.
       * @return True if the SubRoute was found in this SubRouteList, 
       *         otherwise false.
       */
      bool exists( RMSubRoute* subRoute );

      /**
       * Find, if any exist, the SubRoute in this SubRouteList with the
       * same external connections as the SubRoute given as inparameter.
       *
       * @param  subRoute The SubRoute to look for in this SubRouteList.
       * @return          The index in the SubRouteList of the SubRoute
       *                  wanted, MAX\_UINT32 if not found.
       */
      uint32 findSubRouteWithSameExternals(const RMSubRoute& subRoute) const;
      
      /**
       * All the sourroutes in this list.
       */
      std::vector<RMSubRoute*> m_subRouteVect;

      /**
       * The unique ID of this route. Used by leader to know which
       * subroutes that belong to the same route.
       */
      uint32 m_routeID;

      /**
       * The type of routes stored in this SubRouteList. The allowed values
       * are described by the enum list_types.
       */
      uint16 m_listType;

      /**
       * The maximum allowed cost of the routing (the smallest cost found
       * to reach a destination), cut off.
       */
      uint32 m_cutOff;

   /**
    *   Prints the CVS version of the .cpp file.
    */
   static bool printVersion();

   /**
    *   Dummy variable used when calling printVersion on startup.
    */
   static bool versionPrinted;

}; // SubRouteList

inline uint32 SubRouteList::getCutOff() const
{
   return m_cutOff;
}

inline void SubRouteList::setCutOff( uint32 cutOff )
{
   m_cutOff = cutOff;
}

inline uint16 SubRouteList::getListType() const
{
   return m_listType;
}

inline void SubRouteList::setListType( uint16 listType ) 
{
   m_listType = listType;
}

inline uint32 SubRouteList::getRouteID() const
{
   return m_routeID;
}

inline void SubRouteList::setRouteID( uint32 id ) 
{
   m_routeID = id;
}

inline bool SubRouteList::getRouteOnHigherLevel() const
{
   return ( ( m_listType == SubRouteListTypes::HIGHER_LEVEL_FORWARD ) ||
            ( m_listType == SubRouteListTypes::HIGHER_LEVEL_BACKWARD ) );
}

inline bool SubRouteList::isForwardRouting() const
{
   return ( m_listType != SubRouteListTypes::HIGHER_LEVEL_BACKWARD ); 
}


inline bool
SubRouteList::checkForLoops(uint32 subRouteID)
{
   bool status = false;
   
   RMSubRoute* subRoute = getSubRoute(subRouteID);

   if (subRoute != NULL) {
      uint32 prevSubRouteID = subRoute->getPrevSubRouteID();
      while ((prevSubRouteID != MAX_UINT32) &&
             (prevSubRouteID != subRouteID)) {
         prevSubRouteID = (getSubRoute(prevSubRouteID))->getSubRouteID();
      }
      if (prevSubRouteID == subRouteID) {
         mc2log << warn << "subRoute loops" << "ID " << subRouteID << endl;
         status = true;
      }
   }

   return status;
}


inline uint32 SubRouteList::getTotalSize() const 
{
   uint32 totSize = 0;
   for( uint32 i = 0; i < m_subRouteVect.size(); i++ ) {
      totSize += m_subRouteVect[ i ]->getPacketSize() + 4;
   }
   return totSize;
}

inline uint32 SubRouteList::getNbrSubRoutes() const
{
   return m_subRouteVect.size();
}


inline RMSubRoute*
SubRouteList::getSubRoute(uint32 index)
{ 
   if (index < m_subRouteVect.size()) {
      return m_subRouteVect[ index ];
   }
   else {
      return NULL;
   }
}

inline RMSubRoute*
SubRouteList::setSubRouteAt( uint32 index,
                             RMSubRoute* val )
{
   RMSubRoute* oldVal = getSubRoute(index);
   m_subRouteVect[index] = val;
   return oldVal;
}

#endif
