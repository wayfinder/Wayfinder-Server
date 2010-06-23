import java.awt.Dialog;
import java.awt.Font;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

class LogWindow extends Dialog {
   public LogWindow(Frame parent, String routeName) {
      super(parent, "LogWindow", false);
      setTitle("Log " + routeName);
      setResizable(true);
      setSize(300, 450);
      // setBackground(Color.red);
      // setForeground(Color.white);
      addWindowListener(new WindowAdapter() {
         public void windowClosing(WindowEvent e) {
            setVisible(false);
         }
      });
   }

   public void log(String data) {

   }

   public void paint(Graphics g) {
      Font f = g.getFont();
      // setForeground(Color.white);
      // System.out.println("paint was called");
      g.drawString("LOGGING", 0, 0);
   }

}
