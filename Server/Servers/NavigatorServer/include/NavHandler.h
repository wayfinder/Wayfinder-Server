/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVHANDLER_H
#define NAVHANDLER_H

#include "config.h"
#include "NParam.h"
#include "InterfaceParserThreadConfig.h"
#include "NavParserThreadGroup.h"

class NavReplyPacket;
class NParamBlock;
class MC2Coordinate;


/**
 * Super class for all Nav request handling classes.
 *
 */
class NavHandler {
   public:
      /**
       * Constructor.
       */
   NavHandler( InterfaceParserThread* thread,
                  NavParserThreadGroup* group );


   protected:
      /// The thread
      InterfaceParserThread* m_thread;

      /// The group.
      NavParserThreadGroup* m_group;

      /**
       * Get and set if present.
       */
      bool getParam( uint16 id, uint32& val, NParamBlock& params ) const;

      /**
       * Get and set if present.
       */
      bool getParam( uint16 id, int32& val, NParamBlock& params ) const;

      /**
       * Get and set if present.
       */
      bool getParam( uint16 id, uint16& val, NParamBlock& params ) const;

      /**
       * Get and set if present.
       */
      bool getParam( uint16 id, byte& val, NParamBlock& params ) const;

      /**
       * Get and set if present.
       */
      bool getParam( uint16 id, MC2String& val, NParamBlock& params ) const;

      /**
       * Get and set if present.
       */
      bool getParam( uint16 id, MC2Coordinate& val, NParamBlock& params ) const;

      /**
       * Get and set if present.
       */
      bool getParam( uint16 id, bool& val, NParamBlock& params ) const;

      /**
       * Checks if the params meets the expectations, an error is set in
       * reply if not and false is returned. True if all is ok.
       */
      bool checkExpectations( NParamBlock& params, NavReplyPacket* reply );


      /**
       * Encodes a parameter expectation.
       */
      class ParamExpectation {
      private:
         /**
          * Find correct min length from type and given length.  If
          * the type is string or one of the array type, the returned
          * value will be the same as the minLength argument (unless
          * it is invalid). Otherwise the length depends entirely on
          * the type.
          * @param t The parameter data type.
          * @param minLength The given minimum length.
          */
         static uint16 minLengthFromType(NParam::NParam_t t, uint16 minLength);
         /**
          * Find correct max length from type and given length.  If
          * the type is string or one of the array type, the returned
          * value will be the same as the maxLength argument (unless
          * it is invalid). Otherwise the length depends entirely on
          * the type.
          * @param t The parameter data type.
          * @param minLength The given minimum length.
          */
         static uint16 maxLengthFromType(NParam::NParam_t t, uint16 maxLength); 
      public:
         /**
          * Constructor
          * @param ID        The parameter ID.
          * @param t         The parameter type. 
          * @param minLength Minimum length. Used only for parameters
          *                  of type string or array. Defaults to 0.
          * @param maxLength Maximum length. Used only for parameters
          *                  of type string or array. Defaults to
          *                  MAX_UINT16.
          */
         ParamExpectation( uint16 ID,
                           NParam::NParam_t t, 
                           uint16 minLength = 0, 
                           uint16 maxLength = MAX_UINT16 )
            : paramID( ID ), type( t ), 
              minLen( minLengthFromType( t, minLength ) ),
              maxLen( maxLengthFromType( t, maxLength ) )
         {}

         /**
          * Check a NParam object against this expectian.  Note that
          * ID match is not checked by this function.  Length check is
          * performed, and special checks depending on expected type
          * are performed.
          * @param param The NParam object to check.
          * @return True if the NParam object met expectations, false otherwise.
          */
         bool checkParam(const NParam* param) const;
         /** The parameter ID. */
         uint16 paramID;
         /** The parameter data type. */
         NParam::NParam_t type;
         /** The minumum valid length if string or array. */
         uint16 minLen;
         /** The maximum valid length if string or array. */
         uint16 maxLen;
      };


      /**
       * Vector of known/used parameters for the request type and
       * what is expected of the parameters.
       */
      vector< ParamExpectation > m_expectations;
};


#endif // NAVHANDLER_H

