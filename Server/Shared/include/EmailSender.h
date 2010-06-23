/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EMAILSENDER_H
#define EMAILSENDER_H

#include "config.h"

/**
 * Abstract base class for email senders.
 */
class EmailSender {
public:
   virtual ~EmailSender() {}

   /**
    *    Send one email.
    *    @param   fromAddress          The address in the from-field.
    *    @param   toAddress            The recipents email address.
    *    @param   subject              The subject of the email.
    *    @param   nbrOptionalHeaders   The number of headerlines in
    *                                  optionalHeaderTypes and 
    *                                  optionalHeaderValues.
    *    @param   optionalHeaderTypes  The types of the optional 
    *                                  headerlines.
    *    @param   optionalHeaderValues The values of the optional 
    *                                  headerlines.
    *    @param   message              The body in the email.
    *    @return  True if the message is sent properly, false
    *             otherwise.
    */
   virtual bool sendMail(const char* fromAddress,
                         const char* toAddress,
                         const char* subject,
                         uint32 nbrOptionalHeaders,
                         const char * const * const optionalHeaderTypes,
                         const char * const * const optionalHeaderValues,
                         const char* message) = 0;

};

#endif // EMAILSENDER_H
