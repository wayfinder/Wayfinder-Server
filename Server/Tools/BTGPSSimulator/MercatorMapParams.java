/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 *   Class describing mercator map parameters
 */
public class MercatorMapParams {

   public static final int IMAGE_MAP = 1;
   public static final int SIMPLEPOI_MAP = 2;

   private int x;
   private int y;
   private String lang; // What is this?
   private int type;
   private int hash; // What is this?
   private int zoom;

   public MercatorMapParams(int x, int y, int zoom, String lang, int type) {
      this.x = x;
      this.y = y;
      this.zoom = zoom;
      this.lang = lang;
      this.type = type;
   }

   public final int getX() {
      return x;
   }

   public final int getY() {
      return y;
   }

   public final int getZoom() {
      return zoom;
   }

   public final String getLang() {
      return lang;
   }

   public final int getType() {
      return type;
   }

   public final boolean equals(Object o) {
      try {
         MercatorMapParams m = (MercatorMapParams) o;
         return x == m.x && y == m.y && type == m.type && lang.equals(m.lang);
      } catch (ClassCastException e) {
         return false;
      }
   }

   public final int hashCode() {
      if (hash == 0) {
         hash = (lang + x + y + type + zoom).hashCode();
      }
      return hash;
   }

}
