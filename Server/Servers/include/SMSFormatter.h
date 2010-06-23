/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SMSFormatter_H
#define SMSFormatter_H

#include "config.h"
#include "StringTable.h"
#include "UserConstants.h"

/*
 *   The maximum size of one SMS.
 */
#define MAX_SMS_SIZE 160
#define MAX_CONCATENATED_SMS_SIZE 152

class SMSUserSettings; // forward
class ExpandItemID; //forward
class ExpandRouteReplyPacket;
class SearchMatch;
class VanillaMatch;
class OverviewMatch;
class UserCellular;
class CellularPhoneModel;
class SMSSendRequest;


/**
  *   Create SMS(es) from given indata. The type of phone etc. are
  *   considered during the formatting.
  *
  */
class SMSFormatter {
  public:
   /**
    *   @name Constants used when printing the choice lists.
    */
   //@{
   /// The maximum number of rows in one printed choice.
   static const uint32 maxNbrChoiceRows;
   //@}
   
   /**
    *   Write a route to SMS(es).
    *
    *   @param   ePack          Packet containing the route-data.
    *   @param   senderPhone    Phonenumber of the sender.
    *   @param   receiverPhone  Phonenumber of the receiver.
    *   @param   requestID      ID of the created request.
    *   @param   smsUserFormat  The format for the telephone.
    *   @param   language       The language of the message text.
    *   @return  A SMSSendRequest with one or more SMSSendRequestPackets.
    *            If something went wrong, NULL is returned.
    */
   static SMSSendRequest* getRouteSMSes(
      ExpandRouteReplyPacket* ePack,
      const char* const senderPhone,
      const char* const receiverPhone,
      bool bConcatenatedSMS,
      uint16 requestID,
      VanillaMatch *origin,
      OverviewMatch *origLocation,
      VanillaMatch *destination,
      OverviewMatch *destLocation,
      SMSUserSettings *smsUserFormat);

   /**
    *   Write one choice to a databuffer, formatted for an SMS.
    *
    *   @param   i     Choice number
    *   @param   vm    Data for the match.
    *   @param   locationName Name of the location that the search was in.
    *   @param   data  Where to write the choice.
    *   @param   smsUserFormat The formatting information.
    *   @return  The number of characters written to data.
    */
   static uint32 writeSMSChoice( uint32 i, SearchMatch *vm,
                                 const char* locationName,
                                 char* data,
                                 SMSUserSettings *smsUserFormat);
         
   /**
    *   Calculate the length of one choice if written with 
    *   SMSFormatter::writeSMSChoice(...).
    *
    *   @param   i     Choice number
    *   @param   vm    Data for the match.
    *   @param   locationName Name of the location that the search was in.
    *   @param   smsUserFormat Formatting information.
    *   @return  The number of characters written to data.
    */
   static uint32 calcSMSChoiceLength( uint32 i, SearchMatch *vm,
                                      SMSUserSettings *smsUserFormat,
                                      const char* locationName);
   
   /**
    * Converts a vanillamatch into a locationstring
    * (even if the match is null "<NULL>").
    * @param match A vanillamatch from a search.
    * @return A new string containing a copy of the location name
    *         of the match. NB! You must delete the string!
    */
   static char *getNewMatchStringLocation( VanillaMatch *match );
  
   /**
    * Converts a vanillamatch into a string
    * (even if the match is null "<NULL>").
    * @param match A vanillamatch from a search.
    * @return A new string containing a copy of the first name
    *         of the match. NB! You must delete the string!
    */
   static char *getNewMatchString( VanillaMatch *match );
   
   /**
    * Converts an overviewmatch into a string
    * (even if the match is null "<NULL>").
    * @param location An overviewmatch from a search.
    * @return A new string containing a copy of the first name
    *         of the match. NB! You must delete the string!
    */
   static char *getNewMatchString( OverviewMatch *location );
      
   /**
    * Converts an overviewmatch and a vanillamatch into a string
    * (even if either or both of the matches are null).
    * @param location an overviewmatch from a search.
    * @param match A vanillamatch from a search.
    * @return A new string containing a copy of the first name
    *         of the matches. NB! You must delete the string!
    */
   static char *getNewMatchString( OverviewMatch *location,
                                   VanillaMatch *match );

   /**
    * Converts a vanillamatch into a string.
    * (even if the match is null).
    * @param match A vanillamatch from a search.
    * @return A new string containing a copy of the name
    *         of the match. NB! You must delete the returned string!
    */
   static char *getNewMatchStringWithLocation( VanillaMatch *match );
   
   /**
    * Sets the res string to the street number of match,
    * if match is a VanillaCompanyMatch.
    * @param res the result
    * @param match the match holding the street number.
    */
   static void getCompanyStreetNumber(char *&res, VanillaMatch *match);
   
   /**
    *   Create a reply to a given StringCode/error code from a module.
    *
    *   @param   code           The code in the string table.
    *   @param   origin         The origin.
    *   @param   destination    The destination.
    *   @param   senderPhone    Phonenumber of the sender.
    *   @param   receiverPhone  Phonenumber of the receiver.
    *   @param   smsUserFormat  The format of the user's telephone.
    *   @param   requestID      ID of the created request.
    *   @param   language       The language of the message text.
    *   @return  The SMSSendRequest containing the SMS reply.
    */
   static SMSSendRequest* getReply(StringTable::stringCode code, 
                                   VanillaMatch *origin,
                                   VanillaMatch *destination,
                                   char* senderPhone,
                                   char* receiverPhone,
                                   SMSUserSettings *smsUserFormat,
                                   uint16 requestID);

   /**
    *   Print one string (NULL-terminated) into another, formatted
    *   according to the given parameters. This method might be used 
    *   by the other formatting methods in this
    *   class as well as by other classes.
    *
    *   @param   dest        String where the result is written.
    *   @param   src         String containing the information that
    *                        should be written correctly formatted to 
    *                        dest.
    *   @param   language    The language of the string text if error.
    *   @param   eol         The type of end of line for the receivers
    *                        telephone.
    *   @param   phoneWidth  The number of characters of the receivers
    *                        deisplay.
    *   @param   offetOnFirstLine
    *                        Default value is 0.
    *   @param   maxNbrLines The maximum number of lines (if the 
    *                        result does not fit it is truncated). If 
    *                        less than one then the needed number of 
    *                        lines are used. Default value is 
    *                        MAX_UINT32.
    *   @param   newLineAfterString
    *                        True if the string shouls end with eol,
    *                        false otherwise. Dafault value is true.
    *   @return  The number of characters written to the dest string
    *            dest.
    */
   static uint32 printSMSString(char* const dest,
                                const char* const src,
                                StringTable::languageCode language,
                                UserConstants::EOLType eol, 
                                uint32 phoneWidth,
                                uint32 offsetOnFirstLine = 0,
                                uint32 maxNbrLines = MAX_UINT32, 
                                bool newLineAfterString = true);

   /**
    * Creates and copies the source string to another string.
    * The string will be terminated with the correct eol.
    * @param dest The destination string.
    * @param strLen The length of the copied data (including
    *               eol).
    * @param src The source string.
    * @param smsUserFormat The format of the user terminal.
    * @param language The language of error messages.
    */
   static void newSMSString( char *&dest,
                             uint32 &strLen,
                             const char *src,
                             SMSUserSettings *smsUserFormat);


   /**
    * Make a single SMS from a number of strings with optional linebreak
    * between them. If all the strings doesn't fit into the SMS they are
    * silently left out of the SMS.
    *
    * @param cellular The UserCellular to get formating settings from.
    * @param nbrStrings The number of strings in function call.
    * @param lineBreak If there should be linebreaks between strings.
    * @param ... nbrStrings strings. Assumend to be const char*.
    * @return A SMSSendRequest with the
    *         SMSSendRequestPacket containing the strings.
    */
   static SMSSendRequest* makeTextSMSRequest( const UserCellular* cellular,
                                              uint32 requestID,
                                              const char* userPhone,
                                              const char* serverPhone,
                                              uint32 nbrStrings,
                                              bool lineBreak,
                                              ... );


   /**
    * Make a single SMS from a number of strings with optional linebreak
    * between them. If all the strings doesn't fit into the SMS they are
    * silently left out of the SMS.
    *
    * @param cellular The CellularPhoneModel to get formating settings 
    *                 from.
    * @param nbrStrings The number of strings in function call.
    * @param lineBreak If there should be linebreaks between strings.
    * @param ... nbrStrings strings. Assumend to be const char*.
    * @return A SMSSendRequest with the
    *         SMSSendRequestPacket containing the strings.
    */
   static SMSSendRequest* makeTextSMSRequest( const CellularPhoneModel* cellular,
                                              uint32 requestID,
                                              const char* userPhone,
                                              const char* serverPhone,
                                              uint32 nbrStrings,
                                              bool lineBreak,
                                              ... );

   
   /**
    * Gets the sms user settings from a CellularPhoneModel and sets it
    * in a SMSUserSettings.
    * @param smsUserFormat The SMSUserSettings to fill in.
    * @param model The CellularPhoneModel to get settings from.
    * @param lang The prefered language.
    */
   static void setSMSUserSettingsFromCellularPhoneModel( 
      SMSUserSettings* smsUserFormat, CellularPhoneModel* model,
      StringTable::languageCode lang );

      
  private:
   /**
    * Returns a text with the route summary in the ePack
    * @param ePack the expand route reply packet
    * @param phoneWidth the phoneWidth of the phone
    * @param eol the eol of the phone
    * @param language The language of the message text.
    * @return the text of the route summary
    */
   static char *getRouteSummary( ExpandRouteReplyPacket *ePack,
                                 uint32 phoneWidth,
                                 UserConstants::EOLType eol,
                                 StringTable::languageCode language);
      
   /**
    *   Create one SMSSendRequestPacket and add it to the
    *   given SMSSendRequest.
    *
    *   @param   smsNbr         What SMS in order. This variable
    *                           is increased by one for each call.
    *   @param   totalNbrOfSMSes   The number of SMS that are linked
    *                           together.
    *   @param   lastSMS        true if this is the last sms.
    *   @param   sendThisOne    true if this sms is finished and should
    *                           be sent. false if it's not done yet?
    *   @param   senderPhone    Phonenumber of the sender.
    *   @param   receiverPhone  Phonenumber of the receiver of the
    *                           SMS.
    *   @param   data           Pointer to the datafield where the
    *                           content in the SMS is. This must
    *                           be NULL-terminated!
    *   @param   theRequest     The request where to add the new
    *                           SMSSendRequestPacket (that is created
    *                           in this method).
    *   @param   eol            How end of line should be written.
    *   @param   messageID      SMSMessageid (is 0 now)
    *   @param   operationType  Type of operation
    *   @param   operationDescription String describing the operation
    *   @param   language       The language of the message text.
    *   @return  Pointer to a NULL-terminated databuffer that should
    *            be used as the begining of the next SMS.
    */
   static char* createNewPacket( int32 &smsNbr,
                                 uint32 &totalNbrOfSMSes,
                                 bool lastSMS,
                                 bool sendThisOne,
                                 bool bConcatenatedSMS,
                                 const char* const senderPhone,
                                 const char* const receiverPhone, 
                                 char* data, SMSSendRequest* theRequest,
                                 SMSUserSettings *smsUserFormat);

   /**
    *   Write end of line to a string. The format and the number
    *   of characters required depends on the phonetype (the EOLType
    *   parameter given to this method).
    *
    *   @param   dest  The destination databuffer.
    *   @param   pos   The position of the first character to write
    *                  to dest (e.g the first character is written
    *                  to dest[pos]).
    *   @param   eol   What end of line type the receivers telephone
    *                  have.
    *   @param   charsLeftOnLine
    *                  The number of characters left on the current
    *                  line. Needed if the phone does not recognize
    *                  any new line character and must be padded
    *                  with spaces.
    *   @return  The number of characters written to dest.
    */
   static uint32 writeEol(char* dest, uint32 pos, 
                          UserConstants::EOLType eol,
                          uint32 charsLeftOnLine);
   
   
   friend class SMSUserSettings;
};

class SMSUserSettings {
  public:
   /// constructor
   SMSUserSettings();
   /// destructor
   virtual ~SMSUserSettings() {};

   /// @return the linelength
   uint8 getSMSLineLength() { return m_SMSLineLength; }

   /// sets the linelength to
   /// @param lineLength
   void setSMSLineLength( uint8 lineLength ) {
      m_SMSLineLength = lineLength;
   }

   /// @return the EOLType
   UserConstants::EOLType getEOLType() { return m_SMSEOLType; }

   /// sets the EOLType to
   /// @param eol
   void setEOLType( UserConstants::EOLType eol )
      {
         m_SMSEOLType = eol;
         m_SMSEOLLength = calcEOLLength();
      }

   /// @return the length of the eol character combination
   uint32 getEOLLength() { return m_SMSEOLLength; }

   /**
    *  Gets the language.
    *  @return The language.
    */
   StringTable::languageCode getLanguage() const
      {
         return m_language;
      }

   /**
    *  Sets the language.
    *  @param language The language.
    */
   void setLanguage(StringTable::languageCode language) 
      {
         m_language = language;
      }

   /**
    *  Get the preferred language in short format.
    *  @return The short language.
    */
   StringTable::languageCode getSMSLanguage() const {
      return StringTable::getShortLanguageCode(getLanguage());
   }
   
  private:
   /// @return The length of the eol character combination
   ///         calculated by counting characters in the string.
   uint32 calcEOLLength();
   
   uint8 m_SMSLineLength;
   UserConstants::EOLType m_SMSEOLType;
   uint32 m_SMSEOLLength;
   
   StringTable::languageCode m_language;
};

#endif
