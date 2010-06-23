import java.awt.AWTEvent;
import java.awt.event.WindowEvent;

import javax.swing.JFrame;

/**
 * A Frame that you can actually quit. Used as the starting point for most Java
 * 1.1 graphical applications.
 * 
 * Taken from Core Web Programming from Prentice Hall and Sun Microsystems
 * Press, http://www.corewebprogramming.com/. &copy; 2001 Marty Hall and Larry
 * Brown; may be freely used or adapted.
 */

public class CloseableFrame extends JFrame {
   public CloseableFrame(String title) {
      super(title);
      enableEvents(AWTEvent.WINDOW_EVENT_MASK);
   }

   public CloseableFrame() {
      enableEvents(AWTEvent.WINDOW_EVENT_MASK);
   }

   /**
    * Since we are doing something permanent, we need to call
    * super.processWindowEvent <B>first</B>.
    */

   public void processWindowEvent(WindowEvent event) {
      super.processWindowEvent(event); // Handle listeners.
      if (event.getID() == WindowEvent.WINDOW_CLOSING) {
         // If the frame is used in an applet, use dispose().
         System.exit(0);
      }
   }
}
