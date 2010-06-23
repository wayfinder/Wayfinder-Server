/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "LandmarkLink.h"
#include "ExpandStringItem.h"
#include "StringUtility.h"
#include "StringTableUtility.h"

LandmarkLink::LandmarkLink(
                      ItemTypes::lmdescription_t description,
                      int32 dist, bool distUpdated, const char* name)
   :  Link(),
      m_lmdesc(description),
      m_lmDist(dist),
      m_lmDistUpdated(distUpdated)
{
   setEndLM(false);
   m_lmName = StringUtility::newStrDup(name);
   m_streetName = NULL;
   m_isDetour  = false;
   m_isStart   = true;
   m_isStop    = true;
   m_isTraffic = false;
   m_distNodeID = MAX_UINT32;
}

LandmarkLink::~LandmarkLink()
{
   delete [] m_lmName;
   delete [] m_streetName;
}

void
LandmarkLink::setLMName(const char* name)
{
   mc2dbg8 << "updating m_lmName from \"" << m_lmName << "\" to \"" << name
        << "\"" << endl;
   delete [] m_lmName;
   m_lmName = StringUtility::newStrDup(name);
}

void
LandmarkLink::setStreetName(const char* name)
{
   if(name != NULL){
      mc2dbg8 << "updating m_streetName from \"" << m_streetName << "\" to \""
              << name << "\"" << endl;
      delete [] m_streetName;
      m_streetName = StringUtility::newStrDup(name);
   }

}

void
LandmarkLink::dump()
{
   mc2dbg << "    landmarkID = " << m_lmdesc.itemID 
          << ", name = \"" << m_lmName << "\"" << endl
          << "    type = " << int(m_lmdesc.type) 
          << ", dist = " << m_lmDist << endl;
   mc2dbg << "    importance = " << m_lmdesc.importance 
          << ", location = " << int(m_lmdesc.location) 
          << ", side = " << int(m_lmdesc.side) << endl;
   mc2dbg << "Landmark ";
   if(!isDetour())
     mc2dbg  << "not ";
   mc2dbg << "detour ";
   if(!isStart())
     mc2dbg  << "not ";
   mc2dbg << "Start ";
   if(!isStop())
     mc2dbg  << "not ";
   mc2dbg << "Stop ";
   if(!isTraffic())
      mc2dbg << "not ";
   mc2dbg << "traffic ";
   mc2dbg << endl;
   
}

bool
LandmarkLink::getRouteDescription(const DescProps& descProps,
                                  char* buf,
                                  uint32 maxLength,
                                  uint32& nbrBytesWritten,
                                  StringTable::stringCode turncode)
{
   // Extract information from the description properties bitfield.
   StringTable::languageCode lang = descProps.getLanguage();
   StringTableUtility::distanceFormat distFormat = descProps.getDistFormat();
   StringTableUtility::distanceUnit distUnit = descProps.getDistUnit();
   bool wapFormat = descProps.getWapFormat();

   buf[0] = '\0';

   char tmpBuf[1024]; // cheaper to allocate on the stack. less errorprone to do it once.
   // also, there will be no mem leaks on the below return false:s.
   tmpBuf[600] = char(255); // write a check on which we may at least assert. (600 instead of 1023 to give us some margin)

  
   if(isDetour()){
      if(isStart() &&  !wapFormat){
         // Add detour string .Not if wapformat == navigator
         strcpy(tmpBuf,
                StringTable::getString(StringTable::DETOUR_LM, lang));
         if (!ExpandStringItem::addString(
                  tmpBuf, buf, maxLength, nbrBytesWritten, false)) {
            return false;
         }
      } else if (!wapFormat){
         // Add end of detour string. Not if wapformat == navigator
         strcpy(tmpBuf,
                StringTable::getString(StringTable::ENDOF_DETOUR_LM, lang));
         if (!ExpandStringItem::addString(
                  tmpBuf, buf, maxLength, nbrBytesWritten, false)) {
            return false;
         }
      }
      
      
      // add space
      if (!ExpandStringItem::addString(
             " ", buf, maxLength, nbrBytesWritten, false)) {
         return false;
      }
      
      
   }
   else if (!m_endLM) {
      // Write "After xx meters |pass/go into|passera/fortsätt in i| "
      if (m_lmDist > 0) {

         // add "after"
         strcpy(tmpBuf,
                StringTable::getString(StringTable::LMLOCATION_AFTER, lang));
         StringUtility::makeFirstCapital(tmpBuf);
         if (!ExpandStringItem::addString(
                tmpBuf, buf, maxLength, nbrBytesWritten, false //wapFormat
                //&&!isTraffic()
                )) {
            return false;
         }

         // add space
         if (!ExpandStringItem::addString(
                  " ", buf, maxLength, nbrBytesWritten, wapFormat
                  &&!isTraffic())) {
            return false;
         }

         // add distance
         StringTableUtility::printDistance(tmpBuf, m_lmDist, lang, distFormat, distUnit);
         mc2dbg8 << "LLink :printDistance " << tmpBuf << endl;
         if (!ExpandStringItem::addString(
                  tmpBuf, buf, maxLength, nbrBytesWritten, wapFormat
                  &&!isTraffic())) {
            return false;
         }
         
         // add space
         if (!ExpandStringItem::addString(
                  " ", buf, maxLength, nbrBytesWritten, wapFormat
                  &&!isTraffic())) {
            return false;
         }
         
      }
      
   } else if ((m_lmdesc.location != ItemTypes::arrive) &&
              (!isTraffic())){
      
      // write the turndesc
      strcpy(tmpBuf, StringTable::getString(turncode, lang));
      StringUtility::makeFirstCapital(tmpBuf);
      if (!ExpandStringItem::addString(
             tmpBuf, buf, maxLength, nbrBytesWritten, false //wapFormat
               )) {
         return false;
      }

      // add a space
      if (!ExpandStringItem::addString(
               " ", buf, maxLength, nbrBytesWritten, false
               //wapFormat
               )) {
         return false;
      }
   }
              
   if(isTraffic()){
      if(m_lmdesc.type == ItemTypes::accidentLM){
         strcpy(tmpBuf,
                StringTable::getString(StringTable::ACCIDENT_LM, lang));
         StringUtility::makeFirstCapital(tmpBuf);
         if (!ExpandStringItem::addString(
                tmpBuf, buf, maxLength, nbrBytesWritten, false)) {
            return false;
         }

      }
      else if(m_lmdesc.type == ItemTypes::roadWorkLM){
         strcpy(tmpBuf,
                StringTable::getString(StringTable::ROADWORK_LM, lang));
         StringUtility::makeFirstCapital(tmpBuf);
         if (!ExpandStringItem::addString(
                tmpBuf, buf, maxLength, nbrBytesWritten, false)) {
            return false;
         }
      }
      else if(m_lmdesc.type == ItemTypes::cameraLM){
         strcpy(tmpBuf,
                StringTable::getString(StringTable::CAMERA_LM, lang));
         StringUtility::makeFirstCapital(tmpBuf);
         if (!ExpandStringItem::addString(
                tmpBuf, buf, maxLength, nbrBytesWritten, false)) {
            return false;
         }
      }
      else if(m_lmdesc.type == ItemTypes::blackSpotLM){
         strcpy(tmpBuf,
                StringTable::getString(StringTable::OTHER_TRAFF_LM, lang));
         StringUtility::makeFirstCapital(tmpBuf);
         if (!ExpandStringItem::addString(
                tmpBuf, buf, maxLength, nbrBytesWritten, false)) {
            return false;
         }
      }
      else if(m_lmdesc.type == ItemTypes::speedTrapLM){
         strcpy(tmpBuf,
                StringTable::getString(StringTable::SPEED_TRAP_LM, lang));
         StringUtility::makeFirstCapital(tmpBuf);
         if (!ExpandStringItem::addString(
                tmpBuf, buf, maxLength, nbrBytesWritten, false)) {
            return false;
         }
      }
      else if(m_lmdesc.type == ItemTypes::trafficOtherLM){
         strcpy(tmpBuf,
                StringTable::getString(StringTable::OTHER_TRAFF_LM, lang));
         StringUtility::makeFirstCapital(tmpBuf);
         if (!ExpandStringItem::addString(
                tmpBuf, buf, maxLength, nbrBytesWritten, false)) {
            return false;
         }
        
      }
      
      // add streetname ? 
      if(getStreetName() != NULL){
         // add " on"
         strcpy(tmpBuf,
                StringTable::getString(StringTable::PREPOSITION_POST_PARK_CAR,
                                       lang));
         if (!ExpandStringItem::addString(
                tmpBuf, buf, maxLength, nbrBytesWritten, false)) {
            return false;
         }
         // add a space
         if (!ExpandStringItem::addString(
                " ", buf, maxLength, nbrBytesWritten, false)) {
            return false;
         }
       
         // add name
         strcpy(tmpBuf, m_streetName);
         if (!ExpandStringItem::addString(
                tmpBuf, buf, maxLength, nbrBytesWritten, false)) {
            return false;
         }
      }
      // If navigator (= wapformat = true) end here.
      if(wapFormat)
         return true;
      
      // Add " :" 
      if (!ExpandStringItem::addString(
             " :", buf, maxLength, nbrBytesWritten, false)) {
         return false;
      }
      
   }
   else {
      
      // add landmark location
      strcpy(tmpBuf, StringTable::getString(
                ItemTypes::getLandmarkLocationSC(m_lmdesc.location), lang));
      if (m_lmdesc.location == ItemTypes::arrive) {
         StringUtility::makeFirstCapital(tmpBuf);
      }
      if (!ExpandStringItem::addString(
             tmpBuf, buf, maxLength, nbrBytesWritten, wapFormat)) {
         return false;
      }
   }
   
   // add a space
   if (!ExpandStringItem::addString(
            " ", buf, maxLength, nbrBytesWritten, wapFormat)) {
      return false;
   }

   // write pass THROUGH the landmark
   if (m_lmdesc.side == SearchTypes::side_does_not_matter) {
      strcpy(tmpBuf, StringTable::getString(
               ItemTypes::getLandmarkSideSC(m_lmdesc.side), lang));
      if (!ExpandStringItem::addString(
               tmpBuf, buf, maxLength, nbrBytesWritten, wapFormat)) {
         return false;
      }

      if (!ExpandStringItem::addString(
               " ", buf, maxLength, nbrBytesWritten, wapFormat)) {
         return false;
      }
   }

   // add landmark name (railway->itemname, else m_lmName) (the, -en/et etc?)
   
   if (m_lmdesc.type == ItemTypes::railwayLM) {
      if (lang == StringTable::ENGLISH) {
         strcpy(tmpBuf, "the ");
      } else {
         strcpy(tmpBuf, "");
      }
      strcat(tmpBuf, StringTable::getString(StringTable::RAILWAYITEM, lang));
      if (lang == StringTable::SWEDISH) {
         strcat(tmpBuf, "en");
      }
   } else {
      strcpy(tmpBuf, m_lmName);
   }
   
   if (!ExpandStringItem::addString(
            tmpBuf, buf, maxLength, nbrBytesWritten, false 
            // wapFormat && isTraffic()
            )) {
      return false;
   }

   // add landmark side (if left or right)
   if ((m_lmdesc.side == SearchTypes::left_side) || 
       (m_lmdesc.side == SearchTypes::right_side)) {
      // first write a space
      if (!ExpandStringItem::addString(
               " ", buf, maxLength, nbrBytesWritten, false
               //wapFormat && isTraffic()
               )) {
         return false;
      }
      // then write the side
      strcpy(tmpBuf, StringTable::getString(
               ItemTypes::getLandmarkSideSC(m_lmdesc.side), lang));
      if (!ExpandStringItem::addString(
               tmpBuf, buf, maxLength, nbrBytesWritten, false
               // wapFormat && isTraffic()
               )) {
         return false;
      }
   }

   almostAssert(tmpBuf[600] == char(255)); // if it isn't, it's been overwritten and the buffer is (close to being) too small.
   
   return true;
}
