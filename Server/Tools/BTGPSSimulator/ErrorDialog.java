import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;

class ErrorDialog extends JDialog {

   JPanel p1;
   int h_size;

   public ErrorDialog(JFrame parent, String title) {

      super(parent, title, true);
      // Default location.
      Toolkit tk = Toolkit.getDefaultToolkit();
      Dimension d = tk.getScreenSize();
      setLocation(d.width * 3 / 8 + 50, d.height * 3 / 8 - 30);

      p1 = new JPanel();
      p1.add(new JLabel());

      getContentPane().add(p1, "Center");

      JPanel p2 = new JPanel();
      JButton ok = new JButton("Ok");
      p2.add(ok);
      getContentPane().add(p2, "South");
      ok.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent ae) {
            setVisible(false);
         }
      });
      addWindowListener(new WindowAdapter() {
         public void windowClosing(WindowEvent e) {
            setVisible(false);
         }
      });
      h_size = 100;
      setSize(300, h_size);
   }

   public void addText(String text) {
      p1.add(new JLabel(text));
      h_size += 20;
      setSize(300, h_size);
   }
}
