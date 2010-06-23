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
#include "MEGdkColors.h"

MEGdkColors* MEGdkColors::m_instance = NULL;

MEGdkColors*
MEGdkColors::getColors() 
{
   if (m_instance == NULL) {
      m_instance = new MEGdkColors();
   }
   return (m_instance);
}

MEGdkColors::MEGdkColors()
{
   Glib::RefPtr<Gdk::Colormap> colormap_ = Gtk::Widget::get_default_colormap ();

   m_black = Gdk::Color("black");
   m_white = Gdk::Color("white");
   m_blue = Gdk::Color("blue");
   m_red = Gdk::Color("red");
   m_green = Gdk::Color("green");
   m_brown = Gdk::Color("brown");
   m_orange = Gdk::Color("orange");
   m_indianRed= Gdk::Color("indian red");
   m_peru = Gdk::Color("peru");
   m_rosyBrown= Gdk::Color("rosy brown");

   // Grey
   m_lightGrey = Gdk::Color("light grey");
   m_grey = Gdk::Color("grey");
   m_lightSlateGrey = Gdk::Color("light slate grey");
   m_slateGrey = Gdk::Color("slate grey");
   m_dimGrey = Gdk::Color("dim grey");
   m_darkSlateGrey= Gdk::Color("dark slate grey");

   // Yellow
   m_lightYellow1 = Gdk::Color("LightYellow1");
   m_lightYellow2 = Gdk::Color("LightYellow2");
   m_lightYellow3 = Gdk::Color("LightYellow3");
   m_lightYellow4 = Gdk::Color("LightYellow4");

   // Blue
   m_lightBlue = Gdk::Color("light blue");

   m_purple1 = Gdk::Color("purple1");

   m_green3 = Gdk::Color("green3");
   m_yellow = Gdk::Color("yellow");
   m_gray16 = Gdk::Color("gray16");

   colormap_->alloc_color(m_black);
   colormap_->alloc_color(m_white);
   colormap_->alloc_color(m_blue);
   colormap_->alloc_color(m_red);
   colormap_->alloc_color(m_green);
   colormap_->alloc_color(m_brown);
   colormap_->alloc_color(m_orange);
   colormap_->alloc_color(m_indianRed);
   colormap_->alloc_color(m_peru );
   colormap_->alloc_color(m_rosyBrown);
   // Grey
   colormap_->alloc_color(m_lightGrey);
   colormap_->alloc_color(m_grey);
   colormap_->alloc_color(m_lightSlateGrey);
   colormap_->alloc_color(m_slateGrey);
   colormap_->alloc_color(m_dimGrey);
   colormap_->alloc_color(m_darkSlateGrey);
   // Yellow
   colormap_->alloc_color(m_lightYellow1);
   colormap_->alloc_color(m_lightYellow2);
   colormap_->alloc_color(m_lightYellow3);
   colormap_->alloc_color(m_lightYellow4);
   // Blue
   colormap_->alloc_color(m_lightBlue);

   colormap_->alloc_color(m_purple1);
   colormap_->alloc_color(m_green3);
   colormap_->alloc_color(m_yellow);
   colormap_->alloc_color(m_gray16);
}


Gdk::Color& 
MEGdkColors::getColor(color_t c)
{
   switch (c) {
      case black : return m_black;
      case white : return m_white;
      case blue : return m_blue;
      case red : return m_red;
      case green : return m_green;
      case orange : return m_orange;
      case indianRed : return m_indianRed;
      case peru : return m_peru;
      case rosyBrown : return m_rosyBrown;
      case lightGrey : return m_lightGrey;
      case grey : return m_grey;
      case lightSlateGrey : return m_lightSlateGrey;
      case slateGrey : return m_slateGrey;
      case dimGrey : return m_dimGrey;
      case darkSlateGrey : return m_darkSlateGrey;
      case lightYellow1 : return m_lightYellow1;
      case lightYellow2 : return m_lightYellow2;
      case lightYellow3 : return m_lightYellow3;
      case lightYellow4 : return m_lightYellow4;
      case lightBlue : return m_lightBlue;
      case purple1 : return m_purple1;
      case green3 : return m_green3;
      case yellow : return m_yellow;
   case gray16 : return m_gray16;
      case invalidColor :
         mc2log << error << here << "request for invalidColor, termintaing"
                << endl;
         exit(1);
   }
   mc2log << error << here << "Reached impossible source-code" << endl;

   return m_red;
}

const char*
MEGdkColors::getColorString(color_t c)
{
   switch (c) {
      case black : return "black";
      case white : return "white";
      case blue : return "blue";
      case red : return "red";
      case green : return "green";
      case orange : return "orange";
      case indianRed : return "indianRed";
      case peru : return "peru";
      case rosyBrown : return "rosyBrown";
      case lightGrey : return "lightGrey";
      case grey : return "grey";
      case lightSlateGrey : return "lightSlateGrey";
      case slateGrey : return "slateGrey";
      case dimGrey : return "dimGrey";
      case darkSlateGrey : return "darkSlateGrey";
      case lightYellow1 : return "lightYellow1";
      case lightYellow2 : return "lightYellow2";
      case lightYellow3 : return "lightYellow3";
      case lightYellow4 : return "lightYellow4";
      case lightBlue : return "lightBlue";
      case purple1 : return "purple1";
      case green3 : return "green3";
      case yellow : return "yellow";
      case gray16 : return "gray16";
      case invalidColor :
         mc2log << error << here << "request for invalidColor, termintaing"
                << endl;
         exit(1);
   }
   mc2log << error << here << "request for invalidColor, termintaing" << endl;
   return NULL;
}

MEGdkColors::color_t
MEGdkColors::getColorFromString(const char* s)
{
   if (StringUtility::strcasecmp(s, "black") == 0)
      return black;
   else if (StringUtility::strcasecmp(s, "white") == 0)
      return white;
   else if (StringUtility::strcasecmp(s, "blue") == 0)
      return blue;
   else if (StringUtility::strcasecmp(s, "red") == 0)
      return red;
   else if (StringUtility::strcasecmp(s, "green") == 0)
      return green;
   else if (StringUtility::strcasecmp(s, "orange") == 0)
      return orange;
   else if (StringUtility::strcasecmp(s, "indianRed") == 0)
      return indianRed;
   else if (StringUtility::strcasecmp(s, "peru") == 0)
      return peru;
   else if (StringUtility::strcasecmp(s, "rosyBrown") == 0)
      return rosyBrown;
   else if (StringUtility::strcasecmp(s, "lightGrey") == 0)
      return lightGrey;
   else if (StringUtility::strcasecmp(s, "grey") == 0)
      return grey;
   else if (StringUtility::strcasecmp(s, "lightSlateGrey") == 0)
      return lightSlateGrey;
   else if (StringUtility::strcasecmp(s, "slateGrey") == 0)
      return slateGrey;
   else if (StringUtility::strcasecmp(s, "dimGrey") == 0)
      return dimGrey;
   else if (StringUtility::strcasecmp(s, "darkSlateGrey") == 0)
      return darkSlateGrey;
   else if (StringUtility::strcasecmp(s, "lightYellow1") == 0)
      return lightYellow1;
   else if (StringUtility::strcasecmp(s, "lightYellow2") == 0)
      return lightYellow2;
   else if (StringUtility::strcasecmp(s, "lightYellow3") == 0)
      return lightYellow3;
   else if (StringUtility::strcasecmp(s, "lightYellow4") == 0)
      return lightYellow4;
   else if (StringUtility::strcasecmp(s, "lightBlue") == 0)
      return lightBlue;
   else if (StringUtility::strcasecmp(s, "purple1") == 0)
      return purple1;
   else if (StringUtility::strcasecmp(s, "green3") == 0)
      return green3;
   else if (StringUtility::strcasecmp(s, "yellow") == 0)
      return yellow;
   else if (StringUtility::strcasecmp(s, "gray16") == 0)
      return gray16;
   else {
      //mc2dbg << error << here << " Failed to find color" << endl;
      return invalidColor;
   }

}

