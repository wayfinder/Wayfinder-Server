/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPDRAWER_H
#define MAPDRAWER_H

#include "config.h"

#include "ImageDrawConfig.h"
#include "GDColor.h"
#include "RouteArrowType.h"

#include <memory>

class ImageDraw;
class GfxData;
class GfxDataFull;
class GfxFeature;
class GfxFeatureMap;
class MapSetting;
class MapSettings;
class POIImageIdentificationTable;
class DrawingProjection;

/**
  *   Objects of this class creates an map in image-format.
  *
  */
class MapDrawer {
public:

   /**
    *    Create a new object.
    */
   MapDrawer();
      
   ~MapDrawer();
   /**
    *   Draws a GfxFeature map
    *

    *                     
    *   @param beforeTurn If this is a map illustrating a specific 
    *                    crossing, this is the id in the route for the 
    *                    feature right before the crossing in question.
    *
    *                    NB.
    *                    This is the index counting from the first rotue
    *                    feature, i.e. if the empty features should be
    *                    used to convert this id to the feature id in
    *                    the GfxFeatureMap, 1 must be added in order to
    *                    compensate for the route origin.
    *
    *   @param afterTurn  If this is a map illustrating a specific 
    *                    crossing, this is the id in the route for the 
    *                    feature right after the crossing in question.
    *    
    *                    NB.
    *                    This is the index counting from the first rotue
    *                    feature, i.e. if the empty features should be
    *                    used to convert this id to the feature id in
    *                    the GfxFeatureMap, 1 must be added in order to
    *                    compensate for the route origin.
    *
    * @param drawCopyRight If we want a copyright in the image.
    * @param imageTable the image translation table for short poi images
    */
   byte* drawGfxFeatureMap(GfxFeatureMap* map,
                           uint32& size, 
                           MapSettings* mapSettings,
                           int16 mapRotation = 0,
                           RouteArrowType::arrowType arrowType = 
                           RouteArrowType::NOT_USED,
                           uint32 beforeTurn = MAX_UINT32, 
                           uint32 afterTurn = MAX_UINT32,
                           int32 arrowAngle = 30, int32 arrowLengh = 6,
                           GDUtils::Color::CoolColor arrowColor = 
                           static_cast<GDUtils::Color::CoolColor>
                           ( 0x000000 ),
                           bool drawText = true,
                           ImageDrawConfig::imageFormat format=
                           ImageDrawConfig::PNG,
                           const char* copyright = "",
                           bool drawCopyRight = true,
                           const POIImageIdentificationTable* 
                           imageTable = NULL );

private:
   /**
    * Sets basic image parameters.
    *
    * @param screenX Size of screen/image.
    * @param screenY Size of screen/image.
    * @param mapSettings Map settings used for drawing the map.
    *                    The color of water item is used for setting
    *                    the image background. If mapSettings == NULL
    *                    the default value for background color is used.
    */
   void init(uint32 screenX, uint32 screenY,
             MapSettings* mapSettings = NULL);
   
      
   auto_ptr<ImageDraw> m_imageDraw;

   const DrawingProjection* m_drawingProjection;


   int m_scaleLevel;
      


};


//==================================================================



#endif






