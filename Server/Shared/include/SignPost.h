/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SIGNPOST_H
#define SIGNPOST_H

#include "config.h"
#include "GDColor.h"

class DataBuffer;
class GenericMap;

/**
 * Class for holding the colors for a signpost.
 * 
 */
class SignColors {
public:
   SignColors( GDUtils::Color::imageColor tColor,
               GDUtils::Color::imageColor fColor,
               GDUtils::Color::imageColor bColor )
         : textColor( tColor ), frontColor( fColor ), backColor( bColor )
   {}

   SignColors() 
         : textColor( GDUtils::Color::BLACK ), 
           frontColor( GDUtils::Color::BLACK ), 
           backColor( GDUtils::Color::BLACK )
   {}

   bool operator == ( const SignColors& other ) const {
      return
         textColor == other.textColor &&
         frontColor == other.frontColor &&
         backColor == other.backColor;
   }

   GDUtils::Color::imageColor textColor;
   GDUtils::Color::imageColor frontColor;
   GDUtils::Color::imageColor backColor;

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
    * Compare this with other SignColors.
    */
   bool operator < ( const SignColors& o ) const;
};


/**
 * Class used for sign posts for connections.
 *
 */
class SignPost {
public:
   /**
    * Constructor.
    */
   SignPost();

   /**
    * Constructor set all values.
    */
   SignPost( const SignColors& colors,
             uint32 dist,
             uint32 stringCode, bool isExit );


   /**
    * Destructor.
    */
   ~SignPost();

   /**
    * Writes the SignPost into a dataBuffer.
    *
    * @param dataBuffer Where to save.
    */
   void save( DataBuffer& dataBuffer ) const;

   /**
    * Fill this SignPost with information from a databuffer.
    *
    * @param dataBuffer The buffer with the data.
    */
   void load( DataBuffer& dataBuffer );

   /**
    * Get the colors of the signpost.
    */
   const SignColors& getColors() const;

   /**
    * Get the text stringcode.
    */
   uint32 getText() const;

   /**
    * Get the distance before the turn that the SignPost should be 
    * displayed.
    */
   uint32 getDist() const;

   /**
    * Get if it is exit sign.
    */
   bool getExit() const;

   /**
    * Set the colors.
    */
   void setColors( const SignColors colors );

   /**
    * Set the text stringcode.
    */
   void setText( uint32 stringCode );

   /**
    * Set the distance before the turn that the SignPost should be 
    * displayed.
    */
   void setDist( uint32 dist );

   /**
    * Set if it is exit sign.
    */
   void setExit( bool s );

   /**
    * Compare this with other SignPost.
    */
   bool operator < ( const SignPost& o ) const;

private:
   /**
    * The colors.
    */
   SignColors m_colors;

   /**
    * The distance.
    */
   uint32 m_dist;

   /**
    * The text.
    */
   uint32 m_stringCode;
};

// =======================================================================
//                                     Implementation of inlined methods =

inline const SignColors&
SignPost::getColors() const {
   return m_colors;
}

inline uint32
SignPost::getText() const {
   return m_stringCode;
}

inline void
SignPost::setColors( const SignColors colors ) {
   m_colors = colors;
}

inline void
SignPost::setText( uint32 stringCode ) {
   m_stringCode = stringCode;
}

inline bool 
SignPost::operator < ( const SignPost& o ) const {
   if ( m_stringCode != o.m_stringCode ) {
      return m_stringCode < o.m_stringCode;
   } else if ( m_dist != o.m_dist ) {
      return m_dist < o.m_dist;
   } else {
      return m_colors < o.m_colors;
   }
}

inline bool
SignColors::operator < ( const SignColors& o ) const {
   if ( textColor != o.textColor ) {
      return textColor < o.textColor;
   } else if ( frontColor != o.frontColor ) {
      return frontColor < o.frontColor;
   } else { // if backColor != o.backColor 
      return backColor < o.backColor;
   }
}

#endif // SIGNPOST_H
