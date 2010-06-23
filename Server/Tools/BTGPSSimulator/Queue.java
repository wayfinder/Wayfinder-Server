/*
 */
import java.util.LinkedList;

/**
 *    Thread-safe Queue that will hold max maxSize entries
 *    after it is full, it will start throwing stuff.
 */
class Queue {

   private LinkedList realQueue;
   private int maxSize;

   public Queue() {
      this(Integer.MAX_VALUE);
   }

   public Queue(int maxSize) {
      this.realQueue = new LinkedList();
      this.maxSize = maxSize;
   }

   void setMaxSize(int maxSize) {
      synchronized (realQueue) {
         this.maxSize = maxSize;
      }
   }

   void clear() {
      synchronized (realQueue) {
         realQueue.clear();
      }
   }

   boolean remove(Object o) {
      synchronized (realQueue) {
         return realQueue.remove(o);
      }
   }

   /**
    *    Waits for an object to become available in the queue.
    */
   public Object dequeue() {
      synchronized (realQueue) {
         while (true) {
            if (realQueue.isEmpty()) {
               try {
                  realQueue.wait();
               } catch (InterruptedException e) {
                  System.err.println("Queue.dequeue() " + e);
                  return null;
               }
            }
            if (!realQueue.isEmpty()) {
               return realQueue.remove(0);
            }
         }
      }
   }

   /**
    *    Returns true if the queue isn't empty.
    */
   public boolean enqueue(Object obj) {
      boolean retval = true;
      synchronized (realQueue) {
         realQueue.addLast(obj);
         if (realQueue.size() <= maxSize) {
         } else {
            retval = false;
            System.err.println("Queue.enqueue() Queue is full");
            while (realQueue.size() > maxSize) {
               realQueue.remove(0);
            }
         }
         realQueue.notify();
      }
      return retval;
   }

}
