/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDSTRINGSIGNPOST_H
#define EXPANDSTRINGSIGNPOST_H

#include "config.h"
#include "MC2String.h"
#include "GDRGB.h"
#include <vector>

class Packet;


/**
 * Class representing a SignPost. From a GMSSignPostSet.
 *
 */
class ExpandStringSignPost {
   public:
      /**
       * The element type.
       */
      enum element_t {
         /// undefined
         undefined = 0,
         /// Exit
         exit = 1,
      };

      /**
       * Create a new ExpandStringSignPost from values.
       *
       * @param textColor Color of the text, index into GDColor.
       * @param frontColor Color of the front, index into GDColor.
       * @param backColor Color of the background, index into GDColor.
       */
      ExpandStringSignPost( const MC2String& text, 
                            byte textColor,
                            byte frontColor,
                            byte backColor,
                            uint32 priority, uint32 elementType, 
                            uint32 dist, bool ambiguous );

      /**
       * Create a new ExpandStringSignPost to be called load on later.
       */
      ExpandStringSignPost();

      /**
       * Get the text of this signPost.
       */
      const MC2String& getText() const;

      /**
       * Get the text color of this signPost.
       */
      byte getTextColor() const;

      /**
       * Get the front color of this signPost.
       */
      byte getFrontColor() const;

      /**
       * Get the back color of this signPost.
       */
      byte getBackColor() const;

      /**
       * Get priority of this signPost.
       */
      uint32 getPriority() const;

      /**
       * Get element type of this signPost.
       */
      uint32 getElementType() const;

      /**
       * The distance for the sign. Distance into this segment.
       */
      uint32 getDist() const;

      /**
       * Set the distance for the sign.
       */
      void setDist( uint32 d );

      /**
       * Get if this is an ambiguous sign to this signPost.
       */
      bool getAmbiguous() const;

      /**
       * Load from packet.
       */
      void load( const Packet* p, int& pos );

      /**
       * Save to packet.
       */
      void save( Packet* p, int& pos ) const;

      /**
       * @return Size in packet. 
       */
      uint32 getSizeInPacket() const;

      /**
       * Prints signs to out.
       */
      void dump( ostream& out ) const;

      /**
       * Stream output.
       */
      inline friend ostream& operator << ( 
         ostream& os, const ExpandStringSignPost& s )
      {
         s.dump( os );
         return os;
      }

   private:
      /**
       * The text of this signpost 
       */
      MC2String m_text;

      /**
       * The text color of this signpost.
       */
      byte m_textColor;

      /**
       * The front color of this signpost.
       */
      byte m_frontColor;

      /**
       * The back color of this signpost.
       */
      byte m_backColor;

      /**
       * Priority of this signpost.
       */
      uint32 m_priority;

      /**
       * The element type.
       */
      uint32 m_elementType;

      /**
       * The distance.
       */
      uint32 m_dist;

      /**
       * There is other sign with same information, low prio on this.
       */
      bool m_ambiguous;
};


/**
 * Class holding SignPosts.
 *
 */
class ExpandStringSignPosts : public vector< ExpandStringSignPost > {
   public:
      /**
       * New vector of signPosts.
       */
      ExpandStringSignPosts();

      /**
       * Load from packet.
       */
      void load( const Packet* p, int& pos );

      /**
       * Save to packet.
       */
      void save( Packet* p, int& pos ) const;

      /**
       * @return Size in packet. 
       */
      uint32 getSizeInPacket() const;

      /**
       * Add a SignPost to this.
       */
      void addSignPost( const ExpandStringSignPost& l );

      /**
       * Prints signs to out.
       */
      void dump( ostream& out ) const;

      /**
       * Stream output.
       */
      inline friend ostream& operator << ( 
         ostream& os, const ExpandStringSignPosts& s )
      {
         s.dump( os );
         return os;
      }

   private:
};


// =======================================================================
//                                     Implementation of inlined methods =


inline 
ExpandStringSignPost::ExpandStringSignPost( 
   const MC2String& text, 
   byte textColor,
   byte frontColor,
   byte backColor,
   uint32 priority, uint32 elementType, uint32 dist, bool ambiguous )
      : m_text( text ), m_textColor( textColor ), m_frontColor( frontColor ),
        m_backColor( backColor ), m_priority( priority ),
        m_elementType( elementType ), m_dist( dist ), m_ambiguous( ambiguous )
{
   mc2dbg4 << "ExpandStringSignPost m_dist " << m_dist << endl;
}

inline 
ExpandStringSignPost::ExpandStringSignPost() 
      : m_text(), m_textColor(), m_frontColor(),
        m_backColor(), m_priority( 0 ), 
        m_elementType( 0 ), m_dist( 0 ), m_ambiguous( false )
{
}

inline const MC2String&
ExpandStringSignPost::getText() const {
   return m_text;
}

inline byte
ExpandStringSignPost::getTextColor() const {
   return m_textColor;
}

inline byte
ExpandStringSignPost::getFrontColor() const {
   return m_frontColor;
}

inline byte
ExpandStringSignPost::getBackColor() const {
   return m_backColor;
}

inline uint32 
ExpandStringSignPost::getPriority() const {
   return m_priority;
}

inline uint32
ExpandStringSignPost::getElementType() const {
   return m_elementType;
}

inline uint32
ExpandStringSignPost::getDist() const {
   return m_dist;
}

inline void
ExpandStringSignPost::setDist( uint32 d ) {
   m_dist = d;
}

inline bool 
ExpandStringSignPost::getAmbiguous() const {
   return m_ambiguous;
}


inline 
ExpandStringSignPosts::ExpandStringSignPosts() {
}

inline void
ExpandStringSignPosts::addSignPost( const ExpandStringSignPost& p ) {
   push_back( p );
}

#endif // EXPANDSTRINGSIGNPOST_H

