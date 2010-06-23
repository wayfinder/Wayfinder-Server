/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <assert.h>
#include <ctype.h>
#include "MC2String.h"
#include "Types.h"
#include "SMSTPDU.h"
#include <stdio.h>
#include "SMSUtil.h"

static inline uint8
asc2hexnibble(const char c)
{
   if (isdigit(c)) {
      return c - '0';
   } else if ((c >= 'A') && (c <= 'F')) {
      return c - 'A' + 10;
   } else 
      return 0;
}


SMSTPDUContainer::SMSTPDUContainer(char* hexdata)
{
   reset();
   decodeHexMessage(hexdata);
}

size_t
packAddressInto(uint8 *pchBuffer, size_t bufferSize, const char *sPhoneNo)
{
   uint8 *pch = pchBuffer;
   size_t len = strlen(sPhoneNo);
   
   for (size_t i=0; (i < len) && (pch - pchBuffer < (int)bufferSize); ) {
      uint8 ln = asc2hexnibble(sPhoneNo[i++]);
      uint8 hn = (i < len) ? asc2hexnibble(sPhoneNo[i++]) : 0xF;
      *pch++ = (hn << 4) | ln;
   }
   return pch - pchBuffer;
}


void
SMSTPDUContainer::unpackAddress(char* outBuffer,
                                uint8* inBuffer,
                                int offset,
                                int len)
{
   const char digit[] = "0123456789ABCDE\0";
   int i;
   for ( i=0; i < len; i++) {
      if ( ( i & 1 ) == 0 ) 
         // I is even
         outBuffer[i] = digit[data[offset + ( i / 2 )] & 0xf ];
      else
         // I is odd
         outBuffer[i] = digit[data[offset + ( i / 2)] >> 4 ];
   }
   outBuffer[i] = '\0';
}

/*
static inline uint8 *
SMSTPDUContainer::putByte(uint8 n)
{
  *pnext++ = n;
  assert(pnext - data < sizeof(data));
  return pnext;
}
*/

#if 0
static inline size_t
SMSTPTDUContainer::countFree(void)
{
  return sizeof(data) - count();
}
#endif


void
SMSTPDUContainer::makeMessage(uint8 *pPayload,
                              size_t nPayloadSize,
                              const char *rxPhone,
                              const char *sServiceCenter)
{
   size_t len;
  
   reset();
  
  // If there is a service center entry for this message - use it.
  if (strlen(sServiceCenter) > 0) {
    len = packAddressInto(position() + 2, countFree() - 2, sServiceCenter);
    if (len > 0) {
      *this << (uint8)(len + 1);
      *this << (uint8)(SMS_(SCA_ADDRESS_TYPE));
      pnext += len;
    }
    else {
      *this << (uint8)0;
    }
  }

  /* start of the SMS TPDU */
  //  ptpdu = position();
  
  /* TODO: concatenated SMS */
  *this << SMS_(TP_MESSAGE_TYPE);
  *this << SMS_(TP_MESSAGE_REFERENCE);
  
  // destination address
  // Skip two bytes from pnext and write the phonenumber
  len = packAddressInto(position() + 2,
			countFree() - 2,
			rxPhone);
  // Write the length ( remember that pnext isn't affected by
  // packaddress. 
  *this << ((uint8)strlen(rxPhone));
  //  putByte(strlen(sRecipientPhone));
  *this << SMS_(DEST_ADDRESS_TYPE);
  // Now skip over the phonenumber.
  pnext += len;
  
  *this << SMS_(MESSAGE_TP_PID);
  *this << SMS_(MESSAGE_TP_DCS);
  *this << SMS_(MESSAGE_TP_VP);

  // TODO: check concatenation
  *this << (uint8)(SMS_(nPayloadSize));
  pnext += SMSUtil::putEncoded120x8(pnext, pPayload, nPayloadSize);
}


int
SMSTPDUContainer::getMessageAsHex(char* hexData, size_t maxLength)
{
   char tempBuf[5]; // Put the two hexdigits here.
   // Make a TPDU in binary format
   makeMessage(m_payLoad, m_payLoadSize, m_recipientPhone, m_serviceCenter);
   /* Now data will contain the message that we want to send */
   hexData[0] = '\0';
   size_t pos = 0; /* Position in data */
   while ( pos < maxLength && pos < count() ) {
      sprintf(tempBuf, "%02X", data[pos]);
      strcat(hexData, tempBuf);
      pos++;
   }
   return strlen(hexData);
}


int
SMSTPDUContainer::decodeHexMessage(char* hexData)
{
   DEBUG1(cerr << "hexData = " << hexData << endl);
   int length = strlen(hexData);
   int size = 0;
   for(int i=0; i < length; i+=2 ) {
      // Put two of the hex characters in a string
      char hexChars[3] = { hexData[i], hexData[i+1], '\0'};
      DEBUG8(cerr << "hexChars=" << hexChars << endl);
      unsigned int value = 0;
      sscanf(hexChars, "%x", &value);
      *pnext++ = (uint8)value;
      size++;
   }
   reset();
   // Now it's time to decode the real message and set
   // the member variables accordingly.
   int position = 0;
   if ( 0 ) {
      // Skip SMSC-address
      if ( data[position] != 0 )
         position += data[position] + 1;
   }
   strcpy(m_serviceCenter, "");
   // Flags ( I think they are always == 4 )
   int flags = data[position++];
   int addressLength = data[position++];
   // Skip addresstype
   position++;
   char* senderAddress = new char[addressLength+1];
   unpackAddress(senderAddress, data, position, addressLength);
   strcpy(m_recipientPhone, senderAddress);
   delete [] senderAddress;
   // Skip the address (the length is given in nibbles) + PID + DCS + timestamp
   position += ((addressLength + 1) / 2) + 1 + 1 + 7;
   int dataLength = data[position++];
   m_payLoadSize = SMSUtil::getDecoded160x7(m_payLoad,
                                            &data[position],
                                            dataLength);
   if ( ( flags & 0x40 ) == 0x40 ) {
      DEBUG2(cout << "Skipping udh!!!!" << endl);
      // Header indication set.
      int udh_len = data[position++];
      // Remove udh from payload
      uint8 tempPay[256];
      memcpy(tempPay, m_payLoad, m_payLoadSize);
      int udh_offset = (( udh_len + 1 ) * 8) / 7 + 2;
      m_payLoadSize -= udh_offset;
      memcpy(m_payLoad, tempPay + udh_offset, m_payLoadSize);
   }
   return size;
}


