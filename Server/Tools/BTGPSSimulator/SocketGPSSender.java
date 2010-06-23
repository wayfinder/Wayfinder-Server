/*
 *
 */

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.net.ServerSocket;
import java.net.Socket;

/**
 *
 */
public class SocketGPSSender extends GPSSender {

   Socket socket;
   ServerSocket serverSocket;
   String host;
   int port;
   String sendingString;

   /**
    * @deprecated ?
    */
   public class ReaderThread extends Thread {

      SocketGPSSender parent;

      /**
       *   Detects read errors and terminates the other
       *   thread if it happens.
       */
      public ReaderThread(SocketGPSSender parent) {
         this.parent = parent;
      }

      public void run() {
         try {
            InputStream in = socket.getInputStream();
            while (in.read() != -1) {
            }
         } catch (IOException e) {
            System.out.println("ReaderThread.run(): " + e);
            listener.gpsSenderStatusUpdate(parent, false, "read error");
         } finally {
            System.out.println("ReaderThread.run() finally");
            try {
               synchronized (this) {
                  try {
                     wait(5000);
                  } catch (InterruptedException ie) {
                  }
               }
            } finally {
               try {
                  if (socket != null) {
                     socket.close();
                  }
               } catch (IOException e) {
               }
               sendQueue.enqueue(null);
               parent.interrupt();
            }
         }
      }
   }

   public SocketGPSSender(Socket sock, GPSSenderListener listener) {
      super(null, listener, null);
      listener.gpsSenderStatusUpdate(this, false, "Starting");
      socket = sock;
   }

   public SocketGPSSender(String host, int port, GPSSenderListener listener) {
      super(null, listener, null);
      this.socket = null;
      this.host = host;
      this.port = port;
   }

   public SocketGPSSender(int port, GPSSenderListener listener) {
      super(null, listener, null);
      this.socket = null;
      this.host = "";
      this.port = port;
   }

   public void terminate() {
      // This is not thread safe.
      super.terminate();
      try {
         if (serverSocket != null) {
            serverSocket.close();
         }
      } catch (Exception e) {
         System.err.println("SocketGPSSender.terminate() " + e);
      } finally {
      }
      try {
         if (socket != null) {
            socket.close();
         }
      } catch (Exception e) {
         System.err.println("SocketGPSSender.terminate() " + e);
      } finally {
      }

   }

   public void connect() {
      int try_nbr = 0;
      while (!terminated) {
         try {
            String text = "" + try_nbr + " Trying conn to " + host + ":" + port;
            listener.gpsSenderStatusUpdate(this, false, text);
            socket = new Socket(host, port);
            return;
         } catch (IOException ioe) {
            ++try_nbr;
            try {
               sleep(1000);
            } catch (InterruptedException e) {
            }

         }
      }
   }

   public void accept() throws IOException {
      listener.gpsSenderStatusUpdate(this, false, "Accepting port " + port);
      serverSocket = new ServerSocket(port);
      System.out.println("SocketGPSSender.accept() Waiting for connection...");
      socket = serverSocket.accept();
      System.out.println("SocketGPSSender.accept() Connection received!");
      try {
         serverSocket.close();
      } catch (IOException e) {
      }
   }

   private String getSendingString() {
      return sendingString;
   }

   private void inner_run() {
      try {
         if (socket == null) {
            if (host.length() != 0) {
               // Means we want to connect.
               connect();
            } else {
               // Means we want to accept.
               accept();
            }
         }

         if (!terminated) {
            if (socket != null) {
               String addr = socket.getInetAddress().toString() + ":"
                     + socket.getPort();
               sendingString = "Connected to" + addr;
               listener.gpsSenderStatusUpdate(this, false, getSendingString());
               sendingString = "Sending to " + addr;
            }

            try {
               socket.setTcpNoDelay(true);
            } catch (Exception e) {
               // Not very very important
            }
            OutputStream out = socket.getOutputStream();
            Writer writer = new OutputStreamWriter(out);

            // ReaderThread causes the the socket connection to terminate 
            // every five seconds when running against the emulator for Java.
            //new ReaderThread(this).start();

            while (!terminated) {
               String msg = (String) sendQueue.dequeue();
               if (!terminated && msg != null) {
                  writer.write(msg);
                  writer.flush();
                  if (wasFull) {
                     listener.gpsSenderStatusUpdate(this, true,
                           getSendingString());
                     wasFull = false;
                  }
                  sendQueue.setMaxSize(10);
               } else {
                  // Try to get io error if any.
                  writer.flush();
               }
            }
         }
      } catch (IOException e) {
         listener.gpsSenderStatusUpdate(this, false, "" + e);
      } finally {
         try {
            if (socket != null) {
               socket.close();
               socket = null;
            }
         } catch (IOException ioe) {
         }
         if (port == 0) {
            // We cannot reconnect
            terminated = true;
         }
      }
   }

   public void run() {
      try {
         while (!terminated) {
            inner_run();
         }
      } finally {
         listener.gpsSenderStatusUpdate(this, false, "SENDER TERMINATED");
         listener.removeSender(this);
      }
   }
}
