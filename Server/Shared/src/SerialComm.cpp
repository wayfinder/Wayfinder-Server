/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <sys/time.h>
#include "SerialComm.h"
#include "StringUtility.h"


SerialComm::SerialComm() : portOpen(false)
{
   
}

SerialComm::~SerialComm()
{
   // Flush the buffer and close the port.
   tcflush(fd, TCIFLUSH);
   close(fd);
}
   
bool
SerialComm::openPort(unsigned int portnumber, speed_t speed, bool rtscts)
{
   char portname[40];
   sprintf(portname, "/dev/ttyS%d", portnumber);
   return openPort(portname, speed, rtscts);
}

bool
SerialComm::openPort(const char* device,
                     speed_t speed,
                     bool rtscts)
{
   char* portname;
 
   if ( device == NULL ) {
      portname = StringUtility::newStrDup("/dev/ttyS0");
   } else {
      char* endPtr;
      int portnumber = strtol(device, &endPtr, 10);
      if ( endPtr == device + strlen(device) ) {
         // Just a number
         char buffer[1024];
         sprintf(buffer, "/dev/ttyS%d", portnumber);
         portname = StringUtility::newStrDup(buffer);
      } else {
         // Complete device name
         portname = StringUtility::newStrDup(device);
      }
   }

   DEBUG1(cerr << "Trying to open " << portname << endl);
   fd = open(&(portname[0]), O_RDWR | O_NOCTTY);
   if ( fd < 0 ) {
      // Couldn't open port!
      perror(portname);
      delete [] portname;
      return false;
   } else {
      delete [] portname;
   }
   // Save the old attributes
   tcgetattr(fd, &oldtio);
   // Set the new attribs
   memset(&newtio, 0, sizeof(newtio));
   // CS8    = 8 bits
   // CLOCAL = Ignore modem control lines
   // CREAD  = Enable receiver
   newtio.c_cflag = speed | CS8 | CLOCAL | CREAD;
   if ( rtscts )
      newtio.c_cflag |= CRTSCTS;
   // Ignore the parity bit.
   newtio.c_iflag = IGNPAR;
   // Raw output
   newtio.c_oflag = 0;
   // No canonical mode
   newtio.c_lflag = 0;
   // Flags
   newtio.c_cc[VTIME] = 0; // No intercharacter timer.
   newtio.c_cc[VMIN]  = 1; // Blocking reads until 1 char arrives

   waitTime = 0;
   
   // Flush the input buffer and set the settings
   tcflush(fd, TCIFLUSH);
   tcsetattr(fd, TCSANOW, &newtio);

   portOpen = true;   
   return true;
}

int
SerialComm::send(const char* buf, int numchars)
{
   cout << "Sending: " << buf << endl;
   return write(fd, buf, numchars);
}


int
SerialComm::receive(char* buf, int bufsize)
{
   int result = read(fd, buf, bufsize);
   if ( result )
      cout << "Received: " << buf[0] << endl;
   return result;
}


int
SerialComm::receive(char* buf, int bufsize, uint32 micros)
{
   if (micros == 0)
      return receive(buf, bufsize); // Wait forever
   else {
      fd_set rfds;
      struct timeval tv;
      int retval;
      
      /* Add our fd to the set */
      FD_ZERO(&rfds);
      FD_SET(fd, &rfds);
      
      /* Put the microseconds into tv */
      tv.tv_sec = micros / 1000000;
      tv.tv_usec = micros % 1000000;
      
      /* Do the selection */
#ifdef __unix
      retval = select(getdtablesize(), &rfds, NULL, NULL, &tv);
#endif
#ifdef _WIN32
      retval = select(0, &rfds, NULL, NULL, &tv);
#endif
      
      if(FD_ISSET(fd, &rfds)) {
         return receive(buf, bufsize);
      } else {// Timeout
         return -1;
      }
   }
}
