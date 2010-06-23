import java.awt.Choice;
import java.awt.Dimension;
import java.awt.Event;
import java.awt.Frame;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 *  Class that handles choice of Repeat mode.
 */
public class RepeatDialog extends JDialog implements ItemListener {
   protected Choice choiceBox;
   protected int repeat;
   protected boolean setAsDefault;
   protected int defRepeat;

   protected JPanel p1;
   protected JPanel p2;
   private JButton cancelButton;

   public RepeatDialog(int defaultRepeat, Frame caller) {
      super(caller, "Choose repeat mode", true);
      defRepeat = defaultRepeat;
      Toolkit tk = Toolkit.getDefaultToolkit();
      Dimension d = tk.getScreenSize();
      setLocation(d.width * 3 / 8 + 50, d.height * 3 / 8 - 30);
      p1 = new JPanel();
      p2 = new JPanel();
      add("Center", p1);
      repeat = defaultRepeat;
      choiceBox = new Choice();
      choiceBox.addItemListener(this);
      choiceBox.add("No Repeat");
      choiceBox.add("Repeat Loop");
      choiceBox.add("Repeat Reverse");
      p1.add(new JLabel("Choose Repeat Mode!"));
      p2.add(choiceBox);

      cancelButton = new JButton("Cancel");
      cancelButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            repeat = defRepeat;
            dispose();
         }
      });
      p2.add(cancelButton);
      add("South", p2);
      resize(400, 100);
   }

   public int getRepeat() {
      return repeat;
   }

   public void itemStateChanged(ItemEvent evt) {
      String state = (String) evt.getItem();
      if (state.equals("No Repeat")) {
         repeat = Simulator.NO_REPEAT;
      } else if (state.equals("Repeat Loop")) {
         repeat = Simulator.REPEAT_LOOP;
      } else if (state.equals("Repeat Reverse")) {
         repeat = Simulator.REPEAT_REVERSE;
      }

      setVisible(false);
   }

   public boolean action(Event evt, Object arg) {
      if (arg.equals("Cancel")) {
         repeat = defRepeat;
         dispose();
      } else
         return super.action(evt, arg);
      return true;
   }

   public boolean handleEvent(Event evt) {
      if (evt.id == Event.WINDOW_DESTROY) {
         repeat = defRepeat;
         dispose();
      } else
         return super.handleEvent(evt);
      return true;
   }

}
