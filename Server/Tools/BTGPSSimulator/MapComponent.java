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
import java.awt.Rectangle;
import java.awt.image.ImageObserver;
import java.util.ArrayList;

public abstract class MapComponent {

   public final int start_size = 5000;

   protected ArrayList coords = new ArrayList(start_size);
   protected ArrayList poiCoords = new ArrayList(start_size);
   protected ArrayList pois = new ArrayList(start_size);
   protected ArrayList poiComments = new ArrayList(start_size);
   protected ArrayList boxes = new ArrayList(start_size);
   protected int delayTime = 1000; // Simulation update time
   protected String mapServer = "";
   protected boolean showTraffic = true;
   protected boolean showPois = false;
   protected Point carPosition = new Point(0, 0);
   protected Point carDirection = new Point(0, 0);
   protected Point cPoint = new Point(0, 0); // What is this?
   protected Point markedPoint = new Point(0, 0);
   protected int screenHeight;
   protected int screenWidth;
   protected int screenDistance;
   protected Point nwPoint = new Point(0, 0);
   protected Point sePoint = new Point(0, 0);
   protected double xyRatio;
   protected SPDesc spDesc;
   protected Component toRepaint;

   private Point start = null;
   private Point dest = null;

   private class Poi {
      public int x;
      public int y;
      public String name;

      public Poi(int mc2_x, int mc2_y, String name) {
         this.x = mc2_x;
         this.y = mc2_y;
         this.name = name;
      }
   }

   // Maximum route acceleration and speed. In m/s & m/(s^2).
   protected int maxAcceleration = 2;
   protected int defaultMaxSpeed = 56;
   protected int maxSpeed = 20;

   protected final double wgs84Tomc2 = 11930464.7111;
   protected final double mc2ToM = (double) (40075016.69 / 4294967296.0);

   //Interface 

   public abstract void zoomIn(int mc2_x, int mc2_y);

   public abstract void zoomOut(int mc2_x, int mc2_y);

   public abstract void moveCenter(int mc2_x, int mc2_y);

   public abstract void zoomToSquare(int x1, int y1, int x2, int y2); //x2 and y2 is width and height of zoomSquare!

   public abstract void loadMap() throws Exception;

   public abstract int mc2LatToY(int mc2Lat);

   public abstract int mc2LonToX(int mc2Lon);

   public abstract int xToMc2Lon(int x);

   public abstract int yToMc2Lat(int y);

   public abstract boolean drawMap(Graphics g, int width, int height,
         ImageObserver obs);

   public abstract void zoomToRoute();

   protected void setSPDesc(SPDesc spDesc) {
      this.spDesc = spDesc;
   }

   protected void setMarkedPoint(Point p) {
      markedPoint = p;
   }

   protected Point getMarkedPoint() {
      return markedPoint;
   }

   protected void setShowPoi(boolean en) {
      showPois = en;
   }

   protected boolean showPoi() {
      return showPois;
   }

   protected void setStartPoint(int mc2_x, int mc2_y) {
      start = new Point(mc2_x, mc2_y);
   }

   protected void setDestPoint(int mc2_x, int mc2_y) {
      dest = new Point(mc2_x, mc2_y);
   }

   protected Point getStartPoint() {
      return start;
   }

   protected Point getDestPoint() {
      return dest;
   }

   protected void clearStartEndPoints() {
      start = null;
      dest = null;
   }

   protected int getDelayTime() {
      return delayTime;
   }

   protected void setScreenHeight(int height) {
      screenHeight = height;
   }

   protected void setScreenWidth(int width) {
      screenWidth = width;
   }

   protected Point getCPoint() {
      return cPoint;
   }

   protected Point getCarDir() {
      return carDirection;
   }

   protected Point getCarPos() {
      return carPosition;
   }

   protected Rectangle getBox(int i) {
      return (Rectangle) boxes.get(i);
   }

   protected Point getCoordPoiPoint(int i) {
      return (Point) poiCoords.get(i);
   }

   protected String getCoordPoiComment(int i) {
      return (String) poiComments.get(i);
   }

   protected int getPoiX(int i) {
      return ((Poi) pois.get(i)).x;
   }

   protected int getPoiY(int i) {
      return ((Poi) pois.get(i)).y;
   }

   protected String getPoiName(int i) {
      return ((Poi) pois.get(i)).name;
   }

   protected Point getRoutePoint(int i) {
      return (Point) coords.get(i);
   }

   protected void addCoordPoiComment(String comment) {
      poiComments.add(comment);
   }

   protected void addCoordPoiPoint(int mc2_x, int mc2_y) {
      poiCoords.add(new Point(mc2_x, mc2_y));
   }

   protected void addPoiPoint(int mc2_x, int mc2_y, String name) {
      Poi poi = new Poi(mc2_x, mc2_y, name);
      pois.add(poi);
   }

   protected void removeCoordPoiPoint(int i) {
      poiCoords.remove(i);
   }

   protected void removeCoordPoiComment(int i) {
      poiComments.remove(i);
   }

   protected void removeRoutePoint(int i) {
      for (int j = i; j < coords.size(); j++)
         coords.remove(j);
   }

   protected ArrayList getRoute() {
      return coords;
   }

   protected void setRoute(ArrayList route) {
      coords = route;
   }

   protected int getNbrBoxes() {
      return boxes.size();
   }

   protected int getNbrRoutePoints() {
      return coords.size();
   }

   protected int getNbrCoordPoiPoints() {
      return poiCoords.size();
   }

   protected int getNbrPoiPoints() {
      return pois.size();
   }

   protected int getDefaultMaxSpeed() {
      return defaultMaxSpeed;
   }

   protected void setMaxSpeed(int speed) {
      maxSpeed = speed;
   }

   protected int getMaxSpeed() {
      return maxSpeed;
   }

   protected void setShowTraffic(boolean en) {
      showTraffic = en;
   }

   protected boolean showTraffic() {
      return showTraffic;
   }

   protected void setDelayTime(int time) {
      delayTime = time;
   }

   protected void resetForLoad(int size) {
      coords = new ArrayList(size);
      //mapWin.setLoadButton(false);
      //mapWin.repaint();
   }

   protected void setRoutePoints(NMEAMessage[] messages) {
      //nbrCoords = 0;
      for (int i = 0, n = messages.length; i < n; ++i) {
         if (messages[i].isRMC()) {
            if (messages[i].getLon() != Double.MAX_VALUE) {
               coords.add(new Point((int) Math.round(wgs84ToMc2(messages[i]
                     .getLon())), (int) Math.round(wgs84ToMc2(messages[i]
                     .getLat()))));
               //nbrCoords++;
            }
         }
      }
   }

   /**
    * Call resetForLoad() first
    */
   protected boolean loadRoutePoint(double x, double y) {
      int newX = (int) Math.round(wgs84ToMc2(x));
      int newY = (int) Math.round(wgs84ToMc2(y));
      
      addRoutePoint(newX, newY);

      // Why use repaint! or something
      //      Graphics g = getGraphics();
      //      drawRoutePoint();
      //      repaint();
      return true;
   }

   protected void addRoutePoint(int mc2_x, int mc2_y) {
      if (coords.size() == 0) {
         coords.add(new Point(mc2_x, mc2_y));
      } else {
         // Max dist & acc
         int max_dd = (int) (((float) delayTime / 1000.0) * maxAcceleration / mc2ToM);
         int max_d = (int) (((float) delayTime / 1000.0) * maxSpeed / mc2ToM);
         float coslat = (float) getCosLat(mc2_y);
         do {
            int lastSpeed = 0;
            float dx;
            float dy;
            if (coords.size() > 1) {
               dx = ((Point) coords.get(coords.size() - 1)).x
                     - ((Point) coords.get(coords.size() - 2)).x;
               dy = ((Point) coords.get(coords.size() - 1)).y
                     - ((Point) coords.get(coords.size() - 2)).y;
               lastSpeed = (int) Math.sqrt(Math.pow(dy, 2)
                     + Math.pow(dx * coslat, 2));
            }
            dx = mc2_x - ((Point) coords.get(coords.size() - 1)).x;
            dy = mc2_y - ((Point) coords.get(coords.size() - 1)).y;

            int thisSpeed = (int) Math.sqrt(Math.pow(dy, 2)
                  + Math.pow(dx * coslat, 2));
            if ((lastSpeed + max_dd > thisSpeed) && (max_d > thisSpeed)) {
               //coords[nbrCoords++] = new Point(mc2_x, mc2_y);
               coords.add(new Point(mc2_x, mc2_y));
               //nbrCoords++;

            } else {
               // Calculate how many between points we need.
               int nbrPoints = 0;
               int totDist = 0;
               int incSpeed = lastSpeed + max_dd;
               while (totDist < thisSpeed) {
                  nbrPoints++;
                  totDist += incSpeed;
                  incSpeed = Math.min(incSpeed + max_dd, max_d);
               }

               // Create new speed
               int newSpeed = Math.min((int) (thisSpeed / nbrPoints) + 1,
                     lastSpeed + max_dd);
               newSpeed = Math.min(newSpeed, max_d);
               float speedFactor = (float) newSpeed / (float) thisSpeed;
               int new_dx = (int) (dx * speedFactor);
               int new_dy = (int) (dy * speedFactor);
               int new_X = ((Point) coords.get(coords.size() - 1)).x + new_dx;
               int new_Y = ((Point) coords.get(coords.size() - 1)).y + new_dy;
               //System.out.println("newSpeed"+newSpeed+", dy = "+new_dy
               //                 +", dx = "+new_dx);
               coords.add(new Point(new_X, new_Y));
               //nbrCoords++;

            }
         } while (/**(nbrCoords < maxNbrPoints) && */
         (((Point) coords.get(coords.size() - 1)).x != mc2_x)
               || (((Point) coords.get(coords.size() - 1)).y != mc2_y));
      }
   }

   protected int findRoutePoint(int mc2_x, int mc2_y) {
      int retVal = -1;
      int index = coords.size() - 1;
      int sc_y = mc2LatToY(mc2_y);
      int sc_x = mc2LonToX(mc2_x);
      //System.out.println("sc_y,sc_x"+sc_y+","+sc_x);

      while (((index + 1) > 0) && (retVal == -1)) {
         int x = mc2LonToX(((Point) coords.get(index)).x);
         int y = mc2LatToY(((Point) coords.get(index)).y);
         //System.out.println("      y,x"+y+","+x);
         if ((sc_x < (x + 5)) && (sc_x > (x - 5)) && (sc_y < (y + 5))
               && (sc_y > (y - 5))) {
            retVal = index;
         }
         index--;
      }
      return retVal;
   }

   protected String getRouteDataString(int i) {
      String data = "";
      if (i < coords.size()) {
         // Lat & lon
         double y = (double) ((double) ((Point) coords.get(i)).y / wgs84Tomc2);
         double x = (double) ((double) ((Point) coords.get(i)).x / wgs84Tomc2);
         double speed_x = 0;
         double speed_y = 0;
         if (i < coords.size() - 1) {
            int d_x = ((Point) coords.get(i + 1)).x - ((Point) coords.get(i)).x;
            int d_y = ((Point) coords.get(i + 1)).y - ((Point) coords.get(i)).y;
            //System.out.println("dy = "+d_y+", dx = "+d_x);
            double coslat = (double) getCosLat(((Point) coords.get(i)).y);
            speed_x = (double) d_x * coslat * mc2ToM * (double) 1000
                  / (double) delayTime;
            speed_y = (double) d_y * mc2ToM * (double) 1000
                  / (double) delayTime;
            if ((speed_x < 0000.1) && (speed_x > -0000.1))
               speed_x = 0;
            if ((speed_y < 0000.1) && (speed_y > -0000.1))
               speed_y = 0;
         }

         data = y + " " + x + " " + speed_x + " " + speed_y;
         //System.out.println(i+")"+data);
      }
      return data;
   }

   protected int findCoordPoiPoint(int sc_x, int sc_y) {
      int retVal = -1;
      int index = poiCoords.size() - 1;
      //System.out.println("(sc_y,sc_x) = "+ "(" + sc_y + "," + sc_x + ")");

      while (((index + 1) > 0) && (retVal == -1)) {
         int x = mc2LonToX(((Point) poiCoords.get(index)).x);
         int y = mc2LatToY(((Point) poiCoords.get(index)).y);
         //System.out.println("      y,x"+y+","+x);
         if ((sc_x < (x + 5)) && (sc_x > (x - 5)) && (sc_y < (y + 5))
               && (sc_y > (y - 5))) {
            retVal = index;
         }
         index--;
      }
      return retVal;
   }

   protected void updateVehicle(double lat, double lon, double speed,
         double bearing, boolean gpsOn) {
      //System.out.println("updateVehicle");

      carPosition.x = (int) Math.round(wgs84ToMc2(lon));
      carPosition.y = (int) Math.round(wgs84ToMc2(lat));
      //System.out.println(carPos.x+" "+carPos.y);
      // Update speed & bearing
      speed = speed / 1.9438445; // knots =>m/s.
      bearing = bearing * Math.PI / 180.0;
      double y_speed = (double) speed * (double) Math.cos(bearing);
      double x_speed = (double) speed * (double) Math.sin(bearing);

      double meterTocoord = 3.0 / mc2ToM;
      double coslat = (double) getCosLat(carPosition.y);

      carDirection.y = carPosition.y + (int) (meterTocoord * y_speed);
      carDirection.x = carPosition.x + (int) (meterTocoord * x_speed / coslat);

      if (false //close to edge)
      ) {
      }
   }

   protected void setServer(String server) {
      this.mapServer = server;
   }

   protected String getServer() {
      return mapServer;
   }

   protected void resetCoordPoiForLoad() {
      poiCoords = new ArrayList(start_size);
      //mapWin.repaint();
   }

   protected void resetPoiForLoad() {
      pois = new ArrayList(start_size);
   }

   protected void addBox(Rectangle r) {
      boxes.add(r);
   }

   protected void removeBox(int i) {
      boxes.remove(i);
   }

   protected double mc2ToWgs84(double mc2) {
      return mc2 / wgs84Tomc2;
   }

   protected double wgs84ToMc2(double wgs84) {
      return wgs84 * wgs84Tomc2;
   }

   protected double getCosLat(int mc2Lat) {
      double latInRads = ((double) mc2Lat * Math.PI) / (wgs84Tomc2 * 180);
      return Math.cos(latInRads);
   }

   protected void setXYRatio(double ratio) {
      xyRatio = ratio;
   }

   protected Point getNWPoint() {
      return nwPoint;
   }

   protected Point getSEPoint() {
      return sePoint;
   }

   protected int getScreenDistance() {
      return screenDistance;
   }

   protected void setScreenDistance(int dist) {
      screenDistance = dist;
   }

}
