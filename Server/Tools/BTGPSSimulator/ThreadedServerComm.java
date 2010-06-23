/** 
 */
public class ThreadedServerComm extends ServerComm {

   private ServerComm realComm;
   private CommThread commThread;
   private Queue queue;

   private abstract class RequestRunner implements Runnable {

      private ServerRequest request;

      public RequestRunner(ServerRequest req) {
         request = req;
      }

      public final boolean isCancelled() {
         return request.isCancelled();
      }

      public final void completeCancelled() {
         if (!isCancelled()) {
            // This should NOT happen.
            System.err.println("RequestRunner.completeCancelled() ERROR - completing non-cancelled request");
         } else {
            completeRequest(request);
         }
      }

      public abstract void run();
   }

   /**
    *   This class cannot refer to the ServerComm since
    *   it must be stopped by the finalize call and if
    *   it refers back, ServerComm can never be collected.
    */
   private static class CommThread extends Thread {

      private Queue queue2;
      private boolean terminated;

      public CommThread(Queue q) {
         queue2 = q;
         setDaemon(true);
      }

      public void run() {
         while (!terminated) {
            try {
               RequestRunner r = (RequestRunner) (queue2.dequeue());
               // Don't do it if the request has been cancelled.
               if (!r.isCancelled()) {
                  r.run();
               } else {
                  System.err.println("CommThread.run() req " + r + " was cancelled");
                  r.completeCancelled();
               }
            } catch (Exception e) {
               System.err.println("CommThread.run()" + e);
            }
         }
         System.err.println("CommThread.run() [TSC]: CommThread exiting");
      }

      public void terminate() {
         terminated = true;
         this.interrupt();
      }
   }

   public ThreadedServerComm(ServerComm realComm) {
      super(realComm.eventQueue);
      queue = new Queue();
      commThread = new CommThread(queue);
      commThread.start();
      this.realComm = realComm;
   }

   protected void finalize() {
      System.out.println("CommThread.finalize() [TSC]: Terminating commthread");
      commThread.terminate();
   }

   /**
    *    Enqueues a mercator request in the real servercomm.
    */
   public void handleMercator(final ServerMercatorRequest req) {
      queue.enqueue(new RequestRunner(req) {
         public void run() {
            realComm.handleMercator(req);
         }
      });
   }

}
