import java.awt.Button;
import java.awt.Dialog;
import java.awt.Event;
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Panel;
import java.awt.TextField;

class SpeedDialog extends Dialog {

   protected TextField maxspeed;
   protected int speed;
   protected boolean valid = false;
   protected int current;

   public SpeedDialog(MainSimulatorWindow parent, String title, int current) {

      super(parent.getFrame(), title, true);
      String speed_string = "" + current;
      this.current = current;
      Panel p1 = new Panel();
      p1.setLayout(new GridLayout(1, 2));
      p1.add(new Label("New max speed : "));
      p1.add(maxspeed = new TextField(speed_string, 8));
      add(p1, "Center");

      Panel p2 = new Panel();
      p2.add(new Button("Ok"));
      p2.add(new Button("Cancel"));
      add(p2, "South");
      resize(240, 100);
   }

   public int getSpeed() {
      return speed;
   }

   public boolean isValid() {
      return valid;
   }

   public boolean action(Event evt, Object arg) {
      if (arg.equals("Ok")) {
         try {
            speed = Integer.parseInt(maxspeed.getText());
            valid = true;
            dispose();
         } catch (NumberFormatException ine) {
            speed = current;
            dispose();
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
