/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.FileDialog;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Scrollbar;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.AdjustmentEvent;
import java.awt.event.AdjustmentListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.text.DecimalFormat;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JRadioButton;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.border.TitledBorder;

/**
 *  Main interface window.
 */
class MainSimulatorWindow implements KeyListener, AdjustmentListener,
      MouseMotionListener, MouseListener {

   private String routeName;
   private boolean wildBreaking;
   private int screenHeight;
   private int screenWidth;
   //private MapWindow mapWindow;
   private LogWindow logWindow;
   private JButton fwdButton;
   private JButton pauseButton;
   private JButton revButton;
   private JButton stopButton;
   private JButton ffwButton;
   private JButton rewButton;
   private JButton incSpeedButton;
   private JButton decSpeedButton;
   //private JPanel slidePanel;
   private JScrollBar scrollbar;
   private JLabel scrollLabel;
   private String srcName = "";
   private SenderStatusComponent senderStatus;
   private ParamFile paramFile;
   private Simulator sim;
   private MapComponent mapComponent;
   private CloseableFrame frame;
   private Thread simThread;
   private SPDesc spDesc;

   private JPopupMenu popup;

   private JMenuItem loadRoute;
   private JMenuItem saveRoute;
   private JMenuItem stopLogging;
   private JMenuItem loadCoords;
   private JMenuItem saveCoords;
   private JMenuItem loadBoxes;
   private JMenuItem saveBoxes;
   private JMenuItem loadNMEA;
   private JMenuItem buildRouteFromId;
   private JMenuItem repeatMode;
   private JMenuItem filter;
   private JRadioButtonMenuItem original;
   private JRadioButtonMenuItem mercator;

   private JPanel mapToolPanel;
   private JPanel leftMapToolPanel;
   private JPanel controlPanel;
   private JPanel mainPanel;
   private JPanel leftPanel;
   private JPanel mapAreaPanel;
   private JPanel statusPanel;
   private JPanel msgPanel;
   private JPanel textPanel;
   private String state;
   private String oldState;
   private int xCoordinate;
   private int yCoordinate;
   private int side;
   private double xyRatio;
   private int mapHeight;
   private int mapWidth;
   private int first_x;
   private int first_y;
   private int mouse_x;
   private int mouse_y;
   private int top_x;
   private int top_y;
   private int current_x;
   private int current_y;
   private int boxTop_x;
   private int boxTop_y;
   private JButton buildButton;
   private JButton routeButton;
   private JButton contButton;
   private JButton loadButton;
   private JButton traffButton;
   private JButton zoomrButton;
   private JButton hideButton;
   private JButton eraseBoxButton;
   private JButton eraseAllBoxesButton;
   private JRadioButton trackON;
   private JRadioButton trackOFF;
   private JRadioButton tunnelModeON;
   private JRadioButton tunnelModeOFF;
   private boolean hide = false;
   private boolean track = false;
   private ImagePanel mapPanel;
   private JTextArea text = new JTextArea();
   private JScrollPane scrollPane = new JScrollPane(text);
   private JTextField textField = new JTextField();

   private JLabel speedLabel = new JLabel(" Speed: 0 km/h");
   private JLabel speedFactorLabel = new JLabel(" Speed Factor: 1.0");
   private JLabel distanceLabel = new JLabel(" Route Distance: 0 km");
   private JLabel distanceLeftLabel = new JLabel(" Distance To Finish: 0 km");
   private JLabel timeLabel = new JLabel(" Time To Finish: 00:00:00");
   private JLabel bearingLabel = new JLabel(" Bearing: 0");
   private JLabel repeatLabel = new JLabel(" No Repeat");
   private JLabel sendingLabel = new JLabel(" Sending Status: NOT SENDING");
   private JLabel latLabel = new JLabel("LAT: 55.718 N");
   private JLabel lonLabel = new JLabel("LON: 13.190 E");

   private static final int maxMapHeight = 1024;
   private static final int maxMapWidth = 1024;

   private class ImagePanel extends JPanel {

      public ImagePanel() {
      }

      public void paintComponent(Graphics g) {
         //System.out.println("Painting");
         super.paintComponent(g);
         int height = this.getSize().height;
         int width = this.getSize().width;
         //if (img != null) { //there is a picture: draw it
         if (mapComponent != null && mapComponent.drawMap(g, width, height, this)) {
            drawRoutePoints(g);
            drawStartEndPoints(g);
            drawPoiPoints(g);
            drawBoxes(g);
            drawVehicle(g, true);
            drawMarkedPoint(g);
         }
      }
   }

   /**
    *   Component that shows the status of the sender.
    */
   private class SenderStatusComponent extends JPanel {
      private JLabel label;
      
      SenderStatusComponent() {
         setLayout(new GridLayout(1, 1));
         //setMinimumSize( new Dimension( 0, 80 ) );
         //setPreferredSize( new Dimension( 80, 80 ) );

         label = new JLabel();
         add(label);

         setStatus(false, "");
      }

      void setStatus(boolean sending, String statusMsg) {
         if (statusMsg.length() == 0) {
            if (sending) {
               label.setText("Sending data");
            } else {
               label.setText("NOT SENDING");
            }
         } else {
            label.setText(statusMsg);
         }
      }
   }

   private void setAllDims(JComponent component, Dimension size,
         Dimension maxsize) {
      component.setPreferredSize(size);
      component.setMinimumSize(size);
      component.setMaximumSize(maxsize);
   }

   public MainSimulatorWindow(String title) {
      final String fname = "MainSimulatorWindow.MainSimulatorWindow() ";

      mainPanel = new JPanel();
      leftPanel = new JPanel();
      mapToolPanel = new JPanel();
      mapAreaPanel = new JPanel();
      statusPanel = new JPanel();
      msgPanel = new JPanel();
      textPanel = new JPanel();
      frame = new CloseableFrame("New window");
      frame.getContentPane().setLayout(new BorderLayout());
      frame.setFocusable(true);
      text.setRows(40);
      text.setFont(new Font("Default", Font.PLAIN, 8));
      speedLabel.setFont(new Font("Tahoma", Font.BOLD, 14));
      speedFactorLabel.setFont(new Font("Tahoma", Font.BOLD, 14));
      distanceLabel.setFont(new Font("Tahoma", Font.BOLD, 14));
      distanceLeftLabel.setFont(new Font("Tahoma", Font.BOLD, 14));
      timeLabel.setFont(new Font("Tahoma", Font.BOLD, 14));
      bearingLabel.setFont(new Font("Tahoma", Font.BOLD, 14));
      repeatLabel.setFont(new Font("Tahoma", Font.BOLD, 14));
      sendingLabel.setFont(new Font("Tahoma", Font.BOLD, 14));
      //scrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
      //scrollPane.setPreferredSize(new Dimension(250, 250));
      //frame.getContentPane().setLayout( new GridLayout(3,1) );

      Toolkit tk = Toolkit.getDefaultToolkit();
      Dimension d = tk.getScreenSize();
      screenHeight = d.height;
      screenWidth = d.width;
      mapHeight = Math.min(d.height * 2 / 3, maxMapHeight);
      mapWidth = Math.min(d.width * 2 / 3, maxMapWidth);
      xyRatio = ((double) mapHeight) / ((double) mapWidth);

      // Show a wait message to reduce the risk of the user to clicking on something until the simulator object has been created.
      JFrame startupMessage = new JFrame("Startup Message");
      JPanel startupPanel = new JPanel();
      startupPanel.add(new JLabel(
            "Please wait while the BTGPSSimulator is starting..."));
      startupMessage.getContentPane().add("Center", startupPanel);
      startupMessage.pack();
      startupMessage.setLocation(d.width * 3 / 8 + 50, d.height * 3 / 8 - 100);
      startupMessage.setSize(350, 60);
      startupMessage.setAlwaysOnTop(true);
      startupMessage.setVisible(true);

      createMenues();
      createControls();

      statusPanel.setLayout(new BoxLayout(statusPanel, BoxLayout.Y_AXIS));
      statusPanel.add(speedLabel);
      statusPanel.add(speedFactorLabel);
      statusPanel.add(distanceLabel);
      statusPanel.add(distanceLeftLabel);
      statusPanel.add(timeLabel);
      statusPanel.add(bearingLabel);
      statusPanel.add(repeatLabel);
      statusPanel.add(sendingLabel);
      leftPanel.add(statusPanel, BorderLayout.CENTER);
      leftPanel.add(textPanel, BorderLayout.SOUTH);

      textPanel.setLayout(new BorderLayout());
      textPanel.add(scrollPane, BorderLayout.CENTER);
      textPanel.add(textField, BorderLayout.SOUTH);

      mapPanel = new ImagePanel();
      mapPanel.setPreferredSize(new Dimension(mapWidth, mapHeight));

      frame.getContentPane().add(leftPanel, BorderLayout.WEST);
      frame.getContentPane().add(mainPanel, BorderLayout.CENTER);

      mainPanel.setLayout(new BorderLayout());
      mainPanel.add(mapAreaPanel, BorderLayout.NORTH);
      mainPanel.add(mapToolPanel, BorderLayout.CENTER);

      mapAreaPanel.add(mapPanel);

      frame.setLocation(screenWidth * 3 / 8, screenHeight * 1 / 2);
      frame.pack();
      frame.setResizable(false);

      Dimension fd = frame.getSize();
      frame.setLocation((d.width - fd.width) / 2, (d.height - fd.height) / 2);
      frame.setVisible(true);

      senderStatus = new SenderStatusComponent();
      //frame.getContentPane().add(senderStatus);

      paramFile = new ParamFile("param.txt");
      paramFile.load();

      NMEAMessage
            .setDefaultTimeFromPrev(paramFile.getDefaultSendTimeInterval());

      if (paramFile.getRepeatMode() == Simulator.NO_REPEAT) {
         repeatLabel.setText(" No Repeat");
      } else if (paramFile.getRepeatMode() == Simulator.REPEAT_LOOP) {
         repeatLabel.setText(" Repeat Loop");
      } else if (paramFile.getRepeatMode() == Simulator.REPEAT_REVERSE) {
         repeatLabel.setText(" Repeat Reverse");
      }

      wildBreaking = false;
      state = "Move Window";
      oldState = "Move Window";

      //mapComp = new OldMapComponent(mapWidth,mapHeight);
      spDesc = new SPDesc(paramFile.getServer());
      mapComponent = new MercatorMapComponent(mapWidth, mapHeight, paramFile
            .getServer(), spDesc, mapPanel);
      mapComponent.setXYRatio(xyRatio);
      mapComponent.setDelayTime(1000);
      mapComponent.moveCenter(paramFile.getCentralX(), paramFile.getCentralY()); // Wayfinder
      System.out.println(fname + "mapComp.moveCenter() OK.");

      try {
         mapComponent.loadMap();
      } catch (Exception e) {
      }
      //System.out.println("CPoint = (" + mapComp.getCPoint().x + "," + mapComp.getCPoint().y + ")");
      // Enable keys in the map window.

      logWindow = new LogWindow(getFrame(), "RouteLog");
      System.out.println(fname + "logWindow OK.");

      /* 
         we will block until BT is up and running. But the rest of the
         UI is still alive and especially the menues. So if the user
         invokes "load route", loadNavFile() will be called and crash
         with an NPE since sim is still null.
       */
      sim = new Simulator(this, mapComponent, paramFile);
      System.out.println(fname + "sim OK, " + sim);

      simThread = new Thread() {
         public void run() {
            while (!isInterrupted())
               sim.loop();
         }
      };

      simThread.start();
      System.out.println(fname + "sim " + sim);
      /**
      new Thread() {
         public void run() {
            sim.loop();
         }
      }.start();*/

      frame.addKeyListener(this);
      mapPanel.addMouseMotionListener(this);
      mapPanel.addMouseListener(this);
      updateMap();

      startupMessage.setVisible(false);
      frame.requestFocus();

   } // ctor

   /**
   public void repaint() {
      mapPanel.repaint();
   }*/

   public CloseableFrame getFrame() {
      return frame;
   }

   public void printText(String s) {
      text.insert(s + "\n", 0);
   }

   public void setStatus(boolean sending, String statusMsg) {
      //senderStatus.setStatus(sending,statusMsg);
      if (sending)
         sendingLabel.setText(" Sending Status: SENDING");
      else
         sendingLabel.setText(" Sending Status: NOT SENDING");

   }

   public void downloadRoute() {
      sim.downLoadRoute();
      saveRoute.setEnabled(true);
      scrollbar.setMaximum(sim.getRouteSize() - 1);
      scrollbar.setValue(sim.getCurrentFrame());
      sim.setState(Simulator.PAUSE);
      //repaint();
   }

   public void enableLoadButton(boolean en) {
      loadButton.setEnabled(en);
   }

   public void update() {
      scrollbar.setValue(sim.getCurrentFrame());

      int size = sim.getRouteSize() - 1;
      String paddString = "";
      if (sim.getCurrentFrame() < 1000) {
         paddString += "  ";
         if (sim.getCurrentFrame() < 100) {
            paddString += "  ";
            if (sim.getCurrentFrame() < 10)
               paddString += "  ";
         }
      }
      if (size < 1000) {
         paddString += "  ";
         if (size < 100) {
            paddString += "  ";
            if (size < 10)
               paddString += "  ";
         }
      }
      if (size < 0)
         size = 0;
      int speed = (int) Math.round(sim.getSpeed() * 3.6);
      if (sim.getState() == Simulator.PAUSE)
         speed = 0;
      String speedPad = "";
      if (speed < 100) {
         speedPad += "  ";
         if (speed < 10)
            speedPad += "  ";
      }

      String bearPad = "";
      if (sim.getBearing() < 100) {
         bearPad += "  ";
         if (sim.getBearing() < 10)
            bearPad += "  ";
      }

      String timeString = "";
      int current = sim.getCurrentFrame();
      if ((int) sim.getAvgSpeed() != 0) {
         int time = (int) Math.round((sim.getDistance(current) * 1000.0)
               / sim.getAvgSpeed());
         int hour = time / 3600;
         if (hour < 10)
            timeString += "0";
         timeString = timeString + hour + ":";
         time -= hour * 3600;
         int min = time / 60;
         if (min < 10)
            timeString += "0";
         timeString = timeString + min + ":";
         time -= min * 60;
         if (time < 10)
            timeString += "0";
         timeString += time;
      } else
         timeString = "--:--:--";

      DecimalFormat decFormat = new DecimalFormat("0.0");
      scrollLabel.setText(paddString + current + "/" + size);
      speedLabel.setText(" Speed: " + speed + " km/h");
      bearingLabel.setText(" Bearing: " + sim.getBearing());
      distanceLeftLabel.setText(" Distance To Finish: "
            + decFormat.format(sim.getDistance(current)) + " km");
      timeLabel.setText(" Time To Finish: " + timeString);

   }

   private void createMenues() {

      popup = new JPopupMenu();

      JMenuItem setStart = new JMenuItem("Set as start");
      setStart.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            mapComponent.setStartPoint(mapComponent.xToMc2Lon(mouse_x), mapComponent
                  .yToMc2Lat(mouse_y));
            updateMap();
            if (mapComponent.getStartPoint() != null
                  && mapComponent.getDestPoint() != null)
               buildButton.setEnabled(true);
         }
      });

      JMenuItem setDest = new JMenuItem("Set as destination");
      setDest.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            mapComponent.setDestPoint(mapComponent.xToMc2Lon(mouse_x), mapComponent
                  .yToMc2Lat(mouse_y));
            updateMap();
            if (mapComponent.getStartPoint() != null
                  && mapComponent.getDestPoint() != null)
               buildButton.setEnabled(true);
         }
      });

      JMenuItem searchCoord = new JMenuItem("Search Coordinate");
      searchCoord.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            searchCoord(mouse_x, mouse_y);
         }
      });

      popup.add(setStart);
      popup.add(setDest);
      popup.add(searchCoord);

      // Menues
      JMenuBar mb = new JMenuBar();

      JMenu fileMenu = new JMenu("File");

      loadRoute = new JMenuItem("Load Route", KeyEvent.VK_O);
      loadRoute.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evO) {
            loadNavFileMenu();
         }
      });

      saveRoute = new JMenuItem("Save Route");
      saveRoute.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            saveNavFileMenu();
         }
      });
      saveRoute.setEnabled(false);

      JMenu advanced = new JMenu("Advanced");

      /**
      JMenuItem load = new JMenuItem("Load Route",KeyEvent.VK_L);
      load.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evO)
               { loadFileMenu();}
         });
      load.setEnabled(false);
      
      
      saveRoute = new JMenuItem("Save Route",KeyEvent.VK_S);
      saveRoute.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent evO)
               { saveFileMenue();}            
         });
      saveRoute.setEnabled(false);*/

      buildRouteFromId = new JMenuItem("Build Route from ID");
      buildRouteFromId.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            String s = JOptionPane.showInputDialog("Enter Route ID:");
            if (s != null) {
               sim.setState(Simulator.PAUSE);
               System.out.println(".createMenues() buildroutefromid actionperformed()");
               sim.buildRoute(0, 0, 0, 0, 0, s);
               DecimalFormat decFormat = new DecimalFormat("0.0");
               System.out.println("Loading done!");
               scrollbar.setMaximum(sim.getRouteSize() - 1);
               scrollbar.setValue(sim.getCurrentFrame());
               distanceLabel.setText(" Route Distance: "
                     + decFormat.format(sim.getDistance(0)) + " km");
               sim.reset();
               sim.setRepeatMode(paramFile.getRepeatMode());
               filter.setEnabled(true);
               saveRoute.setEnabled(true);
            }
         }
      });

      loadNMEA = new JMenuItem("Load NMEA Data", KeyEvent.VK_1);
      loadNMEA.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ev) {
            loadNMEAFile();
         }
      });

      JMenuItem readCOM = new JMenuItem("Read from COM-port", KeyEvent.VK_2);
      readCOM.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ev) {
            setReadFromPort();
         }
      });

      JMenuItem setLogFile = new JMenuItem("Set Log File", KeyEvent.VK_3);
      setLogFile.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ev) {
            chooseLogFileName();
         }
      });

      stopLogging = new JMenuItem("Stop Logging", KeyEvent.VK_4);
      stopLogging.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ev) {
            stopLogging();
         }
      });
      stopLogging.setEnabled(false);

      loadCoords = new JMenuItem("Load Coordinate File", KeyEvent.VK_X);
      loadCoords.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evC) {
            loadCoordsToPlot();
         }
      });

      saveCoords = new JMenuItem("Save Coordinate File");
      saveCoords.setEnabled(false);
      saveCoords.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evC) {
            saveCoords();
         }
      });

      loadBoxes = new JMenuItem("Load Box File");
      loadBoxes.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            loadBoxFile();
         }
      });

      saveBoxes = new JMenuItem("Save Box File");
      saveBoxes.setEnabled(false);
      saveBoxes.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evC) {
            saveBoxFile();
         }
      });

      advanced.add(buildRouteFromId);
      advanced.add(loadNMEA);
      advanced.add(readCOM);
      advanced.add(setLogFile);
      advanced.add(saveCoords);
      advanced.add(loadCoords);
      advanced.add(saveBoxes);
      advanced.add(loadBoxes);

      JMenuItem exit = new JMenuItem("Exit", KeyEvent.VK_C);
      exit.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evC) {
            System.exit(1);
         }
      });

      fileMenu.add(loadRoute);
      fileMenu.add(saveRoute);
      fileMenu.add(advanced);
      fileMenu.add(exit);

      JMenu routeMenu = new JMenu("Route");

      JMenuItem autoZoom = new JCheckBoxMenuItem("Auto Zoom to Route");
      autoZoom.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            if (((JMenuItem) (e.getSource())).isSelected()) {
               sim.setAutoZoom(true);
            } else {
               sim.setAutoZoom(false);
            }
         }
      });
      autoZoom.setSelected(true);
      routeMenu.add(autoZoom);

      repeatMode = new JMenuItem("Set Repeat Mode", KeyEvent.VK_R);
      repeatMode.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evG) {
            setRepeat();
         }
      });
      routeMenu.add(repeatMode);

      filter = new JMenuItem("Distortion Filter");
      filter.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            FilterDialog fd = new FilterDialog(getFrame());
            fd.setLocation(screenWidth * 4 / 9, screenHeight * 1 / 4);
            fd.show();
            System.out.println(fd.getDistribution());
            if (fd.getDistribution().equals("Rectangular"))
               sim.applyRectFilter(fd.getValue());
            else if (fd.getDistribution().equals("Normal"))
               sim.applyNormalFilter(fd.getValue());
         }
      });
      routeMenu.add(filter);
      filter.setEnabled(false);

      JMenu menu2 = new JMenu("Connect");

      JMenuItem m21 = new JMenuItem("Select Port from List");
      m21.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evB) {
            System.out
                  .println("Menu option selected: Connect -> Select Port from List");
            sim.setPort();
         }
      });
      menu2.add(m21);

      JMenuItem m74 = new JMenuItem("Wait for Port Connection");
      m74.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evB) {
            System.out
                  .println("Menu option selected: Connect -> Wait for Port Connection");
            sim.setPortAutomatically();
         }
      });
      menu2.add(m74);

      menu2.addSeparator();

      JMenuItem m24 = new JMenuItem("Connect to Socket Host");
      m24.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evB) {
            System.out.println("Menu option selected: Connect -> Connect to Socket Host");
            sim.setSocket();
         }
      });
      menu2.add(m24);

      JMenuItem m25 = new JMenuItem("Wait for Socket Connection");
      m25.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evB) {
            System.out.println("Menu option selected: Connect -> Wait for Socket Connection");
            sim.setServerSocket();
         }
      });
      menu2.add(m25);

      menu2.addSeparator();

      JMenuItem setServer = new JMenuItem("Set Map/Route Server");
      setServer.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            System.out
                  .println("Menu option selected: Connect -> Set Map/Route Server");
            setServer();
         }
      });
      menu2.add(setServer);
      
      JMenu menu3 = new JMenu("View");

      JMenuItem m31 = new JMenuItem("Log Window");
      m31.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evG) {
            logWindow.show();
         }
      });
      m31.setEnabled(false);
      //menu3.add(m31);

      JMenuItem m32 = new JCheckBoxMenuItem("Advanced Controls");
      m32.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            if (((JMenuItem) (e.getSource())).isSelected()) {
               incSpeedButton.setVisible(true);
               decSpeedButton.setVisible(true);
            } else {
               incSpeedButton.setVisible(false);
               decSpeedButton.setVisible(false);
            }
         }
      });
      menu3.add(m32);

      JMenuItem viewPois = new JCheckBoxMenuItem("Points of Interest");
      viewPois.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            if (((JMenuItem) (e.getSource())).isSelected()) {
               mapComponent.setShowPoi(true);
            } else {
               mapComponent.setShowPoi(false);
            }
            try {
               mapComponent.loadMap();
            } catch (Exception ex) {
            }
            updateMap();
         }
      });
      menu3.add(viewPois);
      viewPois.setSelected(false);

      JMenu mapType = new JMenu("Map Type");
      menu3.add(mapType);

      ButtonGroup mapTypeGroup = new ButtonGroup();

      original = new JRadioButtonMenuItem("Original", false);
      original.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ae) {
            if (mapComponent instanceof MercatorMapComponent)
               changeMapType(mapComponent);
         }
      });
      mapType.add(original);

      mercator = new JRadioButtonMenuItem("Mercator", true);
      mercator.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ae) {
            if (mapComponent instanceof OldMapComponent)
               changeMapType(mapComponent);
         }
      });
      mapType.add(mercator);

      mapTypeGroup.add(original);
      mapTypeGroup.add(mercator);

      JMenu menu4 = new JMenu("Help");

      JMenuItem m41 = new JMenuItem("Help");
      m41.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evG) {
            displayHelp();
         }
      });
      m41.setEnabled(true);

      JMenuItem m42 = new JMenuItem("About");
      m42.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evG) {
            displayAbout();
         }
      });
      menu4.add(m41);
      menu4.add(m42);

      mb.add(fileMenu);
      mb.add(routeMenu);
      mb.add(menu2);
      mb.add(menu3);
      mb.add(menu4);
      frame.setJMenuBar(mb);
   }

   private void changeMapType(MapComponent comp) {
      if (comp instanceof OldMapComponent) {
         mapComponent = new MercatorMapComponent(comp);
      } else if (comp instanceof MercatorMapComponent)
         mapComponent = new OldMapComponent(comp);
      sim.setMapComponent(mapComponent);

      if (mapComponent.getNbrRoutePoints() > 0)
         mapComponent.zoomToRoute();

      try {
         mapComponent.loadMap();
      } catch (Exception e) {
      }
      updateMap();
   }

   private void createControls() {
      controlPanel = new JPanel();
      JPanel gpsPanel = new JPanel();
      JPanel modePanel = new JPanel();
      JPanel gpsAndModePanel = new JPanel();
      JPanel controlButtons = new JPanel();

      controlButtons.setLayout(new FlowLayout());
      TitledBorder title = BorderFactory.createTitledBorder(BorderFactory
            .createLineBorder(Color.black), " Simulation Control ");
      title.setTitleJustification(TitledBorder.CENTER);
      controlPanel.setBorder(title);
      controlPanel.setLayout(new BorderLayout());

      title = BorderFactory.createTitledBorder(BorderFactory
            .createLineBorder(Color.black), " GPS ");
      title.setTitleJustification(TitledBorder.CENTER);
      gpsPanel.setBorder(title);
      gpsPanel.setLayout(new FlowLayout());

      title = BorderFactory.createTitledBorder(BorderFactory
            .createLineBorder(Color.black), " Simulation Mode ");
      title.setTitleJustification(TitledBorder.CENTER);
      modePanel.setBorder(title);
      modePanel.setLayout(new FlowLayout());

      rewButton = new JButton(new ImageIcon("images/rew.gif"));
      rewButton.setMargin(new Insets(0, 0, 0, 0));
      //rewButton = new JButton("<<");
      controlButtons.add(rewButton);
      rewButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evS) {
            if (sim.getState() != Simulator.WILD) {
               sim.setState(Simulator.REVERSE);
               sim.setSpeedFactor(100.0);
               speedFactorLabel.setText(" Speed Factor: 100.0");
            }
            frame.requestFocus();
         }
      });

      revButton = new JButton(new ImageIcon("images/back.gif"));
      revButton.setMargin(new Insets(0, 0, 0, 0));
      //revButton = new JButton("<");
      controlButtons.add(revButton);
      revButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evR) {
            sim.setState(Simulator.REVERSE);
            if ((int) sim.getSpeedFactor() == 100) {
               sim.setSpeedFactor(1.0);
               speedFactorLabel.setText(" Speed Factor: 1.0");
            }
            frame.requestFocus();
         }
      });
      //revButton.addKeyListener(this);

      fwdButton = new JButton(new ImageIcon("images/play.gif", ""));
      fwdButton.setMargin(new Insets(0, 0, 0, 0));
      //fwdButton = new JButton(">");
      controlButtons.add(fwdButton);
      fwdButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evS) {
            sim.setState(Simulator.FORWARD);
            if ((int) sim.getSpeedFactor() == 100) {
               sim.setSpeedFactor(1.0);
               speedFactorLabel.setText(" Speed Factor: 1.0");
            }
            frame.requestFocus();
         }
      });
      //fwdButton.addKeyListener(this);

      pauseButton = new JButton(new ImageIcon("images/pause.gif", ""));
      pauseButton.setMargin(new Insets(0, 0, 0, 0));
      //pauseButton = new JButton("||");
      controlButtons.add(pauseButton);
      pauseButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evP) {
            sim.setState(Simulator.PAUSE);
            frame.requestFocus();
         }
      });
      //pauseButton.addKeyListener(this);

      stopButton = new JButton(new ImageIcon("images/stop.gif", ""));
      stopButton.setMargin(new Insets(0, 0, 0, 0));
      //stopButton = new JButton("[ ]");
      controlButtons.add(stopButton);
      stopButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evP) {
            sim.reset();
            speedFactorLabel.setText(" Speed Factor: 1.0");
            frame.requestFocus();
         }
      });

      ffwButton = new JButton(new ImageIcon("images/ffw.gif", ""));
      ffwButton.setMargin(new Insets(0, 0, 0, 0));
      //ffwButton = new JButton(">>");
      controlButtons.add(ffwButton);
      ffwButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evP) {
            sim.setState(Simulator.FORWARD);
            sim.setSpeedFactor(100.0);
            speedFactorLabel.setText(" Speed Factor: 100.0");
            frame.requestFocus();
         }
      });

      incSpeedButton = new JButton(new ImageIcon("images/plus.gif"));
      incSpeedButton.setMargin(new Insets(0, 0, 0, 0));
      controlButtons.add(incSpeedButton);
      incSpeedButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evR) {
            DecimalFormat decFormat = new DecimalFormat("0.0");
            double f = sim.getSpeedFactor();
            if (f <= 2.95) {
               sim.setSpeedFactor(f + 0.1);
               speedFactorLabel.setText(" Speed Factor: "
                     + decFormat.format(sim.getSpeedFactor()));
            }
            frame.requestFocus();
         }
      });
      //incSpeedButton.addKeyListener(this);
      incSpeedButton.setVisible(false);

      decSpeedButton = new JButton(new ImageIcon("images/minus.gif"));
      decSpeedButton.setMargin(new Insets(0, 0, 0, 0));
      controlButtons.add(decSpeedButton);
      decSpeedButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent evR) {
            DecimalFormat decFormat = new DecimalFormat("0.0");
            double f = sim.getSpeedFactor();
            if (f >= 0.55 && f <= 3.05) {
               sim.setSpeedFactor(f - 0.1);
               speedFactorLabel.setText(" Speed Factor: "
                     + decFormat.format(sim.getSpeedFactor()));
            }
            frame.requestFocus();
         }
      });
      //decSpeedButton.addKeyListener(this);
      decSpeedButton.setVisible(false);

      scrollbar = new JScrollBar(Scrollbar.HORIZONTAL, 0, 0, 0, 0);
      scrollLabel = new JLabel("0/0");
      controlPanel.add(controlButtons, BorderLayout.NORTH);
      controlPanel.add(scrollbar, BorderLayout.CENTER);
      controlPanel.add(scrollLabel, BorderLayout.SOUTH);
      scrollbar.setBlockIncrement(1);
      scrollbar.addAdjustmentListener(this);

      ButtonGroup gpsGroup = new ButtonGroup();

      JRadioButton gpsON = new JRadioButton("ON", false);
      JRadioButton gpsOFF = new JRadioButton("OFF", true);

      gpsON.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            sim.enableGPS(true);
            frame.requestFocus();
         }
      });

      gpsOFF.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            sim.enableGPS(false);
            sendingLabel.setText(" Sending Status: NOT SENDING");
            frame.requestFocus();
         }
      });

      gpsGroup.add(gpsON);
      gpsGroup.add(gpsOFF);

      gpsPanel.setLayout(new FlowLayout());
      gpsPanel.add(gpsON);
      gpsPanel.add(gpsOFF);

      ButtonGroup modeGroup = new ButtonGroup();

      JRadioButton route = new JRadioButton("Run Route", true);
      JRadioButton drive = new JRadioButton("Drive Free", false);

      route.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            sim.setState(Simulator.PAUSE);
            enableControl(true);
            sim.reset();
            update();
            hide = false;
            updateMap();
            frame.requestFocus();
         }
      });

      drive.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            sim.setState(Simulator.PAUSE);
            sim.setSpeedFactor(1.0);
            enableControl(false);
            hide = true;
            int n = JOptionPane.showConfirmDialog(frame,
                  "Do you want to place the vehicle on the map?", "",
                  JOptionPane.YES_NO_OPTION);
            if (n == JOptionPane.YES_OPTION) {
               oldState = state;
               state = "place";
            }
            sim.setState(Simulator.WILD);
            updateMap();
            frame.requestFocus();
         }
      });
      //drive.setEnabled(false);

      modeGroup.add(route);
      modeGroup.add(drive);

      modePanel.setLayout(new FlowLayout());
      modePanel.add(route);
      modePanel.add(drive);

      gpsAndModePanel.setLayout(new FlowLayout());
      gpsAndModePanel.add(gpsPanel);
      gpsAndModePanel.add(modePanel);

      leftPanel.setLayout(new BorderLayout());
      JPanel leftTopPanel = new JPanel();
      leftTopPanel.setLayout(new BorderLayout());
      leftTopPanel.add(controlPanel, BorderLayout.NORTH);
      leftTopPanel.add(gpsAndModePanel, BorderLayout.CENTER);
      leftPanel.add(leftTopPanel, BorderLayout.NORTH);

      leftMapToolPanel = new JPanel();
      GridLayout layout = new GridLayout(2, 2);
      layout.setVgap(10);
      layout.setHgap(10);
      leftMapToolPanel.setLayout(layout);

      buildButton = new JButton("Build Route");
      buildButton.setEnabled(false);
      leftMapToolPanel.add(buildButton);
      buildButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            Object[] modes = { "Distance", "Time",
                  "Time with traffic information" };
            String s = (String) JOptionPane.showInputDialog(frame,
                  "Optimization", "Build Route", JOptionPane.PLAIN_MESSAGE,
                  null, modes, "Time");
            if (s != null) {
               sim.setState(Simulator.PAUSE);
               int x1 = mapComponent.getStartPoint().x;
               int y1 = mapComponent.getStartPoint().y;
               int x2 = mapComponent.getDestPoint().x;
               int y2 = mapComponent.getDestPoint().y;
               int mode = 0;
               if (s.equals((String) modes[0]))
                  mode = 0;
               else if (s.equals((String) modes[1]))
                  mode = 1;
               else if (s.equals((String) modes[2]))
                  mode = 2;
               System.out.println(".createControls() build button actionPerformed()");
               sim.buildRoute(x1, y1, x2, y2, mode, null);
               DecimalFormat decFormat = new DecimalFormat("0.0");
               System.out.println("buildButton.actionPerformed() Loading done!");
               scrollbar.setMaximum(sim.getRouteSize() - 1);
               scrollbar.setValue(sim.getCurrentFrame());
               distanceLabel.setText(" Route Distance: "
                     + decFormat.format(sim.getDistance(0)) + " km");
               sim.reset();
               sim.setRepeatMode(paramFile.getRepeatMode());
               filter.setEnabled(true);
               saveRoute.setEnabled(true);
            }
         }
      });

      routeButton = new JButton("New Route");
      //leftMapToolPanel.add(routeButton);
      //routeButton.addKeyListener(this);
      routeButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ae) {
            state = "Build route";
            //nbrCoords = 0;
            //maxNbrPoints = 2000;
            //coords = new ArrayList(start_size);
            mapComponent.resetForLoad(10000);
            contButton.setEnabled(false);
            loadButton.setEnabled(false);
            updateMap();
            frame.requestFocus();
         }
      });

      zoomrButton = new JButton("Zoom to Route");
      leftMapToolPanel.add(zoomrButton);
      //zoomrButton.addKeyListener(this);
      zoomrButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ae) {
            mapComponent.zoomToRoute();
            try {
               mapComponent.loadMap();
            } catch (Exception me) {
               System.out.println("MalformedURLException");
               //System.out.println(urlString);
               ErrorDialog err = new ErrorDialog(getFrame(), "Map load error");
               err.addText("Was not able to find the map");
               err.addText("Check maphosts in param file.");
               err.setLocation(mapWidth * 3 / 8, mapHeight * 3 / 8);
               err.show();
               System.exit(1);
            }
            updateMap();
            frame.requestFocus();
         }
      });

      eraseBoxButton = new JButton("Erase Box");
      //leftMapToolPanel.add(eraseBoxButton);
      eraseBoxButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ae) {
            if (mapComponent.getNbrBoxes() > 0) {
               mapComponent.removeBox(mapComponent.getNbrBoxes() - 1);
               updateMap();
               if (mapComponent.getNbrBoxes() == 0) {
                  eraseBoxButton.setEnabled(false);
                  saveBoxes.setEnabled(false);
               }
            }
         }
      });

      contButton = new JButton("Continue Route");
      //leftMapToolPanel.add(contButton);
      //contButton.addKeyListener(this);
      //contButton.setEnabled(false);
      contButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ae) {
            state = "Build route";
            frame.requestFocus();
         }
      });

      loadButton = new JButton("Build Custom Route");
      //leftMapToolPanel.add(loadButton);
      //loadButton.addKeyListener(this);
      loadButton.setEnabled(false);
      loadButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ae) {
            state = "Build route";
            downloadRoute();
            DecimalFormat decFormat = new DecimalFormat("0.0");
            distanceLabel.setText(" Route Distance: "
                  + decFormat.format(sim.getDistance(0)) + " km");
            loadButton.setEnabled(false);
            frame.requestFocus();
         }
      });

      hideButton = new JButton("Hide Route");
      //hideButton.addKeyListener(this);
      hideButton.setEnabled(false);
      hideButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ae) {
            hide = !hide;
            if (hide) {
               hideButton.setLabel("Show Route");
            } else {
               hideButton.setLabel("Hide Route");
            }
            contButton.setEnabled(!hide);
            updateMap();
            frame.requestFocus();
         }
      });

      traffButton = new JButton("Traffic is Off");
      //traffButton.addKeyListener(this);
      traffButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ae) {
            mapComponent.setShowTraffic(!mapComponent.showTraffic());
            state = "None";
            if (mapComponent.showTraffic())
               traffButton.setLabel("Traffic is On");
            else
               traffButton.setLabel("Traffic is Off");
            try {
               mapComponent.loadMap();
            } catch (Exception me) {
               System.out.println("MalformedURLException");
               //System.out.println(urlString);
               ErrorDialog err = new ErrorDialog(getFrame(), "Map load error");
               err.addText("Was not able to find the map");
               err.addText("Check maphosts in param file.");
               err.setLocation(mapWidth * 3 / 8, mapHeight * 3 / 8);
               err.show();
               System.exit(1);
            }
            updateMap();
            frame.requestFocus();
         }
      });

      JPanel mapModePanel = new JPanel();
      title = BorderFactory.createTitledBorder(BorderFactory
            .createLineBorder(Color.black), " Map Mode ");
      title.setTitleJustification(TitledBorder.CENTER);
      mapModePanel.setBorder(title);

      ButtonGroup mapModeGroup = new ButtonGroup();

      JRadioButton zoomIn = new JRadioButton("Zoom In", false);
      JRadioButton zoomOut = new JRadioButton("Zoom Out", false);
      JRadioButton moveCenter = new JRadioButton("Move Center", true);
      JRadioButton coords = new JRadioButton("Coordinates", false);
      JRadioButton boxes = new JRadioButton("Boxes", false);
      JRadioButton customRoute = new JRadioButton("Custom Route", false);

      zoomIn.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            state = "Zoom in";
            setDefaultButtonConfig();
            frame.requestFocus();
         }
      });

      zoomOut.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            state = "Zoom out";
            setDefaultButtonConfig();
            frame.requestFocus();
         }
      });

      moveCenter.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            state = "Move Window";
            setDefaultButtonConfig();
            frame.requestFocus();
         }
      });

      coords.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            state = "Coords";
            setDefaultButtonConfig();
            frame.requestFocus();
         }
      });

      boxes.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            state = "Boxes";
            setDefaultButtonConfig();
            leftMapToolPanel.add(eraseBoxButton);
            if (mapComponent.getNbrBoxes() == 0)
               eraseBoxButton.setEnabled(false);
            frame.pack();
            frame.requestFocus();

         }
      });

      customRoute.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            state = "Build route";
            mapComponent.resetForLoad(10000);
            loadButton.setEnabled(false);
            sim.setRepeatMode(paramFile.getRepeatMode());
            updateMap();
            leftMapToolPanel.removeAll();
            leftMapToolPanel.add(loadButton);
            frame.pack();
            frame.requestFocus();
         }
      });

      mapModeGroup.add(zoomIn);
      mapModeGroup.add(zoomOut);
      mapModeGroup.add(moveCenter);
      mapModeGroup.add(coords);
      mapModeGroup.add(boxes);
      mapModeGroup.add(customRoute);

      mapModePanel.setLayout(new GridLayout(2, 3));
      mapModePanel.add(zoomIn);
      mapModePanel.add(zoomOut);
      mapModePanel.add(moveCenter);
      mapModePanel.add(coords);
      mapModePanel.add(boxes);
      mapModePanel.add(customRoute);

      JPanel trackPanel = new JPanel();
      title = BorderFactory.createTitledBorder(BorderFactory
            .createLineBorder(Color.black), " Tracking ");
      title.setTitleJustification(TitledBorder.CENTER);
      trackPanel.setBorder(title);

      ButtonGroup trackGroup = new ButtonGroup();

      trackON = new JRadioButton("ON", false);
      trackOFF = new JRadioButton("OFF", true);

      trackON.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            track = true;
            frame.requestFocus();
         }
      });

      trackOFF.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            track = false;
            frame.requestFocus();
         }
      });

      trackGroup.add(trackON);
      trackGroup.add(trackOFF);

      trackPanel.setLayout(new FlowLayout());
      trackPanel.add(trackON);
      trackPanel.add(trackOFF);

      JPanel tunnelPanel = new JPanel();
      title = BorderFactory.createTitledBorder(BorderFactory
            .createLineBorder(Color.black), " Tunnel Mode ");
      title.setTitleJustification(TitledBorder.CENTER);
      tunnelPanel.setBorder(title);

      ButtonGroup tunnelGroup = new ButtonGroup();

      tunnelModeON = new JRadioButton("ON", false);
      tunnelModeOFF = new JRadioButton("OFF", true);

      tunnelModeON.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            sim.setTunnelMode(true);
            frame.requestFocus();
         }
      });

      tunnelModeOFF.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            sim.setTunnelMode(false);
            frame.requestFocus();
         }
      });

      tunnelGroup.add(tunnelModeON);
      tunnelGroup.add(tunnelModeOFF);

      tunnelPanel.setLayout(new FlowLayout());
      tunnelPanel.add(tunnelModeON);
      tunnelPanel.add(tunnelModeOFF);

      JPanel pointerPanel = new JPanel();
      pointerPanel.setLayout(new BoxLayout(pointerPanel, BoxLayout.Y_AXIS));
      pointerPanel.add(latLabel);
      pointerPanel.add(lonLabel);

      mapToolPanel.setLayout(new FlowLayout());
      mapToolPanel.add(leftMapToolPanel);
      mapToolPanel.add(mapModePanel);
      mapToolPanel.add(trackPanel);
      mapToolPanel.add(tunnelPanel);
      mapToolPanel.add(pointerPanel);
   }

   private void setDefaultButtonConfig() {
      leftMapToolPanel.removeAll();
      leftMapToolPanel.add(buildButton);
      leftMapToolPanel.add(zoomrButton);
      frame.pack();
   }

   private void enableControl(boolean en) {
      rewButton.setEnabled(en);
      revButton.setEnabled(en);
      fwdButton.setEnabled(en);
      pauseButton.setEnabled(en);
      stopButton.setEnabled(en);
      ffwButton.setEnabled(en);
      incSpeedButton.setEnabled(en);
      decSpeedButton.setEnabled(en);
      //trackON.setEnabled(en);
      //trackOFF.setEnabled(en);
      routeButton.setEnabled(en);
      zoomrButton.setEnabled(en);
      loadRoute.setEnabled(en);
      loadNMEA.setEnabled(en);
      repeatMode.setEnabled(en);
      speedFactorLabel.setVisible(en);
      distanceLabel.setVisible(en);
      distanceLeftLabel.setVisible(en);
      repeatLabel.setVisible(en);
      timeLabel.setVisible(en);
   }

   public void adjustmentValueChanged(AdjustmentEvent aev) {
      sim.setCurrentFrame(scrollbar.getValue());
      /**
      drawVehicle(null,true);
      mapComp.updateVehicle(sim.getMessage(sim.getCurrentFrame()).getLat(),
                            sim.getMessage(sim.getCurrentFrame()).getLon(),
                            sim.getMessage(sim.getCurrentFrame()).getSpeed(),
                            sim.getMessage(sim.getCurrentFrame()).getBearing(),
                            sim.GPSOn());
      drawVehicle(null,true);
      update();
       */
      /**
      if(sim.getState() == Simulator.WILD){
         setWild(false);
         m12.setLabel("Run Wild");
      }
       */

      //repaint();
   }

   private void setServer() {
      String address = JOptionPane.showInputDialog("Enter Server address:", ""
            + mapComponent.getServer());
      if (address != null) {
         mapComponent.setServer(address);
         spDesc = new SPDesc(address);
         mapComponent.setSPDesc(spDesc);
         sim.saveServer(address);
         try {
            mapComponent.loadMap();
         } catch (Exception e) {
         }
         updateMap();
      }
   }

   public void keyPressed(KeyEvent kev) {
      int keyCode = kev.getKeyCode();
      if (keyCode == KeyEvent.VK_RIGHT && sim.getState() != Simulator.WILD) {
         DecimalFormat decFormat = new DecimalFormat("0.0");
         double f = sim.getSpeedFactor();
         if (f <= 2.95) {
            sim.setSpeedFactor(f + 0.1);
            speedFactorLabel.setText(" Speed Factor: "
                  + decFormat.format(sim.getSpeedFactor()));
         }
      } else if (keyCode == KeyEvent.VK_LEFT
            && sim.getState() != Simulator.WILD) {
         DecimalFormat decFormat = new DecimalFormat("0.0");
         double f = sim.getSpeedFactor();
         if (f >= 0.55 && f <= 3.05) {
            sim.setSpeedFactor(f - 0.1);
            speedFactorLabel.setText(" Speed Factor: "
                  + decFormat.format(sim.getSpeedFactor()));
         }
      }

      //System.out.println("KeyPressed");
      if (sim.getState() == Simulator.WILD) { // Only when running wild.
         //System.out.println("KeyCode : "+keyCode);
         if (keyCode == KeyEvent.VK_NUMPAD9) {
            sim.setBearing((sim.getBearing() + 1) % 360);
         } else if (keyCode == KeyEvent.VK_NUMPAD8 || keyCode == KeyEvent.VK_UP) {
            sim.changeSpeed(1);
         } else if (keyCode == KeyEvent.VK_NUMPAD7) {
            sim.setBearing((sim.getBearing() - 1) % 360);
         } else if (keyCode == KeyEvent.VK_NUMPAD6 || keyCode == KeyEvent.VK_RIGHT) {
            sim.setBearing((sim.getBearing() + 10) % 360);
         } else if (keyCode == KeyEvent.VK_NUMPAD5) {

         } else if (keyCode == KeyEvent.VK_NUMPAD4 || keyCode == KeyEvent.VK_LEFT) {
            sim.setBearing((sim.getBearing() - 10) % 360);
         } else if (keyCode == KeyEvent.VK_NUMPAD3) {
            sim.setBearing((sim.getBearing() + 30) % 360);
         } else if (keyCode == KeyEvent.VK_NUMPAD2 || keyCode == KeyEvent.VK_DOWN) {
            sim.changeSpeed(-1);
         } else if (keyCode == KeyEvent.VK_NUMPAD1) {
            sim.setBearing((sim.getBearing() - 30) % 360);
         } else if (keyCode == KeyEvent.VK_NUMPAD0) {
            if (sim.getSpeed() > 15) {
               sim.changeSpeed(-15);
               wildBreaking = true;
            } else if (sim.getSpeed() < -15) {
               sim.changeSpeed(15);
               wildBreaking = true;
            } else {
               sim.setSpeed(0);
            }
         }
         if (sim.getSpeed() > mapComponent.getDefaultMaxSpeed())
            sim.setSpeed(mapComponent.getDefaultMaxSpeed());
         if (sim.getSpeed() < -10)
            sim.setSpeed(-10);
         if (sim.getBearing() < 0)
            sim.setBearing(sim.getBearing() + 360);
         //repaint();
      }
   }

   public void keyReleased(KeyEvent kev) {
      //System.out.println("KeyReleased");
      wildBreaking = false;
   }

   public void keyTyped(KeyEvent kev) {
      //System.out.println("KeyTyped");
   }

   /**
    *   Called when the "Load Route" menue item is choosen.
    */
   public void loadFileMenu() {
      final String fname = "MainSimulatorWindow.loadFileMenu() ";
System.out.println("MainSimulatorWindow.loadFileMenu()");
      boolean succ = true;
      FileDialog d = new FileDialog(frame, "Load Coordinate File",
            FileDialog.LOAD);
      d.setDirectory(".");
      d.show();
      if (d.getFile() != null) {
         System.out.println(fname + "try to load " + d.getDirectory()
               + d.getFile());
         try {
            sim.loadFile(d.getDirectory() + d.getFile());
         } catch (FileNotFoundException fe) {
            System.err.println("No such file found ");
            ErrorDialog err = new ErrorDialog(frame, "File not found error");
            err.addText("Was not able to find the file!");
            err.setLocation(screenWidth * 3 / 8, screenHeight * 3 / 8);
            err.show();
            succ = false;
         } catch (IOException ioe) {
            System.err.println("Read error! Load failed");
            System.err.println("Size : " + sim.getLoadedRoute().length);
            System.err.println("Caught: " + ioe);

            ErrorDialog err = new ErrorDialog(frame, "File read error");
            err.addText("There was an error reading from file!");
            err.setLocation(screenWidth * 3 / 8, screenHeight * 3 / 8);
            err.show();
            succ = false;
         } catch (FileReadException e) {
            ErrorDialog err = new ErrorDialog(frame, "File read error");
            err.addText("Was not able to decode route file!");
            err.addText("This is not a proper route file!");
            err.setLocation(screenWidth * 3 / 8, screenHeight * 3 / 8);
            err.show();
            succ = false;
         }

         if (succ) {
            srcName = d.getFile();
            updateTitle();
         }

      }

      saveRoute.setEnabled(true);
      int size = sim.getLoadedRoute().length;
      NMEAMessage[] route = sim.getLoadedRoute();
      scrollbar.setMaximum(sim.getRouteSize() - 1);
      scrollbar.setValue(sim.getCurrentFrame());
      mapComponent.resetForLoad(10000);
      for (int i = 0; i < size; i++) {
         mapComponent.loadRoutePoint(route[i].getLon(), route[i].getLat());
      }
      sim.setState(Simulator.PAUSE);
      frame.requestFocus();
      //repaint();
   }

   public void saveFileMenue() {
      boolean succ = true;
      FileDialog d = new FileDialog(frame, "Save Coordinate File",
            FileDialog.SAVE);
      d.setDirectory(".");
      d.show();
      if (d.getFile() != null) {
         System.out.println(d.getDirectory() + " + " + d.getFile());
         try {
            sim.saveFile(d.getDirectory() + d.getFile());
         } catch (IOException ioe) {
            System.err.println("Couldn't write file : " + d.getFile());
            ErrorDialog err = new ErrorDialog(frame, "File save error");
            err.addText("Was not able to save file!");
            err.setLocation(screenWidth * 3 / 8, screenHeight * 3 / 8);
            err.show();
            succ = false;
         }

         if (succ) {
            srcName = d.getFile();
            updateTitle();
         }
      }
      frame.requestFocus();
      //repaint();
   }

   public void saveNavFileMenu() {
      FileDialog d = new FileDialog(frame, "Save Route", FileDialog.SAVE);
      d.setDirectory(paramFile.getLoadDir());
      d.setFile("route.r");
      d.show();
      if (d.getFile() != null) {
         try {
            FileOutputStream fo = new FileOutputStream(d.getDirectory()
                  + d.getFile());
            byte[] data = sim.getRouteData();
            for (int i = 0; i < data.length; i++)
               fo.write(data[i]);
            fo.close();
         } catch (FileNotFoundException e) {
            System.out.println("File not found!");
         } catch (IOException e) {
            System.out.println("IOException: " + e.getMessage());
         }
      }
   }

   public void loadNavFileMenu() {
      final String fname = "MainSimulatorWindow.loadNavFileMenu() ";

      boolean succ = true;
      FileDialog d = new FileDialog(frame, "Load Route", FileDialog.LOAD);
      d.setDirectory(paramFile.getLoadDir());
      d.setFile(paramFile.getLoadFile());
      d.show();
      if (d.getFile() != null) {
         System.out.println(fname + "try to load " + d.getDirectory()
               + d.getFile());
         if (paramFile.getLoadDir().compareTo(d.getDirectory()) != 0
               || paramFile.getLoadFile().compareTo(d.getFile()) != 0) {
            paramFile.setLoadDir(d.getDirectory());
            paramFile.setLoadFile(d.getFile());
            paramFile.save();
         }

         try {
            sim.loadNavFile(d.getDirectory() + d.getFile());
         } catch (FileNotFoundException fe) {
            System.err.println("No such file found ");
            ErrorDialog err = new ErrorDialog(frame, "File not found error");
            err.addText("Was not able to find the file!");
            err.setLocation(screenWidth * 3 / 8, screenHeight * 3 / 8);
            err.show();
            succ = false;
         } catch (FileReadException e) {
            System.out.println("File read exception");
            ErrorDialog err = new ErrorDialog(frame, "File read error");
            err.addText("There was an error reading from file!");
            err.setLocation(screenWidth * 3 / 8, screenHeight * 3 / 8);
            err.show();
            succ = false;
         }

         if (succ) {
            srcName = d.getFile();
            updateTitle();
            update();
         }

         DecimalFormat decFormat = new DecimalFormat("0.0");
         System.out.println("Loading done!");
         scrollbar.setMaximum(sim.getRouteSize() - 1);
         scrollbar.setValue(sim.getCurrentFrame());
         distanceLabel.setText(" Route Distance: "
               + decFormat.format(sim.getDistance(0)) + " km");
         sim.reset();
         sim.setRepeatMode(paramFile.getRepeatMode());
         filter.setEnabled(true);
      }

      frame.requestFocus();
      //repaint();
   }

   public void loadNMEAFile() {
      final String NMEADIR = "NMEADIR";
      final String NMEAFILE = "NMEAFILE";

      FileDialog d = new FileDialog(frame, "Load NMEA File", FileDialog.LOAD);

      d.setDirectory(paramFile.getNMEADir());
      d.setFile(paramFile.getNMEAFile());
      d.show();

      // File is chosen ...

      if (d.getFile() != null) {
         String[] names = getDirAndFile(d.getDirectory(), d.getFile());
         String dir = names[0];
         String file = names[1];
         String fullname = names[2];
         paramFile.setNMEAFile(file);
         paramFile.setNMEADir(dir);
         paramFile.save();
         sim.loadNMEADataFile(fullname);
         srcName = fullname;
         updateTitle();
         DecimalFormat decFormat = new DecimalFormat("0.0");
         System.out.println("Loading done!");
         scrollbar.setMaximum(sim.getRouteSize() - 1);
         scrollbar.setValue(sim.getCurrentFrame());
         distanceLabel.setText(" Route Distance: "
               + decFormat.format(sim.getDistance(0)) + " km");
         sim.reset();
         sim.setRepeatMode(paramFile.getRepeatMode());
      }

      frame.requestFocus();
   }

   public void saveBoxFile() {
      FileDialog d = new FileDialog(frame, "Save Box File", FileDialog.SAVE);
      d.setDirectory(".");
      d.show();
      if (d.getFile() != null) {
         try {
            sim.saveBoxFile(d.getDirectory() + d.getFile());
         } catch (FileNotFoundException e) {
            System.out.println("FileNotFoundException: " + e.getMessage());
         } catch (IOException e) {
            System.out.println("IOException: " + e.getMessage());
            JOptionPane.showMessageDialog(getFrame(), "Invalid file",
                  "File Error", JOptionPane.ERROR_MESSAGE);
         }
         updateMap();
         frame.requestFocus();
      }
   }

   public void loadBoxFile() {
      FileDialog d = new FileDialog(frame, "Load Box File", FileDialog.LOAD);
      d.setDirectory(".");
      d.show();
      if (d.getFile() != null) {
         try {
            sim.loadBoxFile(d.getDirectory() + d.getFile());
         } catch (FileNotFoundException e) {
            System.out.println("FileNotFoundException: " + e.getMessage());
         } catch (IOException e) {
            System.out.println("IOException: " + e.getMessage());
            JOptionPane.showMessageDialog(getFrame(), "Invalid file",
                  "File Error", JOptionPane.ERROR_MESSAGE);
         } catch (FileReadException e) {
            System.out.println("FileReadException");
            JOptionPane.showMessageDialog(getFrame(), "Invalid file",
                  "File Error", JOptionPane.ERROR_MESSAGE);
         }
         updateMap();
         frame.requestFocus();
      }
   }

   public void loadCoordsToPlot() {
      boolean succ = true;
      FileDialog d = new FileDialog(frame, "Load Coordinate File",
            FileDialog.LOAD);
      d.setDirectory(".");
      d.show();
      if (d.getFile() != null) {
         System.out.println(d.getDirectory() + " + " + d.getFile());
         try {
            sim.loadCoordFile(d.getDirectory() + d.getFile());
         } catch (FileNotFoundException fe) {
            System.err.println("No such file found ");
            ErrorDialog err = new ErrorDialog(frame, "File not found error");
            err.addText("Was not able to find the file!");
            err.setLocation(screenWidth * 3 / 8, screenHeight * 3 / 8);
            err.show();
            succ = false;
         } catch (IOException ioe) {
            System.err.println("Couldn't read from coordfile : " + d.getFile());
            ErrorDialog err = new ErrorDialog(frame, "Couldnot read from file");
            err.addText("Reading failed from " + d.getFile());
            err.setLocation(screenWidth * 3 / 8, screenHeight * 3 / 8);
            err.show();
            succ = false;
         }
      }
      updateMap();
      frame.requestFocus();
   }

   public void saveCoords() {
      FileDialog d = new FileDialog(frame, "Save Coordinate File",
            FileDialog.SAVE);
      d.setDirectory(".");
      d.show();
      if (d.getFile() != null) {
         System.out.println(d.getDirectory() + " + " + d.getFile());
         try {
            sim.saveCoordFile(d.getDirectory() + d.getFile());
         } catch (FileNotFoundException fe) {
            System.err.println("No such file found ");
            ErrorDialog err = new ErrorDialog(frame, "File not found error");
            err.addText("Was not able to find the file!");
            err.setLocation(screenWidth * 3 / 8, screenHeight * 3 / 8);
            err.show();
         } catch (IOException ioe) {
            System.err.println("Couldn't write to coordfile : " + d.getFile());
            ErrorDialog err = new ErrorDialog(frame, "Could not write to file");
            err.addText("Writing failed: " + d.getFile());
            err.setLocation(screenWidth * 3 / 8, screenHeight * 3 / 8);
            err.show();
         }
      }
      updateMap();
      frame.requestFocus();
   }

   public void chooseLogFileName() {
      String LOG_DIR = "LOG_DIR";
      String LOG_FIL = "LOG_FIL";
      FileDialog d = new FileDialog(frame, "Set log file", FileDialog.SAVE);
      d.setDirectory(paramFile.getLogDir());
      d.setFile(paramFile.getLogFile());
      d.show();

      // File is chosen ...
      if (d.getFile() != null) {
         String[] names = getDirAndFile(d.getDirectory(), d.getFile());
         String dir = names[0];
         String file = names[1];
         String fullname = names[2];
         try {
            sim.setLogFile(fullname);
            paramFile.setLogDir(dir);
            paramFile.setLogFile(file);
            paramFile.save();
            updateTitle();
            stopLogging.setEnabled(true);
         } catch (IOException e) {
            showExceptionDialog("Choosing logfile", e);
         }
      }
      frame.requestFocus();
   }

   void showExceptionDialog(Object title, Object contents) {
      ErrorDialog dialog = new ErrorDialog(frame, title.toString());
      dialog.addText(contents.toString());
      dialog.show();
   }

   /**
    *   Clean up the file and dir from a FileDialog since
    *   it has some problems with drive letters.
    *   @return dir in [0] and file in [1]. dir+file in [2]
    */
   private String[] getDirAndFile(String fileDialogDir, String fileDialogFile) {
      String filename = fileDialogDir + fileDialogFile;
      File file = new File(filename);
      // Dir doesn't work with drive letters
      String dir = file.getParent();
      if (dir == null) {
         dir = fileDialogDir;
      } else {
         if (dir.endsWith(".")) {
            dir = dir.substring(0, dir.length() - 1);
         }
      }
      return new String[] { dir, fileDialogFile, file.toString() };
   }

   /**
    *   Called when the "Set BT Port" menue item is choosen.
    */

   public void setRepeat() {
      RepeatDialog rd = new RepeatDialog(sim.getRepeatMode(), frame);
      rd.setLocation(screenWidth * 3 / 8, screenHeight * 1 / 4);
      rd.show();
      sim.setRepeatMode(rd.getRepeat());
      sim.saveRepeatMode(rd.getRepeat());
      int mode = rd.getRepeat();
      if (mode == Simulator.NO_REPEAT) {
         repeatLabel.setText(" No Repeat");
      } else if (mode == Simulator.REPEAT_LOOP) {
         repeatLabel.setText(" Repeat Loop");
      } else {
         repeatLabel.setText(" Repeat Reverse");
      }
      //repaint();
      updateMap();
      frame.requestFocus();
   }

   public void stopLogging() {
      sim.stopLogging();
      stopLogging.setEnabled(false);
      updateTitle();
   }

   private void setReadFromPort() {
      PortDialog dialog = new PortDialog(paramFile.getReadPort(), frame);
      dialog.show();
      String portName = dialog.getPortName();
      if (portName != null) {
         try {
            LineNumberReader inReader = new LineNumberReader(
                  new InputStreamReader(dialog.getInputStream()));
            sim.setState(Simulator.PORT);
            sim.resetLoadedRoute();
            updateLoaded();

            sim.setNMEAReader(inReader);
            System.err.println("InputStream set");
            paramFile.setReadPort(portName);
            paramFile.save();
            srcName = portName;
            updateTitle();
         } catch (IOException e) {
            showExceptionDialog("While choosing port:", e);
         }
      }
      frame.requestFocus();
   }

   /**
    *   Called when the "Change GPS strength" menue item is choosen.
    */
   public void setGPS() {

      // Pop up slide menu.
      SlideDialog d = new SlideDialog(frame, "Set GPS signal", sim.getGPSSig());
      d.setLocation(screenWidth * 3 / 8, screenHeight * 1 / 4);
      d.show();
      sim.setGPSSig(d.getValue());
      // if(gpssig > 40)
      //    delayTime = 500;
      // else
   }

   public void displayAbout() {
      // Pop up slide menu.
      ErrorDialog about = new ErrorDialog(frame, "About BTGPSSimulator!");
      about.addText("BTGPSSimulator ver. 2.11.3");
      about.addText("\u00A9 Wayfinder Systems 2004-2008");
      about.addText("All Rights Reserved.");
      about.addText("No warranty.");
      about.setLocation(screenWidth * 3 / 8, screenHeight * 3 / 8);
      about.show();
      frame.requestFocus();
   }

   public void displayHelp() {
      // Pop up slide menu.
      HelpDialog help = new HelpDialog();
      help.show();
      frame.requestFocus();
   }

   public void updateTitle() {
      String newTitle = "BTGPSSimulator [";
      if (srcName.length() != 0) {
         newTitle = newTitle + srcName;
      }
      if (sim.getLogFileName().length() != 0) {
         newTitle = newTitle + "->" + sim.getLogFileName();
      }
      newTitle += "]";
      frame.setTitle(newTitle);
   }

   public void updateLoaded() {
      //sim.setRouteSize(sim.getLoadedRoute().length);
      //boolean hide = mapWindow.getHide();
      mapComponent.setRoutePoints(sim.getLoadedRoute());
      if (sim.getRouteSize() != 0) {
         contButton.setEnabled(true);
         hideButton.setEnabled(true);
      }
      //mapWindow.setHide( true );
      //mapWindow.setHide( hide );
      sim.setCurrentFrame(0);
      scrollbar.setMaximum(sim.getRouteSize() - 1);
      scrollbar.setValue(sim.getCurrentFrame());
   }

   public void mouseClicked(MouseEvent evt) {
      //float mc2ToPix = screenDistance/mapHeight;
      int x = mouse_x = evt.getX();
      int y = mouse_y = evt.getY();
      int mc2_x = mapComponent.xToMc2Lon(x);
      int mc2_y = mapComponent.yToMc2Lat(y);
      
      textField.setText("(" + mc2_y + "," + mc2_x + ")");
      //System.out.println(evt.getModifiers());
      if (evt.getModifiers() == 4 && !state.equals("Build route")) {
         popup.show(evt.getComponent(), x, y);
      } else {
         if (state.equals("Build route")) {
            if (evt.getModifiers() == 4) {
               int current = (int) ((float) mapComponent.getMaxSpeed() * 3.6);
               SpeedDialog sp = new SpeedDialog(this, "Set Max Speed", current);
               sp.setLocation(x, y);
               sp.show();
               int newSpeed;
               if (sp.isValid()) {
                  newSpeed = Math.round((float) (0.5 + sp.getSpeed() / 3.6));
                  mapComponent.setMaxSpeed(Math.min(newSpeed, mapComponent
                        .getDefaultMaxSpeed()));
                  //System.err.println("New max_speed :"+max_sp+" m/s");
                  //System.out.println("max_sp/mc2ToM :"+max_sp/mc2ToM);
               }
            } else if (evt.isShiftDown() && (mapComponent.getNbrRoutePoints() > 0)) {
               int i = mapComponent.findRoutePoint(mc2_x, mc2_y);
               if (i != -1) {
                  mapComponent.removeRoutePoint(i);
                  if (mapComponent.getNbrRoutePoints() == 0) {
                     contButton.setEnabled(false);
                     loadButton.setEnabled(false);
                  }
               }
            } else {
               contButton.setEnabled(true);
               loadButton.setEnabled(true);
               mapComponent.addRoutePoint(mc2_x, mc2_y);
            }
            updateMap();
         }

         if (state.equals("Coords")) {

            if (!evt.isShiftDown()) {
               String name = JOptionPane
                     .showInputDialog("Enter name of coordinate:");
               mapComponent.addCoordPoiPoint(mc2_x, mc2_y);
               if (name != null)
                  mapComponent.addCoordPoiComment(name);
               else
                  mapComponent.addCoordPoiComment("");
               saveCoords.setEnabled(true);
            } else {
               int i = mapComponent.findCoordPoiPoint(x, y);
               if (i != -1) {
                  mapComponent.removeCoordPoiPoint(i);
                  mapComponent.removeCoordPoiComment(i);
                  if (mapComponent.getNbrCoordPoiPoints() == 0)
                     saveCoords.setEnabled(false);
               }
            }
            updateMap();
         }

         if (state.equals("place")) {
            sim.setLon(mapComponent.mc2ToWgs84(mapComponent.xToMc2Lon(x)));
            sim.setLat(mapComponent.mc2ToWgs84(mapComponent.yToMc2Lat(y)));
            state = oldState;
            updateMap();
         }

         if (state.equals("Zoom in")) {
            mapComponent.zoomIn(mc2_x, mc2_y);
            try {
               mapComponent.loadMap();
            } catch (Exception me) {
               System.out.println("MalformedURLException");
               //System.out.println(urlString);
               ErrorDialog err = new ErrorDialog(getFrame(), "Map load error");
               err.addText("Was not able to find the map");
               err.addText("Check maphosts in param file.");
               err.setLocation(mapWidth * 3 / 8, mapHeight * 3 / 8);
               err.show();
               System.exit(1);
            }
            updateMap();
         }
         if (state.equals("Zoom out")) {
            mapComponent.zoomOut(mc2_x, mc2_y);
            try {
               mapComponent.loadMap();
            } catch (Exception me) {
               System.out.println("MalformedURLException");
               //System.out.println(urlString);
               ErrorDialog err = new ErrorDialog(getFrame(), "Map load error");
               err.addText("Was not able to find the map");
               err.addText("Check maphosts in param file.");
               err.setLocation(mapWidth * 3 / 8, mapHeight * 3 / 8);
               err.show();
               System.exit(1);
            }
            updateMap();
         }
         if (state.equals("Move Window")) {
            mapComponent.moveCenter(mc2_x, mc2_y);
            try {
               mapComponent.loadMap();
            } catch (Exception me) {
               System.out.println("MalformedURLException");
               //System.out.println(urlString);
               ErrorDialog err = new ErrorDialog(getFrame(), "Map load error");
               err.addText("Was not able to find the map");
               err.addText("Check maphosts in param file.");
               err.setLocation(mapWidth * 3 / 8, mapHeight * 3 / 8);
               err.show();
               System.exit(1);
            }
            updateMap();
         }
      }
   }

   private void searchCoord(int x, int y) {

      int mc2_x = mapComponent.xToMc2Lon(x);
      int mc2_y = mapComponent.yToMc2Lat(y);
      double lon = mapComponent.mc2ToWgs84(mc2_x);
      double lat = mapComponent.mc2ToWgs84(mc2_y);

      CoordDialog cd = new CoordDialog(frame, "Enter coordinate", mc2_y, mc2_x,
            true);
      cd.setLocation(x, y);
      cd.show();

      if (cd.isValid()) {
         mc2_x = cd.getLon();
         mc2_y = cd.getLat();
         mapComponent.setMarkedPoint(new Point(mc2_x, mc2_y));
         mapComponent.moveCenter(mc2_x, mc2_y);
         try {
            mapComponent.loadMap();
         } catch (Exception e) {
         }
         updateMap();
      }
   }

   public void mouseMoved(MouseEvent evt) {
      DecimalFormat decFormat = new DecimalFormat("00.000");
      double lat = mapComponent.mc2ToWgs84(mapComponent.yToMc2Lat(evt.getY()));
      double lon = mapComponent.mc2ToWgs84(mapComponent.xToMc2Lon(evt.getX()));
      if (lat > 0)
         latLabel.setText("LAT: " + decFormat.format(lat) + " N");
      else
         latLabel.setText("LAT: " + decFormat.format(-lat) + " S");
      if (lon > 0)
         lonLabel.setText("LON: " + decFormat.format(lon) + " E");
      else
         lonLabel.setText("LON: " + decFormat.format(-lon) + " W");
      int dist = 100;
      int min_dist = 100;
      int min_i = 0;
      String name = "";
      if (evt != null) {
         for (int i = 0; i < mapComponent.getNbrCoordPoiPoints(); i++) {
            int mc2_x = mapComponent.getCoordPoiPoint(i).x;
            int mc2_y = mapComponent.getCoordPoiPoint(i).y;
            int x = mapComponent.mc2LonToX(mc2_x);
            int y = mapComponent.mc2LatToY(mc2_y);

            int dist_x = Math.abs(evt.getX() - x);
            int dist_y = Math.abs(evt.getY() - y);
            //System.out.println("Dist = " + dist);
            if (dist_x + dist_y < min_dist) {
               min_dist = dist_x + dist_y;
               min_i = i;
               name = mapComponent.getCoordPoiComment(min_i);
            }

         }

         if (mapComponent.showPoi()) {
            for (int i = 0; i < mapComponent.getNbrPoiPoints(); i++) {
               int mc2_x = mapComponent.getPoiX(i);
               int mc2_y = mapComponent.getPoiY(i);
               //System.out.println(mc2_x + " " +mc2_y);
               int x = mapComponent.mc2LonToX(mc2_x); // + 10;  //compensate for centre of icon
               int y = mapComponent.mc2LatToY(mc2_y); // - 10;  //compensate for centre of icon
               //System.out.println(x +" "+y);
               int dist_x = Math.abs(evt.getX() - x);
               int dist_y = Math.abs(evt.getY() - y);
               //System.out.println("Dist = " + dist);
               if (dist_x + dist_y < min_dist) {
                  min_dist = dist_x + dist_y;
                  min_i = i;
                  name = mapComponent.getPoiName(min_i);
                  //System.out.println(name);
               }
            }
         }

         if (min_dist < 15)
            mapPanel.setToolTipText(name);
         else
            mapPanel.setToolTipText("");
         //mapPanel.setToolTipText(evt.getX() + " " +evt.getY());
         //System.out.println("mouseMoved");
      }
   }

   public void mouseDragged(MouseEvent evt) {
      if (state.equals("Zoom in")) {
         int x = evt.getX();
         int y = evt.getY();
         Graphics g = mapPanel.getGraphics();
         //g.translate(getInsets().left, getInsets().top);
         g.setXORMode(Color.white);
         if ((current_x >= 10) && (current_y >= 10))
            drawRectangle(g);
         int absY = Math.abs(y - first_y);
         int ce_y = (y + first_y) / 2;
         int absX = Math.abs(x - first_x);
         int ce_x = (x + first_x) / 2;
         if (xyRatio * absX > absY) {
            current_x = absX;
            current_y = (int) Math.round(xyRatio * ((double) current_x));
         } else {
            current_y = absY;
            current_x = (int) Math.round(((double) current_y) / xyRatio);
         }
         top_x = ce_x - current_x / 2;
         top_y = ce_y - current_y / 2;

         //if((y-first_y) < 0)
         //   current_y = -current_y;
         //if((x-first_x) < 0)
         //   current_x = -current_x;
         if ((current_x >= 10) && (current_y >= 10))
            drawRectangle(g);
         g.dispose();
      } else if (state.equals("Boxes")) {

         Graphics g = mapPanel.getGraphics();
         //g.translate(getInsets().left, getInsets().top);
         g.setXORMode(Color.white);
         g.drawRect(boxTop_x, boxTop_y, mouse_x - boxTop_x, mouse_y - boxTop_y);

         mouse_x = evt.getX();
         mouse_y = evt.getY();
         g.drawRect(boxTop_x, boxTop_y, mouse_x - boxTop_x, mouse_y - boxTop_y);
         g.dispose();
      }
   }

   public void mousePressed(MouseEvent evt) {
      first_x = mouse_x = evt.getX();
      first_y = mouse_x = evt.getY();
      if (state.equals("Boxes"))
         boxTop_x = evt.getX();
      boxTop_y = evt.getY();
      //if(evt.getMods)
   }

   public void mouseReleased(MouseEvent evt) {
      if ((current_x > 10) || (current_y > 10) || (current_x < -10)
            || (current_y < -10)) {
         //int fac = mapWidth/Math.abs(current_x);
         /**
         if((state.equals("Zoom out") && (evt.getModifiers() != 4)) ||
            (state.equals("Zoom in") && (evt.getModifiers() == 4))
            ){
            int dx = mapComp.getCPoint().x + (mapComp.getCPoint().x - newX)*fac;
            int dy = mapComp.getCPoint().y + (mapComp.getCPoint().y - newY)*fac;
            zoomOut(dx, dy, fac);
         } **/
         if ((state.equals("Zoom in") && (evt.getModifiers() != 4))
               || (state.equals("Zoom out") && (evt.getModifiers() == 4))) {
            mapComponent.zoomToSquare(top_x, top_y, current_x, current_y);
            try {
               mapComponent.loadMap();
            } catch (Exception me) {
               System.out.println("MalformedURLException");
               //System.out.println(urlString);
               ErrorDialog err = new ErrorDialog(getFrame(), "Map load error");
               err.addText("Was not able to find the map");
               err.addText("Check maphosts in param file.");
               err.setLocation(mapWidth * 3 / 8, mapHeight * 3 / 8);
               err.show();
               System.exit(1);
            }
            updateMap();
         }
      }
      /**
           Graphics g = mapPanel.getGraphics();
           g.setXORMode(Color.white);
           if((current_x >= 10) && (current_y >= 10))
              drawRectangle(g);*/

      first_x = 0;
      first_y = 0;
      current_x = 0;
      current_y = 0;

      if (state.equals("Boxes") && boxTop_x < evt.getX()
            && boxTop_y < evt.getY()) {
         int mc2_x1 = mapComponent.xToMc2Lon(boxTop_x);
         int mc2_y1 = mapComponent.yToMc2Lat(boxTop_y);
         int mc2_x2 = mapComponent.xToMc2Lon(evt.getX());
         int mc2_y2 = mapComponent.yToMc2Lat(evt.getY());
         //System.out.println(boxTop_x+" "+boxTop_y+" "+evt.getX()+" "+evt.getY());
         Rectangle r = new Rectangle(mc2_x1, mc2_y1, mc2_x2, mc2_y2);
         mapComponent.addBox(r);
         updateMap();
         eraseBoxButton.setEnabled(true);
         saveBoxes.setEnabled(true);
      }
   }

   public void mouseEntered(MouseEvent evt) {
      //  System.out.println("mouseEntered");
   }

   public void mouseExited(MouseEvent evt) {
      // System.out.println("mouseExited");
   }

   public void drawRectangle(Graphics g) {
      int x = top_x;
      int y = top_y;
      int dx = current_x;
      int dy = current_y;
      //if(current_x < 0){
      //   dx = - current_x;
      //   x  = first_x - dx;
      //}
      //if(current_y < 0){
      //   dy = - current_y;
      //   y  = first_y - dy;
      //} 
      g.drawRect(x, y, dx, dy);
   }

   private void drawMarkedPoint(Graphics g) {
      g.setColor(Color.BLACK);
      int x = mapComponent.mc2LonToX(mapComponent.getMarkedPoint().x);
      int y = mapComponent.mc2LatToY(mapComponent.getMarkedPoint().y);
      g.fillOval(x, y, 7, 7);
   }

   public void drawVehicle(Graphics g, boolean xor) {
      if (g == null)
         g = mapPanel.getGraphics();
      //g.translate(mapComp.getCarPos().x,mapComp.getCarPos().y);
      //if (sim.getState() == Simulator.FORWARD || sim.getState() == Simulator.REVERSE)
      if (xor)
         g.setXORMode(Color.WHITE);
      g.setColor(Color.black);
      ((Graphics2D) g).setStroke(new BasicStroke(5));
      // De plot old location
      /**
      if(deDraw) {
         if(gpsOn){
            g.setColor(Color.blue);
         } else {
            g.setColor(Color.black);
         }
      }*/

      // Draw position
      //System.out.println("Draw vehicle");
      //System.out.println(mapComp.getCarPos().x+" "+mapComp.getCarPos().y);
      int x = mapComponent.mc2LonToX(mapComponent.getCarPos().x) - 10;
      int y = mapComponent.mc2LatToY(mapComponent.getCarPos().y) - 10;
      //System.out.println(x+" "+y);
      g.drawOval(x, y, 20, 20);
      int dx = mapComponent.mc2LonToX(mapComponent.getCarDir().x) - 2;
      int dy = mapComponent.mc2LatToY(mapComponent.getCarDir().y) - 2;
      g.drawOval(dx, dy, 4, 4);
      // draw line.
      g.drawLine(x + 10, y + 10, dx + 2, dy + 2);
      if (track) {
         if (mapComponent instanceof OldMapComponent) {
            if ((x < mapWidth / 20) || (x > (19 * mapWidth) / 20)
                  || (y < mapHeight / 20) || (y > (19 * mapHeight) / 20)) {
               int x_speed = Math.abs(mapComponent.getCarDir().x
                     - mapComponent.getCarPos().x);
               int y_speed = Math.abs(mapComponent.getCarDir().y
                     - mapComponent.getCarPos().y);
               //int speed = Math.max(xspeed, yspeed);
               int speed = (int) Math.round(Math.sqrt(x_speed * x_speed
                     + y_speed * y_speed));

               int d_x = mapComponent.getCarDir().x - mapComponent.getCarPos().x;
               int d_y = mapComponent.getCarDir().y - mapComponent.getCarPos().y;
               int newX = mapComponent.getCarDir().x + d_x;
               int newY = mapComponent.getCarDir().y + d_y;
               if (speed * 3 > mapComponent.getScreenDistance())
                  mapComponent.setScreenDistance(2 * mapComponent.getScreenDistance());
               mapComponent.moveCenter(newX, newY);
               try {
                  mapComponent.loadMap();
               } catch (Exception e) {
               }
               try {
                  mapComponent.loadMap();
               } catch (Exception me) {
                  System.out.println("MalformedURLException");
                  //System.out.println(urlString);
                  ErrorDialog err = new ErrorDialog(getFrame(),
                        "Map load error");
                  err.addText("Was not able to find the map");
                  err.addText("Check maphosts in param file.");
                  err.setLocation(mapWidth * 3 / 8, mapHeight * 3 / 8);
                  err.show();
                  System.exit(1);
               }
               updateMap();
            }
         } else if (mapComponent instanceof MercatorMapComponent) {
            int car_x = mapComponent.mc2LonToX(mapComponent.getCarPos().x);
            int car_y = mapComponent.mc2LatToY(mapComponent.getCarPos().y);
            //System.out.println(car_x+" "+car_y);
            int dist_x = Math.abs(car_x - mapWidth / 2);
            int dist_y = Math.abs(car_y - mapHeight / 2);
            //System.out.println("Dist = " + dist);
            if (dist_x >= 15 || dist_y >= 15) {
               mapComponent.moveCenter(mapComponent.getCarPos().x, mapComponent.getCarPos().y);
               try {
                  mapComponent.loadMap();
               } catch (Exception e) {
               }
               updateMap();
            }
         }
      }
   }

   public void drawStartEndPoints(Graphics g) {
      if (mapComponent.getStartPoint() != null) {
         int x = mapComponent.mc2LonToX(mapComponent.getStartPoint().x);
         int y = mapComponent.mc2LatToY(mapComponent.getStartPoint().y);
         int[] x_points = { x - 6, x, x + 6 };
         int[] y_points = { y + 6, y - 16, y + 6 };
         g.setColor(Color.BLUE);
         g.fillPolygon(x_points, y_points, 3);
      }

      if (mapComponent.getDestPoint() != null) {
         int x = mapComponent.mc2LonToX(mapComponent.getDestPoint().x);
         int y = mapComponent.mc2LatToY(mapComponent.getDestPoint().y);
         g.setColor(Color.BLACK);
         g.fillRect(x - 8, y - 8, 4, 4);
         g.fillRect(x - 4, y - 4, 4, 4);
         g.fillRect(x, y, 4, 4);
         g.fillRect(x + 4, y + 4, 4, 4);
         g.fillRect(x, y - 8, 4, 4);
         g.fillRect(x + 4, y - 4, 4, 4);
         g.fillRect(x - 8, y, 4, 4);
         g.fillRect(x - 4, y + 4, 4, 4);
         g.setColor(Color.WHITE);
         g.fillRect(x - 4, y - 8, 4, 4);
         g.fillRect(x, y - 4, 4, 4);
         g.fillRect(x + 4, y, 4, 4);
         g.fillRect(x + 4, y - 8, 4, 4);
         g.fillRect(x - 8, y - 4, 4, 4);
         g.fillRect(x - 4, y, 4, 4);
         g.fillRect(x, y + 4, 4, 4);
         g.fillRect(x - 8, y + 4, 4, 4);
      }

   }

   public void drawRoutePoints(Graphics g) {
      if (hide)
         return;
      //      Graphics g = getGraphics();
      int dist = mapComponent.getScreenDistance() / 1200000;
      if (dist == 0)
         dist = 1;
      if (mapComponent.getNbrRoutePoints() > 0) {
         g.setColor(new Color(255, 69, 0));
         //float mc2ToPix = ((float)mapHeight)/screenDistance;
         int lastX = -1;
         int lastY = -1;
         for (int i = 0; i < mapComponent.getNbrRoutePoints(); i++) {
            if (i % dist == 0) {
               int x = mapComponent.mc2LonToX(mapComponent.getRoutePoint(i).x) - 3;
               int y = mapComponent.mc2LatToY(mapComponent.getRoutePoint(i).y) - 3;
               if (x >= 0 && x <= mapWidth && y >= 0 && y <= mapHeight) {
                  g.drawOval(x, y, 6, 6);
                  if ((lastX > -1) && (lastY > -1)) {
                     g.drawLine(lastX, lastY, x + 3, y + 3);
                  }
                  lastX = x + 3;
                  lastY = y + 3;
               }
            }
         }

         // System.out.println("MapWindow.drawRoutePoint() elements of coords draw: " + (nbrCoords - 1));
      }
   }

   public void drawBoxes(Graphics g) {
      //System.out.println("Nbr boxes = " + mapComp.getNbrBoxes());
      for (int i = 0; i < mapComponent.getNbrBoxes(); i++) {
         Rectangle r = mapComponent.getBox(i);
         int x1 = mapComponent.mc2LonToX(r.x);
         int y1 = mapComponent.mc2LatToY(r.y);
         int x2 = mapComponent.mc2LonToX(r.width);
         int y2 = mapComponent.mc2LatToY(r.height);
         //System.out.println(x1+" "+y1+" "+x2+" "+y2);
         g.setColor(Color.black);
         //System.out.println(x1+" "+y1+" "+(x2-x1)+" "+(y2-y1));
         g.drawRect(x1, y1, x2 - x1, y2 - y1);
      }
   }

   public void drawPoiPoints(Graphics g) {
      //System.out.println("drawPoiPoints");
      //for (int i=0; i<mapComp.getNbrPoiPoints(); i++)
      //System.out.println(mapComp.getPoiPoint(i).x + " " + mapComp.getPoiPoint(i).y);
      //if(hide)
      //return;
      //      Graphics g = getGraphics();

      if (mapComponent.getNbrCoordPoiPoints() > 0) {
         //float mc2ToPix = ((float)mapHeight)/screenDistance;
         for (int i = 0; i < mapComponent.getNbrCoordPoiPoints(); i++) {
            int x = mapComponent.mc2LonToX(mapComponent.getCoordPoiPoint(i).x) - 7;
            int y = mapComponent.mc2LatToY(mapComponent.getCoordPoiPoint(i).y) - 7;
            g.setColor(Color.black);
            g.fillOval(x, y, 14, 14);
            g.setColor(Color.green);
            g.fillOval(x + 2, y + 2, 10, 10);

         }
      }

      /**
      if(mapComp.isMarkedLocation()) {
         //System.out.println("nbrPois"+nbrPois);
         int mx = mapComp.mc2LonToX(mapComp.getPoiPoint(mapComp.getNbrPoiPoints()-1).x) -4;
         int my = mapComp.mc2LonToX(mapComp.getPoiPoint(mapComp.getNbrPoiPoints()-1).y) -4;
         //System.out.println(mx+","+my);
         g.setColor(Color.black );
         g.drawOval(mx, my, 8, 8);
         g.fillOval(mx, my, 8, 8);
      }
       */
   }

   public void updateMap() {
      //System.out.println("Update map!");
      Graphics g = mapPanel.getGraphics();
      //g.clearRect( 0, 0, mapWidth, mapHeight );
      //Image img = mapComp.getImage();
      //mapPanel.setImage(img);
      //mapPanel.repaint();
      //if (img == null)
      // System.out.println("Img == null");
      //      else
      //System.out.println("Img OK");
      //System.out.println(mapComp.useMap());
      /**if ( img != null && mapComp.useMap() && img.getWidth( frame ) > 0 ) {
         //System.out.println("Drawing image");
         g.drawImage( img, 0, 0, frame );
      }*/

      //mapComp.setNoDePlot(true);
      //drawRoutePoint(g);
      //drawPoiPoint(g);
      g.setColor(Color.pink);
      int x = mapComponent.mc2LonToX(mapComponent.getSEPoint().x);
      int y = mapComponent.mc2LatToY(mapComponent.getSEPoint().y);
      g.drawOval(x, y, 10, 10);
      x = mapComponent.mc2LonToX(mapComponent.getNWPoint().x);
      y = mapComponent.mc2LatToY(mapComponent.getNWPoint().y);
      g.drawOval(x + 15, y + 15, 10, 10);
      x = mapComponent.mc2LonToX(mapComponent.getCPoint().x);
      y = mapComponent.mc2LatToY(mapComponent.getCPoint().y);
      /**
      System.out.println("CPoint = (" + mapComp.getCPoint().x + "," + mapComp.getCPoint().y + ")");
      System.out.println("SEPoint = (" + mapComp.getSEPoint().x + "," + mapComp.getSEPoint().y + ")");
      System.out.println("NWPoint = (" + mapComp.getNWPoint().x + "," + mapComp.getNWPoint().y + ")");
       */
      //System.out.println("CPoint = (" + x + "," + y + ")");
      g.drawOval(x - 5, y - 5, 10, 10);
      //frame.repaint();
      mapPanel.repaint();
   }

   public static void main(String[] args) {
      //System.out.println("Starting program...");
      final MainSimulatorWindow main = new MainSimulatorWindow("New window");
      main.updateTitle();
      //main.setResizable(true);
      //main.pack();
      //main.setVisible(true);
      //main.show();
   }

}
