import junit.framework.TestCase;

/**
 * 
 */

/**
 */
public class TimeTest extends TestCase {

   private Time[] time;

   public void setUp() {
      time = new Time[27];
      time[0] = new Time();
      time[1] = new Time("105345.076");
      time[2] = new Time("105346.076");
      time[3] = new Time("105346.076");
      time[4] = new Time();
      time[5] = new Time("235958.076");
      time[6] = new Time("000001.076");
      time[7] = new Time("235957");
      time[8] = new Time("000000.0");
      time[9] = new Time(null);
      time[10] = new Time("sdf");
      time[11] = new Time("");
      time[12] = new Time("234.3");
      time[13] = new Time("105246.167462");
      time[14] = new Time("15a234.34");
      time[15] = new Time("15151515.1");
      time[16] = new Time("151515.999999999");
      time[17] = new Time("151515.15");
      time[18] = new Time("151515.1");
      time[19] = new Time("151515.");
      time[20] = new Time("151515");
      time[21] = new Time("15151");
      time[22] = new Time("1515");
      time[23] = new Time("151");
      time[24] = new Time("15");
      time[25] = new Time("1");
      time[26] = new Time("151515.999999c99");
   }

   public void testOutputNull() {
      assertEquals("Outout of null failed", "null time", time[0].toString());
      assertEquals("Outout of null failed", "null time", time[4].toString());
      assertEquals("Outout of null failed", "null time", time[9].toString());
   }

   public void testOutputNormal() {
      assertEquals("Outout of 10:53:45 failed ", "10:53:45.076", time[1]
            .toString());
      assertEquals("Outout of 10:53:46 failed ", "10:53:46.076", time[2]
            .toString());
      assertEquals("Outout of 10:53:46 failed ", "10:53:46.076", time[3]
            .toString());
      assertEquals("Outout of 23:59:58 failed ", "23:59:58.076", time[5]
            .toString());
      assertEquals("Outout of 00:00:01 failed ", "00:00:01.076", time[6]
            .toString());
   }

   public void testOutputDecimals() {
      assertEquals(".000", "12:12:12.000", new Time("121212.000").toString());
      assertEquals(".001", "12:12:12.001", new Time("121212.001").toString());
      assertEquals(".010", "12:12:12.010", new Time("121212.010").toString());
      assertEquals(".100", "12:12:12.100", new Time("121212.100").toString());
      assertEquals(".011", "12:12:12.011", new Time("121212.011").toString());
      assertEquals(".110", "12:12:12.110", new Time("121212.110").toString());
      assertEquals(".101", "12:12:12.101", new Time("121212.101").toString());
      assertEquals(".111", "12:12:12.111", new Time("121212.111").toString());
   }

   public void testOutputIncomplete() {
      assertEquals("Output of 23:59:57 failed", "23:59:57.000", time[7]
            .toString());
      assertEquals("Output of 00:00:00 failed", "00:00:00.000", time[8]
            .toString());
      assertEquals("Incomplete decimal input should be filled out",
            "15:15:15.150", time[17].toString());
      assertEquals("Incomplete decimal input should be filled out",
            "15:15:15.100", time[18].toString());
      assertEquals("Incomplete decimal input should be filled out",
            "15:15:15.000", time[19].toString());
      assertEquals("Incomplete decimal input should be filled out",
            "15:15:15.000", time[20].toString());
      assertEquals("Incomplete input should be nulled", "null time", time[21]
            .toString());
      assertEquals("Incomplete input should be nulled", "null time", time[22]
            .toString());
      assertEquals("Incomplete input should be nulled", "null time", time[23]
            .toString());
      assertEquals("Incomplete input should be nulled", "null time", time[24]
            .toString());
      assertEquals("Incomplete input should be nulled", "null time", time[25]
            .toString());
   }

   public void testOutputIllegal() {
      assertEquals("Illegal input should be null", "null time", time[10]
            .toString());
      assertEquals("Illegal input should be null", "null time", time[11]
            .toString());
      assertEquals("Illegal input should be null", "null time", time[12]
            .toString());
      assertEquals("Illegal input should be null", "null time", time[14]
            .toString());
      assertEquals("Illegal input should be null", "null time", time[15]
            .toString());
   }

   public void testOutputOvercomplete() {
      assertEquals("Overcomplete input should be truncated", "10:52:46.167",
            time[13].toString());
      assertEquals("Overcomplete input should be truncated", "15:15:15.999",
            time[16].toString());
      assertEquals("Overcomplete input should be truncated", time[26]
            .toString(), time[16].toString());
   }

   public void testOutputOdd() {
      assertEquals("null time", new Time(null).toString());
   }

   public void testCompareToIncomplete() {
      assertEquals(-50, time[17].compareTo(time[18]));
      assertEquals(-100, time[18].compareTo(time[19]));
      assertEquals(0, time[19].compareTo(time[20]));
      assertEquals(150, time[19].compareTo(time[17]));
      assertEquals(0, time[17].compareTo(time[21]));
      assertEquals(0, time[18].compareTo(time[21]));
      assertEquals(0, time[19].compareTo(time[21]));
      assertEquals(0, time[1].compareTo(time[21]));
      assertEquals(0, time[1].compareTo(time[22]));
      assertEquals(0, time[1].compareTo(time[23]));
      assertEquals(0, time[1].compareTo(time[24]));
      assertEquals(0, time[1].compareTo(time[25]));

      assertFalse(time[1].compareTo(time[17]) == 0);
      assertFalse(time[1].compareTo(time[18]) == 0);
      assertFalse(time[1].compareTo(time[19]) == 0);
      assertFalse(time[1].compareTo(time[20]) == 0);
      assertTrue(time[1].compareTo(time[21]) == 0);
      assertTrue(time[1].compareTo(time[22]) == 0);
      assertTrue(time[1].compareTo(time[23]) == 0);
      assertTrue(time[1].compareTo(time[24]) == 0);
      assertTrue(time[1].compareTo(time[25]) == 0);
   }

   public void testCompareToOverComplete() {
      assertEquals(1, new Time("101010.555").compareTo(new Time(
            "101010.5569999999")));
      assertEquals(-1, new Time("101010.556999999").compareTo(new Time(
            "101010.555")));
      assertEquals(700, new Time("101010.103").compareTo(new Time(
            "101010.803999999")));
      assertEquals(-700, new Time("101010.803999999").compareTo(new Time(
            "101010.103")));
      assertEquals(0, new Time("101010.803999999").compareTo(new Time(
            "101010.803")));
      assertEquals(0, time[16].compareTo(time[26]));
      assertEquals(0, time[26].compareTo(time[16]));
      assertEquals(-2, time[26].compareTo(new Time("151515.997")));
      assertEquals(2, new Time("151515.997").compareTo(time[26]));
   }

   public void testCompareToIllegal() {

      // compare to a real value
      assertEquals(0, time[10].compareTo(time[1]));
      assertEquals(0, time[11].compareTo(time[1]));
      assertEquals(0, time[12].compareTo(time[1]));
      assertEquals(0, time[14].compareTo(time[1]));
      assertEquals(0, time[15].compareTo(time[1]));

      // compare to a null value
      assertEquals(0, time[10].compareTo(time[0]));
      assertEquals(0, time[11].compareTo(time[0]));
      assertEquals(0, time[12].compareTo(time[0]));
      assertEquals(0, time[14].compareTo(time[0]));
      assertEquals(0, time[15].compareTo(time[0]));

      // compare to an illegal value
      assertEquals(0, time[10].compareTo(time[10]));
      assertEquals(0, time[10].compareTo(time[11]));
      assertEquals(0, time[10].compareTo(time[12]));
      assertEquals(0, time[10].compareTo(time[14]));
      assertEquals(0, time[10].compareTo(time[15]));
      assertEquals(0, time[11].compareTo(time[12]));
      assertEquals(0, time[11].compareTo(time[14]));
      assertEquals(0, time[11].compareTo(time[15]));
      assertEquals(0, time[12].compareTo(time[14]));
      assertEquals(0, time[12].compareTo(time[15]));
      assertEquals(0, time[14].compareTo(time[15]));
   }

   public void testCompareToSameTime() {
      assertEquals("1. Comparison of time to same time failed.", 0, time[2]
            .compareTo(time[2]));
      assertEquals("2. Comparison of time to same time failed.", 0, time[2]
            .compareTo(time[3]));
      assertEquals("3. Comparison of time to same time failed.", 0, time[3]
            .compareTo(time[2]));
   }

   public void testCompareToMidnight() {
      assertEquals(3000, time[5].compareTo(time[6]));
      assertEquals(-1076, time[5].compareTo(time[7]));
      assertEquals(1924, time[5].compareTo(time[8]));
      assertEquals(-3000, time[6].compareTo(time[5]));
      assertEquals(-4076, time[6].compareTo(time[7]));
      assertEquals(-1076, time[6].compareTo(time[8]));
      assertEquals(1076, time[7].compareTo(time[5]));
      assertEquals(4076, time[7].compareTo(time[6]));
      assertEquals(3000, time[7].compareTo(time[8]));
      assertEquals(-1924, time[8].compareTo(time[5]));
      assertEquals(1076, time[8].compareTo(time[6]));
      assertEquals(-3000, time[8].compareTo(time[7]));
   }

   public void testCompareToNullTimes() {
      // Standard Case
      assertEquals("Comparison of first null time to first null time failed.",
            0, time[0].compareTo(time[0]));
      assertEquals("Comparison of first null time to second null time failed.",
            0, time[0].compareTo(time[4]));
      assertEquals("Comparison of first null time to third null time failed.",
            0, time[0].compareTo(time[9]));
      assertEquals("Comparison of second null time to first null time failed.",
            0, time[4].compareTo(time[0]));
      assertEquals(
            "Comparison of second null time to second null time failed.", 0,
            time[4].compareTo(time[4]));
      assertEquals("Comparison of second null time to third null time failed.",
            0, time[4].compareTo(time[9]));
      assertEquals("Comparison of third null time to first null time failed.",
            0, time[9].compareTo(time[0]));
      assertEquals("Comparison of third null time to second null time failed.",
            0, time[9].compareTo(time[4]));
      assertEquals("Comparison of third null time to third null time failed.",
            0, time[9].compareTo(time[9]));

      // Midnight case
      assertEquals("Comparison of null time to first midnight time failed.", 0,
            time[0].compareTo(time[8]));
      assertEquals("Comparison of first midnight time to null time failed.", 0,
            time[8].compareTo(time[0]));
      assertEquals("Comparison of null time to second midnight time failed.",
            0, time[0].compareTo(time[5]));
      assertEquals("Comparison of second midnight time to null time failed.",
            0, time[5].compareTo(time[0]));
   }

   public void testCompareToStandard() {
      assertEquals("Time difference of  1000 ms failed!", 1000, time[1]
            .compareTo(time[2]));
      assertEquals("Time difference of -1000 ms failed!", -1000, time[2]
            .compareTo(time[1]));
   }

   public void testCompareToMidday() {
      assertEquals(2 * 60 * 60 * 1000, new Time("110000").compareTo(new Time(
            "130000")));
      assertEquals(-2 * 60 * 60 * 1000, new Time("130000").compareTo(new Time(
            "110000")));
   }

   public void testCompareToOdd() {
      assertEquals("Comparison of time to null failed.", -1, time[1]
            .compareTo(null));
      assertEquals("Comparison of time to strange object failed.", -1, time[1]
            .compareTo("String"));
      assertEquals("Comparison of null time to null failed.", -1, time[0]
            .compareTo(null));
      assertEquals("Comparison of null time to strange object failed.", -1,
            time[0].compareTo("String"));
      assertEquals("Comparison of illegal time to null failed.", -1, time[10]
            .compareTo(null));
      assertEquals("Comparison of illegal time to strange object failed.", -1,
            time[10].compareTo("String"));
   }

}
