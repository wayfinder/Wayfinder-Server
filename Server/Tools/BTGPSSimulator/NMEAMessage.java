/**
 *  Class describing a message to send by the Bluetooth connection.
 */

//  (NmeaParser.cpp KNOTS_TO_METERS_PER_S 0.51444444
//  Knots2mpers  = 1.9438445
import java.util.Calendar;
import java.util.TimeZone;

// Checksum The checksum field consists of a '*' and two hex digits
// representing the exclusive OR of all characters between, but not
// including, the '$' and '*'. A checksum is required on some sentences.

public class NMEAMessage implements Cloneable {

   protected String text;
   protected double latitude;
   protected double longitude;
   protected double knots;
   protected double bearing;
   protected boolean isRMC;
   protected boolean isGGA;
   protected int timeFromPrev;
   private Time time;
   private static int DEFAULT_TIME_FROM_PREV; // [ms]

   private String getGPGSA() {
      // From Ilmari when it has a fix.
      return "GPGSA,A,3,19,03,22,15,,,,,,,,,2.3,4.8,21.8";
   }

   public NMEAMessage() {
      timeFromPrev = DEFAULT_TIME_FROM_PREV;
      makeSetText(getGPGSA());
      isRMC = false;
      time = new Time();
   }

   public NMEAMessage(int gpsStrenght) {
      timeFromPrev = DEFAULT_TIME_FROM_PREV;
      makeSetText(getGPGSA());
      isRMC = false;
      time = new Time();
   }

   public NMEAMessage(String text) {
      timeFromPrev = DEFAULT_TIME_FROM_PREV;
      setText(text);
      time = new Time();
      parseMsg();
   }

   public NMEAMessage(double lat, double lon, double sp_la, double sp_lo) {
      timeFromPrev = DEFAULT_TIME_FROM_PREV;
      isRMC = setRMCMessage(lat, lon, sp_la, sp_lo);
      time = new Time();
   }

   final int getTimeFromPrev() {
      return timeFromPrev;
   }

   final void setTimeFromPrev(int time_ms) {
      timeFromPrev = time_ms;
   }

   /**
    * Set the time of this message if it has noe time, and set the time difference
    * to the previous message
    * @param msg the previous message
    */
   public final void updateTime(final NMEAMessage msg) {
      if (msg != null) {
         if (time.isNull()) {
            time = msg.time;
         }
         if (time.isNull()) {
            setTimeFromPrev(0);
         } else {
            if (msg.time.isNull()) {
               setTimeFromPrev(0);
            } else {
               setTimeFromPrev(Math.abs(time.compareTo(msg.time)));
            }
         }
      } else {
         setTimeFromPrev(0);
      }
   }

   public void setReceiverWarning() {
      StringBuffer sb = new StringBuffer(text);
      sb.setCharAt(18, 'V');
      // checksum needs update too! This is the ugly way, we need a rewrite
      sb.delete(sb.length() - 3, sb.length()); // *3f
      sb.deleteCharAt(0); // $
      text = makeText(sb.toString());
   }

   protected Object clone() throws java.lang.CloneNotSupportedException {
      return super.clone();
   }

   protected static String makeText(String textWithoutDollarAndCRLF) {
      final String ending = "\r\n";
      StringBuffer buf = new StringBuffer();
      buf.append('$');
      buf.append(textWithoutDollarAndCRLF);
      buf.append('*');
      buf.append(calcCheckSum(textWithoutDollarAndCRLF));
      buf.append(ending);
      return buf.toString();
   }

   protected void makeSetText(String textWithoutDollarAndCRLF) {
      setText(makeText(textWithoutDollarAndCRLF));
   }

   protected void setText(String text) {
      final String ending = "\r\n";
      if (!text.endsWith(ending)) {
         text = text + ending;
      }
      this.text = text;
   }

   private double parseCoord(String coord, boolean lat) {
      double res = 0;
      try {
         int nbrDeg = lat ? 2 : 3;
         String degs = coord.substring(0, nbrDeg);
         res += Double.parseDouble(degs);
         String min = coord.substring(nbrDeg);
         res += Double.parseDouble(min) / 60.0;
      } catch (IndexOutOfBoundsException e) {
         System.err.println("NMEAMessage.parseCoord() parseCoord " + e);
      } catch (NumberFormatException ne) {
         System.err.println("NMEAMessage.parseCoord() parseCoord " + ne);
      }
      return res;
   }

   private void parseMsg() {
      if (text == null) {
         return;
      }
      if (text.indexOf("GPRMC") >= 0) {
         String[] tokens = Util.splitStr(text, ",");
         if (tokens.length < 7) {
            return;
         }
         final String time = tokens[1];
         String avail = tokens[2];
         String lat = tokens[3];
         String lon = tokens[5];
         this.latitude = parseCoord(lat, true);
         this.longitude = parseCoord(lon, false);
         this.time = new Time(time);
         if (tokens[4].equalsIgnoreCase("S")) {
            this.latitude = -this.latitude;
         }
         if (tokens[6].equalsIgnoreCase("W")) {
            this.longitude = -this.longitude;
         }

         try {
            String speed_knots = tokens[7];
            knots = Double.parseDouble(speed_knots);
            String course = tokens[8];
            bearing = Double.parseDouble(course);
         } catch (Exception e) {
            System.err.println("NMEAMessage.parseMsg() " + e);
         }

         if ((!avail.equalsIgnoreCase("A")) || (this.latitude == 0.0 && this.longitude == 0.0)) {
            this.latitude = Double.MAX_VALUE;
            this.longitude = Double.MAX_VALUE;
            isRMC = true;
         }

         System.err.println("NMEAMessage.parseMsg() lat = " + this.latitude + " lon = " + this.longitude);
         isRMC = true;
      } else {
         //System.err.println("Not GPRMC text is " + text );
      }
   }

   public static NMEAMessage getGSVMessage() {
      //String text = "GPGSV,3,1,10,20,78,331,45,01,59,235,47,"+
      //   "22,41,069,,13,32,252,45";
      // From Ilmari
      String completeText = "$GPGSV,3,1,11,19,67,276,00,03,67,181,,22,62,101,,15,51,080,*77\r\n"
            + "$GPGSV,3,2,11,18,39,058,,11,14,264,,14,12,132,,01,10,133,*74\r\n"
            + "$GPGSV,3,3,11,21,09,079,,08,06,305,,26,06,013,*45\r\n";

      NMEAMessage tmp = new NMEAMessage(completeText);
      tmp.text = completeText;
      return tmp;
   }

   // Probably not needed.
   public static NMEAMessage getGLLMessage() {
      String text = "GPGLL,4916.45,N,12311.12,W,225444,A,";
      return new NMEAMessage(makeText(text));
   }

   // Probably not needed.
   public static NMEAMessage getVTGMessage() {
      String text = "GPVTG,054.7,T,034.4,M,005.5,N,010.2,K";
      return new NMEAMessage(makeText(text));
   }

   private static Calendar getCurrentUTC() {
      // If this weren't a Pentium 4 with lots of mem,
      // the TimeZone should be allocated once.
      Calendar cal = Calendar.getInstance();
      cal.setTimeZone(TimeZone.getTimeZone("UTC"));
      return cal;
   }

   public static String getNowTime(boolean nineninenine) {

      Calendar d = getCurrentUTC();
      int time = d.get(Calendar.SECOND) + 100 * d.get(Calendar.MINUTE) + 10000
            * d.get(Calendar.HOUR_OF_DAY);
      String timestring = "";
      if (time < 100000)
         timestring = "0";
      timestring += time;
      if (nineninenine) {
         timestring += ".999";
      } else {
         timestring += ".000";
      }
      return timestring;
   }

   public static String getNowTime() {
      return getNowTime(false);
   }

   public static String getNowDate() {
      Calendar cal = getCurrentUTC();
      int date = (cal.get(Calendar.YEAR) % 100) + 100
            * (1 + cal.get(Calendar.MONTH)) + 10000
            * cal.get(Calendar.DAY_OF_MONTH);
      String datestring = "";
      if (date < 100000)
         datestring = "0";
      datestring += date;
      return datestring;
   }

   public NMEAMessage getGGAMessage() {
      // This is not the same as for Ilmari
      StringBuffer tmp = new StringBuffer(256);
      tmp.append("GPGGA,");
      tmp.append(getNowTime());
      tmp.append(',');
      tmp.append(makeLatLonString(latitude, true));
      tmp.append(',');
      tmp.append(makeLatLonString(longitude, false));
      tmp.append(',');
      tmp.append("1,04,4.8,74.0,M,40.3,M,0.0,0000");
      return new NMEAMessage(makeText(tmp.toString()));
   }

   protected NMEAMessage modRMCMessage(int mode, int delay) {

      // delay is ignored for now.
      if (!isRMC) {
         return this;
      }
      double modKnots = knots;
      if (mode == 0)
         modKnots = 0;
      double modBearing = bearing;
      if (mode == -1) {
         modBearing += 180;
         if (modBearing > 360)
            modBearing -= 360;
      }
      try {
         NMEAMessage newMess = (NMEAMessage) (clone());
         newMess.makeSetText(makeRMC(latitude, longitude, modBearing, modKnots));
         return newMess;
      } catch (CloneNotSupportedException clo) {
         System.err.println("NMEAMessage.modRMCMessage() modRMCMessage: " + clo);
         return this;
      }

   }

   /**
    *    Creates a GPRMC message
    *    @param lat     The latitude in degrees.
    *    @param lon     The longitude in degrees
    *    @param bearing The bearing
    *    @param knots   The speed.
    */
   protected static String makeRMC(double lat, double lon, double bearing,
         double knots) {

      StringBuffer res = new StringBuffer();
      res.append("GPRMC,"); // 6 6
      res.append(getNowTime()); // 10 16
      res.append(",A,"); // GPS available   // 3 19
      res.append(makeLatLonString(lat, true)); // 11 30
      res.append(',');
      res.append(makeLatLonString(lon, false));
      res.append(',');
      res.append(knots);
      res.append(',');
      res.append(bearing);
      res.append(',');
      res.append(getNowDate());
      res.append(",,");
      return res.toString();
   }

   protected boolean setBearing(int bearing) {
      // delay is ignored for now.
      if (!isRMC)
         return false;
      if ((bearing > 360) || (bearing < 0))
         return false;
      this.bearing = bearing;
      makeSetText(makeRMC(latitude, longitude, this.bearing, knots));
      return true;
   }

   /**
    *   Gives the number the correct number of digits before.
    */
   private static String expandNbr(int nbr, int minDigits) {
      String tmpStr = Integer.toString(nbr);
      int lengthDiff = minDigits - tmpStr.length();
      if (lengthDiff > 0) {
         tmpStr = "00000".substring(0, lengthDiff) + tmpStr;
      }
      return tmpStr;
   }

   /**
    *   Returns the lat or lon string in a buffer that
    *   can be converted to string with toString() or
    *   simply appended to a StringBuffer.
    */
   public static String makeLatLonString(double latlon, boolean islat) {
      char dirChar = 'E';
      if (islat) {
         if (latlon < 0) {
            latlon = -latlon;
            dirChar = 'S';
         } else {
            dirChar = 'N';
         }
      } else {
         if (latlon < 0) {
            latlon = -latlon;
            dirChar = 'W';
         } else {
            dirChar = 'E';
         }
      }

      // The stuff is supposed to look like this:
      // "DDDMM.MMMM" if lon and "DDMM.MMMM" if lat.
      // Would be easy with printf...

      StringBuffer res = new StringBuffer();

      // Take integer part of the degrees
      int degrees = (int) latlon;
      // Keep the decimals on the minutes. Also round it.
      double minutes = (latlon - degrees) * 60 + 0.00005;

      // Put the degrees back on the number.
      double printNumber = degrees * 100 + minutes;

      // Split into integer and decimals
      int printNumberInt = (int) printNumber;
      int decimals = (int) (10000.0 * (printNumber - printNumberInt));

      // At least two digits for the degrees if lat, three if lon
      // Also add the minutes - so 4 or 5
      res.append(expandNbr(printNumberInt, islat ? 4 : 5));

      // Add integer part

      res.append('.');

      // Add decimal part.
      String dec = expandNbr(decimals, 4) + "0000";
      dec = dec.substring(0, 4);

      res.append(dec);

      res.append(',');
      res.append(dirChar);
      return res.toString();
   }

   public boolean setRMCMessage(double lat, double lon, double sp_la,
         double sp_lo) {

      if (true) {
         this.latitude = lat;
         this.longitude = lon;
         knots = 1.9438445 * Math.sqrt(Math.pow(sp_la, 2)
               + Math.pow(sp_lo, 2));

         int speed = 10 * (int) knots;
         bearing = ((Math.atan2(sp_la, sp_lo) * 180) / Math.PI);
         if (bearing < 0)
            bearing += 360;
         int angle = 10 * (int) bearing;

         // Getting right amount of dec.
         knots = speed / 10;
         bearing = angle / 10;

         makeSetText(makeRMC(this.latitude, this.longitude, bearing, knots));
         isRMC = true;
         return true;

      }
      if ((lat > 1) || (lat == 0)) {
         System.err.println("NMEAMessage.setRMCMessage() Invalid lat value");
      }
      if ((lon > 1) || (lon == 0)) {
         System.err.println("NMEAMessage.setRMCMessage() Invalid lon value");
      }

      if (sp_la > 100) {
         System.err.println("NMEAMessage.setRMCMessage() Unrealistic lat_speed value");
      }
      if (sp_lo > 100) {
         System.err.println("NMEAMessage.setRMCMessage() Unrealistic lon_speed value");
      }
      System.err.println("NMEAMessage.setRMCMessage() Failed to create RMC message");
      // Not enough correct info to create a RMC, keeping GSA
      return false;
   }

   public double getLat() {
      if (isRMC) {
         return latitude;
      }
      return Double.MAX_VALUE;
   }

   public double getLon() {
      if (isRMC) {
         return longitude;
      }
      return Double.MAX_VALUE;
   }

   public double getSpeed() {
      if (isRMC) {
         return knots;
      }
      return Double.MAX_VALUE;
   }

   public void setSpeed(double speed) {
      knots = speed;
      makeSetText(makeRMC(latitude, longitude, bearing, knots));
   }

   public void setLat(double lat) {
      this.latitude = lat;
      makeSetText(makeRMC(this.latitude, this.longitude, bearing, knots));
   }

   public void setLon(double lon) {
      this.longitude = lon;
      makeSetText(makeRMC(this.latitude, this.longitude, bearing, knots));
   }

   public double getBearing() {
      if (isRMC) {
         return bearing;
      }
      return Double.MAX_VALUE;
   }

   public boolean isRMC() {
      return isRMC;
   }

   public final boolean isValidRMC() {
      return isRMC && latitude != Double.MAX_VALUE && longitude != Double.MAX_VALUE;
   }

   public boolean isGSA() {
      return !isRMC;
   }

   public String getMessage() {
      return toString();
   }

   public String toString() {
      return text;
   }

   public static int calcCheckSumInt(String text) {
      int stringLength = text.length();

      final int skipping = 0;
      final int summing = 1;

      int state = skipping;

      int checksum = 0;
      for (int i = 0; i < stringLength; ++i) {
         char c = text.charAt(i);
         if (c == '\n' || c == '\r' || c == '*') {
            return checksum;
         }
         // Skip the dollar.
         if (c != '$') {
            state = summing;
         }
         if (state == summing) {
            // We know that only ASCII exists in the messages.
            checksum ^= (int) c;
         }
      }
      return checksum;
   }

   public static String calcCheckSum(String text) {
      int checksum = calcCheckSumInt(text);
      String returnString = "";

      if (checksum < 16) {
         returnString += "0";
      }
      returnString += Integer.toHexString(checksum).toUpperCase();
      return returnString;

   }

   public static void setDefaultTimeFromPrev(int defaultTimeFromPrev) {
      DEFAULT_TIME_FROM_PREV = defaultTimeFromPrev;
   }

}
