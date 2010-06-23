/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVINFOHANDLER_H
#define NAVINFOHANDLER_H

#include "config.h"
#include "NavHandler.h"
#include "ItemInfoEnums.h"

#include <memory>

class NavRequestPacket;
class NavReplyPacket;
class UserItem;
class InfoTypeConverter;


/**
 * Class handling NavServerProt, v10+, infos.
 *
 */
class NavInfoHandler : public NavHandler {
public:
   /**
    * Constructor.
    */
   NavInfoHandler( InterfaceParserThread* thread,
                   NavParserThreadGroup* group );
   
   
   /**
    * Handles a poi info packet.
    *
    * @param userItem The user.
    * @param req The request to info.
    * @param reply Set status of if problem.
    * @return True if info ok, false if not and then reply's status
    *         is set.
    */
   bool handleInfo( UserItem* userItem, 
                    NavRequestPacket* req, NavReplyPacket* reply );

   
   /**
    * Handles a poi detail packet.
    *
    * @param userItem The user.
    * @param req The request to info.
    * @param reply Set status of if problem.
    * @return True if info ok, false if not and then reply's status
    *         is set.
    */
   bool handleDetail( UserItem* userItem, 
                      NavRequestPacket* req, 
                      NavReplyPacket* reply );

   /**
    * Converts from MC2 to Nav2.
    */
   static uint16 infoTypeToAdditionalInfoType( 
      ItemInfoEnums::InfoType type, byte reqVer );


   /**
    * Converts from Nav2 to MC2.
    */
   static ItemInfoEnums::InfoType additionalInfoTypeToInfoType( 
      uint16 type, byte reqVer );
   
   
private:
   static std::auto_ptr<InfoTypeConverter> m_infoTypeConverter; 
};


#endif // NAVINFOHANDLER_H

