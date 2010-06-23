/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETDUMP_H
#define PACKETDUMP_H

#include <iosfwd>
#include <cstdio>

#include "MC2String.h"

class Packet;


namespace PacketUtils {

/**
 * Dumps entire package (header + data) to file
 * @param filename the name of the file to dump to
 * @param packet the packet to dump
 */ 
void dumpToFile( const MC2String& filename, const Packet& packet );

/**
 * Dumps header and data to file
 * @param file dump to this file
 * @param packet the package to dump
 */
void dumpToFile( FILE* file, const Packet& packet );

/**
 * Dumps package header to file
 * @param file the file to dump to
 * @param packet the package to dump
 * @param lookupIP whether the ip in the package should be looked up
 */
void dumpHeaderToFile( FILE* file, const Packet& packet, 
                       bool lookupIP = false );

/**
 * Dumps all data after position HEADER_SIZE 
 * @param file the file to dump to
 * @param packet the package to dump
 */
void dumpDataToFile( FILE* file, const Packet& packet );

/**
 * Dumps data portion of the file
 * @param file the file
 * @param packet the packet to dump
 * @param startPosition start of data in packet
 */
void dumpDataToFile( FILE* file, const Packet& packet, 
                     uint32 startPosition );

/**
 * Dumps entire package to stdout. @see dumpToFile
 * @param packet the packet to dump
 */
void printPacket( const Packet& packet );

/**
 * Writes packet in binary form to a file
 * @param filename the name of the file to append package to
 * @param packet the packet to write
 */
void dumpBinaryToFile( const MC2String& filename, const Packet& packet );

/**
 * Writes packet in binary form to a file
 * @param file the file to append packet to
 * @param packet the packet which will be written
 */
void dumpBinaryToFile( FILE* file, const Packet& packet );


/**
 * Loads a single packet from a file.
 * @param filename the name of the file to load from.
 * @param packet the packet to load the data into,
 * @return allocated packet.
 */
Packet* loadPacketFromFile( const MC2String& filename );

/**
 * Loads a packet from a file
 * @param file the current file to load from
 * @param packet the packet to load the data into
 * @return allocated packet
 */
Packet* loadPacketFromFile( FILE* file );

}


#endif
