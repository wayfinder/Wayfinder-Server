/**
 */
import java.io.FileReader;
import java.io.IOException;
import java.io.LineNumberReader;
import java.util.ArrayList;
import java.util.Collection;

class NMEALoader {

   private NMEAMessage[] data;
   private String fileName;

   NMEALoader(String filename) {
      fileName = filename;
      data = new NMEAMessage[0];
   }

   NMEAMessage makeMessage(String row) {
      String[] splitStr = Util.splitStr(row, " ");
      try {
         long timeStamp = Long.parseLong(splitStr[0]);
         if (!String.valueOf(timeStamp).equals(splitStr[0])) {
            // Not exactly one number. Inspired by Indians.
            throw new NumberFormatException("xox");
         }
         // Now the rest must be the real GPS message.
         StringBuffer buf = new StringBuffer(100);
         String maybeSpace = "";
         for (int i = 1; i < splitStr.length; ++i) {
            buf.append(maybeSpace);
            buf.append(splitStr[i]);
            maybeSpace = " ";
         }
         // Copy the string buffer once to get rid of the 100 bytes.
         return new NMEAMessage(new String(buf.toString()));
      } catch (NumberFormatException e) {
         // Not one with a timestamp
         return new NMEAMessage(row);
      }
   }

   void load() throws IOException {
      LineNumberReader reader = new LineNumberReader(new FileReader(fileName));

      Collection list = new ArrayList();
      NMEAMessage msg = null;
      for (String cur_row = reader.readLine(); cur_row != null; cur_row = reader
            .readLine()) {
         final NMEAMessage tmpMsg = makeMessage(cur_row + "\r\n");
         tmpMsg.updateTime(msg);
         msg = tmpMsg;
         list.add(msg);
      }
      data = (NMEAMessage[]) list.toArray(new NMEAMessage[0]);
   }

   NMEAMessage[] getLoadedRoute() {
      return data;
   }

}
