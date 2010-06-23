/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

import java.awt.Component;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.image.ImageObserver;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.StringTokenizer;

public class MercatorMapComponent extends MapComponent {

   private ZoomLevel[] zoomLevels;
   private int nbrZoomLevels;
   private int zoomFactor;
   private ServerComm serverComm;
   private Component toRepaint;

   private ServerComm getServerComm() {
      // Should get this from other place.
      return serverComm;
   }

   //Image[] map = new Image[25];
   //SPMap[] spMap = new SPMap[25];
   MercatorMap[] imMaps = new MercatorMap[25]; // What is this?
   HashMap requests = new HashMap();

   private final double DEGREE_FACT = 11930464.7111;
   private final double RADIAN_FACT = DEGREE_FACT * 180.0 / Math.PI;
   private final double INV_RADIAN_FACT = 1.0 / RADIAN_FACT;

   private class MapRequest extends ServerMercatorRequest {

      private MercatorMap map;

      public MapRequest(MercatorMapParams params, MercatorMap map) {
         super(params);
         this.map = map;
      }

      public void requestDone() {
         map.load(data);
         toRepaint.repaint();
      }
   }

   private class ZoomLevel {
      public int min_x;
      public int min_y;
      public int max_x;
      public int max_y;
      public int nbrSq;
      public int squareWidth;
      public int squareHeight;
      public double radius;
      public int zoomLevelNbr;

      public ZoomLevel(int min_x, int min_y, int max_x, int max_y,
            int squareSize, int zoomLevelNumber) {
         this.min_x = min_x;
         this.min_y = min_y;
         this.max_x = max_x;
         this.max_y = max_y;
         this.squareWidth = squareSize;
         this.squareHeight = squareSize;
         this.nbrSq = (max_x - min_x) / squareWidth + 1;
         this.radius = ((double) (max_x - min_x) + squareWidth) / 2.0 / Math.PI;
         zoomLevelNbr = zoomLevelNumber;
      }
   }

   public MercatorMapComponent(int width, int height, String server,
         SPDesc spDesc, Component toRedraw) {
      toRepaint = toRedraw;
      this.spDesc = spDesc;
      mapServer = server;
      parseZoomLevelXML(); // if this fails we will crash with NPE later
      cPoint.x = 157362825; // Wayfinder mc2_x
      cPoint.y = 664749105; // Wayfinder mc2_y
      //cPoint.x = 448809183;
      //cPoint.y = 665140572;
      screenWidth = width;
      screenHeight = height;
      zoomFactor = 2;
      //setRadius(zoomFactor);
      nwPoint = new Point(xToMc2Lon(0), yToMc2Lat(0));
      sePoint = new Point(xToMc2Lon(screenWidth), yToMc2Lat(screenHeight));
      screenDistance = xToMc2Lon(screenWidth) - xToMc2Lon(0);
      //System.out.println("NW = " + nwPoint.x+","+nwPoint.y);
      //System.out.println("SE = " + sePoint.x+","+sePoint.y);

      // Should be sent in from the outside!
      try {
         serverComm = new XMLServerComm(mapServer,
               12211, new AWTServerEventQueue());
      } catch (IOException e) {
         System.err.println("MercatorMapComponent.MercatorMapComponent(5 args) " + e);
      }
   }

   public MercatorMapComponent(MapComponent oldComp) {
      spDesc = oldComp.spDesc;
      showPois = oldComp.showPois;
      mapServer = oldComp.mapServer;
      parseZoomLevelXML();
      zoomFactor = 2;
      //setRadius(zoomFactor);
      coords = oldComp.coords;
      poiCoords = oldComp.poiCoords;
      poiComments = oldComp.poiComments;
      boxes = oldComp.boxes;
      showTraffic = oldComp.showTraffic;
      carPosition = oldComp.carPosition;
      carDirection = oldComp.carDirection;
      cPoint = oldComp.cPoint;
      xyRatio = oldComp.xyRatio;
      screenHeight = oldComp.screenHeight;
      screenWidth = oldComp.screenWidth;
      nwPoint = new Point(xToMc2Lon(0), yToMc2Lat(0));
      sePoint = new Point(xToMc2Lon(screenWidth), yToMc2Lat(screenHeight));
      screenDistance = xToMc2Lon(screenWidth) - xToMc2Lon(0);

      // Should be sent in from the outside!
      try {
         serverComm = new XMLServerComm(mapServer, 12211,
               new AWTServerEventQueue());
      } catch (IOException e) {
         System.err.println("MercatorMapComponent.MercatorMapComponent(1 arg) " + e);
      }
   }

   private void parseZoomLevelXML() {
      System.out.println("MercatorMapComponent.parseZoomLevelXML() Parsing ZoomLevel XML...");
      // FIXME: Move this code to ServerComm and make it parse the
      // zoomsettings correctly.
      try {
         URL url = new URL("http", mapServer, 12211, "//ZoomSettings");
         System.out.println("MercatorMapComponent.parseZoomLevelXML() using "
               + url);
         HttpURLConnection conn = (HttpURLConnection) url.openConnection();

         BufferedReader br = new BufferedReader(new InputStreamReader(conn
               .getInputStream()));
         // Some servers (head) sends all on zero lines (<xml ... settings_reply>) but I changed that back 20070821 /Dp But head sends crc rel doesn't.
         String line = br.readLine();
         if (line.indexOf("zoom_levels") == -1) {
            line = br.readLine();
         }

         if (line.indexOf("zoom_levels") != -1) {
            line = line.substring(line.indexOf("zoom_levels"));
         }

         StringTokenizer st = new StringTokenizer(line, "\"");
         String token = st.nextToken(); // All before first "
         if (token.indexOf(" crc=") != -1) {
            st.nextToken(); // Skipp crc
            st.nextToken(); // Skipp all before next "
         }
         nbrZoomLevels = Integer.parseInt(st.nextToken());
         zoomLevels = new ZoomLevel[nbrZoomLevels];
         st.nextToken();
         int squareSize = Integer.parseInt(st.nextToken());
         for (int i = 0; i < nbrZoomLevels; i++) {
            line = br.readLine();
            st = new StringTokenizer(line, "\"");
            st.nextToken();
            int max_x = Integer.parseInt(st.nextToken());
            st.nextToken();
            int max_y = Integer.parseInt(st.nextToken());
            st.nextToken();
            int min_x = Integer.parseInt(st.nextToken());
            st.nextToken();
            int min_y = Integer.parseInt(st.nextToken());
            zoomLevels[i] = new ZoomLevel(min_x, min_y, max_x, max_y,
                  squareSize, i + 1);
         }
         br.close();
      } catch (Exception e) {
         System.out.println("MercatorMapComponent.parseZoomLevelXML() Error parsing ZoomLevel XML: " + e.getMessage());
         e.printStackTrace();
         return;
      }

      System.out.println("MercatorMapComponent.parseZoomLevelXML() Done!");
      //for (int i=0; i<nbrZoomLevels; i++)
      // System.out.println(zoomLevels[i].max_x+" "+zoomLevels[i].max_y+" "+zoomLevels[i].min_x+" "+zoomLevels[i].min_y);
   }

   public void zoomIn(int mc2_x, int mc2_y) {
      int fac = zoomFactor;
      if (fac < nbrZoomLevels) {
         fac++;
      }
      zoomIn(mc2_x, mc2_y, fac);
   }

   public void zoomOut(int mc2_x, int mc2_y) {
      cPoint.x = mc2_x;
      cPoint.y = mc2_y;
      if (zoomFactor > 1) {
         zoomFactor--;
         //setRadius(zoomFactor);
         screenDistance = xToMc2Lon(screenWidth) - xToMc2Lon(0);
      }
   }

   public void moveCenter(int mc2_x, int mc2_y) {
      cPoint.x = mc2_x;
      cPoint.y = mc2_y;
   }

   public void zoomToSquare(int x1, int y1, int x2, int y2) {
      int c_x = x1 + x2 / 2;
      int c_y = y1 + y2 / 2;
      int mc2_x = xToMc2Lon(c_x);
      int mc2_y = yToMc2Lat(c_y);
      //System.out.println(x1 + " "+x2);
      int nbrSq = (zoomLevels[zoomFactor - 1].nbrSq / 2) * screenWidth
            / Math.abs(x2);
      //System.out.println("Nbrsq = " + nbrSq);
      int i = 0;
      for (i = 0; i < nbrZoomLevels && zoomLevels[i].nbrSq < nbrSq; i++) {
      }
      int fac = i + 1;
      //System.out.println("ZoomFactor = " + zoomFactor);
      zoomIn(mc2_x, mc2_y, fac);
   }

   private void zoomIn(int mc2_x, int mc2_y, int fac) {
      if (fac <= nbrZoomLevels) {
         zoomFactor = fac;
         //setRadius(zoomFactor);
         screenDistance = xToMc2Lon(screenWidth) - xToMc2Lon(0);
         cPoint.x = mc2_x;
         cPoint.y = mc2_y;
      }
   }

   public void zoomToRoute() {
      if (coords.size() == 1) {
         moveCenter(((Point) coords.get(0)).x, ((Point) coords.get(0)).y);
      }

      else if (coords.size() > 0) {
         int maxX = Integer.MIN_VALUE;
         int minX = Integer.MAX_VALUE;
         int maxY = Integer.MIN_VALUE;
         int minY = Integer.MAX_VALUE;
         for (int i = 0; i < coords.size(); i++) {
            int x = ((Point) coords.get(i)).x;
            int y = ((Point) coords.get(i)).y;
            if (x < minX) {
               minX = x;
            }
            if (x > maxX) {
               maxX = x;
            }
            if (y < minY) {
               minY = y;
            }
            if (y > maxY) {
               maxY = y;
            }
            /**
            if (i%10 == 0) {
               System.out.println("minX = " + minX);
               System.out.println("maxX = " + maxX);
               System.out.println("minY = " + minY);
               System.out.println("maxY = " + maxY + "\n");
            }*/
         }
         // Add border
         int xBorderSize = Math.abs(maxX - minX) / 10;
         int yBorderSize = Math.abs(maxY - minY) / 10;
         maxX += xBorderSize;
         minX -= xBorderSize;
         maxY += yBorderSize;
         minY -= yBorderSize;

         //System.out.println(mc2_min + " "+mc2_max);
         //System.out.println(minX + " " + maxX);
         //System.out.println(Math.abs(mc2_max-mc2_min)/Math.abs(maxX-minX));
         int nbrSq1 = 5 * Math.abs(sePoint.x / Math.abs(maxX - minX)
               - nwPoint.x / Math.abs(maxX - minX));
         int nbrSq2 = 5 * Math.abs(sePoint.y / Math.abs(maxY - minY)
               - nwPoint.y / Math.abs(maxY - minY));
         int nbrSq = Math.min(nbrSq1, nbrSq2);
         //System.out.println("NbrSq = " + nbrSq);
         int i = 0;
         for (i = 0; i < nbrZoomLevels && zoomLevels[i].nbrSq < nbrSq; i++) {
         }
         int fac = i + 2;

         int xc = maxX / 2 + minX / 2;
         int yc = maxY / 2 + minY / 2;

         zoomIn(xc, yc, fac);
      }
   }

   private int getLeftWorldX() {
      return (int) (mc2ToMapX(cPoint.x) - screenWidth / 2);
   }

   private int getTopWorldY() {
      return (int) (mc2ToMapY(cPoint.y) + screenHeight / 2);
   }

   public void loadMap() throws Exception {
      if (showPois)
         resetPoiForLoad();

      int sq_height = zoomLevels[zoomFactor - 1].squareHeight;
      int sq_width = zoomLevels[zoomFactor - 1].squareWidth;
      int top_left_world_x = getLeftWorldX();
      int top_left_world_y = getTopWorldY();
      //System.out.println(top_left_world_x + " " + top_left_world_y);
      if (top_left_world_x < 0)
         top_left_world_x -= zoomLevels[zoomFactor - 1].squareWidth;
      int top_left_rounded_x = (top_left_world_x / sq_width) * sq_width;
      int top_left_rounded_y = (top_left_world_y / sq_height) * sq_height;

      ArrayList list = new ArrayList();
      HashMap oldReqs = requests;
      requests = new HashMap();
      for (int i = 0; i < 2; ++i) {
         // Don't show pois if we shouldn't
         if (!showPois && i == 1) {
            continue;
         }
         for (int x = top_left_rounded_x; x <= top_left_rounded_x + screenWidth
               + sq_width; x += sq_width) {
            for (int y = top_left_rounded_y; y >= top_left_rounded_y
                  - screenHeight - sq_height; y -= sq_height) {
               // FIXME: Make functions of the common stuff in this loop.
               if (i == 0) {
                  // Make params
                  MercatorMapParams params = new MercatorMapParams(x, y,
                        zoomFactor, "en", MercatorMapParams.IMAGE_MAP);

                  // Check if outstanding request has the map
                  Object o = oldReqs.remove(params);
                  MapRequest req = (MapRequest) o;
                  if (req == null) {
                     // Request maps
                     // FIXME: Create map inside
                     req = new MapRequest(params, new ImageMercatorMap(params,
                           sq_width, sq_height));
                     getServerComm().handleMercator(req);
                  }
                  list.add(req.map);
                  requests.put(params, req);
               }
               if (i == 1) {
                  // Make params
                  MercatorMapParams params = new MercatorMapParams(x, y,
                        zoomFactor, "en", MercatorMapParams.SIMPLEPOI_MAP);
                  // Check if outstanding request has the map, in that
                  // case move it here
                  Object o = oldReqs.remove(params);
                  MapRequest req = (MapRequest) o;
                  if (req == null) {
                     req = new MapRequest(params, new SPMercatorMap(this,
                           spDesc, params, sq_width, sq_height));
                     getServerComm().handleMercator(req);
                  }
                  list.add(req.map);
                  requests.put(params, req);
               }
            }
         }
      }

      imMaps = (MercatorMap[]) list.toArray(new MercatorMap[0]);
      // Cancel the requests still in the map
      for (Iterator it = oldReqs.values().iterator(); it.hasNext();) {
         ServerRequest req = (ServerRequest) it.next();
         req.cancel();
      }

   }

   public int mc2LonToX(int mc2Lon) {
      double x = mc2ToMapX(mc2Lon);
      return screenWidth / 2 + (int) Math.round((x - mc2ToMapX(cPoint.x)));
   }

   public int mc2LatToY(int mc2Lat) {
      double y = mc2ToMapY(mc2Lat);
      return screenHeight / 2 - (int) Math.round((y - mc2ToMapY(cPoint.y)));
   }

   public int xToMc2Lon(int x) {
      double x2 = mc2ToMapX(cPoint.x) + (double) (x - screenWidth / 2);
      return mapXToMc2(x2);
   }

   public int yToMc2Lat(int y) {
      double y2 = mc2ToMapY(cPoint.y) - (double) (y - screenHeight / 2);
      return mapYToMc2(y2);
   }

   public boolean drawMap(Graphics g, int width, int height, ImageObserver obs) {
      if (imMaps.length != 0) {
         int left_world_x = getLeftWorldX();
         int top_world_y = getTopWorldY();

         //System.out.println(x + " "+y);
         for (int i = 0; i < imMaps.length; ++i) {
            if (imMaps[i] != null) {
               imMaps[i].draw(g, obs, left_world_x, top_world_y);
            }
         }

         return true;
      } else
         return false;
   }

   public int mapXToMc2(double x) {
      return (int) Math.round(x * RADIAN_FACT
            / zoomLevels[zoomFactor - 1].radius);
   }

   public int mapYToMc2(double y) {
      return (int) Math.round(Math.atan(sinh(y
            / zoomLevels[zoomFactor - 1].radius))
            * RADIAN_FACT);
   }

   private double mc2ToMapX(int mc2_x) {
      return mc2_x * INV_RADIAN_FACT * zoomLevels[zoomFactor - 1].radius;
   }

   private double mc2ToMapY(int mc2_y) {
      double lat = mc2_y * INV_RADIAN_FACT;
      return Math.log(Math.tan(lat) + 1.0 / Math.cos(lat))
            * zoomLevels[zoomFactor - 1].radius;
   }

   /**
   private void setRadius(int zoomFac) {
      // Seems that the size of the level from the server is somewhat strange
      radius = ((double) (zoomLevels[zoomFac-1].max_x-zoomLevels[zoomFac-1].min_x)+zoomLevels[zoomFac-1].squareWidth) / 2.0 / Math.PI;
   } */

   private double sinh(double x) {
      return Math.exp(x) / 2.0 - Math.exp(-x) / 2.0;
   }

}
