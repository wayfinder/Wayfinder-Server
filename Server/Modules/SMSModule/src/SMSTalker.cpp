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
#include "StringUtility.h"
#include "SMSTalker.h"

#include <sstream>

SMSTalker::SMSTalker() : m_numberOfPhones(0), m_numberOfServices(0),
                         m_phoneNumbers(NULL), m_serviceNames(NULL)
{
   m_logFile = NULL;
   m_logFileName = StringUtility::newStrDup("");
}

SMSTalker::~SMSTalker()
{
   if ( m_phoneNumbers != NULL ) {
      for ( int i = 0; i < m_numberOfPhones; ++ i ) {
         delete [] m_phoneNumbers[ i ];
      }
      delete [] m_phoneNumbers;
   }

   delete [] m_logFileName;
   if ( m_logFile != NULL ) {
      fclose(m_logFile);
   }
}

void
SMSTalker::setPhoneNumbers(int nbrPhones, char** phones)
{
   m_phoneNumbers = new char*[nbrPhones];
   for ( int i = 0; i < nbrPhones; i++ ) {
      m_phoneNumbers[i] = new char[strlen(phones[i])+1];      
      strcpy(m_phoneNumbers[i], phones[i] );      
   }
   m_numberOfPhones = nbrPhones;
}


void
SMSTalker::setServiceNames(int nbrServices, char** services)
{
   m_serviceNames = new char*[nbrServices];
   for ( int i = 0; i < nbrServices; i++ ) {
      m_serviceNames[i] = new char[strlen(services[i])+1];      
      strcpy(m_serviceNames[i], services[i] );      
   }
   m_numberOfServices = nbrServices;
}

bool
SMSTalker::logLine(bool sending,
                   const char* senderPhone,
                   const char* recipientPhone,
                   int nbrBytes,
                   const byte* contents)
{
   if ( m_logFile == NULL ) {
      return false;
   }
   // Get the time
   char timeBuf[100];
   
   time_t tt;
   struct tm* tm;

   ::time(&tt);
   struct tm result;
   tm = localtime_r( &tt, &result );

   sprintf(timeBuf, "%04d-%02d-%02d_%02d:%02d:%02d ",
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
           tm->tm_hour, tm->tm_min, tm->tm_sec);

   char* senderURL = new char[strlen(senderPhone)*3+1];
   int len = StringUtility::URLEncode(senderURL, senderPhone);
   senderURL[len+1] = '\0';
   
   char* contentsURL = new char[nbrBytes*3+1];

   int len2 = StringUtility::URLEncode(contentsURL,
                                       (const char*)contents,
                                       nbrBytes);
   contentsURL[len2] = '\0';
   
   
   stringstream strstr;
   strstr << (timeBuf+2)
          << (sending ? 'S' : 'R')
          << " "
          << senderURL
          << " "
          << recipientPhone
          << " "
          << contentsURL
          << endl << ends;
   
   
   int res = fputs(strstr.str().c_str(), m_logFile);
   if ( res >=0 ) {
      fflush(m_logFile);
   }
   
   delete [] senderURL;
   delete [] contentsURL;
   
   return res >= 0;
}

MC2String
SMSTalker::createLogFileName(const char* inpath)
{
   // Add date to the name
   char timeBuf[2000];
   time_t tt;
   struct tm* tm;
   
   ::time(&tt);
   struct tm result;
   tm = localtime_r( &tt, &result );
   sprintf(timeBuf, "%04d-%02d-%02d_%02d_%02d",
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
           tm->tm_hour, tm->tm_min);

   return MC2String(inpath) + "SMSLOG_" + MC2String(timeBuf+2) + ".log";
}

const char*
SMSTalker::getLogFileName() const
{
   return m_logFileName;
}

bool
SMSTalker::openLogFile(const char* logfilePath)
{
   // Close old file.
   if ( m_logFile != NULL ) {
      fclose(m_logFile);
   }
   if ( logfilePath != NULL ) {
      // Set filename
      {
         MC2String logfileName ( createLogFileName(logfilePath) );
         delete [] m_logFileName;
         m_logFileName = StringUtility::newStrDup(logfileName.c_str());
      }
      // Open the file.
      m_logFile = fopen(m_logFileName, "a");
      if ( m_logFile == NULL ) {
         mc2log << warn << "[SMSTalker]: Could not open logfile "
                << MC2CITE(m_logFileName) << endl;
      }
   }
   return m_logFile != NULL;
}
       


