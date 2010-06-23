/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavLogElement.h"
#include "StringUtility.h"


NavLogElement::NavLogElement(const Packet* packet, int& pos)
{
   m_comment = NULL;
   m_startStr = NULL;
   m_endStr = NULL;
   load(packet, pos);
}

NavLogElement::NavLogElement( 
   uint32 dist, uint32 meterCounter, bool work, 
   const char* comment,
   uint32 startTime, int32 startLat, int32 startLon,
   const char* startStr,
   uint32 endTime, int32 endLat, int32 endLon,
   const char* endStr, uint32 id )
      : m_id(id),
        m_dist( dist ),
        m_meterCounter( meterCounter ),
        m_work( work ),
        m_comment( StringUtility::newStrDup( comment ) ),
        m_startTime( startTime ),
        m_startLat( startLat ),
        m_startLon( startLon ),
        m_startStr( StringUtility::newStrDup( startStr ) ),
        m_endTime( endTime ),
        m_endLat( endLat ),
        m_endLon( endLon ),
        m_endStr( StringUtility::newStrDup( endStr ) )
{
}


NavLogElement::~NavLogElement() {
   delete [] m_comment;
   delete [] m_startStr;
   delete [] m_endStr;
}

uint32 
NavLogElement::save(Packet* p, int& pos) const
{
   // Order:
   // * dist
   // * meterCounter
   // * startTime
   // * startLat
   // * startLon
   // * endTime
   // * endLat
   // * endLon
   // * work? (stored in 8 bits)
   // * comment
   // * startStr
   // * endStr

   uint32 startOffset = p->getLength();
   p->incWriteLong(pos, m_id);
   p->incWriteLong(pos, m_dist);
   p->incWriteLong(pos, m_meterCounter);
   p->incWriteLong(pos, m_startTime);
   p->incWriteLong(pos, m_startLat);
   p->incWriteLong(pos, m_startLon);
   p->incWriteLong(pos, m_endTime);
   p->incWriteLong(pos, m_endLat);
   p->incWriteLong(pos, m_endLon);
   if (m_work)
      p->incWriteByte(pos, MAX_BYTE);
   else 
      p->incWriteByte(pos, 0);
      
   if (m_comment != NULL)
      p->incWriteString(pos, m_comment);
   else 
      p->incWriteString(pos, "");
   if (m_startStr != NULL)
      p->incWriteString(pos, m_startStr);
   else 
      p->incWriteString(pos, "");
   if (m_endStr != NULL)
      p->incWriteString(pos, m_endStr);
   else 
      p->incWriteString(pos, "");

   p->setLength(pos);
   return (p->getLength() - startOffset);
}

uint32
NavLogElement::getSizeAsBytes() const
{
   // id+dist+meterCounter+sTime+sLat+sLon+eTime+eLat+eLon+work
   uint32 size = 4 * 9 + 1;

   // three bytes for '\0' in comment,sStr and eStr
   uint32 strSize = 3;
   if (m_comment != NULL)
      strSize += strlen(m_comment);

   if (m_startStr != NULL)
      strSize += strlen(m_startStr);
 
   if (m_endStr != NULL)
      strSize += strlen(m_endStr);

   mc2dbg4 << "Adding " << strSize%4 << " extra bytes due to padding" 
           << "(size = " << size << ")" << endl;
   strSize += strSize%4;

   return (strSize+size);
}

void
NavLogElement::load(const Packet* p, int& pos)
{
   m_id = p->incReadLong(pos);
   m_dist = p->incReadLong(pos);
   m_meterCounter = p->incReadLong(pos);
   m_startTime = p->incReadLong(pos);
   m_startLat = p->incReadLong(pos);
   m_startLon = p->incReadLong(pos);
   m_endTime = p->incReadLong(pos);
   m_endLat = p->incReadLong(pos);
   m_endLon = p->incReadLong(pos);

   if (p->incReadByte(pos) == 0)
      m_work = false;
   else 
      m_work = true;

   char* tmpStr;
   p->incReadString(pos, tmpStr);
   delete m_comment;
   m_comment = StringUtility::newStrDup(tmpStr);

   p->incReadString(pos, tmpStr);
   delete m_startStr;
   m_startStr = StringUtility::newStrDup(tmpStr);

   p->incReadString(pos, tmpStr);
   delete m_endStr;
   m_endStr = StringUtility::newStrDup(tmpStr);
}

void
NavLogElement::dump( ostream& out ) const
{
   out << m_id << " " << m_startStr << "(" << m_startLat << ","
       << m_startLon << "), " << m_startTime 
       << " -- " << m_endStr << "(" << m_endLat << ","
       << m_endLon << "), " << m_endTime << endl;
   out << "     " << m_comment << endl;
}


// =======================================================================
//                                                    NavLogElementsList =
// =======================================================================

NavLogElementsList::NavLogElementsList()
{
   // Nothing to do
}

NavLogElementsList::NavLogElementsList(const NavLogElementsList& src)
{
   mc2log << error << "NavLogElementsList copy contructor called "
          << "This is not implemented..." << endl;
   MC2_ASSERT(false);
}


NavLogElementsList::~NavLogElementsList()
{
   mc2dbg8 << "NavLogElementsList::~NavLogElementsList size " << size() 
           << endl;
   iterator i = begin();
   while (i != end()) {
      delete (*i);
      erase(i++);
   }
}

void
NavLogElementsList::store(Packet* packet, int& pos)
{
   if (packet->getBufSize()-pos < getSizeAsBytes()) {
      mc2log << warn << "NavLogElemensList (" << getSizeAsBytes() 
             << " bytes) to large for packet (" << packet->getBufSize()
             << " - " << pos << " bytes). Resizing!" << endl;
      packet->resize(pos + getSizeAsBytes() + 8);
   }

   iterator i;

   packet->incWriteLong(pos, size());
   for (i = begin(); i != end(); i++) {
      mc2dbg8 << "NavLogElementList::store(): storing favorite id: " 
              << (*i)->getID() << endl;
      (*i)->save(packet, pos);
   }
}

void
NavLogElementsList::restore(const Packet* packet, int& pos)
{
   uint32 nbrElm = packet->incReadLong(pos);
   
   for (uint32 i = 0; i < nbrElm; i++) {
      push_back(new NavLogElement(packet, pos));
   }
}

void
NavLogElementsList::addNavLogElement(NavLogElement* elm)
{
   push_back(elm);
}


uint32
NavLogElementsList::getSizeAsBytes()
{
   uint32 size = 0;
   for (iterator i = begin(); i != end(); i++) {
      size += (*i)->getSizeAsBytes();
   }
   return size;
}

