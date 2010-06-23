/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavLocalCategoryTreeHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "NavRequestData.h"
#include "ParserExternalAuth.h"
#include "ParserExternalAuthHttpSettings.h"
#include "ServerRegionIDs.h"
#include "TimedOutSocketLogger.h"
#include "STLStringUtility.h"
#include "categoryRegionIDFromCoord.h"
#include "LocalCategoryTrees.h"
#include "NavMessage.h"
#include "StringTable.h"
#include "BinaryCategoryTreeFormat.h"
#include "MC2CRC32.h"
#include "NavParamHelper.h"


NavLocalCategoryTreeHandler::NavLocalCategoryTreeHandler( InterfaceParserThread* thread,
                                    NavParserThreadGroup* group )
      : NavHandler( thread, group )
{
   m_expectations.push_back( ParamExpectation( 6, NParam::Uint16 ) );
   m_expectations.push_back( ParamExpectation( 4308, NParam::Int32_array ) );
   m_expectations.push_back( ParamExpectation( 6500, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6501, NParam::Uint16 ) );
}


NavLocalCategoryTreeHandler::~NavLocalCategoryTreeHandler() {
}

bool
NavLocalCategoryTreeHandler::handleLocalCategoryTree( NavRequestData& rd )
{
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   // Start parameter printing
   mc2log << info << "[NavLocalCategoryTreeHandler]::handleLocalCategoryTree";

   // Get the version
   uint16 ver = 0;
   if ( getParam( 6501, ver, rd.params ) ) {
      mc2log << " version " << ver;
   }

   // Get the location
   MC2Coordinate coord;
   uint16 angle = 0;
   const NParam* coordParam = rd.params.getParam( 4308 );
   NavParamHelper::getCoordAndAngle( coordParam, coord, angle );
   if ( coordParam != NULL ) {
      mc2log << " location " << coord;
   }

   // Get the crc
   MC2String crcIn = "";
   if ( getParam( 6500, crcIn, rd.params ) ) {
      mc2log << " crc " << crcIn;
   }

   // End parameter printing
   mc2log << endl;

   using namespace CategoryTreeUtils;

    // Get the region for which to get the category tree
   CategoryRegionID regionID = categoryRegionIDFromCoord( coord, m_thread );

   // Get the Category tree for this client's region.
   const LocalCategoryTreesPtr localCatTrees =  m_group->getLocalCategoryTrees();

   CategoryTreePtr categoryTree = localCatTrees->getTree( regionID );

   // Serialize the tree to binary format
   BinaryCategoryTreeFormat serializedTree;
   serializeTree( categoryTree.get(), rd.clientLang, &serializedTree );
 
   //Get crc for the tree
   MC2String crcOut = getCrcForTree( serializedTree );

   NParamBlock& rparams = rd.reply->getParamBlock();
   if ( crcOut == crcIn ) {
      rparams.addParam( NParam( 6505, true ) );

   } else {
      // crc did not match
      rparams.addParam( NParam( 6505, false ) );

      // Add category table
      NParam& catTablePar = rparams.addParam( NParam( 6502 ) );
      catTablePar.addByteArray( 
         serializedTree.m_categoryTable->getBufferAddress(),
         serializedTree.m_categoryTable->getBufferSize() );

      // Add lookup table
      NParam& lookupTablePar = rparams.addParam( NParam( 6503 ) );
      lookupTablePar.addByteArray( 
         serializedTree.m_lookupTable->getBufferAddress(),
         serializedTree.m_lookupTable->getBufferSize() );
      
      // Add string table
      NParam& strTablePar = rparams.addParam( NParam( 6504 ) );
      strTablePar.addByteArray( 
         serializedTree.m_stringTable->getBufferAddress(),
         serializedTree.m_stringTable->getBufferSize() );

      // Add CRC
      rparams.addParam( NParam( 6500, crcOut ) );

   } // End else not same crc

     
   return true;
}

