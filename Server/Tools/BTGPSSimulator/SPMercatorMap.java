import java.awt.Graphics;
import java.awt.Image;
import java.awt.Toolkit;
import java.awt.image.ImageObserver;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;

class SPMercatorMap extends MercatorMap {

   private String serverURL;
   private String server;
   private SPDesc spDesc;
   private MercatorMapComponent mapComp;
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

   public SPMercatorMap(MercatorMapComponent mapComp, SPDesc spDesc,
         MercatorMapParams params, int width, int height) {
      super(params, width, height);
      this.spDesc = spDesc;
      this.mapComp = mapComp;
   }

   public void load(byte[] data) {
      if (data == null) {
         return;
      }
      try {
         load2(data);
      } catch (IOException e) {
         e.printStackTrace();
      }
   }

   public void load2(byte[] data) throws IOException {
      DataInputStream di = new DataInputStream(new ByteArrayInputStream(data));

      //System.out.println("\nSize = " + di.readInt());
      di.readInt();

      int b = di.readUnsignedByte();

      int coords16Bit = ((b & 0x40) >> 7);
      //System.out.println("Coords16Bit = "+ coords16Bit);

      int type16Bit = ((b & 0x80) >> 6);
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

            int dx, dy;
            if (coords16Bit == 0) {
               dx = di.readUnsignedByte();
               dy = di.readUnsignedByte();
            } else {
               dx = di.readUnsignedShort();
               dy = di.readUnsignedShort();
            }

            //System.out.println("Type = " + type);
            // FIXME: No communication allowed!!!
            URL url = spDesc.getBitmapURL(type);

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
               Image img = Toolkit.getDefaultToolkit().getImage(url);
               poiList.add(new POIEntry(dx, dy, img, name));
               int mc2_x = mapComp.mapXToMc2((double) (x + dx));
               int mc2_y = mapComp.mapYToMc2((double) (y + height - dy));
               mapComp.addPoiPoint(mc2_x, mc2_y, name);
            } else {
               //System.out.println("No match!");
            }
         }
      }
      di.close();
      //System.out.println("Done!");
   }

   public void draw(Graphics g, ImageObserver obs, int left_world_x,
         int top_world_y) {
      int imageX = x - left_world_x;
      int imageY = top_world_y - y - height;
      for (int i = 0; i < poiList.size(); ++i) {
         Image img = ((POIEntry) poiList.get(i)).img;
         int x = imageX + ((POIEntry) poiList.get(i)).dx - img.getWidth(obs)
               / 2;
         int y = imageY + ((POIEntry) poiList.get(i)).dy - img.getHeight(obs)
               / 2;
         //System.out.println("POI: " + x + " " + y);
         g.drawImage(img, x, y, obs);
      }
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
