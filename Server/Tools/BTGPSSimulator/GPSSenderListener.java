/*
 *
 */
public interface GPSSenderListener {

   /**
    *   Sends information about if the sender is working or not.
    */
   public void gpsSenderStatusUpdate(GPSSender source, boolean sending,
         String statusMsg);

   /**
    *   Called by sender when it stops running.
    */
   public void removeSender(GPSSender source);

}
