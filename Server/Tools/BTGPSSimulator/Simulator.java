/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
import java.awt.BorderLayout;
import java.awt.Cursor;
import java.awt.Point;
import java.awt.Rectangle;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Random;
import java.util.StringTokenizer;
import java.util.Vector;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JProgressBar;


class Simulator {

   public static final int REVERSE = -1;
   public static final int PAUSE = 0;
   public static final int FORWARD = 1;
   public static final int WILD = 2;
   public static final int PORT = 3;
   public static final int NO_REPEAT = 4;
   public static final int REPEAT_LOOP = 5;
   public static final int REPEAT_REVERSE = 6;

   private PrintWriter logWriter;
   private String logFileName = new String("");
   private boolean logging;
   private int routeSize = 0;
   private NMEAMessage[] loadedRoute;
   private MainSimulatorWindow mainWindow;
   private LineNumberReader nmeaReader;
   private MapComponent mapComponent;
   private int gpsSig; // What is this?
   private NMEAMessage gsaMessage;
   private int delayTime;
   private int currentFrame;
   private int state;
   private long lastModified = 0;
   private boolean gpsOn;
   private double speed;
   private int bearing;
   private boolean autoZoom = true;
   private double speedFactor = 1.0;
   private int repeatMode;
   private ParamFile paramFile;
   private SenderManager senderManager;
   private double latitude;
   private double longitude;
   private double[] distance;
   private LinkedList speedQueue = new LinkedList();
   private double accumSpeed = 0.0;
   private double avgSpeed = 0.0;
   private boolean tunnelMode = false;
   private byte[] routeData;

   private final double mc2ToM = (double) (40075016.69 / 4294967296.0);
   private final double wgs84ToM = (double) (40075016.69 / 360.0);
   private final double RADIUS = 6378.137; //earth radius

   private class ProgressDialog {
      private JProgressBar progressBar;
      private JLabel text = new JLabel("Text");
      private JFrame dialog;

      public ProgressDialog(int min, int max) {
         dialog = new JFrame();
         progressBar = new JProgressBar(min, max);
         progressBar.setValue(0);
         progressBar.setVisible(true);
         dialog.setLayout(new BorderLayout());
         dialog.getContentPane().add(progressBar, BorderLayout.CENTER);
         //dialog.getContentPane().add(text,BorderLayout.CENTER);
         dialog.pack();
         dialog.setVisible(true);
      }

      public void setValue(int v) {
         progressBar.setValue(v);
      }

      public void dispose() {
         dialog.dispose();
      }
   }

   private class SenderStatusListener implements GPSSenderListener {
      public void gpsSenderStatusUpdate(GPSSender source, boolean sending,
            String statusMsg) {
         mainWindow.setStatus(sending, statusMsg);
      }

      public void removeSender(GPSSender source) {

      }
   }

   /**
    * this method won't return until the SenderManager is succesfullt started.
    */
   public Simulator(MainSimulatorWindow simWin, MapComponent mapComp,
         ParamFile paramFile) {
      final String fname = "Simulator.Simulator(): ";

      this.mainWindow = simWin;
      this.mapComponent = mapComp;
      this.paramFile = paramFile;
      System.out.println(fname);

      logging = false;
      repeatMode = Simulator.REPEAT_LOOP;
      gpsSig = 13;
      gsaMessage = new NMEAMessage(gpsSig);
      delayTime = 1000;
      currentFrame = 0;
      state = Simulator.FORWARD;
      gpsOn = false;
      speed = 0.0;
      bearing = 180;
      System.out.println(fname + "starting SenderManager...");
      senderManager = new SenderManager(paramFile.getDefaultPort(), simWin, this,
            new SenderStatusListener());
      latitude = 55.71826; // Wayfinder 
      longitude = 13.18994; // Wayfinder
      loadedRoute = new NMEAMessage[4];
      NMEAMessage message = new NMEAMessage(gpsSig);
      message.setRMCMessage(latitude, longitude, 0.0, 0.0);
      message.setBearing(bearing);
      loadedRoute[0] = message;
      loadedRoute[1] = gsaMessage;
      NMEAMessage tmp1 = message.getGGAMessage();
      tmp1.setTimeFromPrev(0);
      loadedRoute[2] = tmp1;
      NMEAMessage tmp2 = message.getGSVMessage();
      tmp2.setTimeFromPrev(0);
      loadedRoute[3] = tmp2;
      routeSize = 4;
      currentFrame = 0;
      System.out.println(fname + "DONE.");
   }

   public synchronized void setMapComponent(MapComponent mapComp) {
      this.mapComponent = mapComp;
   }

   public synchronized byte[] getRouteData() {
      return routeData;
   }

   public synchronized void setAutoZoom(boolean en) {
      autoZoom = en;
   }

   public synchronized boolean autoZoom() {
      return autoZoom;
   }

   public synchronized void setTunnelMode(boolean mode) {
      tunnelMode = mode;
   }

   public synchronized boolean getTunnelMode() {
      return tunnelMode;
   }

   public synchronized void setPortAutomatically() {
      senderManager.setPortAutomatically();
   }

   public synchronized void setPort() {
      senderManager.changePort();
   }

   public synchronized void setSocket() {
      senderManager.setSocket(paramFile.getSocketHost(), paramFile.getSocketPort());
   }

   public synchronized void setServerSocket() {
      senderManager.setServerSocket(paramFile.getSocketPort());
   }

   public synchronized double getSpeedFactor() {
      return speedFactor;
   }

   public synchronized void setSpeedFactor(double factor) {
      speedFactor = factor;
   }

   public synchronized int getDelayTime() {
      return delayTime;
   }

   public synchronized int getBearing() {
      return bearing;
   }

   public synchronized void setBearing(int b) {
      bearing = b;
   }

   public synchronized void setAvgSpeed(double speed) {
      avgSpeed = speed;
   }

   public synchronized double getAvgSpeed() {
      return avgSpeed;
   }

   public synchronized int getRepeatMode() {
      return repeatMode;
   }

   public synchronized void setRepeatMode(int mode) {
      repeatMode = mode;
   }

   public synchronized double getSpeed() {
      return speed;
   }

   public synchronized void setSpeed(double speed) {
      this.speed = speed;
   }

   public synchronized void changeSpeed(double diff) {
      speed += diff;
   }

   public synchronized double getLat() {
      return latitude;
   }

   public synchronized double getLon() {
      return longitude;
   }

   public synchronized void setLat(double lat) {
      latitude = lat;
   }

   public synchronized void setLon(double lon) {
      longitude = lon;
   }

   public synchronized int getState() {
      return state;
   }

   public synchronized NMEAMessage getCurrentMessage() {
      return loadedRoute[currentFrame];
   }

   public synchronized NMEAMessage getMessage(int i) {
      return loadedRoute[i];
   }

   public synchronized void setState(int state) {
      //System.out.println("Setting state!");
      this.state = state;
      if (this.state == Simulator.PAUSE)
         speedFactor = 1.0;
   }

   public synchronized int getCurrentFrame() {
      return currentFrame;
   }

   public synchronized void stepForward() {
      currentFrame++;
   }

   public synchronized void stepBackward() {
      currentFrame--;
   }

   public synchronized void setCurrentFrame(int frame) {
      currentFrame = frame;
   }

   public synchronized int getRouteSize() {
      return routeSize;
   }

   public synchronized boolean GPSOn() {
      return gpsOn;
   }

   public synchronized void enableGPS(boolean en) {
      gpsOn = en;
   }

   public synchronized int getGPSSig() {
      return gpsSig;
   }

   public synchronized void setGPSSig(int sig) {
      gpsSig = sig;
      delayTime = 1000;
      gsaMessage = new NMEAMessage(gpsSig);
      gsaMessage.setTimeFromPrev(0);
   }

   public synchronized double getDistance(int i) {
      if (i < routeSize && routeSize > 4)
         return distance[i];
      else
         return 0.0;
   }

   public synchronized String getLogFileName() {
      return logFileName;
   }

   public synchronized void stopLogging() {
      logging = false;
      logFileName = "";
      logWriter = null;
   }

   public synchronized void setNMEAReader(LineNumberReader reader) {
      nmeaReader = reader;
   }

   public synchronized void resetLoadedRoute() {
      loadedRoute = new NMEAMessage[0];
   }

   public synchronized NMEAMessage[] getLoadedRoute() {
      return loadedRoute;
   }

   public synchronized void setRoute(NMEAMessage[] route) {
      loadedRoute = new NMEAMessage[route.length];

      for (int i = 0; i < route.length; i++)
         loadedRoute[i] = route[i];
   }

   public synchronized void setLogFile(String filename) throws IOException {
      if (logWriter != null) {
         logWriter.close();
         logFileName = "";
      }
      logWriter = new PrintWriter(new FileWriter(filename));
      logFileName = filename;
   }

   private void closeReader() {
      if (nmeaReader != null) {
         //try {
         new ErrorDialog(mainWindow.getFrame(), "Input port will not be used")
               .show();
         nmeaReader = null;
         //nmeaReader.close();
         //} catch ( IOException e ) {
         //}
      }
   }

   public synchronized void reset() {
      state = Simulator.PAUSE;
      speedFactor = 1.0;
      avgSpeed = 0.0;
      currentFrame = 0;
      speedQueue.clear();
      accumSpeed = 0.0;
      mainWindow.update();
      mainWindow.updateMap();
      //simWin.repaint();
   }

   public synchronized void applyRectFilter(int n) {
      int j = 0;
      for (int i = 0; i < mapComponent.getNbrRoutePoints(); i++) {
         Point p = mapComponent.getRoutePoint(i);
         Random r = new Random();
         int r1 = r.nextInt(2 * n + 1) - n;
         int r2 = r.nextInt(2 * n + 1) - n;
         p.x += (int) Math.round(r1 / mc2ToM);
         p.y += (int) Math.round(r2 / mc2ToM);
         loadedRoute[4 * i].setLat(mapComponent.mc2ToWgs84(p.y));
         loadedRoute[4 * i].setLon(mapComponent.mc2ToWgs84(p.x));
         if (i > 0) {
            Point p2 = mapComponent.getRoutePoint(i - 1);
            if (p.x != p2.x) {
               int b = (int) Math.round(180.0
                     * Math.atan2(p.y - p2.y, p.x - p2.x) / Math.PI);
               if (b < 0)
                  b += 360;
               b = 90 - b;
               if (b < 0)
                  b += 360;
               loadedRoute[4 * (i - 1)].setBearing(b);
            }
         }
      }
      mainWindow.updateMap();
   }

   public synchronized void applyNormalFilter(int d) {
      int j = 0;
      for (int i = 0; i < mapComponent.getNbrRoutePoints(); i++) {
         Point p = mapComponent.getRoutePoint(i);
         Random r = new Random();
         int r1 = (int) Math.round(r.nextGaussian() * d);
         int r2 = (int) Math.round(r.nextGaussian() * d);
         p.x += (int) Math.round(r1 / mc2ToM);
         p.y += (int) Math.round(r2 / mc2ToM);
         loadedRoute[4 * i].setLat(mapComponent.mc2ToWgs84(p.y));
         loadedRoute[4 * i].setLon(mapComponent.mc2ToWgs84(p.x));
         if (i > 0) {
            Point p2 = mapComponent.getRoutePoint(i - 1);
            if (p.x != p2.x) {
               int b = (int) Math.round(180.0
                     * Math.atan2(p.y - p2.y, p.x - p2.x) / Math.PI);
               if (b < 0)
                  b += 360;
               b = 90 - b;
               if (b < 0)
                  b += 360;
               loadedRoute[4 * (i - 1)].setBearing(b);
            }
         }
      }
      mainWindow.updateMap();
   }

   public synchronized void buildRoute(int x1, int y1, int x2, int y2,
         int optMode, String id) {
      /**
      x1 = 157362825; y1 = 664749105;
      x2 = 149830012; y2 = 664282266;
       */
      
      String line, route_id = "";
      URL url;
      HttpURLConnection conn;
      boolean idFound = false;
      mainWindow.getFrame().setCursor(new Cursor(Cursor.WAIT_CURSOR));

      try {

         if (id == null) {
            String costA = "0";
            String costB = "0";
            String costC = "0";
            if (optMode == 0)
               costA = "1";
            else if (optMode == 1)
               costB = "1";
            else
               costC = "1";

            url = new URL("http://" + paramFile.getServer() + ":12211/xmlfile");
            conn = (HttpURLConnection) url.openConnection();
            conn.setDoOutput(true);
            conn.addRequestProperty("Accept-Charset", "utf-8");

            OutputStream os = conn.getOutputStream();
            PrintWriter w = new PrintWriter(os);
            //PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter("xml.txt")));

            w.write("<?xml version='1.0' encoding='utf-8' ?>\n"
                  + "<!DOCTYPE isab-mc2 SYSTEM 'isab-mc2.dtd'>\n");
            w.write("<isab-mc2>" + "<auth development=\"true\">"
                  + "<auth_user>" + paramFile.getLogin() + "</auth_user>"
                  + "<auth_passwd>" + paramFile.getPassword() 
                  + "</auth_passwd>" + "</auth>");
            w.write("<route_request transaction_id=\"IDRoute1\">"
                  + "<route_request_header>"
                  + "<route_preferences route_description_type=\"normal\">"
                  + "<route_settings route_vehicle=\"passengercar\">"
                  + "<route_costA>" + costA + "</route_costA>"
                  + "<route_costB>" + costB + "</route_costB>"
                  + "<route_costC>" + costC + "</route_costC>"
                  + "<language>Swedish</language>" + "</route_settings>"
                  + "</route_preferences>" + "</route_request_header>");
            w.write("<routeable_item_list>"
                  + "<position_item position_system=\"MC2\">" + "<lat>" + y1
                  + "</lat>" + "<lon>" + x1 + "</lon>" + "</position_item>"
                  + "</routeable_item_list>");
            w.write("<routeable_item_list>"
                  + "<position_item position_system=\"MC2\">" + "<lat>" + y2
                  + "</lat>" + "<lon>" + x2 + "</lon>" + "</position_item>"
                  + "</routeable_item_list>");
            w.write("</route_request>" + "</isab-mc2>\n");

            w.close();
            
            /**
            String header;
            int n = 0;
            while((header = conn.getHeaderField(n)) != null) {
               System.out.println(header);
               n++;
            } */
            //InputStreamReader is = new InputStreamReader(conn.getInputStream());
            BufferedReader br = new BufferedReader(new InputStreamReader(conn
                  .getInputStream()));

            System.out.println("Simulator.buildRoute() reading route response ");
            
            while ((line = br.readLine()) != null && !idFound) {
               //System.out.println("" + line);
               StringTokenizer st = new StringTokenizer(line);
               while (st.hasMoreTokens() && !idFound) {
                  String s = st.nextToken();
                  //System.out.println(s);
                  if (s.startsWith("route_id=")) {
                     StringTokenizer st2 = new StringTokenizer(s, "\"");
                     st2.nextToken();
                     route_id = st2.nextToken();
                     System.out.println("Simulator.buildRoute() route_id = " + route_id);
                     idFound = true;
                  }
               }
            }
            
            br.close();

         } else {
            route_id = id;
            idFound = true;
         }

         if (!idFound || route_id.equals("0_0")) {
            JOptionPane.showMessageDialog(mainWindow.getFrame(),
                  "Could not build route", "Server Error",
                  JOptionPane.ERROR_MESSAGE);
            mapComponent.clearStartEndPoints();
            mainWindow.getFrame().setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
            return;
         } else {
            System.out.println("Simulator.buildRoute() getting route from server, routeid: " + route_id);
            url = new URL("http://" + paramFile.getServer()
                  + ":12211/navigatorroute.bin?r=" + route_id
                  + "&protoVer=10&s=150000");
            conn = (HttpURLConnection) url.openConnection();
            //conn.setDoOutput(true);
            conn.addRequestProperty("Accept-Charset", "utf-8");

            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            //is = new InputStreamReader(conn.getInputStream());
            InputStream i = conn.getInputStream();

            //System.out.println("Simulator.buildRoute() Loading...");
            int b;
            while ((b = i.read()) != -1) {
               bos.write(b);
            }
            i.close();

            routeData = bos.toByteArray();
            ByteArrayInputStream bis = new ByteArrayInputStream(routeData);

            try {
               loadNavFile(bis);
            } catch (Exception e) {
            }

            System.out.println("Simulator.buildRoute() done!");

            //is.close();
         }

      } catch (MalformedURLException e) {
         System.out.println("Simulator.buildRoute() MalformedURLException: " + e.getMessage());
      } catch (IOException e) {
         System.out.println("Simulator.buildRoute() IOException: " + e.getMessage());
      }

      //start = null;
      //dest = null;
      mainWindow.getFrame().setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
   }

   public synchronized void loadNMEADataFile(String filename) {
      try {
         NMEALoader loader = new NMEALoader(filename);
         loader.load();
         loadedRoute = loader.getLoadedRoute();
         routeSize = loadedRoute.length;
         System.err.println("Simulator.loadNMEADataFile() routeSize = " + routeSize);
         distance = new double[routeSize];
         double dist = 0.0;
         NMEAMessage msg = loadedRoute[0];
         NMEAMessage lastMsg = loadedRoute[0];
         for (int i = 0; i < routeSize; i++) {
            msg = loadedRoute[i];
            if (msg.isRMC()) {
               double lat1 = msg.getLat() * Math.PI / 180.0;
               double lat2 = lastMsg.getLat() * Math.PI / 180.0;
               double lon1 = msg.getLon() * Math.PI / 180.0;
               double lon2 = lastMsg.getLon() * Math.PI / 180.0;
               double d = Math.sin(lat1) * Math.sin(lat2) + Math.cos(lat1)
                     * Math.cos(lat2) * Math.cos(Math.abs(lon2 - lon1));
               if (Math.abs(d) <= 1.0)
                  dist += RADIUS * Math.acos(d);
               lastMsg = msg;
            }
            distance[routeSize - 1 - i] = dist;
         }
         //simWin.updateLoaded();
         mapComponent.setRoutePoints(getLoadedRoute());
         // Zoom to route
         if (autoZoom())
            mapComponent.zoomToRoute();
         try {
            mapComponent.loadMap();
         } catch (Exception me) {
         }
         mainWindow.updateMap();
         setState(Simulator.PAUSE);
         mainWindow.update();
         //simWin.repaint();
      } catch (IOException e) {
         mainWindow.showExceptionDialog("NMEA load error", e);
      }
   }

   public synchronized void downLoadRoute() {
      loadedRoute = null;
      ArrayList route = mapComponent.getRoute();
      //ProgressMonitor pm = new ProgressMonitor(simWin.getFrame(),"Loading route...","",0,route.size());
      //pm.setMillisToDecideToPopup(0);
      //pm.setMillisToPopup(0);
      ArrayList vec = new ArrayList(route.size() * 5);
      NMEAMessage message;
      double speed_x = 0.0;
      double speed_y = 0.0;
      
      for (int i = 0; i < route.size(); i++) {
         //pm.setProgress(i);

         double y = (double) mapComponent.mc2ToWgs84(((Point) route.get(i)).y);
         double x = (double) mapComponent.mc2ToWgs84(((Point) route.get(i)).x);

         if (i < route.size() - 1) {
            int d_x = ((Point) route.get(i + 1)).x - ((Point) route.get(i)).x;
            int d_y = ((Point) route.get(i + 1)).y - ((Point) route.get(i)).y;
            //System.out.println("dy = "+d_y+", dx = "+d_x);
            double coslat = (double) mapComponent
                  .getCosLat(((Point) route.get(i)).y);
            speed_x = (double) d_x * coslat * mc2ToM * 1000.0
                  / (double) mapComponent.getDelayTime();
            speed_y = (double) d_y * mc2ToM * 1000.0
                  / (double) mapComponent.getDelayTime();
            if ((speed_x < 0000.1) && (speed_x > -0000.1))
               speed_x = 0.0;
            if ((speed_y < 0000.1) && (speed_y > -0000.1))
               speed_y = 0.0;
         }

         message = new NMEAMessage(gpsSig);
         if (message.setRMCMessage(y, x, speed_x, speed_y)) {
            vec.add(message);
            vec.add(gsaMessage);
            if (message.isRMC()) {
               // Add other packets as well
               NMEAMessage tmp1 = message.getGGAMessage();
               tmp1.setTimeFromPrev(0);
               vec.add(tmp1);
               NMEAMessage tmp2 = message.getGSVMessage();
               tmp2.setTimeFromPrev(0);
               vec.add(tmp2);
            }
         }
      }
      loadedRoute = (NMEAMessage[]) vec.toArray(new NMEAMessage[0]);
      routeSize = loadedRoute.length;

      distance = new double[routeSize];
      double dist = 0.0;
      NMEAMessage msg = loadedRoute[0];
      NMEAMessage lastMsg = loadedRoute[0];
      for (int i = 0; i < routeSize; i++) {
         msg = loadedRoute[i];
         if (msg.isRMC()) {
            double lat1 = msg.getLat() * Math.PI / 180.0;
            double lat2 = lastMsg.getLat() * Math.PI / 180.0;
            double lon1 = msg.getLon() * Math.PI / 180.0;
            double lon2 = lastMsg.getLon() * Math.PI / 180.0;
            double d = Math.sin(lat1) * Math.sin(lat2) + Math.cos(lat1)
                  * Math.cos(lat2) * Math.cos(Math.abs(lon2 - lon1));
            if (Math.abs(d) <= 1.0)
               dist += RADIUS * Math.acos(d);
            lastMsg = msg;
         }
         distance[routeSize - 1 - i] = dist;
      }
      //System.out.println("Dist = " + dist);
      //pm.close(); 
      mainWindow.update();
   }

   public synchronized void loadFile(String filename)
         throws FileNotFoundException, IOException, FileReadException {
      final String fname = "Simulator.loadFile(): ";

      FileInputStream finstream;
      LineNumberReader datastream;

      File coordfile;
      coordfile = new File(filename);
      finstream = new FileInputStream(coordfile);
      datastream = new LineNumberReader(new InputStreamReader(finstream));
      System.out.println(fname + "Opening file : " + filename);

      // Check route size.
      int size = 0;

      while (datastream.readLine() != null) {
         size++;
      }
      datastream.close();
      loadedRoute = new NMEAMessage[size];
      finstream = new FileInputStream(coordfile);
      datastream = new LineNumberReader(new InputStreamReader(finstream));

      String data = null;
      for (int i = 0; i < size; i++) {
         data = datastream.readLine();
         NMEAMessage message = parseCoordString(data);
         if (message == null) {

            routeSize = 0;
            currentFrame = 0;
            throw new FileReadException();
         } else {
            System.out.println(fname + " at " + i + " lon " + message.getLon() + " lat "
                  + message.getLat());
            loadedRoute[i] = message;
         }
      }

      routeSize = size;
      if (state == Simulator.REVERSE)
         currentFrame = routeSize - 1;
      else
         currentFrame = 0;

   }

   public synchronized void loadNavFile(String filename)
         throws FileNotFoundException, FileReadException {
      final String fname = "Simulator.loadNavFile(): ";

      FileInputStream finstream;
      DataInputStream datastream;
      File file;
      Vector coords = new Vector();
      file = new File(filename);
      finstream = new FileInputStream(file);
      datastream = new DataInputStream(finstream);
      System.out.println(fname + "Opening file: " + filename);

      mainWindow.getFrame().setCursor(new Cursor(Cursor.WAIT_CURSOR));

      // Check route size.

      IsabRouteList list = new IsabRouteList();
      lastModified = file.lastModified();

      if (!list.readElements(datastream, coords)) {
         throw new FileReadException();
      }
      mapComponent.resetForLoad(coords.size());
      mainWindow.enableLoadButton(false);
      for (int i = 0; i < coords.size(); i++) {
         int speed = (int) Math.rint(((IsabRouteElement) coords.elementAt(i))
               .getMpsSpeed());
         mapComponent.setMaxSpeed(speed);
         IsabRouteElement el = (IsabRouteElement) coords.elementAt(i);
         mapComponent.loadRoutePoint(el.getLon(), el.getLat());
      }
      //mapWin.setHide( hide );
      // Zoom to route
      if (autoZoom())
         mapComponent.zoomToRoute();
      try {
         mapComponent.loadMap();
      } catch (Exception me) {
      }
      mainWindow.updateMap();
      // Now get route from Map with many more coords...
      downLoadRoute();
      mainWindow.getFrame().setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
   }

   public synchronized void loadNavFile(InputStream is)
         throws FileNotFoundException, FileReadException {
      FileInputStream finstream;
      DataInputStream datastream;
      File file;
      Vector coords = new Vector();
      //file = new File( filename );
      //finstream  = new FileInputStream( file );
      datastream = new DataInputStream(is);
      //System.out.println( "Opening file : " + filename );

      // Check route size.

      IsabRouteList list = new IsabRouteList();
      //lastModified = file.lastModified();

      if (!list.readElements(datastream, coords)) {
         throw new FileReadException();
      }

      System.out.println("Simulator.loadNavFile() File read!");
      
      mapComponent.resetForLoad(coords.size());
      mainWindow.enableLoadButton(false);
      for (int i = 0; i < coords.size(); i++) {
         int speed = (int) Math.rint(((IsabRouteElement) coords.elementAt(i))
               .getMpsSpeed());
         mapComponent.setMaxSpeed(speed);
         IsabRouteElement el = (IsabRouteElement) coords.elementAt(i);
         mapComponent.loadRoutePoint(el.getLon(), el.getLat());
      }
      //mapWin.setHide( hide );
      // Zoom to route
      if (autoZoom())
         mapComponent.zoomToRoute();
      try {
         mapComponent.loadMap();
      } catch (Exception me) {
      }
      mainWindow.updateMap();
      // Now get route from Map with many more coords...
      downLoadRoute();
   }

   public synchronized void loadBoxFile(String filename)
         throws FileNotFoundException, IOException, FileReadException {
      BufferedReader reader = new BufferedReader(new FileReader(filename));

      String line = null;
      while ((line = reader.readLine()) != null) {
         StringTokenizer st = new StringTokenizer(line, ",");
         if (st.countTokens() != 4 && !line.equals("")) {
            System.out.println("Simulator.loadBoxFile() Number of tokens = " + st.countTokens());
            throw new FileReadException();
         } else if (!line.equals("")) {
            String s1 = st.nextToken();
            String s2 = st.nextToken();
            String s3 = st.nextToken();
            String s4 = st.nextToken();
            //System.out.println(s1+" "+s2+" "+s3+" "+s4);
            s1 = s1.substring(2);
            s2 = s2.substring(0, s2.length() - 1);
            s3 = s3.substring(1);
            s4 = s4.substring(0, s4.length() - 2);
            try {
               int y1 = Integer.parseInt(s1);
               int x1 = Integer.parseInt(s2);
               int y2 = Integer.parseInt(s3);
               int x2 = Integer.parseInt(s4);
               //System.out.println(y1+" "+x1+" "+y2+" "+x2);
               Rectangle r = new Rectangle(x1, y1, x2, y2);
               mapComponent.addBox(r);
            } catch (NumberFormatException e) {
               System.out.println("Simulator.loadBoxFile() NumberFormatException: " + e.getMessage());
               throw new FileReadException();
            }
         }

      }
   }

   public synchronized void saveBoxFile(String filename)
         throws FileNotFoundException, IOException {
      PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(
            filename)));

      for (int i = 0; i < mapComponent.getNbrBoxes(); i++) {
         Rectangle r = mapComponent.getBox(i);
         writer.println("[(" + r.y + "," + r.x + "),(" + r.height + ","
               + r.width + ")]\n");
      }

      writer.close();
   }

   public synchronized void loadCoordFile(String filename)
         throws FileNotFoundException, IOException {
      BufferedReader reader = new BufferedReader(new FileReader(filename));

      mapComponent.resetCoordPoiForLoad();
      mainWindow.updateMap();
      String line = null;
      while ((line = reader.readLine()) != null) {
         StringTokenizer st = new StringTokenizer(line, " ");
         if (st.countTokens() >= 3) {
            String s1 = st.nextToken();
            String s2 = st.nextToken();
            String s3 = st.nextToken();
            //System.out.println(s1 + " " + s2 + " " + s3);
            while (st.hasMoreTokens())
               s3 += " " + st.nextToken();
            mapComponent
                  .addCoordPoiPoint(Integer.parseInt(s2), Integer.parseInt(s1));
            mapComponent.addCoordPoiComment(s3);
         }
      }
      System.out.println("Simulator.loadCoordFile() Nbr poiPoints = " + mapComponent.getNbrCoordPoiPoints());
      /**
      for (int i=0; i<mapComp.getNbrPoiPoints(); i++)
         System.out.println(mapComp.getPoiPoint(i).x + " " + mapComp.getPoiPoint(i).y);*/
   }

   public synchronized void saveCoordFile(String filename)
         throws FileNotFoundException, IOException {
      PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(
            filename)));

      for (int i = 0; i < mapComponent.getNbrCoordPoiPoints(); i++)
         writer.println(mapComponent.getCoordPoiPoint(i).y + " "
               + mapComponent.getCoordPoiPoint(i).x + " "
               + mapComponent.getCoordPoiComment(i) + "\n");

      writer.close();
      System.out.println("Simulator.loadCoordFile() Nbr poiPoints = " + mapComponent.getNbrCoordPoiPoints());
      /**
      for (int i=0; i<mapComp.getNbrPoiPoints(); i++)
         System.out.println(mapComp.getPoiPoint(i).x + " " + mapComp.getPoiPoint(i).y);*/
   }

   public synchronized void saveFile(String filename) throws IOException {
      File savefile;
      FileOutputStream foutstream;
      PrintStream out;
      String data;
      //try{
      savefile = new File(filename);
      foutstream = new FileOutputStream(savefile);
      out = new PrintStream(foutstream);
      for (int i = 0; i < routeSize; i++) {
         float lat = (float) loadedRoute[i].getLat();
         float lon = (float) loadedRoute[i].getLon();
         double bearing = loadedRoute[i].getBearing() * Math.PI / 180;
         double speed = loadedRoute[i].getSpeed() * 0.51444444;
         float x_speed = (float) (speed * Math.sin(bearing));
         float y_speed = (float) (speed * Math.cos(bearing));
         if ((x_speed < 0.001) && (x_speed > -0.001))
            x_speed = 0;
         if ((y_speed < 0.001) && (y_speed > -0.001))
            y_speed = 0;

         data = lat + " " + lon + " " + x_speed + " " + y_speed;
         out.println(data);
      }
      out.close();
      System.out.println("Simulator.saveFile() Wrote to file : " + filename);
   }

   private NMEAMessage parseCoordString(String data) {
      // Uh! Strings are sent from the MapWindow to this
      // Hack up the string
      int nextSpace;
      String[] dataPart = new String[4];
      for (int j = 1; j < 4; j++) {
         nextSpace = data.indexOf(' ');
         if ((nextSpace > 16) || (nextSpace < 0)) {
            return null;
         }
         dataPart[j - 1] = data.substring(0, nextSpace);
         data = data.substring(nextSpace + 1);
      }
      dataPart[3] = data;
      // Parse the bits to floats
      float lat = Float.parseFloat(dataPart[0]);
      float lon = Float.parseFloat(dataPart[1]);
      float sp_lat = Float.parseFloat(dataPart[2]);
      float sp_lon = Float.parseFloat(dataPart[3]);

      //Create the RMC message.
      NMEAMessage message = new NMEAMessage(gpsSig);
      if (message.setRMCMessage(lat, lon, sp_lat, sp_lon))
         return message;

      return null;
   }

   public void loop() {
      double oldLat = latitude;
      double oldLon = longitude;
      // The loop
      while (true) {

         if (((getLoadedRoute() != null) || (getState() == Simulator.WILD))) {

            NMEAMessage toSend = null;

            switch (getState()) {
            case Simulator.PAUSE:
               // If there were a setstate function...
               closeReader();
               // Pause in route
               toSend = getCurrentMessage();
               sendMessage(toSend);
               setSpeed(0);
               //simWin.repaint();
               break;
            case Simulator.FORWARD:
               closeReader();
               // Running forward in loaded route
               toSend = getCurrentMessage();
               sendMessage(toSend);
               if (getCurrentFrame() < getRouteSize() - 1)
                  stepForward(); //++currentFrame;
               mainWindow.update();
               if (getCurrentFrame() == getRouteSize() - 1
                     && getRepeatMode() != Simulator.NO_REPEAT) {
                  if (getRepeatMode() == Simulator.REPEAT_LOOP)
                     setCurrentFrame(0); //currentFrame = 0;
                  else
                     //  "Reverse"
                     setState(Simulator.REVERSE);
               }
               //simWin.repaint();
               break;
            case Simulator.REVERSE:
               closeReader();
               // Reversing in loaded route.
               toSend = getCurrentMessage();
               sendMessage(toSend);
               if (getCurrentFrame() > 0)
                  stepBackward(); //--currentFrame;
               mainWindow.update();
               if (getCurrentFrame() == 0
                     && getRepeatMode() != Simulator.NO_REPEAT) {
                  if (getRepeatMode() == Simulator.REPEAT_LOOP)
                     setCurrentFrame(getRouteSize() - 1);
                  else
                     //  "Reverse"
                     setState(Simulator.FORWARD);
               }

               //simWin.repaint();
               break;
            case Simulator.WILD:
               closeReader();

               // Also send messages other than RMC, since RMC only did not show up in java client.
               toSend = getCurrentMessage();

               // Replace the RMC with the custom position.
               if (toSend.isRMC()) {
                  toSend = createMessage();
                  if (toSend != null) {
                     if (getSpeed() <= 0) {
                        toSend.setBearing(getBearing());
                     }
                  }
               }

               // Taken from section FORWARD
               sendMessage(toSend);
               if (getCurrentFrame() < getRouteSize() - 1)
                  stepForward(); //++currentFrame;
               mainWindow.update();
               if (getCurrentFrame() == getRouteSize() - 1
                     && getRepeatMode() != Simulator.NO_REPEAT) {
                  if (getRepeatMode() == Simulator.REPEAT_LOOP)
                     setCurrentFrame(0); //currentFrame = 0;
                  else
                     //  "Reverse"
                     setState(Simulator.REVERSE);
               }
               break;
            case Simulator.PORT:
               // Reading from stream
               // Handled outside.
               break;
            }
            if (getCurrentFrame() == getRouteSize()) {
               stepBackward();
            }
            if (getCurrentFrame() < 0) {
               setCurrentFrame(0);
            }
         }

         long waitTime = 1000;

         if (getState() == Simulator.PORT) {
            waitTime = 0;
            // Reading from stream
            try {
               System.err.println("Simulator.loop() Will read from port");
               sendMessage(new NMEAMessage(nmeaReader.readLine()));
            } catch (IOException e) {
               new ErrorDialog(mainWindow.getFrame(), "While reading " + e).show();
            }
         }

         if (getRouteSize() != 0 && getCurrentFrame() < (getRouteSize() - 1)) {
            waitTime = getMessage(getCurrentFrame() % getRouteSize())
                  .getTimeFromPrev();
            waitTime = (int) Math.rint(waitTime / getSpeedFactor());
         }

         //System.err.println( "Will wait " + waitTime );
         try {
            // Wait 0 will wait forever.
            if (waitTime != 0) {
               Thread.sleep(waitTime);
            } else {
               Thread.sleep(1);
            }
         } catch (InterruptedException ie) {
            System.err.println("Simulator.loop() Interrupted! Writing beautiful output");
            System.err.println("Simulator.loop() Caught: " + ie);
         }
         //System.err.println( "Waiting ends " + waitTime );
      }
   }

   private void sendMessage(NMEAMessage message) {
      if (message == null)
         return;

      if (logWriter != null) {
         long time = System.currentTimeMillis();
         String[] strings = Util.splitStr(message.toString(), "\r");
         for (int i = 0; i < strings.length; ++i) {
            String[] newStr = Util.splitStr(strings[i], "\n");
            for (int j = 0; j < newStr.length; ++j) {
               if (newStr[j].length() != 0) {
                  logWriter.print("" + time + " " + newStr[j] + "\r\n");
               }
            }
         }
         logWriter.flush();
      }

      if (message.isValidRMC()) {
         if (getState() == Simulator.PAUSE && getCurrentFrame() != 0) {
            mapComponent.updateVehicle(message.getLat(), message.getLon(), message
                  .getSpeed(), message.getBearing(), GPSOn());
            mainWindow.drawVehicle(null, true);
         } else {
            mainWindow.drawVehicle(null, true);
            mapComponent.updateVehicle(message.getLat(), message.getLon(), message
                  .getSpeed(), message.getBearing(), GPSOn());
            mainWindow.drawVehicle(null, true);
         }

         /**
         mapComp.updateVehicle(message.getLat(),message.getLon(),message.getSpeed(),message.getBearing(),GPSOn());
         if (getState() != Simulator.PAUSE)
            simWin.repaint();
          */
      }

      if ((getState() != Simulator.WILD) && message.isRMC()) {
         setSpeed(0.51444444 * message.getSpeed() * getSpeedFactor());
         setBearing((int) Math.round(message.getBearing()));
         if (getState() == Simulator.FORWARD) {
            accumSpeed += getSpeed();
            speedQueue.addFirst(new Double(getSpeed()));
            if (speedQueue.size() > routeSize / 10)
               accumSpeed -= ((Double) speedQueue.removeLast()).doubleValue();
            setAvgSpeed(accumSpeed / (double) speedQueue.size());
            //System.out.println("Avg speed = " + 3.6*getAvgSpeed());
         }
      }

      /** Send message if GPS is switched on */
      if (GPSOn()) {
         double speed = 0.0;
         if (message.isValidRMC() && getState() != Simulator.WILD) {
            speed = message.getSpeed();
            message.setSpeed(speed * getSpeedFactor());
         }
         if (getTunnelMode()) {
            if (message.isRMC()) {
               message.setReceiverWarning();
            }
         }

         senderManager.sendMessage(message);

         if (message.isValidRMC() && getState() != Simulator.WILD)
            message.setSpeed(speed);
      }
   }

   public synchronized NMEAMessage createMessage() {
      // Create a NMEAMessage when running wild.
      double bearing = getBearing() * Math.PI / 180;
      double x_speed = getSpeed() * Math.sin(bearing);
      double y_speed = getSpeed() * Math.cos(bearing);
      if ((x_speed < 0000.1) && (x_speed > -0000.1))
         x_speed = 0.0;
      if ((y_speed < 0000.1) && (y_speed > -0000.1))
         y_speed = 0.0;
      NMEAMessage message = new NMEAMessage(getLat(), getLon(), x_speed,
            y_speed);
      //String coordString = (float) getLat() +" "+(float) getLon() +" "
      //   +x_speed+" "+y_speed;
      // System.out.println(coordString);
      // Update coords.
      double latInRads = getLat() * Math.PI / 180;
      double coslat = Math.cos(latInRads);
      double meterTocoord = 1.0 / 111320.0;
      double timeFactor = (double) getDelayTime() / 1000.0;
      //System.out.println(meterTocoord+" "+y_speed);
      setLat(getLat() + meterTocoord * y_speed * timeFactor);
      setLon(getLon() + meterTocoord * x_speed * timeFactor / coslat);
      return message;
   }

   public synchronized void savePort(String port) {
      paramFile.setDefaultPort(port);
      paramFile.save();
   }

   public synchronized void saveServer(String address) {
      paramFile.setServer(address);
      paramFile.save();
   }

   public synchronized void saveRepeatMode(int mode) {
      paramFile.setRepeatMode(mode);
      paramFile.save();
   }

   /**
    * @param host supply null if you only want to save the socket port.
    * @param port
    */
   public synchronized void saveSocket(String host, int port) {
      if (host != null) {
         paramFile.setSocketHost(host);
      }
      paramFile.setSocketPort(port);
      paramFile.save();
   }

}
