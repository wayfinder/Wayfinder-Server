/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavHandler.h"
#include "NParamBlock.h"
#include "NavPacket.h"
#include "InterfaceParserThread.h"
#include <sstream>

NavHandler::NavHandler( InterfaceParserThread* thread, 
                        NavParserThreadGroup* group )
      : m_thread( thread ), m_group( group )
{
   //The 0 Parameter is the empty parameter; NullParam.
   m_expectations.push_back( ParamExpectation( 0, NParam::Bool, 0, 0 ) );
}

bool
NavHandler::getParam( uint16 id, uint32& val, NParamBlock& params ) const {
   if ( params.getParam( id ) && params.getParam( id )->getLength() >= 4 ) {
      val = params.getParam( id )->getUint32();
      return true;
   } else {
      return false;
   }
}

bool
NavHandler::getParam( uint16 id, int32& val, NParamBlock& params ) const {
   if ( params.getParam( id ) && params.getParam( id )->getLength() >= 4 ) {
      val = params.getParam( id )->getInt32();
      return true;
   } else {
      return false;
   }
}

bool
NavHandler::getParam( uint16 id, uint16& val, NParamBlock& params ) const {
   if ( params.getParam( id ) && params.getParam( id )->getLength() >= 2 ) {
      val = params.getParam( id )->getUint16();
      return true;
   } else {
      return false;
   }
}

bool
NavHandler::getParam( uint16 id, byte& val, NParamBlock& params ) const {
   if ( params.getParam( id ) && params.getParam( id )->getLength() >= 1 ) {
      val = params.getParam( id )->getByte();
      return true;
   } else {
      return false;
   }
}

bool
NavHandler::getParam( uint16 id, MC2String& val, NParamBlock& params ) 
   const 
{
   if ( params.getParam( id ) && params.getParam( id )->getLength() >= 1 ) {
      val = params.getParam( id )->getString( 
         m_thread->clientUsesLatin1() );
      return true;
   } else {
      return false;
   }
}

bool
NavHandler::getParam( uint16 id, MC2Coordinate& val, NParamBlock& params )
   const 
{
   if ( params.getParam( id ) && params.getParam( id )->getLength() >= 8 ) {
      val = MC2Coordinate( 
         Nav2Coordinate( params.getParam( id )->getInt32Array( 0 ),
                         params.getParam( id )->getInt32Array( 1 ) ) );
      return true;
   } else {
      return false;
   }
}

bool
NavHandler::getParam( uint16 id, bool& val, NParamBlock& params )
   const 
{
   if ( params.getParam( id ) && params.getParam( id )->getLength() <= 1 ) {
      val =  params.getParam( id )->getBool();
      return true;
   } else {
      return false;
   }
}


bool
NavHandler::checkExpectations( NParamBlock& params, NavReplyPacket* reply )
{
   uint16 errParamID = MAX_UINT16;
   NParam::NParam_t errType = NParam::Bool;
   bool ok = true;
   for ( vector< ParamExpectation >::iterator it = m_expectations.begin() ;
         it != m_expectations.end() && ok ; ++it )
   {
      // Check all of this ID
      vector< const NParam* > eps;
      params.getAllParams( (*it).paramID, eps );
      errParamID = (*it).paramID;
      errType = (*it).type;
      //check each parameter with matching id.
      //find the first element in eps where it->checkParam(*eps_it)
      //returns false.
      vector<const NParam*>::iterator found = 
         find_if(eps.begin(), eps.end(), 
                 //mem_fun_ref creates a mem_fun1_ref_t object. Using
                 //bind1st we bind the first argument of that functor
                 //to (*it).  Since ParamExpectation::checkParam
                 //returns true if the check is ok we need to invert
                 //it to find the first element that returns false.
                 //We use not1 to do that inversion.
                 not1( bind1st( mem_fun_ref( &ParamExpectation::checkParam ),
                                *it ) ) );
      //if we didn't find any, all is well 
      ok = (found == eps.end());
      eps.clear();
   }
   if ( !ok ) {
      reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_PARAMBLOCK_INVALID );
      ostringstream errorStream;
      errorStream << "Invalid " << NParam::getTypeAsString( errType )
                  << " param, ID: " << errParamID;
      reply->setStatusMessage( errorStream.str().c_str() );
      mc2log << error << "NavHandler::checkExpectations expectation not "
             << "met: " << errorStream.str() << endl;
   }

   return ok;
}

// ------------- NavHandler::ParamExpectation functions ---------------
namespace{
   inline uint16 lengthHelper(uint16 boolLength,
                              NParam::NParam_t t, 
                              uint16 defaultLength)
   {
      uint16 length = defaultLength;
      switch(t){
      case NParam::Bool:
         length = boolLength;
         break;
      case NParam::Byte:
         length = 1;
         break;
      case NParam::Uint16:
         length = 2;
         break;
      case NParam::Uint32:
      case NParam::Int32:
         length = 4;
         break;
      case NParam::String:
      case NParam::Byte_array:
      case NParam::Uint16_array:
      case NParam::Uint32_array:
      case NParam::Int32_array:
         break;
      }
      return length;
   }
}

uint16 NavHandler::ParamExpectation::minLengthFromType(NParam::NParam_t t, 
                                                       uint16 minLength)
{
   return lengthHelper(0, t, minLength);
}

uint16 NavHandler::ParamExpectation::maxLengthFromType(NParam::NParam_t t,
                                                       uint16 maxLength)
{
   return lengthHelper(1, t, maxLength);
}

bool NavHandler::ParamExpectation::checkParam(const NParam* param) const
{
   const uint16 pLength = param->getLength();
   bool ok = ((minLen <= pLength) && (pLength <= maxLen));
   if(ok && type == NParam::String){
      ok = ((pLength != 0) && //extra length check 
            (param->getBuff()[pLength -1] == '\0')); //0-terminated
   } 
   return ok;
}

