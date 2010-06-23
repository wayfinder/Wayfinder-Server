/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
import java.io.FileInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Vector;
import com.itinerary.ServerCommunicator.*;

public class firstSearchArea {

   public static void main( String[] args ) {
      try { 
         if ( args[ 0 ] == null ) {
            System.out.println("File parameter please.");
            return;
         }
      } catch ( ArrayIndexOutOfBoundsException e ) {
         System.out.println( e );
         System.out.println("File parameter please.");
         return;
      }
      File xmlFile = new File( args[ 0 ] );
      SimpleXMLDocument xmlReply = null;

      try {
         // Stream input
         FileInputStream inStream = null;
         try {
            inStream = new FileInputStream( xmlFile );
         } catch ( FileNotFoundException e ) {
            System.out.println( e );
            System.out.println("File not found.");
            return;
         }
      
         // Parse the document
         SimpleXMLParser parser = new SimpleXMLParser();
         xmlReply = parser.parse( inStream );
      
         if ( xmlReply == null ) {
            System.out.println( "WARNING, xmlReply is null" );
         }

         inStream.close();
      } catch ( IOException iox ) {
         iox.printStackTrace();
         return;
      } catch ( SimpleXMLException e ) {
         e.printStackTrace();
         return;
      }

      SimpleXMLElement searchReply = xmlReply.getSingleTag("search_reply");
      if ( searchReply != null ) {
         // Get search_area_list
         Vector searchAreaLists = 
            searchReply.getChildTags( "search_area_list" );
         if ( searchAreaLists.size() > 0 ) {
            SimpleXMLElement searchAreaList = 
               (SimpleXMLElement)searchAreaLists.elementAt( 0 );
            // Get first search_area
            Vector searchAreas = searchAreaList.getChildTags( "search_area" );
            if ( searchAreas.size() > 0 ) {
               SimpleXMLElement searchArea = 
                  (SimpleXMLElement)searchAreas.elementAt( 0 );
               System.out.print( searchArea.toString( "   " ) );
            } else {
               System.out.println( "No search_areas found." );
            }
         } else {
            System.out.println( "No search_area_list found." ); 
         }
      } else {
         System.out.println( "WARNING, xmlReply is null" );
      }
   }
}
