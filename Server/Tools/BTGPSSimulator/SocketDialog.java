import java.awt.Button;
import java.awt.Dialog;
import java.awt.Event;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Panel;
import java.awt.TextField;

class SocketDialog extends Dialog {

   protected TextField hostname;
   protected TextField portname;
   protected int portNr = -1;
   protected boolean valid = false;

   public SocketDialog(Frame parent, String title, int defPort, String defHost) {

      super(parent, title, true);
      String port = Integer.toString(defPort);
      Panel p1 = new Panel();
      p1.setLayout(new GridLayout(2, 2));
      p1.add(new Label("host : "));
      p1.add(hostname = new TextField(defHost, 8));
      p1.add(new Label("port : "));
      p1.add(portname = new TextField(port, 8));
      add(p1, "Center");

      Panel p2 = new Panel();
      p2.add(new Button("Ok"));
      p2.add(new Button("Cancel"));
      add(p2, "South");
      resize(240, 120);
   }

   public SocketDialog(Frame parent, String title, int defPort) {
      super(parent, title, true);
      String port = Integer.toString(defPort);
      Panel p1 = new Panel();
      p1.setLayout(new GridLayout(2, 2));
      p1.add(new Label("port : "));
      p1.add(portname = new TextField(port, 8));
      add(p1, "Center");
      Panel p2 = new Panel();
      p2.add(new Button("Ok"));
      p2.add(new Button("Cancel"));
      add(p2, "South");
      resize(240, 120);
   }

   public String getHostName() {
      return hostname.getText();
   }

   public int getPort() {
      return portNr;
   }

   public boolean isValid() {
      return valid;
   }

   public boolean action(Event evt, Object arg) {
      if (arg.equals("Ok")) {
         try {
            portNr = Integer.parseInt(portname.getText());
            valid = true;
            dispose();
         } catch (NumberFormatException ine) {
            return false;
         }

      } else if (arg.equals("Cancel")) {
         dispose();
      } else
         return super.action(evt, arg);
      return true;
   }

   public boolean handleEvent(Event evt) {
      if (evt.id == Event.WINDOW_DESTROY)
         dispose();
      else
         return super.handleEvent(evt);
      return true;
   }

}
