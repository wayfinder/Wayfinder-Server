import java.awt.Color;
import java.awt.GridLayout;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.KeyStroke;
import javax.swing.border.TitledBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.text.NumberFormatter;

public class FilterDialog extends JDialog implements PropertyChangeListener,
      ChangeListener, ItemListener {

   private int value = 0;
   private String dist = "Rectangular";
   private JFormattedTextField textField;
   private JSlider slider;
   private JLabel valueLabel = new JLabel("N =");
   private JComboBox distBox;
   private NumberFormatter formatter;
   private JPanel paramPanel = new JPanel();

   private final int MAX_RECT = 50;
   private final int MAX_NORMAL = 50;
   private final String[] distributions = { "Rectangular", "Normal" };

   public FilterDialog(JFrame caller) {
      super(caller, "Distortion Filter", true);
      GridLayout layout = new GridLayout(3, 1);
      layout.setVgap(3);
      getContentPane().setLayout(layout);

      distBox = new JComboBox(distributions);
      distBox.addItemListener(this);

      JPanel distPanel = new JPanel();
      TitledBorder title = BorderFactory.createTitledBorder(BorderFactory
            .createLineBorder(Color.black), " Distribution ");
      distPanel.setBorder(title);
      distPanel.add(distBox);

      java.text.NumberFormat numberFormat = java.text.NumberFormat
            .getIntegerInstance();
      formatter = new NumberFormatter(numberFormat);
      formatter.setMinimum(new Integer(1));
      formatter.setMaximum(new Integer(MAX_RECT));

      textField = new JFormattedTextField(formatter);
      textField.setValue(new Integer(MAX_RECT / 2));
      textField.setColumns(3);

      Action commitAction = new AbstractAction() {
         public void actionPerformed(ActionEvent e) {
            if (!textField.isEditValid()) { // The text is invalid.
               Toolkit.getDefaultToolkit().beep();
               textField.selectAll();
            } else
               try { // The text is valid,
                  textField.commitEdit(); // so use it.
               } catch (java.text.ParseException exc) {
               }
         }
      };

      textField.getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0),
            "check");
      textField.getActionMap().put("check", commitAction);
      textField.addPropertyChangeListener(this);

      slider = new JSlider(JSlider.HORIZONTAL, 1, MAX_RECT, MAX_RECT / 2);
      slider.addChangeListener(this);

      JPanel labelAndTextField = new JPanel();
      labelAndTextField.add(valueLabel);
      labelAndTextField.add(textField);

      paramPanel.setLayout(new BoxLayout(paramPanel, BoxLayout.Y_AXIS));
      title = BorderFactory.createTitledBorder(BorderFactory
            .createLineBorder(Color.black), " Interval (-N,N) meters");
      // title.setTitleJustification(TitledBorder.CENTER);
      paramPanel.setBorder(title);
      paramPanel.add(labelAndTextField);
      paramPanel.add(slider);

      JPanel buttonPanel = new JPanel();

      JButton applyButton = new JButton("Apply");
      applyButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            value = slider.getValue();
            dist = (String) distBox.getSelectedItem();
            dispose();
         }
      });
      buttonPanel.add(applyButton);

      JButton cancelButton = new JButton("Cancel");
      cancelButton.addActionListener(new ActionListener() {
         public void actionPerformed(ActionEvent e) {
            dispose();
         }
      });
      buttonPanel.add(cancelButton);

      getContentPane().add(distPanel);
      getContentPane().add(paramPanel);
      getContentPane().add(buttonPanel);
      pack();
      setResizable(false);
   }

   public String getDistribution() {
      return dist;
   }

   public int getValue() {
      return value;
   }

   public void propertyChange(PropertyChangeEvent e) {
      if ("value".equals(e.getPropertyName())) {
         Number value = (Number) e.getNewValue();
         if (slider != null && value != null)
            slider.setValue(value.intValue());
      }
   }

   public void stateChanged(ChangeEvent e) {
      JSlider source = (JSlider) e.getSource();
      int value = (int) source.getValue();
      if (!source.getValueIsAdjusting()) { // done adjusting
         textField.setValue(new Integer(value));
      } else { // value is adjusting; just set the text
         textField.setText(String.valueOf(value));
      }
   }

   public void itemStateChanged(ItemEvent e) {
      TitledBorder title = BorderFactory.createTitledBorder(BorderFactory
            .createLineBorder(Color.black), " Interval (-N,N) meters");
      if (((String) distBox.getSelectedItem()).equals(distributions[0])) {
         title = BorderFactory.createTitledBorder(BorderFactory
               .createLineBorder(Color.black), " Interval (-N,N) meters");
         valueLabel.setText("N =");
         formatter.setMaximum(new Integer(MAX_RECT));
         textField.setValue(new Integer(MAX_RECT / 2));
         slider.setMaximum(MAX_RECT);
         slider.setValue(MAX_RECT / 2);
      } else if (((String) distBox.getSelectedItem()).equals(distributions[1])) {
         title = BorderFactory.createTitledBorder(BorderFactory
               .createLineBorder(Color.black), " Standard deviation D meters");
         valueLabel.setText("D =");
         formatter.setMaximum(new Integer(MAX_NORMAL));
         textField.setValue(new Integer(MAX_NORMAL / 2));
         slider.setMaximum(MAX_NORMAL);
         slider.setValue(MAX_NORMAL / 2);
      }
      paramPanel.setBorder(title);
   }

}
