/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPRIGHTS_H
#define MAPRIGHTS_H

#include "config.h"
#include <iostream>
#include "UserEnums.h"
#include "PacketRW.h"

class UserUser;
class UserRight;

/**
 * This class handles user rights as applied to maps. Each right will
 * map to a single bit in the mapset, which means that logical 'and'
 * and 'or' can be used to handle these rights.
 *
 */
class MapRights : public PacketRW {
public:
   /**
    * List of available rights. The enum value indicates which bit is
    * used for that right. The binary value of a right is calculated
    * with (1 << RIGHT).
    */
   enum Rights {
      /** The basic right to search and route and so on. */
      BASIC_MAP       = 0,
      // unused bits here 1-6
      TRAFFIC         = 7,
      SPEEDCAM        = 8, 
      // Ununsed bits 9-11
      FREE_TRAFFIC    = 12,
      // unused bits here 13-15
      FLEET           = 16,
      // unused bits here 17-22
      /// Traffic info test right.
      WF_TRAFFIC_INFO_TEST = 23,
      // Ununsed bits 24-26
      /// Access to speedcam information from SCDB.
      SCDB            = 27,
      DISABLE_POI_LAYER = 28,
   };

   /**
    * Some useful combination of rights. 
    */
   enum Masks {
      /** Combines all traffic information rights. */
      ALL_TRAFFIC = ((1 << TRAFFIC ) |
                     (1 << FREE_TRAFFIC ) | 
                     (1 << WF_TRAFFIC_INFO_TEST ) ),
      TRAFFIC_AND_SPEEDCAM = ALL_TRAFFIC | 
                     (1 << SPEEDCAM ) |
                     (1 << SCDB ),
   };

   /**
    * Constructor from enum MapRights::Rights. Note that since this
    * constructor is not explicit it will convert enum values to
    * MapRights objects if given a chance.
    * @param right A MapRights::Right value.
    */
   MapRights(enum Rights right);

   /**
    * Constructor from enum MapRights::Masks. Note that  this
    * constructor is explicit.
    * @param right A MapRights::Masks value.
    */
   explicit MapRights(enum Masks right);

   /**
    * Default constructor.
    * The constructed MapRights object represents no rights. 
    */
   MapRights();

   /**
    * Construct the rights from a string containing a hexadecimal
    * representation of a 64-bit integer.  The string is read from the
    * beginning. Any leading "0[xX]" is ignored. After that as many
    * hexadecimal digits as possible are read. If there is no digits
    * present in the string the constructed MapRights object will hold
    * no rights.
    * @param hexString The string to interpret.
    */
   explicit MapRights( const MC2String& hexString );

   /**
    * Set a given right in this MapRights object.
    * @param right The right to be set.
    * @return A reference to this, which enables chained calls and
    *         other syntactic goodness.
    */
   MapRights& setRight(const MapRights & right);

   /**
    * Remove a given right in this MapRights object.
    * @param right The right to remove.
    * @return A reference to this, which enables chained calls and
    *         other syntactic goodness.
    */
   MapRights& clearRight(const MapRights& right);

   /**
    * Operator &= enables masking of rights in this MapRights object
    * by another set of rights.  This MapRights object will loose all
    * rights except those set in the <code>right</code> MapRights
    * object.
    * @param right The masking set of rights.
    * @return A const reference to this.
    */
   const MapRights& operator&=(const MapRights& right);

   /**
    * Operator |= enables merging of rights in another Maprights
    * object into this MapRights object.
    * @param right The MapRights objects that will be merged into this
    *              object.
    * @return A const reference to this.
    */
   const MapRights& operator|=(const MapRights& right);

   /**
    * The operator ~ creates a MapRights object that has none of the
    * rights of this MapRights object, but all of the rights not held
    * by this MapRights object. Note that some bits will be set in the
    * returned MapRights object that have no corresponding right.
    * @return A inverted MapRights objct.
    */
   const MapRights operator~() const;

   /**
    * Makes it possible to use a MapRights object in boolean
    * expressions.
    * @return NULL if no rights bits are set, otherwise this. 
    */
   operator const void*() const;

   /**
    * This function is probably not correct.
    */
   MC2String getName(LangTypes::language_t lang) const;

   /** @name Functions for storing MapRights objects in Packets. */
   //@{
   /**
    * @return The number of bytes this MapRights object will occupy in
    *         a Packet.
    */
   size_t getSizeInPacket() const;
   /**
    * Store this MapRights object in a Packet.
    * @param packet The Packet to write into. 
    * @param pos The Packet position to start writing at. Will be
    *            updated to point past the end of the MapRights
    *            representation.
    * @return True.
    */
   bool save(Packet* packet,int& pos) const;
   /**
    * Load this MapRights object from a Packet.
    * Any previous informatino in this object is lost. 
    * @param packet The Packet to read from.
    * @param pos The Packet position to start reading at. Will be
    *            updated to point past the end of the MapRights
    *            representation.
    * @return True.
    */
   bool load(const Packet* packet,int& pos);
   //@}
   /**
    * Some types of rights should not be set on the same map.  This
    * function clears those rights that are less important(?) than
    * other present rights.
    * Actually calls the global
    * <code>MapRights filterMapRights(const MapRights&)</code>
    * function.
    * @return A reference to this MapRights object. 
    */
   inline MapRights& filterMapRights();
private:
   /** The type that represents the bitset. */
   typedef uint64 bitset_type;
   /** The actual rights are stored in this bitset. */
   bitset_type m_rights;
   /**
    * Constructs a MapRights object from an instance of the bitset_type.
    * @param rights The rights that will be set in the new object.
    */
   explicit MapRights(const bitset_type& rights);

   //friend declarations. 

   friend ostream& operator<<(ostream& o, const MapRights& r);
   friend bool operator==(const MapRights& lhs, const MapRights& rhs);
   friend bool operator<(const MapRights& lhs, const MapRights& rhs);
   friend bool operator>(const MapRights& lhs, const MapRights& rhs);
};

// ----- inline nonmember functions --------

/** @name Binary MapRights boolean operators. */
//@{
/**
 * Constructs a new MapRights object that has the rights that form the
 * intersection of the set of rights in the <code>lhs</code> and
 * <code>rhs</code> MapRights objects.
 * @param lhs The left side MapRights object.
 * @param rhs The right side MapRights object.
 * @return The intersection of the rights.
 */
inline const MapRights operator&(MapRights lhs, const MapRights& rhs) 
{
   return (lhs &= rhs);
}

/**
 * Constructs a new MapRights object that has the rights that form the
 * union of the set of rights in the <code>lhs</code> and
 * <code>rhs</code> MapRights objects.
 * @param lhs The left side MapRights object.
 * @param rhs The right side MapRights object.
 * @return The intersection of the rights.
 */
inline const MapRights operator|(MapRights lhs, const MapRights& rhs) 
{
   return (lhs |= rhs);
}
//@}

/**
 * @name Comparison operators. 
 * Note that the '==' and '!=' operators test equality, while the '<',
 * '>', '<=', and '>=' operators only define orderings of MapRights
 * objects.
 */
//@{
/**
 * Compare to MapRights objects for inequality.
 * @param lhs The MapRights object to the left of '!='.
 * @param rhs The MapRights object to the right of '!='.
 */
inline bool operator!=(const MapRights& lhs, const MapRights& rhs)
{
   return ! (lhs == rhs);
}

/**
 * Order two MapRights object. This <code>operator<</code> function
 * will form a complete weak ordering.
 * @param lhs The MapRights object to the left of '<'.
 * @param rhs The MapRights object to the right of '<'.
 * @return True if the left hand MapRights object should be ordered
 *         before the right hand side MapRights object, false otherwise.
 */
inline bool operator<(const MapRights& lhs, const MapRights& rhs)
{
   return lhs.m_rights < rhs.m_rights;
}

/**
 * Order two MapRights object. 
 * @param lhs The MapRights object to the left of '>'.
 * @param rhs The MapRights object to the right of '>'.
 * @return True if the left hand MapRights object should be ordered
 *         after the right hand side MapRights object, false otherwise.
 */
inline bool operator>(const MapRights& lhs, const MapRights& rhs)
{
   return lhs.m_rights > rhs.m_rights;
}

/**
 * Order two MapRights object. 
 * @param lhs The MapRights object to the left of '<='.
 * @param rhs The MapRights object to the right of '<='.
 * @return The logical inverse of <code>lhs > rhs</code>
 */
inline bool operator<=(const MapRights& lhs, const MapRights& rhs)
{
   return !(lhs > rhs);
}

/**
 * Order two MapRights object. 
 * @param lhs The MapRights object to the left of '>='.
 * @param rhs The MapRights object to the right of '>='.
 * @return The logical inverse of <code>lhs < rhs</code>
 */
inline bool operator>=(const MapRights& lhs, const MapRights& rhs)
{
   return !(lhs < rhs);
}
//@}

// ----- non-inline non-member functions 

/**
 * Convert a UserEnums::URType object to a MapRights object.
 * @param ur The UserEnums::URType object.
 * @return The resulting MapRights object. 
 */
MapRights toMapRight(const UserEnums::URType& ur);

/**
 * Convert a UserEnums::URType object to a MapRights object.
 * @param ur   The UserEnums::URType object.
 * @param now The current time, used to evaluate whether the rights is
 *             currently valid.
 * @return The resulting MapRights object. 
 */
MapRights toMapRight(const UserRight& ur, uint32 now);

/**
 * Some types of rights should not be set on the same map.  This
 * function clears those rights that are less important(?) than
 * other present rights.
 * @param The rights that should be filtered. 
 * @return The filtered rights.  
 */
MapRights filterMapRights( const MapRights& unfiltered );

/**
 * Prints a MapRights object to an ostream. The rights are printed in
 * hexadecimal format whithout 0-padding.
 * @param o The stream to print to.
 * @param r The MapRights object to print.
 * @return The stream.
 */
ostream& operator<<(ostream& o, const MapRights& r);


// -------- Inline MapRights member functions. 

inline MapRights::MapRights(MapRights::Rights right) : 
   m_rights((1 << right))
{}

inline MapRights::MapRights(MapRights::Masks mask) :
   m_rights( mask )
{}

inline MapRights::MapRights() : 
   m_rights(0)
{}

inline MapRights::MapRights(const MapRights::bitset_type& rights) : 
   m_rights(rights)
{}

inline MapRights& MapRights::setRight(const MapRights & right)
{
   *this |= right;
   return *this;
}

inline MapRights& MapRights::clearRight(const MapRights& right)
{
   *this &= ~right;
   return *this;
}

inline const MapRights& MapRights::operator&=(const MapRights& right)
{
   m_rights &= right.m_rights;
   return *this;
}

inline const MapRights& MapRights::operator|=(const MapRights& right)
{
   m_rights |= right.m_rights;
   return *this;
}

inline const MapRights MapRights::operator~() const
{
   return MapRights(~m_rights);
}

inline bool operator==(const MapRights& lhs, const MapRights& rhs) 
{
   return lhs.m_rights == rhs.m_rights;
}

inline MapRights::operator const void*() const
{
   return m_rights ? this : NULL;
}

inline size_t MapRights::getSizeInPacket() const
{
   return 8; //sizeof(bitset_type) as long as it is a uint64. 
}

inline MapRights& MapRights::filterMapRights()
{
   MapRights tmp = ::filterMapRights(*this);
   *this = tmp;
   return *this;
}

#endif
