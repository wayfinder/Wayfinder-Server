/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ISABROUTEELEMENT_11_H
#define ISABROUTEELEMENT_11_H

#include "DataBuffer.h"

#ifndef THINCLIENT
#  include "RouteElement.h"
#else
#  define VectorElement Object

// include our "RouteElement"
#  include "PdaNavigatorProt.h"
#endif

/**                                                           
 *                                                            
 */                                                           
class IsabRouteElement  {
public:
   
// There is a corresponding enum in PdaNavigatorProt.h
   enum PointType{
      nav_route_point_end       = 0x0000,
      EPT                       = 0x0000,
      nav_route_point_start     = 0x0001,
      SPT                       = 0x0001,
      nav_route_point_ahead     = 0x0002,
      nav_route_point_left      = 0x0003,
      nav_route_point_right     = 0x0004,
      nav_route_point_uturn     = 0x0005,
      nav_route_point_startat   = 0x0006,
      nav_route_point_finally   = 0x0007,
      nav_route_point_enter_rdbt= 0x0008,
      nav_route_point_exit_rdbt = 0x0009,
      nav_route_point_ahead_rdbt= 0x000a,
      nav_route_point_left_rdbt = 0x000b,
      nav_route_point_right_rdbt= 0x000c,
      nav_route_point_exitat    = 0x000d,
      nav_route_point_on        = 0x000e,
      nav_route_point_park_car  = 0x000f,
      nav_route_point_keep_left = 0x0010,
      nav_route_point_keep_right= 0x0011,
      nav_route_point_start_with_uturn = 0x0012,
      nav_rotue_point_uturn_rdbt  = 0x0013,
      nav_route_point_follow_road = 0x0014,
      nav_route_point_enter_ferry = 0x0015,
      nav_route_point_exit_ferry  = 0x0016,
      nav_route_point_change_ferry= 0x0017,
      
      nav_route_point_delta     = 0x03fe,
      FULL_TPT                  = 0x03fe,
      nav_route_point_max       = 0x03ff,
      nav_meta_point_origo               = 0x8000,
      ORIGO                              = 0x8000,
      nav_meta_point_scale               = 0x8001,
      SCALE               = 0x8001,
      nav_meta_point_mini_delta_points   = 0x8002,
      nav_meta_point_micro_delta_points  = 0x8003,
      nav_meta_point_meta                = 0x8005,
      nav_meta_point_time_dist_left_points=0x8006,
      nav_meta_point_landmark            = 0x8007,
      nav_meta_point_lane_info           = 0x8008,
      nav_meta_point_lane_data           = 0x8009,
      nav_meta_point_signpost            = 0x800a,

      nav_meta_point_max                 = 0x807f
   };
   
   enum ObjectType_t {
      IsabRouteType = 0,
      OrigoType,
      ScaleType,
      MiniType,
      MicroDeltaType,
      TPTType,
      WPTType,
      TimeDistLeftType,
      MetaPTType,
      LandmarkPTType,
      LaneInfoPTType,
      LaneDataPTType,
      SignPostPTType,
   };

   /**
    *   Creates a new, empty IsabRouteElement.
    */
   IsabRouteElement(ObjectType_t objType);

   /**
    *   Cleans up.
    */
   virtual ~IsabRouteElement();

   /**
    *   Writes the contents of this element to buf at position pos.
    *   @param buf The buf to write to.
    *   @param pos The position in the buffer.
    */
   virtual int write(uint8* buf, int pos) const;

   // might be difference between thinclient and mc2 alignment in
   // readnextshort / readnextlong
   static IsabRouteElement* createFromBuf(DataBuffer* buf);

   /**
    * Checks if the type of this object is the specified.
    * @param cmp The type to check.
    * @return True if correct type.
    */
   bool typeIs(ObjectType_t cmp) const {
      bool result = false;
      if (m_objectType == cmp) {
         result = true;
      } else {
         if (m_objectType == IsabRouteType) {
#        ifdef THINCLIENT
            showMessage(TEXT("warning: type not set for IsabRouteElement"));
#        endif
         }
      }
      return result;
   }
   
  protected:
   // The type of this object.
   ObjectType_t m_objectType;
   
  private:
     /// Private default.
     IsabRouteElement() {};

   /** Dummy variable for stupid compiler for navigator. Alignment-problem
    *  This one will be removed !!! */
   uint16 m_dummy;

   
};


class OrigoElement : public IsabRouteElement {

  public:

   /**
    *  Creates a new Origo Element.
    *  @param origoLon (X) The longitude origo for this projection.
    *  @param origoLat (Y) The latitude origo for this projection.
    *  @param nextOrigo The position of the next origo in this array.
    */
   OrigoElement(int32 origoLon,
                int32 origoLat,
                int16 nextOrigo);

   /**
    * Constructor.
    * Creates from databuffer.
    * Reverse of write, except for action field.
    * @param buf The buffer to read from.
    */
   OrigoElement(DataBuffer* buf);
   
   /**
    *  Destruct.
    */ 
   
   virtual ~OrigoElement();

   /**
    *   Writes the contents of this element to buf at position pos.
    *   @param buf The buf to write to.
    *   @param pos The position in the buffer.
    */   
   int write(uint8* buf, int pos) const;

   /**
    *  Sets the next origo for this element.
    *  @param pos The next origo position.
    */   
   void setNextOrigo(uint16 pos);

   /**
    * Get the longitude.
    * @return The longitude.
    */
   int32 getLon() const {
      return m_origoLon;
   }

   /**
    * Get the latitude.
    * @return The latitude.
    */
   int32 getLat() const {
      return m_origoLat;
   }
   
  private:

   /**
    * The origo in longitude (X) direction. This is for the projected "minimap".
    */    
   int32 m_origoLon;

   /**
    * The origo in latitude (Y) direction. This is for the projected "minimap".
    */
   int32 m_origoLat;

   /**
    *  The index to the next origo inte the array.
    */
   int16 m_nextOrigo;
};

class WPTorTPTElement : public IsabRouteElement {

  public:

   /**
    *  A genral trackpoint along the route path.
    *  @param lon Distance in 1/4 meters from current origo.
    *  @param lat Distance in 1/4 meters from current origo.   
    */
   WPTorTPTElement( ObjectType_t objType,
                    int16 lon,
                    int16 lat,
                    uint8 flags,
                    uint16 meters,
                    uint8 speedlimit,
                    uint16 nameIndex );
   

   /**
    *  A general trackpoint along the route path.
    *  @param lon Distance in 1/4 meters from current origo.
    *  @param lat Distance in 1/4 meters from current origo.   
    */
   WPTorTPTElement(ObjectType_t objType);
   

   /**
    *  Destruct.
    */
   virtual ~WPTorTPTElement();   


   /**
    * Gets the distance, in meters.
    * 32 bits for thinclient, 16 bits in server and navigator!
    * @return The distance, in meters.
    */
   uint32 getDistance() const {
      return m_meters;
   }

   /**
    * Sets the distance, in meters.
    * 32 bits for thinclient, 16 bits in server and navigator!
    * @param meters The distance, in meters.
    */
   void setDistance(uint32 meters) {
      m_meters=meters;
   }

   /**
    * Returns true if highway bit is set.
    * @return True if highway bit is set.
    */
   bool isHighway() const 
      {
         return m_flags & 1; // first bit is highway bit
      }

   /**
    * Returns all the flags.
    * @return All the flags.
    */
   uint8 getFlags() const 
      {
         return m_flags; // all of them
      }

   /**
    * Returns the speed limit.
    * @return The speed limit.
    */
   uint8 getSpeedLimit() const 
      {
         return m_speedLimit;
      }

   /**
    * Returns the lon (x) value.
    * @return The lon value.
    */
   int16 getLon() const 
      {
         return m_lon;
      }

   /**
    * Returns the lat (y) value.
    * @return The lat value.
    */
   int16 getLat() const
      {
         return m_lat;
      }   
   
   uint16 getTextIndex() const {
      return m_nameIndex;
   }


   /**
    * Set the name index.
    *
    * @param nameIndex The index to set.
    */
   void setNameIndex( uint16 nameIndex );


   /**
    *  Set the latitude.
    */
   void setLat( int16 lat ) {
      m_lat = lat;
   }

   /**
    *  Set the longitude.
    */
   void setLon( int16 lon ) {
      m_lon = lon;
   }
   
  protected:   
   /**
    * Distance in 1/4 meters from current origo.
    */
   int16 m_lon;

   /**
    * Distance in 1/4 meters from current origo.
    */
   int16 m_lat;

   /**
    *  The special flags on this segment. These include 
    *  autobahn etc.
    */
   uint8 m_flags;
   
   /**
    *  The speedLimit on this segment.
    */
   uint8 m_speedLimit;
   
   /**
    *  Meters left to next wpt or tpt.
    * 32 bits for thinclient, 16 bits in server and navigator!
    */
   uint32 m_meters;
      
   /**
    *  Index in NavStringTable.
    */
   uint16 m_nameIndex;
};

class WPTElement : public WPTorTPTElement {

  public:

   /**
    *    Creates a new WPT element.
    *    
    *    @param speedLimit    The speedlimit for this segment.
    *    @param lon           The position in 1/4 meters from local origo.
    *    @param lat           The position in 1/4 meters from local origo.
    *    @param nameIndex     Index in NavStingTable.
    *    @param crossingKind  The type of crossing.
    */
   
   WPTElement(uint16 type,
              uint8 speedLimit,
              int16 lon,
              int16 lat,
	      uint8 flags, 
	      uint16 meters,
              uint16 nameIndex,
              uint8 exitCount,
              RouteElement::navcrossingkind_t crossingKind);
   /**
    *  Destruct.
    */
   virtual ~WPTElement();


   /**
    *   Writes the contents of this element to buf at position pos.
    *   @param buf The buf to write to.
    *   @param pos The position in the buffer.
    */
   virtual int write(uint8* buf, int pos) const;

   /**
    * Constructor.
    * Creates from databuffer.
    * Reverse of write, except for action field.
    * @param buf The buffer to read from.
    * @param pointDesc The action and other bits.
    */
   WPTElement(DataBuffer* buf, uint16 pointDesc);

   uint16 getAction() const {
      return m_type;
   }

   RouteElement::navcrossingkind_t getCrossingType() const {
      return m_crossingKind;
   }

   uint8 getExitCount() const {
      return m_exitCount;
   }

   void setNewType(uint16 type)
      { m_type = type; }

   void setNewExitCount(uint8 exitCount)
      { m_exitCount = exitCount; }
   
      
   
   
  private:

   /**
    *  The type of this WPT. ahead, left, right....
    */
   uint16 m_type;

   /**
    *  Index in NavStingTable.
    */
   uint8 m_exitCount;

   /**
    *    The type of crossing.
    */
   RouteElement::navcrossingkind_t m_crossingKind;
   
};

class TPTElement : public WPTorTPTElement {

  public:

   /**
    *  A trackpoint along the route path.
    *  @param lon Distance in 1/4 meters from current origo.
    *  @param lat Distance in 1/4 meters from current origo.   
    */
   TPTElement(int16 lon,
              int16 lat,
	      uint8 flags,
	      uint16 meters,
              uint8 speedlimit);
   

   /**
    *  Destruct.
    */
   virtual ~TPTElement();   

      /**
    *   Writes the contents of this element to buf at position pos.
    *   @param buf The buf to write to.
    *   @param pos The position in the buffer.
    */
   virtual int write(uint8* buf, int pos) const;
   
   /**
    * Constructor.
    * Creates from databuffer.
    * Reverse of write, except for action field.
    * @param buf The buffer to read from.
    */
   TPTElement(DataBuffer* buf);

  protected:   
};


class ScaleElement : public IsabRouteElement {

  public:

   /**
    * Creates a ScaelElement
    * @param integerPart The integer part of cos(lat).
    * @param restPart The restPart of the cos(lat) calculation.
    */
   ScaleElement(uint32 integerPart,
                uint32 restPart,
                uint16 refLat, // y
                uint16 refLon); // x

   /**
    *  Destruct.
    */
   
   virtual ~ScaleElement();

   /**
    *   Writes the contents of this element to buf at position pos.
    *   @param buf The buf to write to.
    *   @param pos The position in the buffer.
    */
   virtual int write(uint8* buf, int pos) const;
   
   /**
    * Constructor.
    * Creates from databuffer.
    * Reverse of write, except for action field.
    * @param buf The buffer to read from.
    */
   ScaleElement(DataBuffer* buf);

   uint32 getIntegerPart() const {
      return m_integerPart;
   }

   uint32 getRestPart() const {
      return m_restPart;
   }

   uint16 getRefLon() const {
      return m_refLon; // x
   }

   uint16 getRefLat() const {
      return m_refLat; // y
   }

  protected:

   uint32 m_integerPart;

   uint32 m_restPart;

   uint16 m_refLon;

   uint16 m_refLat;
};


class MiniElement : public IsabRouteElement {

  public:

   /**
    *  Creates a MiniElement
    */
   MiniElement();
   
   /**
    * Creates a MiniElement
    * @param lon1 The lon part in first point.
    * @param lat1 The lat part in first point.
    * @param lon2 The lon part in second point.
    * @param lat2 The lat part in second point.
    */
   MiniElement(int16 lon1,
               int16 lat1,
               int16 lon2,
               int16 lat2,
               uint8 speedLimit1,
               uint8 speedLimit2);
  
   /**
    *  Destruct.
    */
   
   virtual ~MiniElement();

   /**
    *   Writes the contents of this element to buf at position pos.
    *   @param buf The buf to write to.
    *   @param pos The position in the buffer.
    */
   virtual int write(uint8* buf, int pos) const;

   /**
    * Constructor.
    * Creates from databuffer.
    * Reverse of write, except for action field.
    * @param buf The buffer to read from.
    */
   MiniElement(DataBuffer* buf);

   /**
    * Set the lon1 and lat1 inpoint.
    * @param lon1 The 1:st point.
    * @param lat1 The 1:st point.
    */
   void set1(int16 lon, int16 lat);
   
   /**
    * Set the speedLimit from lon1 and lat1.
    * @param speedLimit The speedLimit from this point.
    */
   void setSpeedLimit1(uint8 speedLimit);
   
   /**
    * Set the lon2 and lat2 in point.
    * @param lon2 The 2:nd point.
    * @param lat2 The 2:nd point.
    */
   void set2(int16 lon, int16 lat);

   int16 getLat1() const {return m_lat1;}
   int16 getLat2() const {return m_lat2;}
   int16 getLon1() const {return m_lon1;}
   int16 getLon2() const {return m_lon2;}
   uint8 getSpeed1() const {return m_speedLimit1;}
   uint8 getSpeed2() const {return m_speedLimit2;}

   /**
    * Set the speedLimit from lon2 and lat2.
    * @param speedLimit The speedLimit from this point.
    */
   void setSpeedLimit2(uint8 speedLimit);
    
  protected:

   int16 m_lon1;

   int16 m_lat1;
   
   int16 m_lon2;

   int16 m_lat2;

   uint8 m_speedLimit1;

   uint8 m_speedLimit2;   
};


class MicroDeltaElement : public IsabRouteElement {

  public:

   /**
    *  Creates a MiniDeltaElement
    */   
   MicroDeltaElement();
   
   /**
    * Creates a MicroDeltaElement
    * @param x1 The x part in first delta point.
    * @param y1 The y part in first delta point.
    * @param x2 The y part in second delta point.
    * @param y2 The y part in second delta point.
    * @param x3 The y part in 3:d delta point.
    * @param y3 The y part in 3:d delta point.
    * @param x4 The y part in 4:th delta point.
    * @param y4 The y part in 4:th delta point.
    * @param x5 The y part in 5:th delta point.
    * @param y5 The y part in 5:th delta point.
    */
   MicroDeltaElement(int8  x1,
                     int8  y1,
                     int8  x2,
                     int8  y2,
                     int8  x3,
                     int8  y3,
                     int8  x4,
                     int8  y4,
                     int8  x5,
                     int8  y5);
   
   /**
    *  Destruct.
    */
   
   virtual ~MicroDeltaElement();

   /**
    *   Writes the contents of this element to buf at position pos.
    *   @param buf The buf to write to.
    *   @param pos The position in the buffer.
    */
   virtual int write(uint8* buf, int pos) const;

   /**
    * Constructor.
    * Creates from databuffer.
    * Reverse of write, except for action field.
    * @param buf The buffer to read from.
    */
   MicroDeltaElement(DataBuffer* buf);

   /**
    * Set the x1 and y1 in deltapoint.
    * @param x1 The 1:st delta.
    * @param y1 The 1:st delta.
    */
   void setDelta1(int8 x, int8 y);
   
   /**
    * Set the x2 and y2 in deltapoint.
    * @param x2 The 2:st delta.
    * @param y2 The 2:st delta.
    */
   void setDelta2(int8 x, int8 y);

   /**
    * Set the x2 and y2 in deltapoint.
    * @param x2 The 2:st delta.
    * @param y2 The 2:st delta.
    */
   void setDelta3(int8 x, int8 y);

   /**
    * Set the x2 and y2 in deltapoint.
    * @param x2 The 2:st delta.
    * @param y2 The 2:st delta.
    */
   void setDelta4(int8 x, int8 y);

   /**
    * Set the x2 and y2 in deltapoint.
    * @param x2 The 2:st delta.
    * @param y2 The 2:st delta.
    */
   void setDelta5(int8 x, int8 y);

   int8 getLat1() const { return m_y1; }
   int8 getLon1() const { return m_x1; }
   int8 getLat2() const { return m_y2; }
   int8 getLon2() const { return m_x2; }
   int8 getLat3() const { return m_y3; }
   int8 getLon3() const { return m_x3; }
   int8 getLat4() const { return m_y4; }
   int8 getLon4() const { return m_x4; }
   int8 getLat5() const { return m_y5; }
   int8 getLon5() const { return m_x5; }
   
  protected:

   int8 m_x1;
   int8 m_y1;
   int8 m_x2;
   int8 m_y2;
   int8 m_x3;
   int8 m_y3;
   int8 m_x4;
   int8 m_y4;
   int8 m_x5;
   int8 m_y5;
};


/**
 * A meto point telling the total time, estimated, and distance left to goal.
 * Only present for protover >= 0x05.
 *
 */
class TimeDistLeftElement : public IsabRouteElement {
   public:
      /**
       * Creates a TimeDistLeftElement.
       *
       * @param timeLeft The time left.
       * @param distLeft The distance left.
       */
      TimeDistLeftElement( uint32 timeLeft, uint32 distLeft );


      /**
       * Constructor.
       * Creates from databuffer.
       * Reverse of write, except for action field.
       *
       * @param buf The buffer to read from.
       */
      TimeDistLeftElement( DataBuffer* buf );


      /**
       *  Destruct.
       */
      virtual ~TimeDistLeftElement();


      /**
       * Writes the contents of this element to buf at position pos.
       *
       * @param buf The buf to write to.
       * @param pos The position in the buffer.
       */
      virtual int write( uint8* buf, int pos ) const;

      
      /**
       * Get the estimated time left.
       *
       * @return The estimated time left.
       */
      uint32 getTimeLeft() const;


      /**
       * Set the estimated time left.
       *
       * @param time The estimated time left.
       */
      void setTimeLeft( uint32 time);

      
      /**
       * Get the distance left.
       *
       * @return The distance left.
       */
      uint32 getDistLeft() const;


      /**
       * Returns the speed limit.
       * NB! Only used for calculations.
       *
       * @return The speed limit.
       */
      uint8 getSpeed() const {
         return m_speedLimit;
      }

      /**
       * Stss the speed limit.
       * NB! Only used for calculations.
       *
       * @param speed The speed limit.
       */
      void setSpeed( uint8 speed ) {
         m_speedLimit = speed;
      }

   protected:
      /**
       * The time left to goal.
       */
      uint32 m_timeLeft;

      /**
       * The distance left to goal.
       */
      uint32 m_distLeft;

      /**
       * The speed limit, NB! Only used for calculations.
       */
      uint8 m_speedLimit;
};



/**
 * A meta point.
 * Only present for protover >= 0x07.
 * Meta point 0x8005 1000000000000101 - Meta point, uint16 type + 4 x uint16
 *The meta point can be used for any kind of values which need data in 16-bit units. uint16 type; uint16 a; uint16 b; uint16 c; uint16 d; 

The following types are defined:   

   0x1 Additional Text Information This datum may be appended once or several times after a wpt+tde combo. Additional text information to be displayed between that point in the route and the next crossing (the one the waypoint describes). The first uint16 is a flag field telling if to pop up a box or just present the text. The second uint16 is an index into the street name buffer, the following two are taken together to form an uint32 that is the distance before the crossing the waypoint refers to. The distance is set to zero for signs at the turning point.

   0x2 Additional Sound Information Additional sound to be played at this point More detailed sound information, Half way reached, Commercial etc.

   0x3 Additional Trackpoint Information Additional information needed for next trackpoint. Currently the first uint16 holds an extended flags section. Only two extended flag is defined to describe left-hand/right-hand side driving. The two least significant bits are 0=no change, 1=change to left-hand, 2=change to right-hand driving and 3=unused.

   0x4 Segmented Route And End Is Near Advise that new phonecall is made to Navigator Server to download next part of a segmented route. If a route is too large to transfer to the Navigator in one piece, the route can be sent in a number of smaller segments with automatic routing at the end of the segment.

   0x5 Report Point Reached Instructs the Navigator to initiate contact with server to report when the point is passed. Can be used for vehicle tracking, automatic road pricing etc.
 *
 */
class MetaPTElement : public IsabRouteElement {
   public:
      /**
       * Types of meta points.
       */
      enum metapoint_t {
         /// Additional Text Information
         additional_text = 0x1,
         /// Additional Sound Information
         additional_sound = 0x2,
         /// Additional Trackpoint Information
         additional_trackpoint = 0x3,
         /// Segmented Route And End Is Near Advise
         segmented_route_end_is_near = 0x4,
         /// Report Point Reached
         report_point_reached = 0x5
      };


      /**
       * Create a MetaPTElement.
       *
       * @param a The a data.
       * @param b The b data.
       * @param c The c data.
       * @param d The d data.
       */
      MetaPTElement( metapoint_t type, 
                     uint16 a, uint16 b, uint16 c, uint16 d );


      /**
       * Constructor.
       * Creates from databuffer.
       * Reverse of write, except for action field.
       *
       * @param buf The buffer to read from.
       */
      MetaPTElement( DataBuffer* buf );


      /**
       *  Destruct.
       */
      virtual ~MetaPTElement();


      /**
       * Writes the contents of this element to buf at position pos.
       *
       * @param buf The buf to write to.
       * @param pos The position in the buffer.
       */
      virtual int write( uint8* buf, int pos ) const;


      /**
       * Get the type of meta point.
       */
      metapoint_t getType() const;


      /**
       * Get the a data.
       */
      uint16 getDataA() const;


      /**
       * Get the b data.
       */
      uint16 getDataB() const;


      /**
       * Get the c data.
       */
      uint16 getDataC() const;


      /**
       * Get the D data.
       */
      uint16 getDataD() const;


      /**
       * Set the a data.
       *
       * @param value The new value.
       */
      void setDataA( uint16 value );
      

      /**
       * Set the b data.
       *
       * @param value The new value.
       */
      void setDataB( uint16 value );
      

      /**
       * Set the C data.
       *
       * @param value The new value.
       */
      void setDataC( uint16 value );
      

      /**
       * Set the d data.
       *
       * @param value The new value.
       */
      void setDataD( uint16 value );
      

   protected:
      /**
       * The type of meta point.
       */
      metapoint_t m_type;


      /**
       * The a data.
       */
      uint16 m_dataa;


      /**
       * The b data.
       */
      uint16 m_datab;


      /**
       * The c data.
       */
      uint16 m_datac;


      /**
       * The d data.
       */
      uint16 m_datad;
};

class LandmarkPTElement : public IsabRouteElement {
   public:
      /**
       * Create a LandmarkPTElement.
       *
       * @param detour.
       * @param lm_side.
       * @param lm_type.
       * @param distance.
       * @param lm_location.
       * @param isStart.
       * @param isStop.
       * @param landmarkID.
       */
      LandmarkPTElement(bool detour,
                        uint8 lm_side,
                        uint8 lm_type,
                        uint8 lm_location,
                        int32 distance,
                        uint16 strNbr,
                        bool isStart,
                        bool isStop,
                        int16 landmarkID,
                        uint32 distID = MAX_UINT32);


      /**
       * Constructor.
       * Creates from databuffer.
       * Reverse of write, except for action field.
       *
       * @param buf The buffer to read from.
       */
      LandmarkPTElement( DataBuffer* buf );


      /**
       *  Destruct.
       */
      virtual ~LandmarkPTElement();


      /**
       * Writes the contents of this element to buf at position pos.
       *
       * @param buf The buf to write to.
       * @param pos The position in the buffer.
       */
      virtual int write( uint8* buf, int pos ) const;
      
      uint32 getLandmarkRouteID() const;

      uint32 getLandmarkServerID() const;

      /**
       * Set the isStart.
       */
      void setisStart( bool on );

      /**
       * Set the isEnd.
       */
      void setisEnd( bool on );

      /**
       * Get the isStart.
       */
      bool getisStart() const;

      /**
       * Get the isEnd.
       */
      bool getisEnd() const;

      /**
       * Set the isSetour.
       */
      void setisDetour( bool on );

      /**
       * Get the isDetour.
       */
      bool getisDetour() const;
 

      /**
       * Set the distance.
       */
      void setDistance( int32 distance );

      /**
       * Get the distance.
       */
      int32 getDistance() const;

      /**
       * Get the strNameIdx.
       */
      uint16 getStrNameIdx() const;

   protected:
      /**
       * The a data.
       */
      uint16 m_flags;


      /**
       * The b data.
       */
      int32 m_distance;


      /**
       * The c data.
       */
      uint16 m_strNameIdx;


      /**
       * The d data.
       */
      uint16 m_idStartStop;

      /**
       *   The id of the distrubance.
       */
      uint32 m_distID; 
      
};

/**
 * A start of lane data, with some lanes.
 *
 */
class LaneInfoPTElement : public IsabRouteElement {
   public:
      /**
       * Create a LaneInfoPTElement.
       *
       * @param stopOfLanesFlag If this is a end of lanes.
       * @param reminderOfLanesFlag If this is a reminder of lanes.
       * @param nbrLanes The total number of lanes in this and following 
       *                 lane datas.
       * @param lane0 A lane.
       * @param lane1 A lane.
       * @param lane2 A lane.
       * @param lane3 A lane.
       * @param distance Distance from the lanes to the next crossing.
       */
      LaneInfoPTElement( bool stopOfLanesFlag,
                         bool reminderOfLanesFlag,
                         uint8 nbrLanes,
                         uint8 lane0,
                         uint8 lane1,
                         uint8 lane2,
                         uint8 lane3,
                         int32 distance );

      /**
       * Constructor.
       * Creates from databuffer.
       * Reverse of write, except for action field.
       *
       * @param buf The buffer to read from.
       */
      LaneInfoPTElement( DataBuffer* buf );

      /**
       *  Destructor.
       */
      virtual ~LaneInfoPTElement();

      /**
       * Writes the contents of this element to buf at position pos.
       *
       * @param buf The buf to write to.
       * @param pos The position in the buffer.
       */
      virtual int write( uint8* buf, int pos ) const;

      /**
       * Get the stopOfLanesFlag.
       */
      bool getStopOfLanesFlag() const;

      /**
       * Get the reminderOfLanesFlag.
       */
      bool getReminderOfLanesFlag() const;

      /**
       * Get the nbrLanes.
       */
      uint8 getNbrLanes() const;

      /**
       * Get the lane 0.
       */
      uint8 getLane0() const;

      /**
       * Get the lane 1.
       */
      uint8 getLane1() const;

      /**
       * Get the lane 2.
       */
      uint8 getLane2() const;

      /**
       * Get the lane 3.
       */
      uint8 getLane3() const;

      /**
       * Get the distance.
       */
      int32 getDistance() const;

   protected:
      /**
       * Bit-mapped flags
       */
      uint8 m_flags; 

      /**
       * The number of lanes.
       */
      uint8 m_nbrLanes;

      /// A lane.
      uint8 m_lane0;

      /// A lane.
      uint8 m_lane1;

      /// A lane.
      uint8 m_lane2;

      /// A lane.
      uint8 m_lane3;

      /**
       * Distance from the lanes to the next crossing. 
       */
      int32 m_distance;
};

/**
 * Lane data, 10 lanes. Follows a Lane info element.
 *
 */
class LaneDataPTElement : public IsabRouteElement {
   public:
      /**
       * Create a LaneDataPTElement.
       *
       * @param lane0 A lane.
       * @param lane1 A lane.
       * @param lane2 A lane.
       * @param lane3 A lane.
       * @param lane4 A lane.
       * @param lane5 A lane.
       * @param lane6 A lane.
       * @param lane7 A lane.
       * @param lane8 A lane.
       * @param lane9 A lane.
       */
      LaneDataPTElement( uint8 lane0,
                         uint8 lane1,
                         uint8 lane2,
                         uint8 lane3,
                         uint8 lane4,
                         uint8 lane5,
                         uint8 lane6,
                         uint8 lane7,
                         uint8 lane8,
                         uint8 lane9 );

      /**
       * Constructor.
       * Creates from databuffer.
       * Reverse of write, except for action field.
       *
       * @param buf The buffer to read from.
       */
      LaneDataPTElement( DataBuffer* buf );

      /**
       *  Destructor.
       */
      virtual ~LaneDataPTElement();

      /**
       * Writes the contents of this element to buf at position pos.
       *
       * @param buf The buf to write to.
       * @param pos The position in the buffer.
       */
      virtual int write( uint8* buf, int pos ) const;


      /**
       * Get the lane 0.
       */
      uint8 getLane0() const;

      /**
       * Get the lane 1.
       */
      uint8 getLane1() const;

      /**
       * Get the lane 2.
       */
      uint8 getLane2() const;

      /**
       * Get the lane 3.
       */
      uint8 getLane3() const;

      /**
       * Get the lane 4.
       */
      uint8 getLane4() const;

      /**
       * Get the lane 5.
       */
      uint8 getLane5() const;

      /**
       * Get the lane 6.
       */
      uint8 getLane6() const;

      /**
       * Get the lane 7.
       */
      uint8 getLane7() const;

      /**
       * Get the lane 8.
       */
      uint8 getLane8() const;

      /**
       * Get the lane 9.
       */
      uint8 getLane9() const;

   protected:
      /// A lane.
      uint8 m_lane0;

      /// A lane.
      uint8 m_lane1;

      /// A lane.
      uint8 m_lane2;

      /// A lane.
      uint8 m_lane3;

      /// A lane.
      uint8 m_lane4;

      /// A lane.
      uint8 m_lane5;

      /// A lane.
      uint8 m_lane6;

      /// A lane.
      uint8 m_lane7;

      /// A lane.
      uint8 m_lane8;

      /// A lane.
      uint8 m_lane9;
};


/**
 * A signpost.
 *
 */
class SignPostPTElement : public IsabRouteElement {
   public:
      /**
       * Create a SignPostPTElement.
       *
       * @param signPostTextIndex The index in the string table of the text.
       * @param textColor The color of the text.
       * @param backgroundColor The color of the background.
       * @param frontColor The color of the front.
       * @param distance Distance from the sign post to the next crossing.
       */
      SignPostPTElement( uint16 signPostTextIndex,
                         byte textColor,
                         byte backgroundColor,
                         byte frontColor,
                         int32 distance );

      /**
       * Constructor.
       * Creates from databuffer.
       * Reverse of write, except for action field.
       *
       * @param buf The buffer to read from.
       */
      SignPostPTElement( DataBuffer* buf );

      /**
       *  Destructor.
       */
      virtual ~SignPostPTElement();

      /**
       * Writes the contents of this element to buf at position pos.
       *
       * @param buf The buf to write to.
       * @param pos The position in the buffer.
       */
      virtual int write( uint8* buf, int pos ) const;

      /**
       * Get the SignPostTextIndex.
       */
      uint16 getSignPostTextIndex() const;

      /**
       * Get the textColor.
       */
      byte getTextColor() const;

      /**
       * Get the backgroundColor.
       */
      byte getBackgroundColor() const;

      /**
       * Get the frontColor.
       */
      byte getFrontColor() const;

      /**
       * Get the distance.
       */
      int32 getDistance() const;

   protected:
      /**
       * Index into the string table. 
       */
      uint16 m_signPostTextIndex;

      /**
       * The color of the text in the signpost. 
       */
      byte m_textColor;

      /**
       * The color of the background of the signpost. 
       */
      byte m_backgroundColor; 

      /**
       * The color of the front of the signpost. 
       */
      byte m_frontColor; 

      /**
       * Not used right now. 
       */
      byte m_reserved;

      /**
       * Distance from the signpost to the next crossing. 
       */
      int32 m_distance; 
};




// =======================================================================
//                                     Implementation of inlined methods =

inline bool
LaneInfoPTElement::getStopOfLanesFlag() const {
   return (m_flags & 0x1) != 0;
}

inline bool
LaneInfoPTElement::getReminderOfLanesFlag() const {
   return (m_flags & 0x2) != 0;
}

inline uint8
LaneInfoPTElement::getNbrLanes() const {
   return m_nbrLanes;
}

inline uint8
LaneInfoPTElement::getLane0() const {
   return m_lane0;
}

inline uint8
LaneInfoPTElement::getLane1() const {
   return m_lane1;
}

inline uint8
LaneInfoPTElement::getLane2() const {
   return m_lane2;
}

inline uint8
LaneInfoPTElement::getLane3() const {
   return m_lane3;
}

inline int32
LaneInfoPTElement::getDistance() const {
   return m_distance;
}


inline uint8
LaneDataPTElement::getLane0() const {
   return m_lane0;
}

inline uint8
LaneDataPTElement::getLane1() const {
   return m_lane1;
}

inline uint8
LaneDataPTElement::getLane2() const {
   return m_lane2;
}

inline uint8
LaneDataPTElement::getLane3() const {
   return m_lane3;
}

inline uint8
LaneDataPTElement::getLane4() const {
   return m_lane4;
}

inline uint8
LaneDataPTElement::getLane5() const {
   return m_lane5;
}

inline uint8
LaneDataPTElement::getLane6() const {
   return m_lane6;
}

inline uint8
LaneDataPTElement::getLane7() const {
   return m_lane7;
}
inline uint8
LaneDataPTElement::getLane8() const {
   return m_lane8;
}

inline uint8
LaneDataPTElement::getLane9() const {
   return m_lane9;
}

inline uint16
SignPostPTElement::getSignPostTextIndex() const {
   return m_signPostTextIndex;
}

inline byte
SignPostPTElement::getTextColor() const {
   return m_textColor;
}

inline byte
SignPostPTElement::getBackgroundColor() const {
   return m_backgroundColor;
}

inline byte
SignPostPTElement::getFrontColor() const {
   return m_frontColor;
}

inline int32
SignPostPTElement::getDistance() const {
   return m_distance;
}

#endif


