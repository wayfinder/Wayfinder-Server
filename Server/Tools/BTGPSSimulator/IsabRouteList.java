import java.io.DataInputStream;
import java.io.EOFException;
import java.io.IOException;
import java.util.Vector;

// First an uint16 action

class IsabRouteList {

   /**
    * @param scale must not be 0
    */
   public double toCoord(int val, int origo, double scale) {
      return (double) (val + origo) / scale * 180 / Math.PI;
   }

   public boolean readElements(DataInputStream datastream, Vector coords) {
      /* read the navigatorroute.bin format. This is NOT a
         NavigatorServer protocol v10 packet but an older
         format. See the following code in MC2:

         Server/Servers/src/HttpNavigatorFunctions.cpp
         HttpNavigatorFunctions::htmlNavigatorRoute()
         
          Server/Servers/src/isabBoxRouteMessage.cpp
              isabBoxRouteReply::convertToBytes(...)
          
          Server/Servers/src/isabBoxNavMessage.cpp
              isabBoxNavMessageUtil::convertHeaderToBytes(...)

          The code assumes protoVer >= 0x08
       */

      // Reads an element and adds coord to coords
      int lastX = 0;
      int lastY = 0;
      int y = 0;
      int x = 0;
      int lastSpeed = 0;
      int origoLon = 0;
      int origoLat = 0;
      double scaleX = 0.0;
      double scaleY = 6378137.0; // EARTH_RADIUS
      double heading = 0.0;
      int count = 0;

      // If STX,length,protover then...
      // Read 6 bytes and check for packet header
      int nbrBytes = 0;
      try {
         int stx = datastream.readByte();
         nbrBytes += 1;
         int len = datastream.readInt();
         nbrBytes += 4;
         int protoVer = datastream.readByte();
         nbrBytes += 1;

         if (stx == 0x02 && protoVer >= 0x7 && protoVer < 0xf) {
            log("IsabRouteList.readElements() Looks like packet header.");
            // Read header
            // type 2
            datastream.readShort();
            nbrBytes += 2;
            // req_id 1
            datastream.readByte();
            nbrBytes += 1;
            // crc 4
            datastream.readInt();
            nbrBytes += 4;
            // status_code 1
            datastream.readByte();
            nbrBytes += 1;

            // alignShort - not needed, the above is fixed length!
            /*
            if ( (nbrBytes & 0x01) != 0 ) {
               datastream.readByte();
               nbrBytes++;
            }
             */

            // end of header, start of route reply
            // Route id
            datastream.readInt();
            nbrBytes += 4;

            // route creation time
            datastream.readInt();
            nbrBytes += 4;

            // Route bbox
            // this is not usable, it is always
            // 314159265 0 314159265 0 
            datastream.readInt();
            nbrBytes += 4;
            datastream.readInt();
            nbrBytes += 4;
            datastream.readInt();
            nbrBytes += 4;
            datastream.readInt();
            nbrBytes += 4;

            // distance left to goal
            datastream.readInt();
            nbrBytes += 4;
            // dist2nextWPTFromTrunk
            datastream.readInt();
            nbrBytes += 4;
            // phoneHomeDist
            datastream.readInt();
            nbrBytes += 4;

            // alignShort - not needed, the above is fixed length!
            /*
            if ( (nbrBytes & 0x01) != 0 ) {
               datastream.readByte();
               nbrBytes++;
            }
             */

            // Strings size
            int charSize = datastream.readShort();
            nbrBytes += 2;
            // The strings
            datastream.skipBytes(charSize);
            nbrBytes += charSize;

            // alignShort - needed, the strings may be uneven number of bytes
            if ((nbrBytes & 0x01) != 0) {
               datastream.readByte();
               nbrBytes++;
            }
         } else {
            // Skip to even 12 bytes (6 bytes)
            datastream.readShort();
            datastream.readInt();
         }
      } catch (EOFException eofe) {
         System.err.println("IsabRouteList.readElements() Read eof when reading header!");
         return false;
      } catch (IOException ioe) {
         System.err.println("IsabRouteList.readElements() Read header error! Load failed");
         System.err.println("IsabRouteList.readElements() Caught: " + ioe);

         return false;
      }

      while (count >= 0) {
         try {

            int action = datastream.readUnsignedShort();
            log("Action 0x" + Integer.toHexString(action));

            if (action == 0x8000) {
               // ORIGO                              = 0x8000,
               datastream.readShort(); // nextOrigo
               origoLon = datastream.readInt();
               origoLat = datastream.readInt();
               log("Read Origo " + origoLon + ", " + origoLat);
            } else if (action == 0x8001) {
               // SCALE               = 0x8001,
               datastream.readShort(); // refLon (0)
               datastream.readShort(); // refLat (0)
               int restPart = datastream.readShort(); // restPart
               int integerPart = datastream.readInt(); // integerPart
               scaleX = integerPart + (double) (restPart) / 65536;
               log("Read Scale " + scaleX + " = "
                   + integerPart + "." + restPart);
            } else if (action == 0x8002) {
               //  nav_meta_point_mini_delta_points   = 0x8002,
               int x1 = datastream.readShort();
               int y1 = datastream.readShort();
               int x2 = datastream.readShort();
               int y2 = datastream.readShort();
               int speed1 = datastream.readUnsignedByte();
               int speed2 = datastream.readUnsignedByte();
               log("Read Mini " + x1 + "," + y1 + "@" + speed1
                   + " " + x2 + "," + y2 + "@" + speed2);
               heading = Math.atan2(x1 - lastX, y1 - lastY) * 180 / Math.PI;
               coords.add(new IsabRouteElement(toCoord(y1, origoLat, scaleY),
                     toCoord(x1, origoLon, scaleX), heading, speed1 / 3.6));
               x = x2;
               y = y2;
               lastSpeed = speed2;
               heading = Math.atan2(x2 - x1, y2 - y1) * 180 / Math.PI;
               coords.add(new IsabRouteElement(toCoord(y2, origoLat, scaleY),
                     toCoord(x2, origoLon, scaleX), heading, speed2 / 3.6));
            } else if (action == 0x8003) {
               // nav_meta_point_micro_delta_points  = 0x8003,
               int x1 = datastream.readByte();
               int y1 = datastream.readByte();
               int x2 = datastream.readByte();
               int y2 = datastream.readByte();
               int x3 = datastream.readByte();
               int y3 = datastream.readByte();
               int x4 = datastream.readByte();
               int y4 = datastream.readByte();
               int x5 = datastream.readByte();
               int y5 = datastream.readByte();
               x = lastX + x1;
               y = lastY + y1;
               heading = Math.atan2(x - lastX, y - lastY) * 180 / Math.PI;
               coords.add(new IsabRouteElement(toCoord(y, origoLat, scaleY),
                     toCoord(x, origoLon, scaleX), heading, lastSpeed / 3.6));
               lastX = x;
               lastY = y;
               x = lastX + x2;
               y = lastY + y2;
               heading = Math.atan2(x - lastX, y - lastY) * 180 / Math.PI;
               coords.add(new IsabRouteElement(toCoord(y, origoLat, scaleY),
                     toCoord(x, origoLon, scaleX), heading, lastSpeed / 3.6));
               lastX = x;
               lastY = y;
               x = lastX + x3;
               y = lastY + y3;
               heading = Math.atan2(x - lastX, y - lastY) * 180 / Math.PI;
               coords.add(new IsabRouteElement(toCoord(y, origoLat, scaleY),
                     toCoord(x, origoLon, scaleX), heading, lastSpeed / 3.6));
               lastX = x;
               lastY = y;
               if (x4 != 0 || y4 != 0) {
                  x = lastX + x4;
                  y = lastY + y4;
                  heading = Math.atan2(x - lastX, y - lastY) * 180 / Math.PI;
                  coords.add(new IsabRouteElement(toCoord(y, origoLat, scaleY),
                        toCoord(x, origoLon, scaleX),
                        heading, lastSpeed / 3.6));
                  lastX = x;
                  lastY = y;
               }
               if (x5 != 0 || y5 != 0) {
                  x = lastX + x5;
                  y = lastY + y5;
                  heading = Math.atan2(x - lastX, y - lastY) * 180 / Math.PI;
                  coords.add(new IsabRouteElement(toCoord(y, origoLat, scaleY),
                     toCoord(x, origoLon, scaleX), heading, lastSpeed / 3.6));
                  lastX = x;
                  lastY = y;
               }
               log("Read Micro "
                   + x1 + "," + y1 + " " 
                   + x2 + "," + y2 + " " 
                   + x3 + "," + y2 + " " 
                   + x4 + "," + y4 + " " 
                   + x5 + "," + y5 + " " );
            } else if ((action & 0x8000) == 0      // nav_route_point
                       && (action & 0x3ff) != 1) { 
               /*
                * we skip nav_route_point_start (0x0001) because it
                * comes before the origin and scale datums and thus
                * real lat/lon cannot be calculated.
                *
                * there is no reason to skip WPT with type > 23 (0x17)
                * as was done in the old code.
                *
                * if we encounter the end of the route (0x0000), we return.
                */
               datastream.readByte(); // flags

               int speed = datastream.readUnsignedByte(); // speedLimit
               /*
                * the map component does not handle speed == 0. But
                * the speed is always 0 in the end of route
                * nav_route_point. According to spec this also means
                * "no speed restriction" but we don't allow this to
                * reach customers.
                */
               if (speed > 0) {
                   lastSpeed = speed;
               }
               else {
                   if (lastSpeed <= 0) {
                       lastSpeed = 30; // arbitrary default
                   }
               }
               x = datastream.readShort(); // lon
               y = datastream.readShort(); // lat
               datastream.readShort(); // meters
               datastream.readShort(); // nameIndex
               heading = Math.atan2(x - lastX, y - lastY) * 180 / Math.PI;
               log("Read WPT x " + x + " y " + y 
                   + " speed " + lastSpeed
                   + " heading " + heading
                   + " lon "  + toCoord( x, origoLon, scaleX )
                   + " lat " + toCoord( y, origoLat, scaleY ));
               coords.add(new IsabRouteElement(toCoord(y, origoLat, scaleY),
                     toCoord(x, origoLon, scaleX), heading, lastSpeed / 3.6));

               if (action == 0) {
                  // EPT
                  System.err.println("IsabRouteList.readElements() Read EPT! Load done! "
                                     + count + " Elements and "
                                     + coords.size() + " coords");
                  return true;
               }
            } else {
               // Skipp it
               datastream.skipBytes(10);
               log("Skipped action 0x" + Integer.toHexString(action));
            }

            lastX = x;
            lastY = y;

            count++;
         } catch (EOFException eofe) {
            System.err.println("IsabRouteList.readElements() Read eof! Load done! " + count
                  + " Elements and " + coords.size() + " coords");
            return true;
         } catch (IOException ioe) {
            System.err.println("IsabRouteList.readElements() Read error! Load failed");
            System.err.println("IsabRouteList.readElements() Caught: " + ioe);

            return false;
         }
      } // End while true // Hope for EOF!

      return true;
   }

   /*
    * should be in some util class or we should use a real logging
    * framework
    */
   private void log(String s) {
       // System.out.println(s);
   }
}
