import java.awt.Image;
import java.awt.Toolkit;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;

class SPMap {

   private String serverURL;
   private String server;
   private SPDesc spDesc;
   private ArrayList poiList = new ArrayList();

   private class POIEntry {

      public int dx;
      public int dy;
      public Image img;
      public String name;

      public POIEntry(int dx, int dy, Image img, String name) {
         this.dx = dx;
         this.dy = dy;
         this.img = img;
         this.name = name;
      }

   }

   public SPMap(SPDesc spDesc, int x, int y, int zoom) {
      serverURL = spDesc.getServer() + ":12211/SPMap?x=" + x + "&y=" + y
            + "&zoom=" + zoom + "&lang=en";
      //System.out.println(serverURL);
      this.spDesc = spDesc;
      try {
         init();
      } catch (Exception e) {
         System.err.println("SPMAP.SPMap() " + e);
      }
   }

   public void init() throws IOException, MalformedURLException {
      URL url = new URL(serverURL);
      HttpURLConnection conn = (HttpURLConnection) url.openConnection();
      conn.setRequestProperty("Accept-Charset", "utf-8");
      DataInputStream di = new DataInputStream(conn.getInputStream());

      //System.out.println("\nSize = " + di.readInt());
      di.readInt();

      int b = di.readUnsignedByte();

      int coords16Bit = ((b & 0x80) >> 7);
      //System.out.println("Coords16Bit = "+ coords16Bit);

      int type16Bit = ((b & 0x40) >> 6);
      //System.out.println("Type16Bit = "+ type16Bit + "\n");

      int nbr;
      while ((nbr = di.readUnsignedByte()) != 0) {
         //System.out.println("\nNBR_POIS = " + nbr + "\n");
         for (int i = 0; i < nbr; i++) {
            int type;
            if (type16Bit == 0)
               type = di.readUnsignedByte();
            else
               type = di.readUnsignedShort();

            int x, y;
            if (coords16Bit == 0) {
               x = di.readUnsignedByte();
               y = di.readUnsignedByte();
            } else {
               x = di.readUnsignedShort();
               y = di.readUnsignedShort();
            }

            //System.out.println("Type = " + type);
            url = spDesc.getBitmapURL(type);

            String name;
            ByteArrayOutputStream bais = new ByteArrayOutputStream();
            while ((char) (b = di.readByte()) != '\0')
               bais.write((byte) b);
            name = bais.toString("utf-8");
            bais.close();
            //System.out.println("Name = " + name);

            while ((char) (b = di.readUnsignedByte()) != '\0') {
            }
            //sb.append((char) b);
            //System.out.println("Extra info = " + sb.toString() + "\n");

            if (url != null) {
               //System.out.println("x=" + x + " y=" + y + " url=" + url.toString()+"\n");
               Image img = Toolkit.getDefaultToolkit().getImage(url);
               poiList.add(new POIEntry(x, y, img, name));
            } else {
               //System.out.println("No match!");
            }
         }
      }
      di.close();
      //System.out.println("Done!");
   }

   public Image getImage(int i) {
      return ((POIEntry) poiList.get(i)).img;
   }

   public int getDx(int i) {
      return ((POIEntry) poiList.get(i)).dx;
   }

   public int getDy(int i) {
      return ((POIEntry) poiList.get(i)).dy;
   }

   public String getName(int i) {
      return ((POIEntry) poiList.get(i)).name;
   }

   public int getNbrPoi() {
      return poiList.size();
   }

   public void setSPDesc(SPDesc spDesc) {
      this.spDesc = spDesc;
   }
}
