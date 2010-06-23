/*
 *
 *
 */
public interface GPSSenderAdder {

   /**
    * Notifies the adder that a new sender is added. The adder must return a
    * listener.
    */
   GPSSenderListener addSender(GPSSender sender);

}
