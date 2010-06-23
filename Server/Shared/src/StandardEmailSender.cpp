/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UTF8Util.h"
#include "StandardEmailSender.h"
#include "TCPSocket.h"
#include "StringUtility.h"


StandardEmailSender::StandardEmailSender()
{
   m_smtphost = StringUtility::newStrDup( "localhost.localdomain" );
   m_smtpport = 25;
}

StandardEmailSender::StandardEmailSender(const char* smtphost)
{
   m_smtphost = StringUtility::newStrDup( smtphost );
   m_smtpport = 25;
}

MC2String
StandardEmailSender::createHeaderLine( const char* headerData )
{
   int len = strlen( headerData );
   bool highbit = false;
   for ( int i = 0; i < len; ++i ) {
      if ( headerData[i] & 0x80 ) {
         highbit = true;
         break;
      }
   }

   bool notlatin = false;
   for ( mc2TextIterator it = headerData;
         *it != 0;
         ++it ) {
      if ( *it > 256 ) {
         notlatin = true;
         break;
      }
   }
   MC2String stringToSend;
   MC2String encoding;
   bool useiterator = false;
   if ( highbit ) {
      if ( ! notlatin ) {
         stringToSend = UTF8Util::mc2ToIso( headerData );
         encoding = "iso-8859-1";
      } else {
         stringToSend = UTF8Util::mc2ToUtf8( headerData );
         encoding = "utf-8";
         useiterator = true;
      }
   } else {
      encoding = "";
      stringToSend = headerData;
   }

   MC2String result;
   uint32 maxLength = encoding.empty() ? MAX_UINT16 : 30;
   while ( ! stringToSend.empty() ) {
      if ( ! result.empty() ) {
         // Add newline and such and one space.
         result.append( "\r\n ");
      }

      // Nu blev det mycket kod.
      MC2String curRow;
      if ( ! useiterator ) {
         for ( isoTextIterator it = stringToSend;
               curRow.length() < maxLength && *it != 0;
               ++it ) {
            curRow += (char)*it;
            if ( ! encoding.empty() && *it == 32 ) {
               break;
            } 
         }
      } else {
         for ( utf8TextIterator it = stringToSend;
               curRow.length() < maxLength && *it != 0;
               ++it ) {
            curRow += UTF8Util::ucsToUtf8( *it );
            if ( *it == 32 ) {
               break;
            } 
         }
         
      }
      // Remove written part.
      stringToSend.erase( 0, curRow.length() );
      
      if ( !encoding.empty() ) {
         // Something difficult.
         result.append( "=?" + encoding + "?b?" + 
                        StringUtility::base64Encode( curRow ) + "?=");
      } else {
         // ASCII
         result.append( curRow );
      }
   }
   return result;
}

bool
StandardEmailSender::sendMail(const char* fromAddress,
                              const char* toAddress,
                              const char* subject,
                              uint32 nbrOptionalHeaders,
                              const char * const * const optionalHeaderTypes,
                              const char * const * const optionalHeaderValues,
                              const char* message)
{
   TCPSocket sock;
   const uint32 inbuffSize = 4096;
   char inbuff [inbuffSize];
   int32 nbr = 0;
   int32 code = 0; 

   // Open the socket
   if ( !sock.open() ) {
      mc2log << error << "SendMail:: Failed opening socket" << endl;
      sock.close();
      return (false);
   }

   // Connect to the SMTP-host
   if ( !sock.connect(m_smtphost, m_smtpport) ) {
      mc2log << error << "SendMail:: Failed connecting to smtp-host" << endl;
      sock.close();
      return (false);
   }

   // Read hello message and check the response
   if ( (nbr = sock.readLine(inbuff, inbuffSize)) <= 0 ) {
      mc2log << error << "SendMail:: Failed reading hello message" << endl;
      sock.close();
      return (false);
   }
   if ( !responseOK(inbuff, nbr, "220") ) { 
      // Error
      mc2log << error << "SendMail:: Error, wrong responce" << endl;
      mc2dbg2 << inbuff << endl;
      sock.close();
      return (false);
   }

   // Login and check response
   MC2String heloMsg( MC2String( "HELO " ) + m_smtphost + "\r\n" );
   sock.writeAll( (byte*)heloMsg.c_str(), heloMsg.size() );
   if ( (nbr = sock.readLine(inbuff, inbuffSize)) <= 0 ) {
      mc2log << error << "SendMail:: Failed reading logon message" << endl;
      sock.close();
      return (false);
   }
   if ( !responseOK(inbuff, nbr, "250") ) { 
      // Error
      mc2log << error << "SendMail:: Error wrong responce login" << endl; 
      mc2dbg1 << "   code = " << code << endl;
      sock.close();
      return (false);
   }

   // Send "MAIL FROM: " and check the response
   MC2String from( fromAddress );
   MC2String mailFrom( fromAddress );
   char* findLtChr = StringUtility::strchr( fromAddress, '<' );
   if ( findLtChr != NULL ) {
      uint32 ltpos = findLtChr - fromAddress;
      mailFrom = from.substr( ltpos, from.find( ltpos, '>' ) );
   }
   const char* mailFromStr = "MAIL FROM:";
   sock.writeAll( (byte*) mailFromStr, strlen(mailFromStr) );
   sock.writeAll( (byte*) mailFrom.c_str(), mailFrom.size() );
   sock.writeAll( (byte*) "\r\n", 2 );
   if ( (nbr = sock.readLine(inbuff, inbuffSize)) <= 0 ) {
      mc2log << error << "SendMail:: Failed reading MAIL responce" << endl;
      sock.close();
      return (false);
   }
   if ( !responseOK(inbuff, nbr, "250") ) { 
      // Error
      mc2log << error << "SendMail:: Error wrong responce MAIL " << endl;
      mc2dbg1 << "   code = " << code << endl;
      sock.close();
      return (false);
   }

   // Send "RCPT TO: " and check the response
   MC2String to( toAddress );
   MC2String mailTo( toAddress );
   findLtChr = StringUtility::strchr( toAddress, '<' );
   if ( findLtChr != NULL ) {
      uint32 ltpos = findLtChr - toAddress;
      mailTo = to.substr( ltpos, to.find( ltpos, '>' ) );
   }
   const char* mailRecipentStr = "RCPT TO: ";
   sock.writeAll( (byte*) mailRecipentStr, strlen(mailRecipentStr) );
   sock.writeAll( (byte*) mailTo.c_str(), mailTo.size() );
   sock.writeAll( (byte*) "\r\n", 2 );
   if ( (nbr = sock.readLine(inbuff, inbuffSize)) <= 0 ) {
      mc2log << error << "SendMail:: Failed reading RCPT responce" << endl;
      sock.close();
      return (false);
   }
   if ( !responseOK(inbuff, nbr, "250") ) { 
      // Error
      mc2log << error << "SendMail:: Error wrong responce RCPT " << endl;
      mc2dbg1 << "   code = " << code << endl; 
      sock.close();
      return (false);
   }

   // Send "DATA" and check the response
   sock.writeAll( (byte*) "DATA\r\n", 6 );
   if ( (nbr = sock.readLine(inbuff, inbuffSize)) <= 0 ) {
      mc2log << error << "SendMail:: Failed reading DATA responce" << endl;
      sock.close();
      return (false);
   }
   if ( !responseOK(inbuff, nbr, "354") ) { 
      // Error
      mc2log << error << "SendMail:: Error wrong responce DATA" << endl;
      mc2dbg1 << "   code = " << code << endl;
      sock.close();
      return (false);
   } 

   //** Header of email

   // Fix the subject. 
   MC2String newSubject = createHeaderLine(subject);
   subject = newSubject.c_str();
   
   
   // Send subject other header lines
   const char* subjectStr = "Subject: ";
   sock.writeAll( (byte*) subjectStr, strlen(subjectStr) );
   sock.writeAll( (byte*) subject, strlen(subject) );
   sock.writeAll( (byte*) "\r\n", 2 );
   for ( uint32 i = 0 ; i < nbrOptionalHeaders ; i++ ) {
      sock.writeAll( (byte*) optionalHeaderTypes[ i ], 
                  strlen(optionalHeaderTypes[ i ] ) );
      sock.writeAll( (byte*) ": ", 2 );
      sock.writeAll( (byte*) optionalHeaderValues[ i ], 
                  strlen(optionalHeaderValues[ i ] ) );
      sock.writeAll( (byte*) "\r\n", 2 );
   }
   // From:
   const MC2String fromStr( "From: " );
   sock.writeAll( (byte*) fromStr.c_str(), fromStr.size() );
   sock.writeAll( (byte*) from.c_str(), from.size() );
   sock.writeAll( (byte*) "\r\n", 2 );
   // Send "To:"
   const char* toStr = "To: ";
   sock.writeAll( (byte*) toStr, strlen( toStr ) );
   sock.writeAll( (byte*) to.c_str(), to.size() );
   sock.writeAll( (byte*) "\r\n", 2 );
   // Send "Sender:"
   const char* senderStr = "Sender: ";
   sock.writeAll( (byte*) senderStr, strlen( senderStr ) );
   sock.writeAll( (byte*) fromAddress, strlen( fromAddress ) );
   sock.writeAll( (byte*) "\r\n", 2 );

   //** End header with empty line
   sock.writeAll( (byte*) "\r\n", 2 );

   // Send body
   // FIXME: MUST REPLACE ALL LINES THAT ARE SINGLE DOT '.' WITH '..' in message
   sock.writeAll( (byte*) message, strlen( message ) );
   sock.writeAll( (byte*) "\r\n.\r\n", 5 );
   if ( (nbr = sock.readLine(inbuff, inbuffSize)) <= 0 ) {
      mc2log << error << "SendMail:: Failed reading ENDmessage responce"
             << endl;
      sock.close();
      return (false);
   }
   if ( !responseOK(inbuff, nbr, "250") ) { 
      // Error
      mc2log << error << "SendMail:: Error wrong responce ENDMEssage" << endl;
      mc2dbg1 << "   code = " << code << endl;
      sock.close();
      return (false);
   } 
   
   // Send "QUIT" and check the reply
   sock.writeAll( (byte*)"QUIT\r\n", 6 );
   if ( (nbr = sock.readLine(inbuff, inbuffSize)) <= 0 ) {
      mc2log << error << "SendMail:: Failed reading QUIT responce" << endl;
      sock.close();
      return (false);
   }
   if ( !responseOK(inbuff, nbr, "221") ) { 
      // Error
      mc2log << error << "SendMail:: Error wrong responce QUIT" << endl;
      mc2dbg1 << "   code = " << code << endl; 
      sock.close();
      return (false);
   } 

   // Close the socket and return
   sock.close();
   return (true);
}



bool
StandardEmailSender::responseOK( const char* inbuff, 
                                 int inSize, 
                                 const char* ok )
{
   bool retVal = true;
   int okSize = strlen(ok);
   if (inSize >= okSize) {
      int i=0;
      while ( (i<okSize) && (retVal)) {
         retVal = (inbuff[i] == ok[i]);
         i++;
      }
   } else {
      retVal = false;
   }
   return (retVal);
}
