/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERIALCOMM_4711_H
#define SERIALCOMM_4711_H

#include "Types.h"
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "MC2String.h"


/**
 *   SerialCommunications encapsulated.
 *   Will only work in linux (hopefully unix?) for now.
 *   Should maybe be superclassed with a *Comm-class.
 */
class SerialComm {
   
public:
   /**
    *   Creates a new SerialComm object. Open the port with openPort(..).
    */
   SerialComm();

   /**
    *   Destroys this object and closes the serial port.
    */
   virtual ~SerialComm();
   
   /**
    *    Open a serial port.
    *    @param portnumber          The number of the port to open.
    *                               COM1=ttyS0=0
    *    @param speed               The speed in bps. 
    *    @param rtscts              If this is true, use RTSCTS
    *    @return                    True if success.
    */
   bool openPort(unsigned int portnumber,
                 speed_t speed,
                 bool rtscts);

   /**
    *    Open a serial port.
    *    @param device              The name of the serial device.
    *    @param speed               The speed in bps. 
    *    @param rtscts              If this is true, use RTSCTS
    *    @return                    True if success.
    */
   bool openPort(const char* device,
                 speed_t speed,
                 bool rtscts);
   
   /**
    *    Send a string on the serial port.
    *    @param buf      The string to send.
    *    @param numchars The number of chars to send.
    *    @return         The number of chars actually sent.
    *                    < 0 if an error occured.
    */
   int send(const char* buf, int numchars);

   /**
    *    Receive a maximum of bufsize characters from the serial port
    *    @param buf     The buffer to put the result into.
    *    @param bufsize The maximum number of characters to receive.
    *    @return        The number of characters actually received.
    */
   int receive(char* buf, int bufsize);

   /**
    *    Receive a maximum of bufsize characters from the open serial
    *    port. Wait a maximum of micros usec, then return -1.
    *    @param buf     The buffer to put the result into.
    *    @param bufsize The maximum number of chars to receive.
    *    @param micros  The maximum number of microseconds to wait.
    *                   Wait forever if micros == 0
    *    @return The number of characters received.
    */
   int receive(char* buf, int bufsize, uint32 micros);

private:

   /** Place to signal when something happens on the serial port */
   static int semaphore;
   
   /** Indicates if the port is open or not */
   bool portOpen;

   /** The filedescriptor */
   int fd;
   
   /** Place to save old portsettings */
   struct termios oldtio;

   /** The current terminal settings */
   struct termios newtio;

   /** The current waittime */
   int waitTime;

};


#endif




