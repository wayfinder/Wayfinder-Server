/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPSETTINGSTYPE_H
#define MAPSETTINGSTYPE_H

#include "config.h"


class MapSettingsTypes {
   public:
      /**
       * The supported types of default settings.
       */
      enum defaultMapSetting {
         /// STD, standard, the normal settings.
         MAP_SETTING_STD = 0,
         MAP_SETTING_WAP = 1
      };


      /**
       * Holds a partial MapSettings.
       */
      struct ImageSettings {
         bool m_image_show_street_main;
         bool m_image_show_street_first;
         bool m_image_show_street_second;
         bool m_image_show_street_third;
         bool m_image_show_street_fourth;
         bool m_image_show_builtup_area;
         bool m_image_show_park;
         bool m_image_show_forest;
         bool m_image_show_building;
         bool m_image_show_water;
         bool m_image_show_island;
         bool m_image_show_pedestrianarea;
         bool m_image_show_aircraftroad;
         bool m_image_show_land;

         void reset() {
            m_image_show_street_main = true;
            m_image_show_street_first = true;
            m_image_show_street_second = true;
            m_image_show_street_third = true;
            m_image_show_street_fourth = true;
            m_image_show_builtup_area = true;
            m_image_show_park = true;
            m_image_show_forest = true;
            m_image_show_building = true;
            m_image_show_water = true;
            m_image_show_island = true;
            m_image_show_pedestrianarea = true;
            m_image_show_aircraftroad = true;
            m_image_show_land = true;
         }


         void dump( ostream& out ) {
            out << "   show_street_main " << m_image_show_street_main
                 << endl;
            out << "   show_street_first " << m_image_show_street_first
                 << endl;
            out << "   show_street_second " << m_image_show_street_second
                 << endl;
            out << "   show_street_third " << m_image_show_street_third
                 << endl;
            out << "   show_street_fourth " << m_image_show_street_fourth
                 << endl;
            out << "   show_ builtup_area" << m_image_show_builtup_area
                 << endl;
            out << "   show_park " << m_image_show_park << endl;
            out << "   show_forest " << m_image_show_forest << endl;
            out << "   show_building " << m_image_show_building << endl;
            out << "   show_water " << m_image_show_water << endl;
            out << "   show_island " << m_image_show_island << endl;
            out << "   show_pedestrianarea " 
                 << m_image_show_pedestrianarea << endl;
            out << "   show_aircraftroad " << m_image_show_aircraftroad
                 << endl;
            out << "   show_land " << m_image_show_land << endl;
         }
      };


   private:
      /**
       * Private constructor.
       */
      MapSettingsTypes();
};


#endif // MAPSETTINGSTYPE_H

