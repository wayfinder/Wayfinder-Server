/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VERSIONLOCKPARSER_H
#define VERSIONLOCKPARSER_H

#include "config.h"
#include <set>
#include "MC2String.h"

/**
 * Helper class used in navclientsettings parser.  It parses
 * VersionLock: lines and stores the result.  At the end of th parsing
 * function it is used to set the versionlock of each known client
 * type.
 */
class VersionLockParser
{
public:
   /**
    * Parse a single VersionLock line. VersionLock lines are formatted like this:
    * '^VersionLock:<version>(,<client_type>)*'
    * Whitespace is allowed before and after the separators (:,) and
    * at the end of the string.
    * Whitespace whithin a client_type is accepted.
    * The parsed data is stored in this object.
    * @param data The string to parse. Should start "VersionLock".
    */
   void parseVersionLock( const char* data );

   uint32 findVersionLock( const char* client_type );

   ostream& operator<<(ostream& stream) const;
private:
   /**
    * Represents a single VersionLock: line.
    */
   class VersionLockClientTypes
   {
      /** The versin lock value. */
      uint32 m_versionLock;
      /** The associated client_type expressions. */
      set<MC2String> m_clientTypes;
   public:
      /** 
       * Constructor. 
       * @param versionLock The version lock value.
       */
      VersionLockClientTypes( uint32 versionLock ) : 
         m_versionLock( versionLock )
      {}

      uint32 VersionLock() const
      {
         return m_versionLock;
      }

      /**
       * Add a new client_type expression. 
       * @param client_type The client_type expression.
       */
      void add( const MC2String& client_type );

      /**
       * Add all client_types expressions held by <code>other</code>
       * to this object.
       * @param other A VersionLockClientTypes object to copy
       *              client_type expressions from.
       */
      void add( const VersionLockClientTypes& other );

      bool matches( const char* client_type ) const;

      /**
       * Comparison operator that forms a strict weak ordering, making
       * it possible to store VersionLockClientTypes objects in
       * associative containers.
       * The comparison is done only on the m_versionLock member variable.
       * @param other The object to compare with.
       * @return True if the m_versionLock member variable of this
       *         object is less than the m_versionLock member variable
       *         of the other object.
       */
      bool operator<( const VersionLockClientTypes& other ) const
      {
         return this->m_versionLock < other.m_versionLock;
      }

      void print(ostream& stream) const;
   };

   typedef set<VersionLockClientTypes> VersionHolder;
   /**
    * All the parsed data is stored here. 
    */
   VersionHolder m_versionLockClientTypes;
};
#endif
