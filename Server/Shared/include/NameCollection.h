/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAMECOLLECTION_H
#define NAMECOLLECTION_H

#include "config.h"

#include "ItemTypes.h"
#include "LangTypes.h"
#include <vector>
#include <set>

class Name;
class DataBuffer;
class Packet;

/**
 *   Class representing a name with type and language.
 *
 */
class NameCollection : private vector<Name*>
{
  public:

      /**
       *   Default constructor.
       */
      NameCollection() {}
   
      /**
       *   Copy constructor.
       */
      NameCollection(const NameCollection& other);

      /**
       *   Assignment operator.
       */
      const NameCollection& operator=(const NameCollection& other);
      
      /**
       *    Free allocated memory.
       */
      virtual ~NameCollection();

      /**
       *    Add a name to the collection.
       *    The name will deleted upon destruction of the collection.
       *    @param   name  The name to add.
       */
      inline void addName( Name* name );

      /**
       *    Get the number of names in the collection.
       *    @return  The number of names in the collection.
       */
      inline uint32 getSize() const; 

      /**
       *    Get the name at position i.
       *    @param   i  The position to get the name from. Must be
       *                less than getSize().
       *    @return  The name at position i.
       */
      inline const Name* getName( uint32 i ) const;


      /**
       *    Get the best name in some sense given the language.
       *    @param   lang  The language type.
       *    @return  The best name.
       */
      const Name* getBestName( LangTypes::language_t lang ) const;

      /**
       *    Get a set of names that match the criterias given by the
       *    parameters.
       *    @param   lang  The language of the names.
       *    @param   type  The type of the names, or invalidName if all
       *                   types should be included. Default set to
       *                   invalidName.
       *    @return  A set of names matching the criterias given by
       *             the parameters.
       */
      set<const Name*> getNames( LangTypes::language_t lang, 
                                 ItemTypes::name_t type = 
                                     ItemTypes::invalidName ) const;

      /**
       * Print on ostream.
       *
       * @param stream The stream to print on.
       * @param collection  The NameCollection to print.
       * @return The stream.
       */
      friend ostream& operator<< ( ostream& stream,
                                   const NameCollection& collection );

      // -- Packet and DataBuffer functions

      /**
       *    Loads the ItemIDTree from the 
       *    supplied DataBuffer.
       *    @param buf Buffer to load from.
       *    @return True if ok.
       */
      bool load(DataBuffer* buf);
      
      /**
       *    Saves the NameCollection into the supplied DataBuffer.
       *
       *    @param  buf Buffer to save into.
       *    @return     True if ok.
       */
      bool save(DataBuffer* buf) const;
      
      /**
       *    Returns the number of bytes that this NameCollection
       *    needs in a databuffer.
       */
      int getSizeInDataBuffer() const;

      /**
       *    Loads the namecollection from the packet.
       *    @param p   Packet to load from.
       *    @param pos Position to start at. Will be updated.
       */
      bool load(const Packet* p, int& pos);
        
      /**
       *    Saves the NameCollection to the packet.
       *    @param p Packet to save the collection into.
       *    @param pos Position to start at. Will be updated.
       */
      bool save(Packet* p, int &pos) const;

      /**
       *    Returns the number of bytes that the namecollection
       *    will use in the packet.
       */
      int getSizeInPacket() const;
      
   private:
      /**
       * The static missing name Name node.
       */
      static Name m_missingName;
};

// ========================================================================
//                                      Implementation of inlined methods =

inline void
NameCollection::addName( Name* name ) 
{
   push_back( name );
}

inline uint32
NameCollection::getSize() const 
{
   return size();
}

inline const Name*
NameCollection::getName( uint32 i ) const
{
   return (*this)[i];
}

#endif
