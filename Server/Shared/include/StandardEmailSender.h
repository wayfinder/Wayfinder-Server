/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STANDARDEMAILSENDER_H
#define STANDARDEMAILSENDER_H

#include "config.h"
#include "EmailSender.h"

/**
 *    Objects of this class is to be used when sending emails. The objects
 *    store the SMTP-host as an attribute, and then the sendMail-method
 *    might be called many times. The mail is send with SMTP (Simple Mail
 *    Transfer Protocol), that is described in RFC 821 and 822. The 
 *    possible error codes etc. are also described in that RFC's.
 *
 *    It might be necessery to do some implovements to this class for
 *    example:
 *    <ul>
 *       <li>Option to send one mail to several recipients.
 *       <li>Avoid looking up the name of the SMTP-host every time a
 *           mail should be sent.
 *       <li>User supplied header lines, Content-Type, Mime-Version etc..
 *    </ul>
 *
 */
class StandardEmailSender : public EmailSender {
   public:
      /**
       *    Create a new StandardEmailSender. Default values will be used 
       *    for the SMTP-host and the port there.
       */
      StandardEmailSender();

      /**
       *    Create a new StandardEmailSender. Use the given SMTP-host to 
       *    send the mails.
       *    @param   smtphost The name of the SMTP-host.
       */
      StandardEmailSender(const char* smtpHost);

      /**
       * @copydoc EmailSender::sendMail
       */
      bool sendMail(const char* fromAddress,
                    const char* toAddress,
                    const char* subject,
                    uint32 nbrOptionalHeaders,
                    const char * const * const optionalHeaderTypes,
                    const char * const * const optionalHeaderValues,
                    const char* message);

   private:

      /**
       *    Creates header line. Adds encoding if necessary.
       */
      MC2String createHeaderLine( const char* headerData );
      
      /**
       *    The name of the host, running the SMTP-server.
       */
      char* m_smtphost;

      /**
       *    The port that will be used at m_smtphost. The default value
       *    is 25.
       */
      uint32 m_smtpport;

      /**
       *    Check a SMTP-response. Example of usage:
       *    <tt>if (!responseOK(inbuff, nbrReadBytes, "250")) 
       *       MC2ERROR("...");</tt>
       *    @param   inbuff   The data read from the socket.
       *    @param   size     The size of the indata.
       *    @param   ok       A string containing the "good" answer code
       *                      that should be present in the inbuff to make
       *                      this method return true.
       *    @return  True if the given inbuffer contains the ok-answer.
       */
      bool responseOK(const char* inbuff, int size, const char* ok);
};

#endif

