/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXFEATUREMAPIMAGEPACKET_H
#define GFXFEATUREMAPIMAGEPACKET_H

#include "config.h"
#include "Packet.h"
#include "ImageDrawConfig.h"

#include "RouteArrowType.h"

class MapSettings;
class DataBuffer;

#define GFXFEATUREMAP_IMAGE_REQUEST_PRIO  DEFAULT_PACKET_PRIO

/**
 *   Packet used for requesting an Image over either a specific area
 *   and a route.
 *
 *   After the normal header this packet contains:
 *   Table begins at REQUEST_HEADER_SIZE
 *   \begin{tabular}{lll}
 *      Pos & Size    & \\ \hline
 *      0   &   4     & \\  The image format.
 *      4   &   4     & \\  The size of the GfxFeatureMapData.
 *      8   &   2     & \\  The size parameter from the server.
 *     10   &   1 bit & \\  A boolean which is true if we want a
 *                          GfxExportFeatureMap in return.
 *          &   1 bit & \\  A boolean which is true if we want a 
 *                          GfxExportScalableFeatrueMap in return.
 *     11   &   1     & \\  Minimum scale level to be included in the map.
 *     12   &   1     & \\  Maximum scale level to be included in the map.
 *     13   &   1     & \\  A byte telling what kind of arrow to draw in
 *                          the map if any. Uses beforeturn and afterturn
 *                          to determine where to draw the arrow.
 *     14   &   2     & \\  The number of degrees (0 - 360) the map should
 *                          be rotated around its center. Clockwise
 *                          rotation.
 *     16   &   4     & \\  beforeturn.
 *     20   &   4     & \\  afterturn.
 *     24   &   4     & \\  The size of the MapSettings
 *     28   &   4     & \\  The size of the copyright string
 *     32   &  string & \\  The Copyright string
 *     xxx   &  xxx    & \\  The MapSettings.
 *     xxx  &  xxx    & \\  The GfxFeatureMapData.
 *
 *   \end{tabular}
 *
 */
class GfxFeatureMapImageRequestPacket : public RequestPacket {
  public:
   
   /**
    *   Creates a GfxFeatureMapRequestPacket with all parameters.
    *   @param   reqID          The RequestID.
    *   @param   packetID       The PacketID.
    *   @param   imageFormat    The Image format
    *   @param   exportFormat   If we want a GfxExportFeatureMap.
    *   @param   drawCopyRight  If we want a copyright in the image.
    *   @param   size           The size of the GfxFeatureMap.
    *   @param   minScaleLevel  Minimum scale level included in the returned map.
    *   @param   maxScaleLevel  Maximum scale level included in the returned map.
    *   @param   copyright      The copyright string for the maps data.
    */
   GfxFeatureMapImageRequestPacket(uint16 reqID, uint16 packetID,
                                   ImageDrawConfig::imageFormat format,
                                   bool exportFormat,
                                   bool scalableExportFormat,
                                   bool drawCopyRight,
                                   uint32 size,
                                   uint16 sizeParam,
                                   uint8 minScaleLevel,
                                   uint8 maxScaleLevel,
                                   uint32 mapSettingSize,
                                   const char* copyright);
   
   /**
    *    Returns the Image format.
    */
   ImageDrawConfig::imageFormat getImageFormat() const;
   

  /**
    *    Sets the Image format.
    */
   void setImageFormat(ImageDrawConfig::imageFormat format);
   
   
   /**
    *    Returns the size parameter.
    */
   uint16 getSizeParameter() const;
   
   /**
    *    Returns the exportFormat parameter.
    */
   bool getExportFormat() const;
   
   /**
    *    Returns the scalableExportFormat parameter.
    */
   bool getScalableExportFormat() const;

   
   /**
    * Returns thedrawCopyRight  parameter.
    */
   bool getDrawCopyRight() const;

   /// Returns true if the packet is uint32-aligned before map data starts.
   bool alignedMapData() const;

   /**
    *    Returns minScaleLevel.
    */
   uint8 getMinScaleLevel() const;

   /**
    *    Returns maxScaleLevel.
    */
   uint8 getMaxScaleLevel() const;
   
   /**
    *    Returns the GfxFeatureMap data and MapSettings.
    *    Note that the buffer points into the packet so the
    *    packet must not be deleted during the lifetime of 
    *    the databuffer.
    */
   DataBuffer* getGfxFeatureMapData( MapSettings* setting ) const;
   
   /**
    *    Sets the GfxFeatureMap data and MapSettings.
    */
   void setGfxFeatureMapData(uint32 size,
                             DataBuffer* data, 
                             const MapSettings* setting );

   /**
    *    Sets the route turn to draw on the image. This requires a route.
    * 
    *    @param beforeTurn The index of the feature before the turn.
    *                   
    *                   NB.
    *                   This is the index counting from the first rotue
    *                   feature, i.e. if the empty features should be
    *                   used to convert this id to the feature id in the
    *                   GfxFeatureMap, 1 must be added in order to
    *                   compensate for the route origin.
    *
    *    @param afterTurn The index of the feature after the turn.
    *
    *                   NB.
    *                   This is the index counting from the first rotue
    *                   feature, i.e. if the empty features should be
    *                   used to convert this id to the feature id in the
    *                   GfxFeatureMap, 1 must be added in order to
    *                   compensate for the route origin.
    *
    */
   void setRouteTurn( uint32 beforeTurn, uint32 afterTurn );    

   /**
    *    Requests the route to be drawn as an arrow and sets the route
    *    turn to use when drawing the route as an arrow. This requires a
    *    route.
    *
    *    @param afterTurn The index of the feature after the turn.
    *
    *                   NB.
    *                   This is the index counting from the first rotue
    *                   feature, i.e. if the empty features should be
    *                   used to convert this id to the feature id in the
    *                   GfxFeatureMap, 1 must be added in order to
    *                   compensate for the route origin.
    *
    */
   void setRouteAsArrow( uint32 afterTurn );

   /**
    *    Get the kind of arrow to use when drawing the route in the 
    *    map. If no arrow is to be used, RouteArrowType::NOT_USED is returned.
    */
   RouteArrowType::arrowType getArrowType() const;

   /**
    * Get the beforeturn index.
    *
    * NB.
    * This is the index counting from the first rotue
    * feature, i.e. if the empty features should be
    * used to convert this id to the feature id in the
    * GfxFeatureMap, 1 must be added in order to
    * compensate for the route origin.
    *
    * @return beforeturn index.
    */
   uint32 getBeforeturn() const;

   /**
    * Get the afterturn index.
    *
    * NB.
    * This is the index counting from the first rotue
    * feature, i.e. if the empty features should be
    * used to convert this id to the feature id in the
    * GfxFeatureMap, 1 must be added in order to
    * compensate for the route origin.
    *
    * @return afterturn index.
    */
   uint32 getAfterturn() const;

   /**
    * Sets the angle to rotate the map. If this method is not called,
    * the map rotation is set to 0.
    * @param angle The angle to rotate the map. Clockwise rotation,
    *              0 - 360 degrees.
    */
   void setMapRotation(int16 angle);

   /**
    * Gets the angle for the map rotation. This method returns 0
    * if the map rotation has not been set explicitly.
    * @return The angle to rotate the map before drawing. Clockwise 
    *         rotation, 0 - 360 degrees. 
    */
   int16 getMapRotation() const;
   
   /**
    * Gets the copyright string.
    * 
    * @return The copyright string.
    */
   const char* getCopyright() const;

   private:
   /**
    * The positions of the things with static locations in the packet.
    */
   enum positions {
      imageFormat_POS = REQUEST_HEADER_SIZE,
      size_POS = imageFormat_POS + 4,
      sizePar_POS = size_POS + 4,
      exportFormat_POS = sizePar_POS + 2,
      minScaleLevel_POS = exportFormat_POS + 1,
      maxScaleLevel_POS = minScaleLevel_POS + 1,
      arrowType_POS = maxScaleLevel_POS + 1,
      mapRotation_POS = arrowType_POS + 1, 
      beforeTurn_POS = mapRotation_POS + 2,
      afterTurn_POS = beforeTurn_POS + 4,
      mapSettingsSize_POS = afterTurn_POS + 4,
      copyrightSize_POS = mapSettingsSize_POS + 4,
      copyright_POS = copyrightSize_POS + 4
   };
   
   /**
    * Sets the copyright string.
    * 
    * @param copyright The copyright string.
    */
   void setCopyright( const char* copyright );
};



/**
 *   Packet that is sent as a reply to a GfxFeatureMapRequestPacket.
 *
 *   After the normal header this packet contains:
 *   Table begins at REPLY_HEADER_SIZE
 *   \begin{tabular}{lll}
 *      Pos  & Size    & \\ \hline
 *       0   & 1 bit   & A boolean which is true if the packet contains
 *                       image data and false if it contains
 *                       GfxExportFeatureMap data.
 *           & 1 bit   & If the bit indicating image data is set to false 
 *                       this bit indicates that the package contains
 *                       GfxExportScalableFeatureMap data.
 *       1   & 4 bytes & The size of the Image/GfxExportFeatureMap data.
 *       5   & xxx     & The Image/GfxExportFeatureMap data. 
 *   \end{tabular}
 *     
 */

class GfxFeatureMapImageReplyPacket : public ReplyPacket {
  public:
   
   /**
    *   Creates a GfxFeatureMapImageReplyPacket by filling in the
    *   parameters from the GfxFeatureMapImageRequestPacket.
    *   @param   size      The size of the data.
    *   @param   p         The GfxFeatureMapImageRequestPacket.
    *   @param   imageData Is set to true if the packet contains an
    *                      image, and false if it contains a
    *                      GfxExportFeatureMap.
    */
   GfxFeatureMapImageReplyPacket(uint32 size,
                                 const GfxFeatureMapImageRequestPacket* p,
                                 bool imageData);
       
   
   /**
    *    Returns true if the packet contains an image, and false
    *    if it contains a GfxExportFeatureMap.
    */
   bool isImageData() const;
   

   /**
    *    Returns the Image data.
    */   
   byte* getImageData() const;
   

   /**
    *    Sets the Image data.
    */
   void setImageData(uint32 size, byte* data);
   

   /**
    *    Returns the GfxExportFeatureMap data. Can also be used to get 
    *    GfxExportScalableMapData.
    */   
   byte* getGfxExportFeatureMapData() const;
   
   /**
    *    Sets the GfxExportFeatureMap data. Should not be used when setting
    *    GfxExportScalableFeatureMapData.
    */
   void setGfxExportFeatureMapData(uint32 size, DataBuffer* data);
  
   /**
    *    Returns the GfxExportScalableFeatureMap data. Returns NULL if
    *    the package does not contain this kind of map data.
    */   
   byte* getGfxExportScalableFeatureMapData() const;

   /**
    *    Sets the GfxExportFeatureMap data and puts the bits in the header 
    *    indicating that the package contains ExportScalableFeatureMapData. 
    */ 
   void setGfxExportScalableFeatureMapData(uint32 size, DataBuffer* data);
  
   /**
    *    Returns the size of the Image/GfxExportFeatureMap data.
    */ 
   uint32 getSize() const;


   /**
    * The positions of the things with static locations in the packet.
    */
   enum positions {
      imageData_POS = REPLY_HEADER_SIZE,
      size_POS = imageData_POS + 1,
      data_POS = size_POS + 4
   };

};

#endif







