/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CELL_ID_LOOK_UP_H
#define CELL_ID_LOOK_UP_H

#include "config.h"
#include "MC2Coordinate.h"
#include "MC2String.h"

#include <iostream>

class ParserThreadGroup;

namespace CellIDLookUp {

/// A struct for holding the request parameters.
struct CellIDRequest {
   /// Mobile country code.
   MC2String m_mcc;
   /// Mobile network code.
   MC2String m_mnc;
   /// Local area code.
   MC2String m_lac;
   /// Cell ID.
   MC2String m_cellID;

   // Optional stuff that isnt supported or needed yet.
   /// Type of network.
   MC2String m_networkType;
   /// The signal strength to the cell id.
   MC2String m_signalStrength;
   
   // rnc id not to be used? it is a part of building up a cell id for a 3G
   // network. A so called UC-ID (3G UTRAN Cell ID)
   MC2String m_rncId;
};

/// Holds cell id reply
struct CellIDReply {
   // Maybe radius should be MAX_UINT32 instead
   CellIDReply() :
      m_outerRadius( 1000 ),
      m_innerRadius( 0 ),
      m_altitude( 0 ),
      m_startAngle( 0 ),
      m_endAngle( 360 )
   {}

   /// The coordinate for a specific cell id.
   MC2Coordinate m_coord;

   /// The outer uncertainty radius of the cell.
   uint32 m_outerRadius;

   // For possible future use
   /// The inner uncertainty radius of the cell.
   uint32 m_innerRadius;

   /// The altitude of cell.
   int32 m_altitude;

   /// The start angle of the cell.
   uint16 m_startAngle;

   /// The end angle of the cell.
   uint16 m_endAngle;

   /// Error message.
   MC2String m_errorStr;
};

/**
 * Empty for now, perhaps open cellid can be used?
 *
 * @param req The request parameters.
 * @param rep The cell id reply.
 * @param group The ParserThreadGroup
 * @return Returns false if no coordinates for the Cell ID was found.
 */
bool cellIDLookUp( const CellIDRequest& req, 
                   CellIDReply& rep, 
                   ParserThreadGroup* group );


}
#endif // CELL_ID_LOOK_UP_H
