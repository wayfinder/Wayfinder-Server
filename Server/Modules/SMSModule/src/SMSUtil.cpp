/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SMSUtil.h"

int
SMSUtil::createConcatenatedHeader( char* outBuffer,
                                   int& pos,
                                   int smsNbr,
                                   int nbrParts,
                                   int partNbr)
{
   
   sprintf(&outBuffer[pos],"%02X%02X%02X%02X%02X%02X",
           5,0,3,smsNbr,nbrParts,partNbr);   
   cout << "Creating header Size: " << 5 << " SMSnbr: " << smsNbr
        << " nbrParts " << nbrParts << " partNbr " << partNbr
        << endl << "Data: ";
   for( int i=0;i<12;i++){
      cout << outBuffer[i+pos];
   }
   cout << endl;
   pos += 12;
   return 12; 

   /*
   outBuffer[pos++] = 0; // length of header
   outBuffer[pos++] = 0; // subitem type = concatenation
   outBuffer[pos++] = 3; // length of subitem 
   outBuffer[pos++] = smsNbr; // ID of the concatenated SMS
   outBuffer[pos++] = nbrParts; // nbr of SMS in concatenation
   outBuffer[pos++] = partNbr; // ID of SMS part
   int size = pos - start;
   outBuffer[start]=size-1; // remove the headerlength
   */
}

int
SMSUtil::putEncoded120x8(uint8* dataShort,
                         uint8 *dataLong,
                         int payloadLen)
{
   int i,j,k;
 
   i = 0;
   j = 7;
   k = 0;
   while ( i < payloadLen ) {
      dataShort[k] = (dataLong[i+1] << j )
                     | ( ( dataLong[i] & 0x7f) >> ( 7-j ) );
      if ( j == 1 ) {
         j = 7;
         i += 2;
      } else {
         j--;
         i++;
      }
      k++;
   }
   return k;
}

int
SMSUtil::getDecoded160x7(uint8* dataLong,
                         uint8* dataShort,
                         int count)
{
   int payloadLen = count;
   int i = 1;
   int j = 1;
   int k = 1;
   dataLong[0] = dataShort[0] & 0x7f;
   while ( i < payloadLen ) {
      dataLong[i] =  (( dataShort[j] << k )
                      | ( dataShort[j-1] >> ( 8 -k ))) & 0x7f ;
      if ( k==6 ) {
         k = 0;
         i++;
         dataLong[i] = dataShort[j] >> 1;
         j++;
      } else {
         j++;
         k++;
      }
      i++;
   }
   return i;
}
