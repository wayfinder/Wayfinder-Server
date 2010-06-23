import java.awt.Dimension;

import javax.swing.JEditorPane;
import javax.swing.JFrame;

class HelpDialog extends JFrame 
{
   protected JEditorPane htmlPane = new JEditorPane();
   
   public HelpDialog(){
      setTitle("BTGPSSimulator Help");
      
      htmlPane.setText("Please read the Readme.txt for help.");      
      getContentPane().add(htmlPane);
      // set the dimensions for this frame
      setSize(600, 700);
      Dimension ss = java.awt.Toolkit.getDefaultToolkit().getScreenSize();
      setLocation((int)(ss.getWidth()-getSize().getWidth()),
                  0 //(int)(ss.getHeight()-getSize().getHeight())/2
                  );
   

      
   }
   
   
}

