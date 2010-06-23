/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "MC2String.h"

/**
 *   Conversion routines for the GSM 03.38 character set.
 *   TODO: Properly convert the escaped characters.
 *   TODO: Convert latin-1 to utf-8, not the opposit.
 */
class SMSConvUtil {
public:
   /**
    *    Converts the string from GSM characters to ISO
    *    Outbuffer should be three times as large as inbuffer.
    *    N.B! Some characters don't exist in both formats
    *         and will be converted to spaces.
    *    @param outBuffer Buffer to put the result into.
    *    @param inBuffer  Buffer to read from.
    *    @param length    The length of the in-buffer.
    *    @return Number of characters placed in the out-buffer.
    */
   static int gsmToIso(char* outBuffer, 
                       const char* inBuffer, 
                       int dataSize);
   
   /**
    *    Converts the string from ISO character encoding to GSM.
    *    Outbuffer should be three times as large as inbuffer.
    *    N.B! Some characters don't exist in both formats and
    *         will be converted to spaces.
    *    @param outBuffer The buffer to put the result into.
    *    @param inBuffer  The buffer to read from.
    *    @param length    The length of the in-buffer. 
    *    @return Number of characters placed in the out-buffer.
    */
   static int isoToGsm(char* outBuffer, 
                       const char* inBuffer, 
                       int dataSize);
   
   /**
    *    Converts the string from GSM characters to ISO, then
    *    to UTF8.
    *    Outbuffer should be three times as large as inbuffer.
    *    N.B! Some characters don't exist in both formats
    *         and will be converted to spaces.
    *    @param outBuffer Buffer to put the result into.
    *    @param inBuffer  Buffer to read from.
    *    @param length    The length of the in-buffer.
    *    @return Number of characters placed in the out-buffer.
    */
   static int gsmToUtf8(char* outBuffer, 
                        const char* inBuffer, 
                        int dataSize);
   
   /**
    *    Converts the string from UTF8 to ISO character encoding to GSM.
    *    Outbuffer should be three times as large as inbuffer.
    *    N.B! Some characters don't exist in both formats and
    *         will be converted to spaces.
    *    @param outBuffer The buffer to put the result into.
    *    @param inBuffer  The buffer to read from.
    *    @param length    The length of the in-buffer. 
    *    @return Number of characters placed in the out-buffer.
    */
   static int utf8ToGsm(char* outBuffer, 
                        const char* inBuffer, 
                        int dataSize);
   
   /**
    *    Converts the string from GSM characters to MC2.
    *    Outbuffer should be three times as large as inbuffer.
    *    N.B! Some characters don't exist in both formats
    *         and will be converted to spaces.
    *    @param outBuffer Buffer to put the result into.
    *    @param inBuffer  Buffer to read from.
    *    @param length    The length of the in-buffer.
    *    @return Number of characters placed in the out-buffer.
    */
   static int gsmToMc2(char* outBuffer, 
                       const char* inBuffer, 
                       int dataSize);
   
   /**
    *    Converts the string from MC2 character encoding to GSM.
    *    Outbuffer should be three times as large as inbuffer.
    *    N.B! Some characters don't exist in both formats and
    *         will be converted to spaces.
    *    @param outBuffer The buffer to put the result into.
    *    @param inBuffer  The buffer to read from.
    *    @param length    The length of the in-buffer. 
    *    @return Number of characters placed in the out-buffer.
    */
   static int mc2ToGsm(char* outBuffer, 
                       const char* inBuffer, 
                       int dataSize);
private:
   /// Conversion table GSM -> Latin-1
   static const char GSMToISO_table[];
   
   /// Conversion table Latin-1 -> GSM
   static const char ISOToGSM_table[];

   /// Conversion table GSM Escape -> Latin-1
   static const char GSMEscapeToISO_table[];
   
   /// Conversion table Latin-1 -> GSM Escape
   static const char ISOToGSMEscape_table[];
   
};
