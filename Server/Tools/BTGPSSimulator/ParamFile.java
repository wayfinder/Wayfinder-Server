/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.LineNumberReader;
import java.io.PrintWriter;
import java.util.Iterator;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.Set;
import java.util.StringTokenizer;
import java.util.TreeMap;


/** Class to handle the parameter file */
class ParamFile {

   private Map paramVals = new TreeMap();
   private String fileName = new String();
   /**
    * the directory to load the param file from. No trailing '/'.
    */
   private String dir;

   public ParamFile(String fileName) {
      final String fname = "ParamFile.ParamFile(): ";

      this.fileName = fileName;
      System.out.println(fname + "file name: " + fileName);

      // Do not want to save in "program files" since it requires admin rights.
      // Instead use documents and settings folder for user. This is the best
      // platform independent place available.
      dir = System.getProperty("user.home") + File.separatorChar;

      System.out.println(fname + "directory: " + dir);
      // defalt values
      paramVals.put("PORT", "null"); // A null value means no auto start
      paramVals.put("SERVER", "localhost");
      paramVals.put("STARTX", "157362825"); // wayfinder x
      paramVals.put("STARTY", "664749105"); // wayfinder y
      paramVals.put("LOADDIR", "./routes");
      paramVals.put("LOADFILE", "");
      paramVals.put("SOCKETHOST", "host");
      paramVals.put("SOCKETPORT", "8000");
      paramVals.put("LOG_DIR", ".");
      paramVals.put("LOG_FIL", "gpslog.txt");
      paramVals.put("READPORT", "COM7");
      paramVals.put("NMEADIR", ".");
      paramVals.put("NMEAFILE", "nmea2.txt");
      paramVals.put("REPEAT_MODE", "4");
      paramVals.put("DEFAULT_SEND_TIME_INTERVAL", "1000");
      paramVals.put("XML_LOGIN", "login");
      paramVals.put("XML_PASSWORD", "password");
   }

   public String getDefaultPort() {
      return (String) paramVals.get("PORT");
   }

   public String getServer() {
      return (String) paramVals.get("SERVER");
   }

   public int getCentralX() {
      return Integer.parseInt((String) paramVals.get("STARTX"));
   }

   public int getCentralY() {
      return Integer.parseInt((String) paramVals.get("STARTY"));
   }

   public String getLoadDir() {
      return (String) paramVals.get("LOADDIR");
   }

   public String getLoadFile() {
      return (String) paramVals.get("LOADFILE");
   }

   public String getSocketHost() {
      return (String) paramVals.get("SOCKETHOST");
   }

   public int getSocketPort() {
      return Integer.parseInt((String) paramVals.get("SOCKETPORT"));
   }

   public String getLogDir() {
      return (String) paramVals.get("LOG_DIR");
   }

   public String getLogFile() {
      return (String) paramVals.get("LOG_FIL");
   }

   public String getReadPort() {
      return (String) paramVals.get("READPORT");
   }

   public String getNMEADir() {
      return (String) paramVals.get("NMEADIR");
   }

   public String getNMEAFile() {
      return (String) paramVals.get("NMEAFILE");
   }

   public int getRepeatMode() {
      return Integer.parseInt((String) paramVals.get("REPEAT_MODE"));
   }

   public int getDefaultSendTimeInterval() {
      return Integer.parseInt((String) paramVals
            .get("DEFAULT_SEND_TIME_INTERVAL"));
   }

   public String getLogin() {
      return (String) paramVals.get("XML_LOGIN");
   }

   public String getPassword() {
      return (String) paramVals.get("XML_PASSWORD");
   }

   public void setServer(String server) {
      paramVals.put("SERVER", server);
   }

   public void setDefaultPort(String port) {
      paramVals.put("PORT", port);
   }

   public void setLoadDir(String dir) {
      paramVals.put("LOADDIR", dir);
   }

   public void setLoadFile(String file) {
      paramVals.put("LOADFILE", file);
   }

   public void setSocketHost(String host) {
      paramVals.put("SOCKETHOST", host);
   }

   public void setSocketPort(int port) {
      paramVals.put("SOCKETPORT", String.valueOf(port));
   }

   public void setLogDir(String dir) {
      paramVals.put("LOG_DIR", dir);
   }

   public void setLogFile(String file) {
      paramVals.put("LOG_FIL", file);
   }

   public void setReadPort(String port) {
      paramVals.put("READPORT", port);
   }

   public void setNMEAFile(String file) {
      paramVals.put("NMEAFILE", file);
   }

   public void setNMEADir(String dir) {
      paramVals.put("NMEADIR", dir);
   }

   public void setRepeatMode(int mode) {
      paramVals.put("REPEAT_MODE", String.valueOf(mode));
   }

   public void load() {
      final String fname = "ParamFile.load(): ";

      // FIXME: would be better to use standard java properties mechanism.

      String fullFileName = dir + fileName;
      System.out.println(fname + "loading from: " + fullFileName);
      // I don't understand how I should be able to use the
      // one below, so I add the new stuff here instead.
      {
         try {
            LineNumberReader reader = new LineNumberReader(new FileReader(
                  fullFileName));
            for (String cur_line = reader.readLine(); cur_line != null; cur_line = reader
                  .readLine()) {
               StringTokenizer tok = new StringTokenizer(cur_line, " ");
               try {
                  // We need two
                  String key = tok.nextToken();
                  String val = cur_line.substring(key.length()).trim();
                  paramVals.put(key.trim(), val);
               } catch (NoSuchElementException noe) {
                  // Empty line or whatever
               }
            }
            reader.close();
         } catch (IOException fe) {
            // FIXME: the caller would probably be interested in
            // knowing that loading failed...
            System.err
                  .println(fname
                        + "Unable to load settings file, this might be OK if no settings file has been created yet.");
         }
      }
   }

   public void save() {
      // Params that need to be saved
      final String fname = "ParamFile.save() ";

      try {
         PrintWriter writer = new PrintWriter(new FileWriter(dir + fileName));
         Set keySet = paramVals.keySet();
         for (Iterator it = keySet.iterator(); it.hasNext();) {
            Object key = it.next();
            Object val = paramVals.get(key);
            writer.print(key);
            writer.print(' ');
            writer.println(val);
         }
         writer.close();
         System.out.println(fname + "Parameter file saved");
      } catch (IOException e) {
         System.err.println(fname + "error: " + e.getMessage());
         new ErrorDialog(null, "Param save error " + e).show();
      }
   }

}
