/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UserTrackElement.h"
#include "StringUtility.h"
#include "Packet.h"


UserTrackElement::UserTrackElement(const Packet* packet, int& pos)
{
   m_source = NULL;
   load(packet, pos);
}

UserTrackElement::UserTrackElement( int32 lat, int32 lon, uint32 dist,
                                    uint16 speed, uint16 heading, 
                                    uint32 time, const char* source )
      : m_time( time ), m_lat( lat ), m_lon( lon ), m_dist( dist ),
        m_speed( speed ), m_heading( heading ),
        m_source( StringUtility::newStrDup( source) )
{
}


UserTrackElement::~UserTrackElement() {
   delete [] m_source;
}


uint32 
UserTrackElement::save(Packet* p, int& pos) const {
   // Order:
   // * time
   // * lat
   // * lon
   // dist
   // speed
   // heading
   // * source

   uint32 startOffset = p->getLength();
   p->incWriteLong(pos, m_time);
   p->incWriteLong(pos, m_lat);
   p->incWriteLong(pos, m_lon);
   p->incWriteLong( pos, m_dist );
   p->incWriteShort( pos, m_speed );
   p->incWriteShort( pos, m_heading );
   if (m_source != NULL)
      p->incWriteString(pos, m_source);
   else 
      p->incWriteString(pos, "");

   p->setLength(pos);
   return (p->getLength() - startOffset);
}

uint32
UserTrackElement::getSizeAsBytes() const {
   // time + lat + lon + dist + speed + heading
   uint32 size = 4 * 3 + 4 + 2*2;

   uint32 strSize = 1; // 1 for '\0' in source
   if (m_source != NULL)
      strSize += strlen(m_source);

   mc2dbg4 << "Adding " << strSize%4 << " extra bytes due to padding" 
           << "(size = " << size << ")" << endl;
   strSize += strSize%4;

   return (strSize+size);
}

void
UserTrackElement::load(const Packet* p, int& pos)
{
   m_time =    p->incReadLong(pos);
   m_lat =     p->incReadLong(pos);
   m_lon =     p->incReadLong(pos);
   m_dist =    p->incReadLong( pos );
   m_speed =   p->incReadShort( pos );
   m_heading = p->incReadShort( pos );

   char* tmpStr;
   p->incReadString(pos, tmpStr);
   delete m_source;
   m_source = StringUtility::newStrDup(tmpStr);
}

void
UserTrackElement::dump( ostream& out ) const
{
   out << m_time << " (" << m_lat << "," << m_lon << "), " 
       << " Dist " << int32( m_dist ) << " speed " << m_speed
       << " heading " << m_heading << " source "
       << m_source << endl;
}


// =======================================================================
//                                                 UserTrackElementsList =
// =======================================================================

UserTrackElementsList::UserTrackElementsList()
{
   // Nothing to do
}

UserTrackElementsList::UserTrackElementsList(const UserTrackElementsList& src)
{
   mc2log << error << "UserTrackElementsList copy contructor called "
          << "This is not implemented..." << endl;
   MC2_ASSERT(false);
}


UserTrackElementsList::~UserTrackElementsList()
{
   mc2dbg8 << "UserTrackElementsList::~UserTrackElementsList size " << size() 
           << endl;
   iterator i = begin();
   while (i != end()) {
      delete (*i);
      erase(i++);
   }
}

void
UserTrackElementsList::store(Packet* packet, int& pos)
{
   if (packet->getBufSize()-pos < getSizeAsBytes()) {
      mc2log << warn << "UserTrackElemensList (" << getSizeAsBytes() 
             << " bytes) to large for packet (" << packet->getBufSize()
             << " - " << pos << " bytes). Resizing!" << endl;
      packet->resize(pos + getSizeAsBytes() + 8);
   }

   iterator i;

   packet->incWriteLong(pos, size());
   for (i = begin(); i != end(); i++) {
      mc2dbg8 << "UserTrackElementList::store(): storing element" << endl;
      (*i)->save(packet, pos);
   }
}

void
UserTrackElementsList::restore(const Packet* packet, int& pos)
{
   uint32 nbrElm = packet->incReadLong(pos);
   
   for (uint32 i = 0; i < nbrElm; i++) {
      push_back(new UserTrackElement(packet, pos));
   }
}

void
UserTrackElementsList::addUserTrackElement(UserTrackElement* elm)
{
   push_back(elm);
}


uint32
UserTrackElementsList::getSizeAsBytes()
{
   uint32 size = 0;
   for (iterator i = begin(); i != end(); i++) {
      size += (*i)->getSizeAsBytes();
   }
   return size;
}

