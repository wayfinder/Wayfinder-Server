/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;

/**
 *   ServerComm for communication with the XMLServer
 */
public class XMLServerComm extends ThreadedServerComm {

   public XMLServerComm(String serverName, int serverPort, ServerEventQueue ev)
         throws IOException {
      super(new XMLServerCommImpl(serverName, serverPort, ev));
   }

   private static class XMLServerCommImpl extends ServerComm {

      private URL baseURL;

      public XMLServerCommImpl(String serverName, int serverPort,
            ServerEventQueue ev) throws IOException {
         super(ev);
         baseURL = new URL("http", serverName, serverPort, "/");
      }

      private String makeMercatorQuery(ServerMercatorRequest req) {
         StringBuffer buf = new StringBuffer(40);
         MercatorMapParams params = req.getParams();
         buf.append('?');
         buf.append("x=");
         buf.append(params.getX());
         buf.append('&');
         buf.append("y=");
         buf.append(params.getY());
         buf.append('&');
         buf.append("zoom=");
         buf.append(params.getZoom());
         buf.append('&');
         buf.append("lang=");
         buf.append(params.getLang());
         return buf.toString();
      }

      private byte[] readAll(InputStream ins) throws IOException {
         ByteArrayOutputStream out = new ByteArrayOutputStream(1024);
         byte[] buf = new byte[1024];
         int len;
         while ((len = ins.read(buf)) != -1) {
            out.write(buf, 0, len);
         }
         return out.toByteArray();
      }

      public void handleMercator(ServerMercatorRequest req) {
         InputStream i;
         try {
            int type = req.getParams().getType();
            String file = (type == MercatorMapParams.IMAGE_MAP) ? "/MMap"
                  : "/SPMap";
            URL url = new URL(baseURL, file + makeMercatorQuery(req));
            System.err.println("XMLServerCommImpl.handleMercator() Requesting : " + url);
            // Fetch the data
            URLConnection conn = (URLConnection) url.openConnection();
            i = conn.getInputStream();
            req.setAnswerData(readAll(i));
         } catch (Exception e) {
            // Set the error
            req.setErrorText(e.toString());
         } finally {
            // Close i
            // Done
            // Important - call completeRequest since it must be invoked from
            // the correct thread.
            completeRequest(req);
         }
      }
   }
}
