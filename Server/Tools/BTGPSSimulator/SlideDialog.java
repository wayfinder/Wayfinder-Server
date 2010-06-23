import java.awt.Button;
import java.awt.Dialog;
import java.awt.Event;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Panel;
import java.awt.Scrollbar;
import java.awt.event.AdjustmentEvent;
import java.awt.event.AdjustmentListener;

class SlideDialog extends Dialog implements AdjustmentListener {

   Panel p1;
   int h_size;
   private Scrollbar bar;
   private Label barLabel;
   protected int value;
   protected int startValue;

   public SlideDialog(Frame parent, String title, int startVal) {

      super(parent, title, true);
      startValue = startVal;
      value = startVal;
      Panel p1 = new Panel();
      p1.setLayout(new GridLayout(1, 2));
      p1.add(barLabel = new Label("GPS signal:"));
      p1.add(bar = new Scrollbar(Scrollbar.HORIZONTAL, value, 0, 0, 50));
      bar.setBlockIncrement(1);
      bar.addAdjustmentListener(this);
      add("Center", p1);

      Panel p2 = new Panel();
      p2.add(new Button("OK"));
      p2.add(new Button("Cancel"));
      add("South", p2);
      resize(400, 80);

   }

   public int getValue() {
      return value;
   }

   public void adjustmentValueChanged(AdjustmentEvent aev) {
      barLabel.setText("GPS signal:");
      value = bar.getValue();
      repaint();
   }

   public boolean action(Event evt, Object arg) {
      if (arg.equals("OK")) {
         dispose();
      } else if (arg.equals("Cancel")) {
         value = startValue;
         dispose();
      } else
         return super.action(evt, arg);
      return true;
   }

}
