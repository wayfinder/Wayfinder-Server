import java.io.Writer;
import java.io.IOException;

/**
 *   Class that sends nmea messages. Mostly for the bluetooth
 *   serial port.
 *   <br />
 *   This class is the superclass of SocketGPSSender, but the
 *   design could be better, i.e. this class should also have
 *   a superclass.
 */
public class GPSSender extends Thread {

   public class NullGPSSenderListener implements GPSSenderListener {
      public void gpsSenderStatusUpdate(GPSSender source, boolean sending,
            String statusMsg) {
      }

      public void removeSender(GPSSender source) {
      }
   }

   /// Queue of outgoing messages
   protected Queue sendQueue;
   /// True if we should stop sending
   protected boolean terminated = false;
   /// The writer to write to.
   private Writer writer;
   /// Listener that listens for status
   protected GPSSenderListener listener;
   /// True if the queue was full the last time something was enqueued
   protected boolean wasFull;
   /// Packet counter
   protected int packNbr;

   private MainSimulatorWindow mainWindow;

   public GPSSender(Writer writer, GPSSenderListener listener,
         MainSimulatorWindow simWin) {
      this.mainWindow = simWin;
      this.listener = listener;
      this.writer = writer;
      sendQueue = new Queue(1);
      wasFull = true;
      this.listener.gpsSenderStatusUpdate(this, false, "Not Sending");
   }

   public boolean enqueue(NMEAMessage mess) {
      boolean sending = sendQueue.enqueue(mess.toString());
      boolean full = !sending;
      if (full && !wasFull) {
         wasFull = full;
         listener.gpsSenderStatusUpdate(this, sending, "");
      }
      return sending;
   }

   public void terminate() {
      terminated = true;
      // Wake up
      sendQueue.enqueue(null);
      listener.gpsSenderStatusUpdate(this, false, "Not Sending");
      interrupt();
   }

   public void run() {
      try {
         while (!terminated) {
            //System.err.println("GPSSender - waiting for queue");
            String msg = (String) sendQueue.dequeue();
            //System.err.println("GPSSender - got msg from queue");
            if (!terminated) {
               if (msg == null) {
                  System.err.println("GPSSender.run() msg is null");
               } else {
                  //System.out.print(msg);
                  mainWindow.printText(msg);
                  // This can hang on serial ports.
                  writer.write(msg);
                  writer.flush();
                  if (wasFull || ((++packNbr & 31) == 31)) {
                     wasFull = false;
                     listener.gpsSenderStatusUpdate(this, true, "");
                  }
                  // Now we can increase the send-queue size.
                  sendQueue.setMaxSize(10);
               }
            }
         }
      } catch (IOException e) {
         listener.gpsSenderStatusUpdate(this, false, "" + e);
      } finally {
         // Don't close writer. We can re-use it later.
      }
      System.err.println("GPSSender.run() GPSSender stopping");

   }

}
