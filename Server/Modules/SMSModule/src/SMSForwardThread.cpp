/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SMSForwardThread.h"
#include "Properties.h"
#include "PacketNet.h"

SMSForwardThread::SMSForwardThread()
{
   m_queue = new PacketQueue;
}


void
SMSForwardThread::run()
{
   Packet* packet = NULL;
   
   while(!terminated) {
      packet = m_queue->dequeue();
      mc2dbg2 << "SMSForwardThread: packet dequeued" << endl;
      if ( packet != NULL ) {
         switch ( packet->getSubType() ) {
            case Packet::PACKETTYPE_SMSREQUEST: {
               SMSSendRequestPacket* srp = (SMSSendRequestPacket*)packet;
               char* senderPhone    = NULL;
               char* recipientPhone = NULL;
               int   dataLength     = 0;
               byte* data           = NULL;
               int smsNumber        = -1;
               int nbrParts         = -1;
               int partNbr          = -1;
               srp->getPacketContents(CONVERSION_NO,
                                      senderPhone, recipientPhone,
                                      dataLength, data, smsNumber,
                                      nbrParts, partNbr);
               delete [] data;
               if ( smsNumber == -1 ) {
                  handleSendRequest(packet);
               }
               else {
                  DEBUG1(cout << "  SMSForwardThread - Got concatenated SMS "
                         " part " << partNbr << " of " << nbrParts << endl);
                  SMSSendRequestPacket* srp = (SMSSendRequestPacket*)packet;
                  handleConcatenatedSMS(srp, senderPhone, smsNumber,
                                        nbrParts, partNbr);
               }
            }
            break;            
            default:
              mc2log << warn << "SMSForwardThread: - Wrong packet type!" << endl;
               break;
         }
      } else {
         mc2log << error << "SMSForwardThread: packet is NULL!" << endl;
      }
      delete packet;
   }
}

void
SMSForwardThread::handleSendRequest(Packet* packet)
{
    SMSSendRequestPacket* p = static_cast<SMSSendRequestPacket*>(packet);

    MC2String smsProcessCommand = Properties::getProperty("SMS_PROCESS_COMMAND");
    char* senderPhone;
    char* recipientPhone;
    byte* data;
    int dataLength;

    if (smsProcessCommand == "") {
        mc2log << warn << "SMSForwardThread: Incoming SMS received but "
               <<  "SMS_PROCESS_COMMMAND not set in property file" << endl;
    }

    p->getPacketContents (1, senderPhone, recipientPhone, dataLength, data);

    smsProcessCommand = smsProcessCommand + " " + recipientPhone + " " + senderPhone;

  // sender and receiver added as args
    mc2log << info << "[SMSForwardThread] About to launch: '" << smsProcessCommand
           << "'" << endl;
    FILE* proc = popen(smsProcessCommand.c_str(), "w");
    if (proc == NULL) {
       mc2log << error << "[SMSForwardThread] could not launch process"
              << " command (" << smsProcessCommand << "), strerror: " 
              << strerror(errno) << endl;
    }
    size_t dataWritten = fwrite(data, 1, dataLength, proc);
    if (dataWritten != (size_t)dataLength) {
       mc2log << error << "[SMSForwardThread] could not write SMS content"
              << " to process command (" << smsProcessCommand << ")" << endl;
    }
    int retVal = pclose(proc);
    mc2dbg << "[SMSForwardThread] retVal is: " << retVal << endl;
    if (retVal != 0) {
       mc2log << error << "[SMSForwardThread] error with process"
              << " command (" << smsProcessCommand << "), retVal: "
              << retVal << ", strerror: " << strerror(errno) << endl;
    }
}


void
SMSForwardThread::handleConcatenatedSMS(SMSSendRequestPacket* srp,
                                        char* senderPhone,
                                        int smsNumber,
                                        int nbrParts,
                                        int partNbr)
{
   mc2log << warn << "[SMSForwardThread] Sorry, concatenated SMS handling"
                  << "is not implemented." << endl;
   // WARNING !! Packet is deleted outside (after) this procedure!
}


void
SMSForwardThread::enqueue(Packet *p)
{
   m_queue->enqueue(p);
}
