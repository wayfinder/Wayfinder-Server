/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEGDK_COLORS_H
#define MEGDK_COLORS_H

#include <gtkmm/widget.h>
#include <gdkmm/color.h>
#include <gdkmm/colormap.h>
#include "config.h"
#include "StringUtility.h"

/**
 *    Static class with all the colors that are used in the program.
 *    Has also a type for the colors that are easier to pass around and
 *    can be given default-values.
 *
 */
class MEGdkColors {
   public:
      /**
       *    An enumeration with the valid colors as constants.
       */
      enum color_t {
         // Use these for drawing order of highlight according to rainbow +
         // black and white.
         black, 
         purple1,
         blue,
         lightBlue,
         green3,
         green,
         yellow,
         orange,
         red,
         white,
         
         // Other, non-sorted colors.
         indianRed,
         peru,
         rosyBrown,
         // grey
         lightGrey,
         grey,
         gray16,         
         lightSlateGrey,
         slateGrey,
         dimGrey,
         darkSlateGrey,
         // light yellow
         lightYellow1,
         lightYellow2,
         lightYellow3,
         lightYellow4,


         // Keep last.
         invalidColor
      };

      
      /**
       *    @name Pre-allocated colors
       *    The pre allocated colors, available when an object is 
       *    created,
       */
      //@{
         Gdk::Color m_black;
         Gdk::Color m_white;
         Gdk::Color m_blue;
         Gdk::Color m_red;
         Gdk::Color m_green;
         Gdk::Color m_brown;
         Gdk::Color m_orange;
         Gdk::Color m_indianRed;
         Gdk::Color m_peru;
         Gdk::Color m_rosyBrown;

         // Grey (order: light --> dark)
         Gdk::Color m_lightGrey;
         Gdk::Color m_grey;
         Gdk::Color m_lightSlateGrey;
         Gdk::Color m_slateGrey;
         Gdk::Color m_dimGrey;
         Gdk::Color m_darkSlateGrey;

         // Yellow
         Gdk::Color m_lightYellow1;
         Gdk::Color m_lightYellow2;
         Gdk::Color m_lightYellow3;
         Gdk::Color m_lightYellow4;

         //Blue
         Gdk::Color m_lightBlue;

         Gdk::Color m_purple1;
         Gdk::Color m_green3;
         Gdk::Color m_yellow;
         Gdk::Color m_gray16;
      //@}

      /**
       *    Get the color from a given enumeration-value.
       */
      Gdk::Color& getColor(color_t c);

      /**
       *    Get the one and only object of this class.
       */
      static MEGdkColors* getColors();

      static const char* getColorString(color_t c);
      static color_t getColorFromString(const char* s);
   
   private:
      MEGdkColors();
      static MEGdkColors* m_instance;
};

#endif

