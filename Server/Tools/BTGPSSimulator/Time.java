/**
 * 
 */

/**
 */
public final class Time implements Comparable {

	private static final int[] TEMPLATE = {
		36000000, // 10 h
		3600000,  // 1 h
		600000,   // 10 min
		60000,    // 1 min
		10000,    // 10 s
		1000,     // 1 s
		0,        // decimal sign
		100,      // 100 ms
		10,       // 10 ms
		1         // 1 ms
	};
	
	private final int milliseconds;
	
	/**
	 * Constructs an empty null Time.
	 */
	public Time() {
		milliseconds = -1;
	}
	
	/**
	 * @param time String to parse in format hhmmss[.sss], the part in the [] is
	 * optional. E.g. 041004.0 is as valid as 041004 or 041004.000. If more
	 * decimals are used, they will be discarded.
	 * 
	 * Invalid strings (such as 12 or 050505.1d4) are treated as null times, as 
	 * is null.
	 */
	public Time(final String time) {
		if (time == null) {
			milliseconds = -1;
			return;
		}

		final int len = time.length();
		if (len < 6) {
			milliseconds = -1;
			return;
		}
		
		int tmpMs = 0;
		for (int pos = 0; pos < len && pos < TEMPLATE.length; ++pos) {
			try {
				if (pos != 6) {
					tmpMs += TEMPLATE[pos] * parseNumeral(time.charAt(pos));
				}
			} catch (Exception e) {
				milliseconds = -1;
				return;
			}
		}
		milliseconds = tmpMs;
	}
	
	/**
	 * Human readable String representation of this Time object
	 * @return Time in 24 hour style hh:mm:ss.sss format
	 */
	public String toString() {
		if (isNull()) {
			return "null time";
		}
		int tmpMs = milliseconds;
		final StringBuilder time = new StringBuilder(6);
		for (int pos = 0; pos < 6; ++pos) {
			final int tmp = tmpMs / TEMPLATE[pos];
			tmpMs = tmpMs % TEMPLATE[pos];
			time.append(tmp+(pos % 2 == 1 && pos < 4?":":""));
		}
		time.append(".");
		for (int pos = 7; pos < TEMPLATE.length; ++pos) {
			final int tmp = tmpMs / TEMPLATE[pos];
			tmpMs = tmpMs % TEMPLATE[pos];
			time.append(Integer.toString(tmp));
		}
		return time.toString();
	}


	/**
	 * @return true if this is a null Time object
	 */
	public boolean isNull() {
		return milliseconds == -1;
	}

	/**
	 * 24 hour-aware compareTo method.
	 * 
	 * This compareTo method "spins round" when approaching the end of the day.
	 * E.g. 1 AM (01:00) is considered GREATER than 11 PM (23:00), because 1 AM
	 * is closer in time to 11 PM when it follows.
	 * 
	 * @param time should be a Time object, if not, -1 is returned
	 * @return negative distance in milliseconds between this object and the
	 * object specified in the parameter if this object is lesser than the one 
	 * specified in the parameter; positive distance in milliseconds between 
	 * this object and the object specified in the parameter if this object is 
	 * gheater than the one specified in the parameter; zero if theese two Time
	 * objects are equal; -1 if the parameter isn't a Time object and 0 if the 
	 * parameter is a null Time Object. If this is a Time object, 0 is allways 
	 * returned unless parameter is null or not a Time object.
	 */
	public int compareTo(final Object time) {
		if (time != null && time instanceof Time) {
			final Time tmp = (Time) time;
			if (isNull()) {
				return 0;
			} else { // not null
				if (tmp.isNull()) {
					return 0;
				}
				final int diff = tmp.milliseconds - milliseconds;
				if (diff >= 12*60*60*1000) {
					return diff - 24*60*60*1000;
				} else if (diff <= -12*60*60*1000) {
					return 24*60*60*1000 + diff;
				}
				return diff;
			}
		} else {
			return -1;
		}
	}

	/**
	 * This method parses an arbitary char to its corresponding int. 
	 * Non-numeral chars are discarded and results in 0.
	 * @param numeral is a character in the range 0-9, other chars are discarded
	 * @return the int corresponding to the char, non-numeral chars returns 0
	 * @throws Exception if param numeral isn't a numeral in the range 0-9
	 */
	private static int parseNumeral(final char numeral) throws Exception { 
		switch (numeral) {
			case '0': return 0;
			case '1': return 1;
			case '2': return 2;
			case '3': return 3;
			case '4': return 4;
			case '5': return 5;
			case '6': return 6;
			case '7': return 7;
			case '8': return 8;
			case '9': return 9;
			default: throw new Exception();
		}
	}
}
