/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRANSACTIONPACKET_H
#define TRANSACTIONPACKET_H

#include "config.h"
#include "Packet.h"

class StringCode;

/**
 * Class describing the packet sent to the UserModule to 
 * get/increase/decrease the number of transactions for a user.
 *
 * After the general RequestPacket-header the format is:
 * \begin{tabular}{rll}
 *    Position &  Size     & Description \\ \hline
 *     0       & 4 bytes   & UIN\\
 *     4       & 4 bytes   & Action type\\
 *     8       & 4 bytes   & Number transactions\\
 * \end{tabular}
 *
 */
class TransactionRequestPacket : public RequestPacket {
   public:
      /**
       * The types of actions.
       */
      enum action_t {
         /// Just get the current number of transactions.
         get,
         /// Increase the number of transactions, return new value
         increase,
         /// Decrease the number of transactions, return new value
         decrease
      };


      /**
       * Constructor.
       *
       * @param UIN The User Identification Number.
       * @param action The type of action to take.
       * @param nbrTransactions The number of transactions.
       */
      TransactionRequestPacket( uint32 UIN, action_t action, 
                                uint32 nbrTransactions = 0 );


      /**
       * Get the UIN.
       */
      uint32 getUIN() const;


      /**
       * Get the action.
       */
      action_t getAction() const;


      /**
       * Get the number of transactions.
       */
      uint32 getNbrTransactions() const;
};


/**
 * Class describing the reply to a TransactionRequestPacket.
 *
 * After the general ReplyPacket-header the format is:
 * \begin{tabular}{rll}
 *    Position &  Size     & Description \\ \hline
 *    0        & 4 bytes   & Number transactions\\
 * \end{tabular}
 *
 */
class TransactionReplyPacket : public ReplyPacket {
   public:
      /**
       * Constructor.
       *
       * @param req The TransactionRequestPacket that this is a reply to.
       * @param status The status of the reply.
       * @param nbrTransactions The number of transactions.
       */
      TransactionReplyPacket( const TransactionRequestPacket* req,
                              StringCode status,
                              uint32 nbrTransactions );


      /**
       * Get the number of transactions.
       */
      uint32 getNbrTransactions() const;
};


/**
 * Class describing the packet sent to the UserModule to 
 * check the transaction days for a user.
 *
 * After the general RequestPacket-header the format is:
 * \begin{tabular}{rll}
 *    Position &  Size     & Description \\ \hline
 *     0       & 4 bytes   & UIN\\
 *     4       & 4 bytes   & nbrTransactions \\
 *     8       & 1 bytes   & Check \\
 * \end{tabular}
 *
 */
class TransactionDaysRequestPacket : public RequestPacket {
   public:
      /**
       * Constructor.
       *
       * @param UIN The User Identification Number.
       * @param check If to check if a new day is needed.
       * @param nbrTransactionDays The number of number of transaction 
       *                           days.
       */
      TransactionDaysRequestPacket( uint32 UIN, bool check, 
                                    int32 nbrTransactionDays = 0 );


      /**
       * Get the UIN.
       */
      uint32 getUIN() const;


      /**
       * Get if to check if a new day is needed.
       */
      bool getCheck() const;


      /**
       * Get the number of number of transaction days.
       */
      int32 getNbrTransactionDays() const;
};


/**
 * Class describing the reply to a TransactionDaysRequestPacket.
 *
 * After the general ReplyPacket-header the format is:
 * \begin{tabular}{rll}
 *    Position &  Size     & Description \\ \hline
 *    4        & 4 bytes   & Number transaction days left\\
 *    8        & 4 bytes   & Time when current transaction day started\\
 * \end{tabular}
 *
 */
class TransactionDaysReplyPacket : public ReplyPacket {
   public:
      /**
       * Constructor.
       *
       * @param req The TransactionRequestPacket that this is a reply to.
       * @param status The status of the reply.
       * @param nbrTransactionDays The number of transaction days left.
       * @param currStartTime Time when current transaction day started.
       */
      TransactionDaysReplyPacket( const TransactionDaysRequestPacket* req,
                                  StringCode status,
                                  uint32 nbrTransactionDays,
                                  uint32 currStartTime );


      /**
       * Get the number of transaction days left.
       */
      uint32 getNbrTransactionDays() const;


      /**
       * Get the time when current transaction day started.
       */
      uint32 getCurrStartTime() const;
};


#endif // TRANSACTIONPACKET_H


