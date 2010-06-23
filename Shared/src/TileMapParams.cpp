/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"

#include "TileMapParams.h"
#include "TileMapParamTypes.h"
#include "MC2SimpleString.h"

#include <stdio.h>
#include "BitBuffer.h"
#include <algorithm>

#include "RouteID.h"

#include "MathUtility.h"

// VC++ won't handle init in .h-file and Symbian will not handle
// the opposite.
const char * const
TileMapParams::c_sortedCodeChars = TILEMAP_CODE_CHARS;
const uint32 TileMapParams::c_sortedCodeCharsLen = 
   strlen( c_sortedCodeChars );

TileMapParams::TileMapParams( const char* paramString, bool knownUnknown )
{
   m_routeID = NULL;
   m_paramString = NULL;
   parseParamString( paramString, knownUnknown );
}

TileMapParams::TileMapParams( const MC2SimpleString& paramString, 
                              bool knownUnknown )
{
   m_routeID = NULL;
   m_paramString = NULL;
   parseParamString( paramString.c_str(), knownUnknown );
}

TileMapParams::TileMapParams( const TileMapParams& other ) {
   m_routeID = NULL;
   m_paramString = NULL;
   copyAttributes( other );
}

void
TileMapParams::updateParamString()
{
   // Alloc on stack is probably faster than with new
   byte innerBuf[64];
   // Zero the buffer.
   memset( innerBuf, 0, 64 );
   BitBuffer buf(innerBuf, 64);

   // Low bits first.
   buf.writeNextBits(m_importanceNbr & 0xf, 4);
   buf.writeNextBits(m_detailLevel, 4);
   
   int nbrBits = 15;
   if ( m_detailLevel > 0 ) {
      nbrBits = MathUtility::getNbrBitsSignedGeneric( m_tileIndexLat );
      int tmpNbrBits = 
         MathUtility::getNbrBitsSignedGeneric( m_tileIndexLon );
      nbrBits = MAX( nbrBits, tmpNbrBits );
      buf.writeNextBits( nbrBits, 4 );
   }
   
   if ( nbrBits > 8 ) {
      // Write lower bits first since they seem to differ more often.
      buf.writeNextBits(m_tileIndexLat & 0xff, 8);
      buf.writeNextBits(m_tileIndexLon & 0xff, 8);
      // Then some higher bits.
      buf.writeNextBits(m_tileIndexLat >> 8, nbrBits - 8);
      buf.writeNextBits(m_tileIndexLon >> 8, nbrBits - 8);
   } else {
      // Write all at once.
      buf.writeNextBits(m_tileIndexLat, nbrBits);
      buf.writeNextBits(m_tileIndexLon, nbrBits);
   }
  
   buf.writeNextBits(m_layer, 4);
   
   // Lowest bits of server prefix.
   buf.writeNextBits( m_serverPrefix & 0x1f, 5 ); 
   
   if ( m_mapOrStrings == TileMapTypes::tileMapData ) {
      // The features do not have any language.
   } else {
      // For strings, the language is important.
      // Lowest 3 bits first.
      buf.writeNextBits(m_langType & 0x7, 3);
      // Then next 3 higher bits.
      buf.writeNextBits((m_langType >> 3) & 0x7, 3 );
   }   
   
   // Highest bit of server prefix.
   // Nowdays highest bit of server prefix means special quirk mode.
   // 0x0 means old mode.
   // 0x1 means that an additional 7 bits are used for highest 
   //     bits for language type.
   
   int quirkyMode = 0;
   if ( m_langType >= 64 ) {
      // Language is too big to fit in old 6 bits.
      quirkyMode = 0x1;
   }
   
   // This used to be server prefix.
   buf.writeNextBits( quirkyMode, 1 ); 
   
   if ( quirkyMode == 0x1 ) {
      // Add additional 7 highest bits for language.
      buf.writeNextBits( (m_langType >> 6) & 127, 7 );
   }
   
   // Highest bits of importance nbr.
   buf.writeNextBits(m_importanceNbr >> 4, 1);
   // Inverted gzip
   buf.writeNextBits(!m_gzip, 1);
   
   if ( m_layer == TileMapTypes::c_routeLayer ) {
      if ( m_routeID == NULL ) {
         mc2log << error << "[TileMapParams]: "
                            "No routeID in route layer params" << endl;
         RouteID inval;
         inval.save(&buf);
      } else {
         m_routeID->save(&buf);
      }
   }
   
   // Char-encode. 
   int nbrChars = (buf.getCurrentBitOffset() + 5) / 6;
   // The length of the buffer cannot be larger than 64. I know it.
   // Add space for K and \0
   char* tmpString = new char[nbrChars+1+1];
   
   int pos = 0;
   if ( m_mapOrStrings == TileMapTypes::tileMapData ) {
      tmpString[pos++] = 'G'; // We use G for features now.
   } else {
      tmpString[pos++] = 'T'; // T as in sTring map. errm..
   }
   buf.reset();
   for( int i = 0; i < nbrChars; ++i ) {
      tmpString[pos++] = c_sortedCodeChars[buf.readNextBits(6)];
   }

   // Skip trailing zero '+'.
   while ( (pos > 0 ) && tmpString[pos - 1] == '+' ) {
      tmpString[ --pos ] = '\0';
   }
   tmpString[pos] = '\0';
   // Put the tmpString in the paramstring.
   delete m_paramString;
   m_paramString = new MC2SimpleStringNoCopy(tmpString, pos);

#if 0
   mc2log << "m_paramString = " << *m_paramString << endl;
   mc2log << "m_importanceNbr = " << m_importanceNbr << endl;
   mc2log << "m_detailLevel = " << m_detailLevel << endl;
   mc2log << "m_tileIndexLat = " << m_tileIndexLat << endl;
   mc2log << "m_tileIndexLon = " << m_tileIndexLon << endl;
   mc2log << "m_layer = " << m_layer << endl;
   mc2log << "m_gzip = " << m_gzip << endl;
   mc2log << "m_mapOrStrings = " << m_mapOrStrings << endl;
   mc2log << "m_langType = " << m_langType << endl;
   mc2log << "m_serverPrefix = " << m_serverPrefix << endl;
#endif   
}

void
TileMapParams::parseParamString( const char* paramString, bool knownUnknown ) {
   char firstChar = paramString[0];
   m_valid = true;

   if ( TileMapParamTypes::getParamType( paramString ) == 
        TileMapParamTypes::UNKNOWN ) {
      // Other not supported first chars
      m_valid = false;
      // Initialize members
      m_mapOrStrings = TileMapTypes::tileMapData;
      m_detailLevel = 0;
      m_tileIndexLat = 0;
      m_tileIndexLon = 0;
      m_layer = 0;
      m_serverPrefix = 0;
      m_langType = LangTypes::swedish;
      m_importanceNbr = 0;
      m_gzip = false;
      return;
   }

   // Skip the 'T' or 'G'
   if ( firstChar == 'T' ) {
      m_mapOrStrings = TileMapTypes::tileMapStrings;
   } else {
      m_mapOrStrings = TileMapTypes::tileMapData;
   }
           
   const char* paramChars = firstChar == '\0' ? paramString :
      paramString + 1;
   const int len = strlen( paramString );
   const int sortLen = c_sortedCodeCharsLen;//strlen(c_sortedCodeChars);
   const char* sortedEnd = c_sortedCodeChars + sortLen;
   
   // Alloc on stack is probably faster than with new
   byte innerBuf[ len ];//64]; // len might be more than 64*8/6=85.3
   // Zero the buffer.
   memset( innerBuf, 0, len );
   BitBuffer buf( innerBuf, len );
   
   for ( int i = 0; i < len; ++i ) {
      const char* foundChar = lower_bound(c_sortedCodeChars,
                                          sortedEnd,
                                          paramChars[i]);
      if ( foundChar == sortedEnd ) {
         if ( !knownUnknown ) {
            mc2dbg << "[TMP]: ERROR: Invalid character 0x"
                   << hex << int(paramChars[i]) << dec << " in params" << endl;
         }
         m_valid = false;
         return;
      }      
      buf.writeNextBits(int(foundChar-c_sortedCodeChars), 6);
   }
   buf.reset();
   int lowImportance;
   if ( buf.getBufferSize()*8 - buf.getCurrentBitOffset() >= 8 ) {
      // Low bits first.
      buf.readNextBits( lowImportance, 4 );
      buf.readNextBits( m_detailLevel, 4 );
   } else {
      m_detailLevel = 0;
      if ( m_valid ) {
         if ( !knownUnknown ) {
            mc2dbg << "[TMP]: ERROR: not bits for detailLevel " 
                   << MC2CITE( paramString ) << endl;
         }
         m_valid = false;
      }
   }
   int nbrBits = 15;
   if ( m_valid && m_detailLevel > 0 ) {
      // Read the number of bits used for the lat/lon index.
      if ( buf.getBufferSize()*8 - buf.getCurrentBitOffset() >= 4 ) {
         nbrBits = buf.readNextBits( 4 );
      } else {
         if ( m_valid ) {
            if ( !knownUnknown ) {
               mc2dbg << "[TMP]: ERROR: not bits for nbrBits "
                      << MC2CITE( paramString ) << endl;
            }
            m_valid = false;
         }
      }
   }


   if ( nbrBits == 0 ) {
      if ( m_valid ) {
         if ( !knownUnknown ) {
            mc2dbg << "[TMP]: ERROR: nbrBits is zero! " 
                   << MC2CITE( paramString ) << endl;
         }
         m_valid = false;
      }
      m_tileIndexLat = 0;
      m_tileIndexLon = 0;
   } else if ( nbrBits > 8 ) {
      // Read the lower bits first.
      if ( m_valid && int32( buf.getBufferSize()*8 - 
                             buf.getCurrentBitOffset() ) >=
           8 + (nbrBits - 8)*2 ) {
         int lowLat = buf.readNextBits( 8 );
         int lowLon = buf.readNextBits( 8 );
      
         // And then the higher.
         buf.readNextSignedBits( m_tileIndexLat, nbrBits - 8 );
         buf.readNextSignedBits( m_tileIndexLon, nbrBits - 8 );

         m_tileIndexLat = (m_tileIndexLat << 8) | lowLat;
         m_tileIndexLon = (m_tileIndexLon << 8) | lowLon;
      } else {
         m_tileIndexLat = 0;
         m_tileIndexLon = 0;
         if ( m_valid ) {
            if ( !knownUnknown ) {
               mc2dbg << "[TMP]: ERROR: not bits for tileIndexLat/Lon > 8 " 
                      << MC2CITE( paramString ) << endl;
            }
            m_valid = false;
         }
      }
   } else {
      if ( m_valid && int32(buf.getBufferSize()*8 - 
                            buf.getCurrentBitOffset()) >= nbrBits*2 ) {
         // Read all at once.
         buf.readNextSignedBits(m_tileIndexLat, nbrBits );
         buf.readNextSignedBits(m_tileIndexLon, nbrBits );
      } else {
         m_tileIndexLat = 0;
         m_tileIndexLon = 0;
         if ( m_valid ) {
            if ( !knownUnknown ) {
               mc2dbg << "[TMP]: ERROR: not bits for tileIndexLat/Lon "
                      << MC2CITE( paramString ) << endl;
            }
            m_valid = false;
         }
      }
   } 
   
   if ( m_valid && buf.getBufferSize()*8 - buf.getCurrentBitOffset() >= 4 ) {
      buf.readNextBits( m_layer, 4 );
   } else {
      m_layer = 0;
      if ( m_valid ) {
         if ( !knownUnknown ) {
            mc2dbg << "[TMP]: ERROR: not bits for layer " 
                   << MC2CITE( paramString ) << endl;
         }
         m_valid = false;
      }
   }
   
   // Serverprefix (now 5 bits).
   if ( m_valid && buf.getBufferSize()*8 - buf.getCurrentBitOffset() >= 5 ) {
      buf.readNextBits( m_serverPrefix, 5 );
   } else {
      if ( m_valid ) {
         if ( !knownUnknown ) {
            mc2dbg << "[TMP]: ERROR: not bits for lowerServerPrefix "
                   << MC2CITE( paramString ) << endl;
         }
         m_valid = false;
      }
   }
   
   if ( m_mapOrStrings == TileMapTypes::tileMapData ) {
      // The features do not have any language. Set to swedish.
      m_langType = LangTypes::swedish;
   } else {
      // Lowest bits first.
      uint32 lowLangType = 0;
      if ( m_valid && buf.getBufferSize()*8 - buf.getCurrentBitOffset() >= 6 )
      {
         buf.readNextBits( lowLangType, 3 );
         // Then highest bits.
         buf.readNextBits( m_langType, 3 );
      } else {
         lowLangType = 0;
         m_langType = LangTypes::english;
         if ( m_valid ) {
            if ( !knownUnknown ) {
               mc2dbg << "[TMP]: ERROR: not bits for langType "
                      << MC2CITE( paramString ) << endl;
            }
            m_valid = false;
         }
      }
      m_langType = 
         LangTypes::language_t((m_langType << 3 ) | lowLangType);
   }

   // Then the higher.
   if ( m_valid && buf.getBufferSize()*8 - buf.getCurrentBitOffset() >= 4 ) {
      
      // This bit used to be highest part of m_serverPrefix,
      // but is now hijacked for special occasions, i.e. "quirkymode".
      int quirkymode;
      buf.readNextBits( quirkymode, 1 );

      // Old clients send quirkymode as highest bit of server prefix,
      // which hopefully should be 0, which means don't do anything.
      
      if ( quirkymode == 0x1 ) {
         // Read 7 highests bits for lang type, since original 
         // lang type wasn't wide enough.
         int highestLangType = 0;
         buf.readNextBits( highestLangType, 7 );
         m_langType = 
            LangTypes::language_t( (highestLangType << 6) | m_langType );
      }
      
      // High bits of importance nbr.
      buf.readNextBits(m_importanceNbr, 1);
      m_importanceNbr = (m_importanceNbr << 4 ) | lowImportance;
      
      // Read inverted gzip.
      buf.readNextBits(m_gzip, 1);
      m_gzip = !m_gzip;
   } else {
      m_serverPrefix = 0;
      m_importanceNbr = 0;
      m_gzip = false;
      if ( m_valid ) {
         if ( !knownUnknown ) {
            mc2dbg << "[TMP]: ERROR: not bits for serverPrefix and "
                   << "importanceNbr " << MC2CITE( paramString ) << endl;
         }
         m_valid = false;
      }
   }
   
   // Then also load the route id.
   if ( m_valid && m_layer == TileMapTypes::c_routeLayer ) {
      if ( m_valid && buf.getBufferSize()*8 - buf.getCurrentBitOffset() >= 64 )
      {
         m_routeID = new RouteID();
         m_routeID->load( &buf );
      } else {
         if ( m_valid ) {
            if ( !knownUnknown ) {
               mc2dbg << "[TMP]: ERROR: not bits for RouteID "
                      << MC2CITE( paramString ) << endl;
            }
            m_valid = false;
         }
      }
   }

//    // This doesn't work as buf's buffer length is not the params length
//    if ( m_valid && buf.getBufferSize()*8 - buf.getCurrentBitOffset() >= 8 ) {
//       mc2log << warn << "[TMP]: " << (buf.getBufferSize()*8 - 
//                                       buf.getCurrentBitOffset())
//              << " bits left after tilemap param " << paramString << endl;
//       m_valid = false;
//    }

#if 0
   mc2log << "paramString = " << paramString << endl;
   mc2log << "m_importanceNbr = " << m_importanceNbr << endl;
   mc2log << "m_detailLevel = " << m_detailLevel << endl;
   mc2log << "m_tileIndexLat = " << m_tileIndexLat << endl;
   mc2log << "m_tileIndexLon = " << m_tileIndexLon << endl;
   mc2log << "m_layer = " << m_layer << endl;
   mc2log << "m_gzip = " << m_gzip << endl;
   mc2log << "m_mapOrStrings = " << m_mapOrStrings << endl;
   mc2log << "m_langType = " << m_langType << endl;
   mc2log << "m_serverPrefix = " << m_serverPrefix << endl;
#endif   
}

ostream& operator << ( ostream& stream, 
                       const TileMapParams& params )
{
   stream << "TileMapParams: " << params.getAsString() << endl;
   stream << "Lay: " << params.getLayer() 
          << ", Imp: " << params.getImportanceNbr()
          << ", Det: " << params.getDetailLevel() 
          << ", Lat idx: " << params.getTileIndexLat() 
          << ", Lon idx: " << params.getTileIndexLon() 
          << ", Lang: " << params.getLanguageType()
          << ", Type: " << params.getTileMapType()
          << ", Gzip: " << params.useGZip();
   if ( params.getRouteID() != NULL ) {
      stream << endl << "RouteID: " 
             << params.getRouteID()->toString() 
             << " (" << *params.getRouteID() << ")";
   }
   return stream;
}
