/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
package com.itinerary.ServerCommunicator;


public class Utility{

   static private Utility c_util = new Utility();

   public static final int DummyInt = 12345;
   
   private Utility() {   
   }
   
   /**
    * Prints debug output if level <= currentDebugLevel.
    * @param string A string to print.
    * @param debugLevel The debug level of this output.
    *                   Lower numbers are more important.
    */
   public static final void debugPrint(String string,
                                       int debugLevel)
      {
         if (debugLevel <= DeveloperSettings.getCurrentDebugLevel()) {
            //System.err.println(string);
            System.out.println(string);
         }
      }

   // Added this function so that we can change all System.out.println
   // with a simple query replace.
   
   /**
    * Prints debug output if 3 <= currentDebugLevel.
    * @param string A string to print.
    * @param level The debug level of this output.
    *              Lower numbers are more important.
    */
   public static final void debugPrint(String string)
      {
         debugPrint(string, 3);
      }

   /**
    *   Prints debug output if level <= currentDebugLevel.
    *   Indents printout.
    *
    *   @param string     A string to print.
    *   @param debugLevel The debug level of this output.
    *                     Lower numbers are more important.
    *   @param indent     The number of spaces to indent the string.
    */
   public static final void debugPrint(String string,
                                       int debugLevel,
                                       int indent)
      {
         if (debugLevel <= DeveloperSettings.getCurrentDebugLevel()) {
            //System.err.println(string);
            StringBuffer s = new StringBuffer();
            // Is there a java method to create a string of N copies of char c?
            for (int i = 0; i < indent; i++) {
               s.append(" ");
            }
            s.append(string);
            System.out.println(s.toString());
         }
      }
}
