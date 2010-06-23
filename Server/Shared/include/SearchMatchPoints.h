/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCH_MATCH_POINTS
#define SEARCH_MATCH_POINTS

#include "config.h"
#include "SearchTypes.h"
#include "ItemTypes.h"
#include "StringTable.h"

#define BPR(x) ( (x) ? "true" : "false" )

class Packet;

// - Class SearchMatchPoints ---

/**
 *   Class containing the points for a SearchMatchLink
 *   or SearchMatch. Move to another file later.
 */
class SearchMatchPoints {
public:

   /**
    *   Type of match.
    */
   enum matchedString_t {
      unknown    = 0,
      exact      = 1,
      close      = 2,
      full_close = 3,
      full_exact = 4
   };

   
   /**
    *   Constructor. Initializes values.
    */
   inline SearchMatchPoints();

   /**
    *   Initializes values to default.
    */
   inline void reset();
   
   /**
    *   Returns the calculated number of points.
    *   Moved to the cpp-file by pi to avoid too much
    *   recompiling. Move back when speed is important.
    */
   uint16 getPoints(bool debugp = false) const;

   /**
    *   Sets the itemtype and item subtype for the points.
    */
   void setTypes(uint32 searchType,
                 ItemTypes::itemType itemType,
                 uint16 subType);
   
   /**
    *   Sets the distance from the origin in meters.
    */
   inline void setDistanceMeters(uint32 meters);

   /**
    *   Sets the stringmatching used for finding the
    *   match.
    */
   inline void setMatching(SearchTypes::StringMatching m);

   /**
    *   Sets the type of string matched. Close or exact.
    */
   inline void setMatchedStringIs(SearchMatchPoints::matchedString_t x);

   /**
    *   Returns true if the match is full.
    */
   inline bool isFullMatch() const;
   
   /**
    *   Sets if the location is correct or not.
    */
   inline void setCorrectLocation(bool correct);
   
   /**
    *   Sets the number of points obtained from the BSSSElement
    *   in the SearchModule.
    */
   inline void setElemPoints(uint16 elemPoints);

   /**
    *   Sets the difference in house numbers if a house number
    *   was found.
    *   @param houseNbrDiff The difference in house numbers.
    */
   inline void setHouseNbrDiff(int16 houseNbrDiff);

   /**
    *   Sets the editdistance.
    */
   inline void setEditDistance(uint16 editDistance);

   /**
    *   Returns the edit distance.
    */
   inline uint16 getEditDistance() const;
   
   /**
    *   Sets the position in the string the match was.
    */
   inline void setPositionInString(uint8 pos);

   /**
    *   Returns the position in the string where the match was.
    */
   inline uint8 getPositionInString() const;
   
   /**
    *   Sets if the found ssi was in the correct region.
    */
   inline void setSSICorrectLocation( bool correctRegion );

   /**
    *   Sets if the correct order of the words were found.
    */
   inline void setAllWordsCorrectOrder(bool correctOrder);
   
   /**
    *   Saves the points in a packet.
    *   @param p   The packet to save the points in.
    *   @param pos Starting position of the points. Will
    *              be updated.
    *   @param matchInfoHere True if information that also
    *                        exists in the VanillaMatches should
    *                        be saved or false if it is set later.
    *   @return True if successful.
    */
   bool save(Packet* p, int& pos, bool matchInfoHere) const;

   /**
    *   Loads the points from a packet.
    *   @param p   Packet to load from.
    *   @param pos Position to start at. Will be updated.
    *   @param matchInfoHere True if information that also
    *                        exists in the VanillaMatches was
    *                        saved or false if it is set later.
    */
   bool load(const Packet* p, int& pos, bool matchInfoHere);

   /**
    *   Returns the size of the points in the packet.
    *   @param matchInfoHere True if information that also
    *                        exists in the VanillaMatches is to 
    *                        be saved or false if it is set later.
    */
   int getSizeInPacket(bool matchInfoHere) const;

   /**
    *   Returns true if the match matches the end of
    *   a word.
    */
   inline void setMatchesEndOfWord(bool doesIt);

   /**
    *   Returns true if the match matches the beginning
    *   a word.
    */
   inline void setMatchesBeginningOfWord(bool doesIt);

   /**
    *   Returns true if the number of points for this match
    *   is less than the number of points for the other one.
    *   Should be removed later.
    */
   inline bool operator< ( const SearchMatchPoints& other ) const {
      return getPoints() < other.getPoints();
   }

   inline bool operator== ( const SearchMatchPoints& other) const {
      return getPoints() == other.getPoints();
   }

   inline bool operator!= ( const SearchMatchPoints& other) const {
      return getPoints() != other.getPoints();
   }

   inline bool operator> ( const SearchMatchPoints& other) const {
      return getPoints() > other.getPoints();
   }

   /**
    *   Prints the points on the stream.
    *   @param stream The stream to print to.
    *   @param points The points to print.
    *   @return The stream.
    */
   inline friend ostream& operator<<( ostream& stream,
                                      const SearchMatchPoints& points);
   
private:
   /**
    *   The distance in meters from the 
    */
   uint32 m_distanceMeters;

   /**
    *   Difference in housenumber.
    */
   int16 m_houseNbrDiff;

   /**
    *   The number of points stored in the
    *   binarystringsearchstruct.
    */
   uint16 m_elemPoints;

   /**
    *   The edit distance. MAX_UINT16 = invalid.
    */
   uint16 m_editDistance;
   
   /**
    *   Position in string where the match was.
    */
   uint8 m_positionInString;

   /**
    *   Measure of the string difference.
    */
   uint8 m_stringDiff;

   /**
    *   The matching used.
    */
   SearchTypes::StringMatching m_matching;

   /**
    *   The type of match (close/exact).
    */
   matchedString_t m_matchedStringIs;

   /**
    *   True if the location is correct.
    */
   bool m_correctLocation;

   /**
    *   True if the best SSI (according to housenumbers )
    *   of a street is in the correct region. Only valid
    *   if m_houseNbrDiff is set.
    */
   bool m_ssiCorrectLocation;

   /**
    *   True if the match matches the end of a word.
    */
   bool m_matchesEndOfWord;

   /**
    *   True if the match matches the beginning of a word.
    */   
   bool m_matchesBeginningOfWord;
   
   /**
    *   True if allwords matching was in the correct order.
    *   Only valid if matching = allwords.
    */
   bool m_allWordsCorrectOrder;

   /**
    *   The searchtype of the points.
    */
   uint32 m_searchType;

   /**
    *   The itemType of the points.
    */
   ItemTypes::itemType m_itemType;

   /**
    *   The itemsubtype.
    */
   uint16 m_itemSubType;
   
};

inline void
SearchMatchPoints::reset()
{
   m_distanceMeters         = MAX_UINT32;
   m_houseNbrDiff           = MAX_INT16;
   m_editDistance           = MAX_UINT16;
   m_stringDiff             = MAX_UINT8;
   m_positionInString       = MAX_UINT8;
   m_matching               = SearchTypes::MaxDefinedMatching;
   m_matchedStringIs        = unknown;
   m_elemPoints             = 0;
   m_correctLocation        = false;
   m_ssiCorrectLocation     = false;
   m_matchesEndOfWord       = false;
   m_matchesBeginningOfWord = false;
   m_allWordsCorrectOrder   = true;
   m_itemType               = ItemTypes::numberOfItemTypes;
   m_itemSubType            = MAX_UINT16;
   m_searchType             = 0;
}

inline
SearchMatchPoints::SearchMatchPoints()
{
   reset();
}

inline void
SearchMatchPoints::setDistanceMeters(uint32 dist)
{
   m_distanceMeters = dist;
}

inline void
SearchMatchPoints::setMatching(SearchTypes::StringMatching m)
{
   m_matching = m;
}

inline void
SearchMatchPoints::setMatchedStringIs(SearchMatchPoints::matchedString_t x)
{
   m_matchedStringIs = x;
}

inline bool
SearchMatchPoints::isFullMatch() const
{
   return m_matchedStringIs == full_close ||
      m_matchedStringIs == full_exact;
}

inline void
SearchMatchPoints::setElemPoints(uint16 elemPoints)
{
   m_elemPoints = elemPoints;
}

inline void
SearchMatchPoints::setCorrectLocation(bool correct)
{
   m_correctLocation = correct;
}

inline void
SearchMatchPoints::setSSICorrectLocation(bool correctRegion)
{
   m_ssiCorrectLocation = correctRegion;
}

inline void
SearchMatchPoints::setAllWordsCorrectOrder(bool correctOrder)
{
   m_allWordsCorrectOrder = correctOrder;
}

inline void
SearchMatchPoints::setHouseNbrDiff(int16 houseDiff)
{
   m_houseNbrDiff = houseDiff;
}

inline void
SearchMatchPoints::setEditDistance(uint16 editDistance)
{
   m_editDistance = editDistance;
}

inline uint16
SearchMatchPoints::getEditDistance() const
{
   return m_editDistance;
}

inline void
SearchMatchPoints::setPositionInString(uint8 pos)
{
   m_positionInString = pos;
}

inline uint8
SearchMatchPoints::getPositionInString() const
{
   return m_positionInString;
}

inline void
SearchMatchPoints::setMatchesEndOfWord(bool doesIt)
{
   m_matchesEndOfWord = doesIt;
}

inline void
SearchMatchPoints::setMatchesBeginningOfWord(bool doesIt)
{
   m_matchesBeginningOfWord = doesIt;
}

inline ostream&
operator<<( ostream& stream,
            const SearchMatchPoints& points)
{
   return
      stream << "( m_matching = "
             << SearchTypes::getMatchingString(points.m_matching) << endl
             << "  m_correctLocation = "
             << BPR(points.m_correctLocation) << endl
             << "  m_correctSSILocation = "
             << BPR(points.m_ssiCorrectLocation) << endl
             << "  m_matchedStringIs = "
             << int(points.m_matchedStringIs) << endl
             << "  m_editDistance = " << points.m_editDistance << endl
             << "  m_houseNbrDiff = " << points.m_houseNbrDiff << endl
             << "  m_elemPoints = " << points.m_elemPoints << endl
             << "  m_positionInString = " << int(points.m_positionInString)
             << endl
             << "  m_matchesBegin = " << BPR(points.m_matchesBeginningOfWord)
             << endl
             << "  m_matchesEnd = " << BPR(points.m_matchesEndOfWord)
             << endl
             << "  m_stringDiff = " << int(points.m_stringDiff)
             << "  m_allWordsCorrectOrder = " 
             << BPR(points.m_allWordsCorrectOrder)
             << "  m_searchType = " << hex << points.m_searchType
             << dec << endl
             << "  m_itemType   = "
             << ItemTypes::getItemTypeAsString( points.m_itemType )
             << "  m_itemSubType = " << StringTable::getString( 
                ItemTypes::getPOIStringCode( 
                   ItemTypes::pointOfInterest_t( points.m_itemSubType ) ),
                StringTable::ENGLISH ) 
             << " )";
      
}

// - End of SearchMatchPoints


#endif
