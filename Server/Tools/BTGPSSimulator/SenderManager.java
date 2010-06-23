import java.awt.Dimension;
import java.awt.Toolkit;

/**
 *  Thiss class manages the connection to the Bluetooth or Socket unit.
 *  <br />
 *  NB! Has been sligtly modified to allow use of threads.
 *  The code that does the sending now is in GPSSender and SocketGPSSender.
 *  That code could be even more re-written to get rid of SerialPortConnection and this
 *  class.
 */
public class SenderManager {
   protected SerialPortConnection portConnection;
   protected CloseableFrame caller;
   private GPSSender gpsSender;
   private GPSSenderListener gpsSenderListener;
   private Simulator simulator;
   private MainSimulatorWindow mainWindow;

   public SenderManager(String defPort, MainSimulatorWindow mainWindow,
         Simulator simulator, GPSSenderListener listener) {
      gpsSenderListener = listener;
      caller = mainWindow.getFrame();
      portConnection = new SerialPortConnection(defPort, caller, simulator);
      this.simulator = simulator;
      this.mainWindow = mainWindow;

      // If possible to start GPS-sender
      if (portConnection.isPortOpenAndReadyForUse()) {
         gpsSender = new GPSSender(portConnection.getWriter(), gpsSenderListener,
               mainWindow);
         gpsSender.start();
      }
   }

   /**
    * Determine a port by listening for incoming connections.
    */
   public void setPortAutomatically() {
      System.out.println("SenderManager.setPortAutomatically()");

      // Terminate previous sender
      if (gpsSender != null) {
         gpsSender.terminate();
      }
      if (portConnection != null) {
         portConnection.terminate();
      }

      // Determine and open port
      portConnection.listenForPort();

      // If possible to start GPS-sender
      if (portConnection.isPortOpenAndReadyForUse()) {
         gpsSender = new GPSSender(portConnection.getWriter(), gpsSenderListener,
               mainWindow);
         gpsSender.start();
      }
   }

   /**
    * Select a port from a list of ports.
    */
   public void changePort() {

      // Terminate previous sender
      if (gpsSender != null) {
         gpsSender.terminate();
      }
      if (portConnection != null) {
         portConnection.terminate();
      }

      // Determine and open port
      portConnection.changePort();

      // If possible to start GPS-sender
      if (portConnection.isPortOpenAndReadyForUse()) {
         gpsSender = new GPSSender(portConnection.getWriter(), gpsSenderListener,
               mainWindow);
         gpsSender.start();
      }
   }

   /**
    * Connect to a remote socket host.
    * @param socketHost
    * @param socketPort
    * @return
    */
   public boolean setSocket(String socketHost, int socketPort) {

      // Terminate previous sender
      if (gpsSender != null) {
         gpsSender.terminate();
      }

      Toolkit tk = Toolkit.getDefaultToolkit();
      Dimension d = tk.getScreenSize();

      int port = socketPort;
      String host = socketHost;

      // get Host & port
      SocketDialog dialog = new SocketDialog(caller, "Enter Host & Port",
            socketPort, socketHost);
      dialog.setLocation(d.width * 3 / 8 + 50, d.height * 3 / 8 - 30);
      dialog.show();

      if (!dialog.isValid()) {
         System.out
               .println("SenderManager.setSocket() dialog not valid, returning.");
         return false;
      }
      host = dialog.getHostName();
      port = dialog.getPort();

      simulator.saveSocket(host, port);

      gpsSender = new SocketGPSSender(host, port, gpsSenderListener);
      gpsSender.start();

      return true;
   }

   /**
    * Listen for incoming socket connections.
    * @param socketPort
    * @return
    */
   public boolean setServerSocket(int socketPort) {

      // Terminate previous sender
      if (gpsSender != null) {
         gpsSender.terminate();
      }

      Toolkit tk = Toolkit.getDefaultToolkit();
      Dimension d = tk.getScreenSize();

      int port = socketPort;

      // get Host & port
      SocketDialog dialog = new SocketDialog(caller, "Enter port", socketPort);
      dialog.setLocation(d.width * 3 / 8 + 50, d.height * 3 / 8 - 30);
      dialog.show();

      if (!dialog.isValid()) {
         System.out
               .println("SenderManager.setServerSocket() dialog not valid, returning.");
         return false;
      }
      port = dialog.getPort();

      simulator.saveSocket(null, port);

      gpsSender = new SocketGPSSender(port, gpsSenderListener);
      gpsSender.start();

      return true;
   }

   /**
    * Put a message in the outgoing queue.
    * @param message
    * @return
    */
   public boolean sendMessage(NMEAMessage message) {
      if (gpsSender != null) {
         return gpsSender.enqueue(message);
      } else {
         return false;
      }
   }

}
