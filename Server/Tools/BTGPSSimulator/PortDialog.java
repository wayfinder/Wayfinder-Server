import java.awt.Choice;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.IOException;
import java.io.InputStream;
import java.util.Enumeration;
import java.util.Hashtable;

import javax.comm.CommPortIdentifier;
import javax.comm.NoSuchPortException;
import javax.comm.PortInUseException;
import javax.comm.SerialPort;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

/**
 *  Class that handles choice of BT port.
 */
public class PortDialog extends JDialog implements ItemListener {

   static String TimeStamp;
   private Choice choiceBox;
   private SerialPort port;
   private String defaultPortName;
   private String portName;

   private JPanel p1;
   private JPanel p2;

   /// Ugglehack
   static Hashtable c_ports = new Hashtable();

   public PortDialog(String defaultPort, Frame caller) {
      super(caller, "Select Port", true);
      Toolkit tk = Toolkit.getDefaultToolkit();
      Dimension d = tk.getScreenSize();
      setLocation(d.width * 3 / 8 + 50, d.height * 3 / 8 - 30);
      p1 = new JPanel();
      p2 = new JPanel();
      getContentPane().add("Center", p1);
      portName = null;
      defaultPortName = defaultPort;
      choiceBox = new Choice();
      choiceBox.addItemListener(this);

      /**
      try {
      	//System.out.println("Default port == " + defaultPort);
         defaultPort = CommPortIdentifier.getPortIdentifier(defaultPort);
         portOfChoice = defaultPort;
      }
      catch(NoSuchPortException ne){
      	//System.out.println(ne.getMessage());
         defaultPort = null;
      }
       */

      addWindowListener(new WindowAdapter() {
         public void windowClosing(WindowEvent e) {
            portName = null;
            dispose();
         }
      });

      JButton okButton = new JButton("OK");
      okButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            portName = choiceBox.getSelectedItem();
            dispose();
         }
      });

      JButton cancelButton = new JButton("Cancel");
      cancelButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            portName = null;
            dispose();
         }
      });

      if (!listPorts()) {
         JOptionPane.showMessageDialog(this, "No COM-ports could be found.",
               "Port Error", JOptionPane.ERROR_MESSAGE);
         okButton.setEnabled(false);
      }
      p1.add(new JLabel("Select a Bluetooth-serial-port from the list."));
      p2.add(choiceBox);
      p2.add(okButton);
      p2.add(cancelButton);
      getContentPane().add("South", p2);
      resize(300, 100);
   }

   private boolean listPorts() {
      CommPortIdentifier defaultPort;
      try {
         defaultPort = CommPortIdentifier.getPortIdentifier(defaultPortName);
      } catch (NoSuchPortException e) {
         defaultPort = null;
      }
      Enumeration ports = defaultPort.getPortIdentifiers();
      String name;
      int added = 0;
      for (Enumeration e = ports; e.hasMoreElements();) {

         CommPortIdentifier portIdentifier = (CommPortIdentifier) e
               .nextElement();
         name = portIdentifier.getName();

         int type = portIdentifier.getPortType();

         // Can only use serial ports
         if (type != CommPortIdentifier.PORT_SERIAL) {
            System.out
                  .println("PortDialog.listenForPort() non serial com port, excluding it: "
                        + name);
            continue;
         } else {
            if (name.equals(defaultPortName)) {
               choiceBox.insert(name, 0);
            } else {
               choiceBox.add(name);
            }

            added++;
         }
      }
      return (added != 0);
   }

   /**
   public CommPortIdentifier getPort() {
      return portOfChoice;
   }
    */

   private SerialPort getOpenPort(String portName) {
      Object found = c_ports.get(portName);
      SerialPort port = null;
      CommPortIdentifier portId = null;
      try {
         portId = CommPortIdentifier.getPortIdentifier(portName);
      } catch (Exception e) {
      }
      if (found == null) {
         try {
            port = (SerialPort) portId.open("SerialPortConnection", 2000);
         } catch (PortInUseException e) {
            return null;
         }
         c_ports.put(portName, port);
      } else {
         port = (SerialPort) found;
      }
      return port;
   }

   /**
   public OutputStream getOutputStream() throws IOException {
      if ( portOfChoice != null ) {
         SerialPort port = getOpenPort( portOfChoice );
         if ( port != null ) {
            return port.getOutputStream();
         } else {
            throw new IOException( "port is null");
         }
      } else {
         throw new IOException( "port id is null");
      }
   }
    */

   public InputStream getInputStream() throws IOException {
      if (portName != null) {
         SerialPort port = getOpenPort(portName);
         if (port != null) {
            return port.getInputStream();
         } else {
            throw new IOException("port is null");
         }
      } else {
         throw new IOException("port id is null");
      }
   }

   public String getPortName() {
      return portName;
   }

   public void itemStateChanged(ItemEvent e) {

   }

   /**
   public boolean action(Event evt, Object arg) {
      if(arg.equals("Ok")){
         //String portName = (String)evt.getItem();
         portName = choiceBox.getSelectedItem();
         if(portName.equals(defaultName)){
            isValidChange = false;
         } else {
         
            try{
               
               portOfChoice = CommPortIdentifier.getPortIdentifier(portName);
               isValidChange = true;
               // setVisible(false);
            }
            catch (NoSuchPortException ne) {
               p1.removeAll();
               p1.add(new Label("Unable to find port!"));
               p1.add(new Label(" ????????"));
               p2.removeAll();
               Button ok = new Button("Ok");
               p2.add(ok);
               ok.addActionListener(new ActionListener() {
                     public void actionPerformed(ActionEvent ae)
                        { setVisible(false); }});
               addWindowListener(new WindowAdapter() {
                     public void windowClosing(WindowEvent e)
                        { setVisible(false); }});
            }
         }
         dispose();
      }
      else if (arg.equals("Cancel")){
         setAsDefault = false;
         isValidChange = false;
         portOfChoice = defaultPort;
         dispose();
      }
      else
         return super.action(evt, arg);
      return true;
   }
    */

   /**
   public boolean handleEvent(Event evt){
      if(evt.id == Event.WINDOW_DESTROY){
         setAsDefault = false;
         isValidChange = false;
         portOfChoice = defaultPort;
         dispose();
      } else
         return super.handleEvent(evt);
      return true; 
   }
    */

}
