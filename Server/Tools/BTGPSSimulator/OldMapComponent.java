import java.awt.Graphics;
import java.awt.Image;
import java.awt.Point;
import java.awt.Toolkit;
import java.awt.image.ImageObserver;
import java.net.URL;

public class OldMapComponent extends MapComponent {

   private Image map = null;
   private URL lastMapURL;

   public OldMapComponent(int width, int height, String server) {
      screenWidth = width;
      screenHeight = height;
      screenDistance = 614400000 / 2; // Height in m2cord dist
      mapServer = server;
   }

   public OldMapComponent(MapComponent oldComp) {
      spDesc = oldComp.spDesc;
      showPois = oldComp.showPois;
      coords = oldComp.coords;
      poiCoords = oldComp.poiCoords;
      poiComments = oldComp.poiComments;
      boxes = oldComp.boxes;
      mapServer = oldComp.mapServer;
      showTraffic = oldComp.showTraffic;
      carPosition = oldComp.carPosition;
      carDirection = oldComp.carDirection;
      cPoint = oldComp.cPoint;
      xyRatio = oldComp.xyRatio;
      screenHeight = oldComp.screenHeight;
      screenWidth = oldComp.screenWidth;
      screenDistance = 614400000 / 2; // Height in m2cord dist
      moveCenter(cPoint.x, cPoint.y);
   }

   public void zoomIn(int mc2_x, int mc2_y) {
      zoomIn(mc2_x, mc2_y, 2.0);
   }

   public void zoomOut(int mc2_x, int mc2_y) {
      zoomIn(mc2_x, mc2_y, 0.5);
   }

   public void moveCenter(int mc2_x, int mc2_y) {
      zoomIn(mc2_x, mc2_y, 1.0);
   }

   public void zoomToSquare(int x1, int y1, int x2, int y2) {
      int c_x = x1 + x2 / 2;
      int c_y = y1 + y2 / 2;
      int mc2_x = xToMc2Lon(c_x);
      int mc2_y = yToMc2Lat(c_y);
      int fac = screenWidth / Math.abs(x2);
      zoomIn(mc2_x, mc2_y, fac);
   }

   private void zoomIn(int mc2_x, int mc2_y, double fac) {
      screenDistance = (int) (screenDistance / fac);
      // Max zoom in
      if (screenDistance < 12500)
         screenDistance = 12500;
      // Max zoom out
      if (screenDistance > 614400000)
         screenDistance = 614400000;

      //System.out.println("new screenDistance : "+screenDistance);
      System.out.println("OldMapComponent.zoomIn() Setting central coord: " + mc2_x + " " + mc2_y);
      cPoint.x = mc2_x;
      cPoint.y = mc2_y;
      double coslat = (double) getCosLat(mc2_y);
      nwPoint.x = mc2_x
            - (int) (((double) screenDistance * coslat) / (2 * xyRatio));
      nwPoint.y = mc2_y + screenDistance / 2;
      sePoint.x = mc2_x
            + (int) (((double) screenDistance * coslat) / (2 * xyRatio));
      sePoint.y = mc2_y - screenDistance / 2;
      //System.out.println("setCentralCoord("+mc2_y+","+mc2_x+")");
      /**
      if(mc2_x < -500000000)
         mapHost = mapHostNA;
      if(mc2_x > -500000000) 
         mapHost = mapHostEU; */
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

         int xc = maxX / 2 + minX / 2;
         int yc = maxY / 2 + minY / 2;
         double coslat = (double) getCosLat(yc);
         // Check if wider than high then use x else y
         if (Math.abs(maxY - minY) / (Math.abs(maxX - minX) * coslat) > xyRatio) { // Higher
            screenDistance = Math.abs(maxY - minY);
         } else { // Wider
            screenDistance = (int) Math.rint(coslat * Math.abs(maxX - minX));
         }

         cPoint.x = xc;
         cPoint.y = yc;
         nwPoint.x = xc
               - (int) (((double) screenDistance * coslat) / (2 * xyRatio));
         nwPoint.y = yc + screenDistance / 2;
         sePoint.x = xc
               + (int) (((double) screenDistance * coslat) / (2 * xyRatio));
         sePoint.y = yc - screenDistance / 2;

         /**
         if ( xc < -500000000 ) {
            mapHost = mapHostNA;
         }
         if ( xc > -500000000 ) {
            mapHost = mapHostEU; 
         }
          */

         //System.out.println("c = (" + cPoint.x + "," + cPoint.y + ")");
         //System.out.println("nw = (" + nwPoint.x + "," + nwPoint.y + ")");
         //System.out.println("se = (" + sePoint.x + "," + sePoint.y + ")");
         //loadMap();
      }
   }

   public void loadMap() throws Exception {
      String urlString = "";

      int poi = showPois ? 1 : 0;

      String traffic;
      if (!showTraffic)
         traffic = "0";
      else
         traffic = "1";

      urlString = "http://" + mapServer + ":12211/Map.png?lla=" + sePoint.y
            + "&llo=" + nwPoint.x + "&ula=" + nwPoint.y + "&ulo=" + sePoint.x
            + "&w=" + screenWidth + "&h=" + screenHeight
            + "&s=19096&r=&mt=std&is=%FF%FC%02&map=1&topomap=1&poi=" + poi
            + "&route=1&scale=1&traffic=" + traffic;
      //System.out.println(urlString);
      URL u = new URL(urlString);
      //useMap = false;
      if (map == null || lastMapURL == null || !u.equals(lastMapURL)) {
         System.err.println("OldMapComponent.loadMap() Getting new map " + u);
         map = Toolkit.getDefaultToolkit().getImage(u);
         if (map == null)
            System.out.println("OldMapComponent.loadMap() map == NULL");
         else
            System.out.println("OldMapComponent.loadMap() Map Loaded");
         //useMap = true;
         //map.getWidth( this );
      }
   }

   public boolean drawMap(Graphics g, int width, int height, ImageObserver obs) {
      if (map != null) {
         g.drawImage(map, 0, 0, width, height, obs);
         return true;
      } else
         return false;
   }

   public int mc2LatToY(int mc2Lat) {
      int y = (int) (screenHeight * ((double) (nwPoint.y - mc2Lat) / screenDistance));
      return y;
   }

   public int mc2LonToX(int mc2Lon) {
      double coslat = (double) getCosLat(cPoint.y);
      int mc2dist = mc2Lon - cPoint.x;
      int screendist = (int) (mc2dist * coslat * (double) screenHeight / (double) screenDistance);
      int x = screenWidth / 2 + screendist;
      return x;
   }

   public int yToMc2Lat(int y) {
      int lat = nwPoint.y
            - (int) ((double) (y) * (double) screenDistance / (double) screenHeight);
      //System.out.println("lat:"+lat+", nwPoint.y:"+nwPoint.y);
      return lat;
   }

   public int xToMc2Lon(int x) {
      double coslat = (double) getCosLat(cPoint.y);
      int xdif = x - screenWidth / 2;
      double pixTomc2 = (double) screenDistance / ((double) screenHeight)
            / coslat;
      int lon = cPoint.x + (int) (xdif * pixTomc2);
      return lon;
   }

}
