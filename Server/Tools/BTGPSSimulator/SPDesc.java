import java.io.DataInputStream;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.HashMap;

class SPDesc {

   private String urlBase;
   private String server;
   private String serverURL;
   private boolean descRead = false;
   private HashMap bitMapNamesPerType;

   public SPDesc(String server) {
      bitMapNamesPerType = new HashMap();
      serverURL = "http://" + server + ":12211";
      this.server = server;
      System.out.println("SPDesc.SPDesc() Retreiving POI Desc...");
      try {
         init();
      } catch (IOException e) {
         System.out.println("SPDesc.SPDesc() No POI Desc found!");
         return;
      }
      descRead = true;
      System.out.println("SPDesc.SPDesc() Done!");
   }

   private void init() throws IOException, MalformedURLException {
      URL url = new URL(serverURL + "/SPDesc?lang=en");
      HttpURLConnection conn = (HttpURLConnection) url.openConnection();
      conn.setRequestProperty("Accept-Charset", "utf-8");
      DataInputStream di = new DataInputStream(conn.getInputStream());

      di.skipBytes(4); //Magic
      //System.out.println("Size = " + di.readInt());
      di.readInt();
      StringBuffer sb = new StringBuffer();
      int b;
      while ((char) (b = di.readUnsignedByte()) != '\0')
         sb.append((char) b);
      urlBase = sb.toString();
      //System.out.println("URLBase = " + urlBase);

      int nbrBytesPerType = di.readUnsignedByte();
      //System.out.println("NbrBytesPerType = " + nbrBytesPerType);
      int nbrEntries = di.readInt();
      for (int i = 0; i < nbrEntries; i++) {
         int type;
         String bitMapName;
         sb = new StringBuffer();
         if (nbrBytesPerType == 1)
            type = di.readUnsignedByte();
         else
            type = di.readUnsignedShort();

         sb = new StringBuffer();
         while ((char) (b = di.readUnsignedByte()) != '\0')
            sb.append((char) b);
         bitMapName = sb.toString();

         while ((char) (b = di.readUnsignedByte()) != '\0') {
         }

         //System.out.println("Type = " + type);
         //System.out.println("Bitmap Name = " + bitMapName);
         bitMapNamesPerType.put(new Integer(type), bitMapName);
      }
      di.close();
   }

   public URL getBitmapURL(int type) {
      String bitMapName = (String) bitMapNamesPerType.get(new Integer(type));
      if (bitMapName == null) {
         //System.out.println("Could not find BitMap");
         return null;
      }
      URL url = null;
      try {
         url = new URL(serverURL + urlBase + bitMapName + ".png");
      } catch (Exception e) {
      }

      return url;
   }

   public void setServer(String server) {
      this.server = server;
   }

   public String getServer() {
      return server;
   }

   public boolean descRead() {
      return descRead;
   }
}
