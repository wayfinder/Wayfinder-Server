/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include "MC2String.h"
#include "JobThread.h"
#include "multicast.h"
#include "PacketPool.h"
#include "SMSPacket.h"
#include "SMSReader.h"
#include "SMSProcessor.h"
#include "DummySMSTalker.h"
#include "GM12SMSTalker.h"
#include "CIMDSMSTalker.h"
#include "XMLSMSTalker.h"
#include "PacketReaderThread.h"
#include "MapSafeVector.h"
#include "ModulePacketSenderReceiver.h"

int main(int argc, char *argv[])
{
   const uint16 defPort = 9753;
   uint32 rank = 0;
   uint16 preferredPort = 0;

   bool decodeError = false;
   char **phones = new char*[100];
   char **services = new char*[100];
   int nbrPhones = 0;
   int nbrServices = 0;
   char serviceType = '\0';
   char* cimdPassword = new char[256];
   char* cimdUserName = new char[256];
   char* cimdIP       = new char[256];
   char* gm12Pin = NULL;   // Pincode for the phone module.
   char* gm12Port = NULL;  // Serial device for the phone module.
   char* bindAddress = NULL;
   MC2String xmlURL;
   MC2String logFilePath;        // Path of logfile.
   bool      logFilePathEntered = false; // True if logfile path was entered.
   strcpy(cimdIP, "");
   strcpy(cimdUserName, "");
   strcpy(cimdPassword, "");
   int cimdPort = 1234;
   // Parse the arguments
   int currArg = 1;
   while ( currArg < argc && ! decodeError ) {
      if ( strcmp(argv[currArg], "-p" ) == 0 ) {
         // Phone number entered
         phones[nbrPhones++] = argv[currArg+1];
         currArg += 2;
      } else if ( strcmp( argv[currArg], "-s" ) == 0 ) {
         // Servicename entered
         services[nbrServices++] = argv[currArg+1];
         currArg +=2;
      } else if ( strcmp( argv[currArg], "-l" ) == 0 ) {
         // Logfilpath entered
         logFilePath = argv[currArg+1];
         logFilePathEntered = true;
         currArg +=2;
      } else if ( strcmp( argv[currArg], "-t" ) == 0 ) {
        // Moduletype entered
        if ( currArg + 1 >= argc )
          decodeError = true;
        else {
          serviceType = argv[currArg+1][0];
          currArg += 2;
        }
      } else if ( strcmp ( argv[currArg], "-xa" ) == 0 ) {
         // XMLServer URL entered.
         if ( currArg + 1 >= argc )
            decodeError = true;
         else {
            xmlURL = argv[currArg+1];
            currArg += 2;
         }
      } else if ( strcmp ( argv[currArg], "-cu" ) == 0 ||
                  strcmp ( argv[currArg], "-xu" ) == 0 ) {
         // CIMD or XML Username entered
        if ( currArg + 1 >= argc )
          decodeError = true;
        else {
          strcpy(cimdUserName ,argv[currArg+1]);
          currArg += 2;
        }
      } else if ( strcmp ( argv[currArg], "-cw" ) == 0 ||
                  strcmp ( argv[currArg], "-xw" ) == 0 ) {
         // CIMD Password entered or XML password 
        if ( currArg + 1 >= argc )
          decodeError = true;
        else {
          strcpy(cimdPassword ,argv[currArg+1]);
          currArg += 2;
        }
      } else if ( strcmp ( argv[currArg], "-cp" ) == 0 ) {
        // CIMD or TLR Port entered
        if ( currArg + 1 >= argc )
          decodeError = true;
        else {
          cimdPort = atoi(argv[currArg+1]);
          currArg += 2;
        }
      } else if ( strcmp ( argv[currArg], "-ci" ) == 0 ) {
        // CIMD or TLR IP Entered
        if ( currArg + 1 >= argc )
          decodeError = true;
        else {
          strcpy(cimdIP ,argv[currArg+1]);
          currArg += 2;
        }
      } else if ( strcmp ( argv[currArg], "-gw" ) == 0 ) {
        // GM12 PIN Entered
        if ( currArg + 1 >= argc )
          decodeError = true;
        else {
           gm12Pin = argv[currArg + 1];
           currArg += 2;
        }
      } else if ( strcmp ( argv[currArg], "-gp" ) == 0 ) {
        // GM12 Portnumber entered
        if ( currArg + 1 >= argc )
          decodeError = true;
        else {
           gm12Port = argv[currArg + 1];
           currArg += 2;
        }
      } else if ( strcmp( argv[currArg], "-h" ) == 0 ) {
         // HELP
         decodeError = true;
         currArg++;
      } else if ( strcmp ( argv[currArg], "-b" ) == 0 ) {
         // Bind address for CIMD
         if ( currArg + 1 >= argc )
            decodeError = true;
         else {
            bindAddress = argv[currArg+1];
            currArg += 2;
         }
      } else if ( currArg == 1 ) {         
         preferredPort = (uint16) atoi(argv[currArg]);
         currArg++;
         if ( preferredPort < 5000 ) {
            cerr << "Valid portnumbers are >5000" << endl;
         }
      } else if ( currArg == 2 ) {
         rank = (uint32) atoi(argv[currArg]);
         currArg++;
      } else {    
         cout << currArg << endl;
         decodeError = true;
      }
   }
   if (decodeError) {
      cerr << "Usage: " << argv[0]
           << " [port [rank]][-p phoneNumber][-s serviceName]"
              "[-t [m|c|d|x|g][-b bindaddress]"
           << "[-l logfilepath]"
           << endl << "SMSTalker types: " << endl
           <<         "   -t m = MapCentSMSTalker" << endl
           <<         "   -t c = CIMDSMSTalker" << endl
           <<         "   -t d = DummySMSTalker (default)" << endl
           <<         "   -t x = XMLServerSMSTalker" << endl
           <<         "   -t g = GM12SMSTalker" << endl
           <<  "CIMDSMSTalker options:" << endl
           <<         "   -ci host Set SMSC IP" << endl
           <<         "   -cp port Set SMSC portnumber" << endl
           <<         "   -cu user Set SMSC username" << endl
           <<         "   -cw pin  Set SMSC password (will show is ps)" << endl
           <<  "GM12 Options:" << endl
           <<         "   -gp port = Serial port device name" << endl
           <<         "   -gw pin  = Pin code (will show in ps)" << endl
           <<  "XML Options:" << endl
           <<         "   -xu user = XMLServer user name" << endl
           <<         "   -xw pass = XMLServer passwd" << endl
           <<         "   -xa url  = XMLServer url" << endl;
      
      exit(1);
   }
   if ( preferredPort == 0 ) {
      cout << "Using default port " << defPort << endl;
      preferredPort = defPort;
   }
   /* Init threads */
   JTCInitialize init;
   
   SMSTalker* smsTalker;
   // The arguments are parsed now
   if ( serviceType == 'g' || serviceType == 'G' )
      smsTalker = new GM12SMSTalker(gm12Port, gm12Pin);
   else if ( serviceType == 'c' || serviceType == 'C' ) {
      smsTalker = new CIMDSMSTalker(cimdIP,
                                    cimdPort,
                                    cimdUserName,
                                    cimdPassword,
                                    bindAddress);
   } else if ( serviceType == 'x' || serviceType == 'X' ) {
      smsTalker = new XMLSMSTalker(xmlURL.c_str(),
                                   cimdUserName, cimdPassword);
   } else {
      smsTalker = new DummySMSTalker;
   }
   smsTalker->setPhoneNumbers(nbrPhones, phones);
   smsTalker->setServiceNames(nbrServices, services);
   if ( logFilePathEntered ) {
      if ( ! smsTalker->openLogFile(logFilePath.c_str()) ) {
         mc2log << fatal << "[SMSModule]: Could not open logfile "
                << MC2CITE(smsTalker->getLogFileName()) << endl;
         abort(); exit(1);
      }
   }

   MapSafeVector* loadedMaps = new MapSafeVector();
   SMSProcessor processor(smsTalker, loadedMaps);
   cout << "SMSProcessor constructed" << endl;
   if ( smsTalker->init(&processor) == false ) {
      cerr << "Couldn't init SMSTalker" << endl;
   }      
   cout << "SMSTalker initialized" << endl;


   uint32 leaderIP = MultiCastProperties::getNumericIP( MODULE_TYPE_SMS,
                                                        true );
   uint16 leaderPort = MultiCastProperties::getPort( MODULE_TYPE_SMS,
                                                     true );
   uint32 availIP = MultiCastProperties::getNumericIP( MODULE_TYPE_SMS,
                                                       false );
   uint16 availPort = MultiCastProperties::getPort( MODULE_TYPE_SMS,
                                                    false );

   ModulePacketSenderReceiver senderReceiver( preferredPort,
                                              IPnPort( leaderIP, leaderPort ),
                                              IPnPort( availIP, availPort ) );

   PacketQueue *queue     = new PacketQueue();
   JobThread* jobThread = new JobThread(&processor, queue,
                                        senderReceiver.getSendQueue() );   



   PacketReaderThread* packetReader = 
      new PacketReaderThread( senderReceiver );

   SMSReader* reader = new SMSReader(queue, 
                                     senderReceiver.getSendQueue(),
                                     loadedMaps, 
                                     packetReader,
                                     senderReceiver.getPort(),
                                     rank, nbrPhones, nbrServices,
                                     phones, services);
   delete [] phones;
   delete [] services;
   delete [] cimdIP;
   delete [] cimdPassword;
   delete [] cimdUserName;
   
   jobThread->start();
   packetReader->start();
   reader->start();
   
   while(reader->isAlive()) {
      try {
         reader->join();
      }
      catch (const JTCException &) {}
   }

   cerr << "SMSModule exiting" << endl;
   
   delete jobThread;
   delete reader;
   delete queue;
   delete loadedMaps;
   return 0;
}

