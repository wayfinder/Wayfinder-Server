/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSER_MAP_HANDLER_H
#define PARSER_MAP_HANDLER_H

#include "config.h"

#include "ParserHandler.h"
#include "ImageTable.h"
#include "DrawingProjection.h"
#include "LangTypes.h"

#include <utility>
#include <iosfwd>

class SharedBuffer;
class SimplePoiDescParams;
class SimplePoiMapParams;
class GfxFeatureMapImageRequest;

class SquareMapParams {
public:
   SquareMapParams() : m_x(-1),
                       m_y(-1),
                       m_zoom(-1),
                       m_lang(LangTypes::invalidLanguage),
                       m_nbrPixels( 0 ) {}
   SquareMapParams( int x,
                    int y,
                    int zoom,
                    LangTypes::language_t lang,
                    uint32 pixels = CylindricalProjection::NBR_PIXELS )
      : m_x(x), m_y(y), m_zoom(zoom), m_lang(lang),
        m_nbrPixels( pixels )
   {}

   void setNbrPixels( uint32 pixels ) {
      m_nbrPixels = pixels;
   }

   int x() const {
      return m_x;
   }

   int y() const {
      return m_y;
   }

   int zoom() const {
      return m_zoom;
   }
   
   LangTypes::language_t lang() const {
      return m_lang;
   }

   uint32 nbrPixels() const {
      return m_nbrPixels;
   }

private:
   /// X value
   int m_x;
   /// Y value
   int m_y;
   /// The zoom value
   int m_zoom;
   /// The language
   LangTypes::language_t m_lang;
   /// Number of pixels
   uint32 m_nbrPixels;
};

/// Debug output for SquareMapParams
ostream& operator << ( ostream& o, const SquareMapParams& p );


class SimplePoiDescParams {
};

/**
 *   Map handling functions for a ParserThread.
 */
class ParserMapHandler : public ParserHandler {
public:
   /// Standard ParserHandler contstructor
   ParserMapHandler( ParserThread* thread,
                     ParserThreadGroup* group );
   
   /// Destructor
   ~ParserMapHandler();

   enum errorCode {
      /// It is ok
      OK          =  0,
      /// Unknown error
      ERROR       = -1,
      /// Timeout has occurred
      TIMEOUT     = -2,
      /// Bad parameters supplied
      BAD_PARAMS  = -5,
   };

   /// What the buffer creating functions return.
   typedef pair<errorCode, SharedBuffer*> bufPair_t;
   
   /** Returns a saved SimplePoiMap for the supplied params. Or NULL and err
    * @param params the map params
    * @return saved simple poi map in buffer and error id if buffer is null.
    */
   bufPair_t getSimplePoiMap( const SquareMapParams& params );

   /// Returns a saved SimplePoiDesc for the suppiled params. Or NULL and err
   bufPair_t getSimplePoiDesc( const SimplePoiDescParams& params,
                               ImageTable::ImageSet imageSet );
private:

   /// Creates a GfxFeatureMapRequest or returns NULL if error
   GfxFeatureMapImageRequest* createPoiGfxRequest( const SquareMapParams& params );
};


#endif
