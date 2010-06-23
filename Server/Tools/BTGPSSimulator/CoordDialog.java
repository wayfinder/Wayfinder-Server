import java.awt.Button;
import java.awt.Dialog;
import java.awt.Event;
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Panel;
import java.awt.TextField;

class CoordDialog extends Dialog {

   protected TextField mc2lat, mc2lon, wgs84lat, wgs84lon;
   protected int mc2Latitude, mc2Longitude;
   protected boolean valid = false;

   public CoordDialog(CloseableFrame parent, String title, int lat, int lon,
         boolean changable) {

      super(parent, title, true);
      mc2Latitude = lat;
      mc2Longitude = lon;
      String mc2lat_string = "" + mc2Latitude;
      String mc2lon_string = "" + mc2Longitude;
      float wgs84_x = (float) (mc2Longitude / 11930464.7111);
      float wgs84_y = (float) (mc2Latitude / 11930464.7111);
      String wgslat_string = "" + wgs84_y;
      String wgslon_string = "" + wgs84_x;
      Panel p1 = new Panel();
      if (changable) {
         p1.setLayout(new GridLayout(1, 3));
         p1.add(mc2lat = new TextField(mc2lat_string, 8));
         p1.add(mc2lon = new TextField(mc2lon_string, 8));
         p1.add(new Button("mc2"));
      } else {

         p1.add(new Label("mc2  :"));
         p1.add(new Label("(" + mc2lat_string + ", " + mc2lon_string + ")"));
      }

      add(p1, "North");

      Panel p2 = new Panel();
      if (changable) {
         p2.setLayout(new GridLayout(1, 3));
         p2.add(wgs84lat = new TextField(wgslat_string, 8));
         p2.add(wgs84lon = new TextField(wgslon_string, 8));
         p2.add(new Button("WGS84"));
      } else {

         p2.add(new Label("WGS84:"));
         p2.add(new Label("(" + wgslat_string + ", " + wgslon_string + ")"));
      }

      Panel p3 = new Panel();
      if (changable)
         p3.add(new Button("Cancel"));
      else
         p3.add(new Button("Dismiss"));
      add(p3, "South");
      add(p2, "Center");
      if (changable)
         resize(340, 110);
      else
         resize(230, 130);
   }

   public int getLon() {
      return mc2Longitude;
   }

   public int getLat() {
      return mc2Latitude;
   }

   public boolean isValid() {
      return valid;
   }

   public boolean action(Event evt, Object arg) {
      if (arg.equals("mc2")) {
         try {
            mc2Latitude = Integer.parseInt(mc2lat.getText());
            mc2Longitude = Integer.parseInt(mc2lon.getText());
            valid = true;
            dispose();
         } catch (NumberFormatException ine) {
            System.out.println("CoordDialog.action() mc2 NumberFormatException!");
            dispose();
         }

      }
      if (arg.equals("WGS84")) {
         try {
            double lat = Double.parseDouble(wgs84lat.getText());
            double lon = Double.parseDouble(wgs84lon.getText());
            mc2Latitude = (int) (lat * 11930464.7111);
            mc2Longitude = (int) (lon * 11930464.7111);
            valid = true;
            dispose();
         } catch (NumberFormatException ine) {
            System.out.println("CoordDialog.action() wgs84 NumberFormatException!");
            dispose();
         }

      } else if (arg.equals("Cancel") || arg.equals("Dismiss")) {
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
