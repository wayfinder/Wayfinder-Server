import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.Enumeration;
import java.util.TooManyListenersException;
import java.util.Vector;

import javax.comm.CommPortIdentifier;
import javax.comm.NoSuchPortException;
import javax.comm.PortInUseException;
import javax.comm.SerialPort;
import javax.comm.SerialPortEvent;
import javax.comm.SerialPortEventListener;
import javax.comm.UnsupportedCommOperationException;

/**
 * Class that handles a Bluetooth connection.
 */
public class SerialPortConnection implements SerialPortEventListener {
   static CommPortIdentifier portId;
   static String TimeStamp;
   protected SerialPort port;
   protected OutputStream outputStream;
   protected Writer outputWriter;
   protected BufferedReader inputReader;
   protected CloseableFrame caller;
   protected String defaultPort;
   protected boolean validOldPort;
   private Simulator simulator;
   private boolean portOpenAndReadyForUse = false;

   private Object lock = new Object();
   private boolean isWaitingOnEvent;
   private CancelDialog cancelDialog;
   private Vector portsOpenWhileListening;

   public SerialPortConnection(String defaultPort, CloseableFrame caller,
         Simulator simulator) {
      this.simulator = simulator;
      this.caller = caller;
      this.defaultPort = defaultPort;
      validOldPort = false;
      TimeStamp = new java.util.Date().toString();

      // If automatic port opening at startup is enabled (a non null port is
      // supplied)
      if (!defaultPort.equals("null")) {
         openPort(defaultPort);
      } else {
         System.out
               .println("SerialPortConnection.SerialPortConnection() no default port supplied, will not open port.");
      }
   }

   /**
    * Open a port to make it ready for communication.
    * 
    * @param portParam
    *           the port to open
    */
   private void openPort(String portParam) {
      System.out.println("SerialPortConnection.openPort() port: " + portParam);

      // Close a previously opened port
      if (port != null) {
         port.close();
      }

      // Open the new port
      try {
         portId = CommPortIdentifier.getPortIdentifier(portParam);
         port = (SerialPort) portId.open("SerialPortConnection", 2000);
         port.setSerialPortParams(115200, SerialPort.DATABITS_8,
               SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

         outputStream = port.getOutputStream();
         outputWriter = new OutputStreamWriter(outputStream);
         inputReader = new BufferedReader(new InputStreamReader(port
               .getInputStream()));

         // Success
         portOpenAndReadyForUse = true;
         System.out
               .println("SerialPortConnection.openPort() port was opened successfully");

      } catch (NoSuchPortException ne) {
         System.out
               .println("SerialPortConnection.openPort() Error, Port Not Found "
                     + ne);
         ErrorDialog err = new ErrorDialog(caller, "Port Not Found");
         err.addText("The port was not found: " + portParam);
         err.show();
      } catch (IOException ioe) {
         System.out
               .println("SerialPortConnection.openPort() Error, IOException"
                     + ioe);
         ErrorDialog err = new ErrorDialog(caller, "Error, IOException");
         err.addText("There was a problem when trying to open the port: "
               + portParam);
         err.show();
      } catch (PortInUseException piue) {
         System.out
               .println("SerialPortConnection.openPort() Error, Port in Use "
                     + piue);
         ErrorDialog err = new ErrorDialog(caller, "Error, Port in Use");
         err
               .addText("The port is already in use (by this or another program): "
                     + portParam);
         err.show();
      } catch (UnsupportedCommOperationException ucoe) {
         System.out
               .println("SerialPortConnection.openPort() Error, Unsupported Operation "
                     + ucoe);
         ErrorDialog err = new ErrorDialog(caller,
               "Error, Unsupported Operation");
         err.addText("The port does not support the operaton: " + portParam);
         err.show();
      } catch (Throwable t) {
         System.out
               .println("SerialPortConnection.openPort() Error, Other Error "
                     + t);
         ErrorDialog err = new ErrorDialog(caller, "Error, Other Error");
         err.addText("There was a problem when trying to open the port: "
               + portParam);
         err.show();
      }
   }

   /**
    * This is the procedure to listen for port activity and thereby
    * automatically select what port to use.
    */
   public void listenForPort() {
      isWaitingOnEvent = true;
      portOpenAndReadyForUse = false;
      portsOpenWhileListening = new Vector();
      
      // Go through all available ports
      for (Enumeration e = CommPortIdentifier.getPortIdentifiers(); e
            .hasMoreElements();) {

         CommPortIdentifier portIdentifier = (CommPortIdentifier) e
               .nextElement();
         String name = portIdentifier.getName();

         System.out.println("SerialPortConnection.listenForPort() port found: "
               + name);

         // Can only use serial ports
         if (portIdentifier.getPortType() != CommPortIdentifier.PORT_SERIAL) {
            System.out
                  .println("SerialPortConnection.listenForPort() non serial com port, excluding it: "
                        + name);
            continue;
         }
         
         if (portIdentifier.isCurrentlyOwned()) {
            System.out.println("SerialPortConnection.listenForPort() port already owned, excluding it, owner: " + portIdentifier.getCurrentOwner());
            continue;
         }

         SerialPort serialPort = null;

         // Open the port
         try {
            serialPort = (SerialPort) portIdentifier.open(
                  "BTGPSSimulator.listenForPort()", 2000);
         } catch (PortInUseException pe) {
            System.out
                  .println("SerialPortConnection.listenForPort() port is already used by another application, excluding it: "
                        + name);
            continue;
         } catch (ClassCastException cc) {
            System.out
                  .println("SerialPortConnection.listenForPort() port was not a serial port after all, excluding it: "
                        + name);
            continue;
         }

         // Start listening for events on the port
         if (serialPort != null) {
            try {
               serialPort.addEventListener(this); // This is first call in this
               // section in case of
               // exception
               serialPort.notifyOnDataAvailable(true); // This is the event
               // that is specific
               // when receiving a
               // connection.
               portsOpenWhileListening.add(serialPort);
            } catch (TooManyListenersException te) {
               System.out
                     .println("SerialPortConnection.listenForPort() there was already another listener for the port, excluding it: "
                           + name);
               serialPort.close();
               continue;
            }
         }
      }

      // If no ports found, then return
      if (portsOpenWhileListening.size() == 0) {
         System.out
               .println("SerialPortConnection.listenForPort() No available ports found");
         ErrorDialog errorDialog = new ErrorDialog(caller, "No Ports Found");
         errorDialog.addText("No available ports found.");
         errorDialog.show();
         return;
      }

      // Show a wait message
      cancelDialog = new CancelDialog(caller, "Waiting for Connections");
      cancelDialog
            .addText("Please connect a device, e.g. select your computer as the Bluetooth GPS in WF Navigator.");
      cancelDialog.show();

      isWaitingOnEvent = false;

      // Close all temporary ports
      for (int i = 0; i < portsOpenWhileListening.size(); i++) {
         SerialPort serialPort = (SerialPort) portsOpenWhileListening.get(i);
         serialPort.close();
      }
      portsOpenWhileListening.clear();

      // Confirm that a connection has been made
      if (portOpenAndReadyForUse) {

         // Save as default port
         simulator.savePort(port.getName());

         ErrorDialog errorDialog = new ErrorDialog(caller, "Success!");
         errorDialog.addText("Successfully connected to port: " + port);
         errorDialog.show();
      }
   }

   /**
    * Listen for events on all open ports. This is part of the listenForPort()
    * procedure.
    */
   public void serialEvent(SerialPortEvent event) {

      // Handle one event at a time
      synchronized (lock) {

         // Keep listening until the requested event has been received
         if (isWaitingOnEvent) {

            String portName = ((SerialPort) event.getSource()).getName();
            
            // This is the event we trigger on to detect a connection
            if (event.getEventType() == SerialPortEvent.DATA_AVAILABLE) {
               
               System.out
                     .println("SerialPortConnection.serialEvent() data is available, we connect to the data sender: "
                           + portName);

               // Close all temporary ports
               for (int i = 0; i < portsOpenWhileListening.size(); i++) {
                  SerialPort serialPort = (SerialPort) portsOpenWhileListening
                        .get(i);

                  if (serialPort.getName().equals(portName)) {
                     portsOpenWhileListening.remove(i);
                     serialPort.close();
                  }
               }

               // Open the targeted port in the regular way
               openPort(portName);

               // Hide the wait dialog
               cancelDialog.dispose();
               
            } else {
               System.out
               .println("SerialPortConnection.serialEvent() an event was discarded from: "
                     + portName + ", type: " + event.getEventType());
            }
         }
      }
   }

   /**
    * The dialog to manually select a port from a list.
    */
   public void changePort() {
      TimeStamp = new java.util.Date().toString();
      portOpenAndReadyForUse = false;

      // Show dialog
      PortDialog dialog = new PortDialog(defaultPort, caller);
      dialog.show();

      // Get user input
      String portName = dialog.getPortName();

      // If a port is available
      if (portName != null) {

         // Save port to settings file
         simulator.savePort(portName);

         // Open the port
         openPort(portName);
      }
      return;
   }

   /**
    * @return when a connection has been opened, positions to the client should
    *         be sent here.
    */
   public Writer getWriter() {
      return outputWriter;
   }

   /**
    * Perform termination activities
    */
   public void terminate() {

      /**
       * If a port is already open, then close it to get it included in the next
       * listenForPort() activity.
       */
      if (port != null) {
         port.close();
      }
   }

   /**
    * Check if the port is open and ready for use.
    */
   public boolean isPortOpenAndReadyForUse() {
      return portOpenAndReadyForUse;
   }

}
