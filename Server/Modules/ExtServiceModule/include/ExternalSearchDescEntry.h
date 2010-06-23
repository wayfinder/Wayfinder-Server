/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTERNALSEARCHDESCENTRY_H
#define EXTERNALSEARCHDESCENTRY_H

#include "config.h"
#include "MC2String.h"

#include <vector>

class Packet;

class ExternalSearchDescEntry {
public:
   /// Types of fields. Don't forget to change in servers (e.g. XML)   
   enum val_type_t {
      /// Value can be a string
      string_type  = 0,
      /// Value can be a number
      number_type  = 1,
      /// Value can be one of the values in m_subVals (enum)
      choice_type   = 2,
   };

   /// Type of the vector used for choices
   typedef vector<pair<int, MC2String> > choice_vect_t;

   /**
    *   Creates a new ExternalSearchDescEntry.
    *   @param id   The id to send back to the server together with the value.
    *   @param name The text to display when showing this.
    *   @param type The type of value. If choice_type, then there are choices
    *               in the vector of sub-values.
    *   @param required Bitfield which is used to check if a value is required.
    *                   It is done like so:
    *                   <code>
    *                   uint32 x = MAX_UINT32;
    *                   for ( i in filled_in_fields ) x &= i.value;
    *                   for ( i in not_filled_in_fields && i.value != 0 )
    *                             x &= ~i.value;
    *                   if ( x ) then all is ok.
    *                   </code>
    */
   ExternalSearchDescEntry( int id,
                            const MC2String& name,                        
                            val_type_t type,
                            uint32 required );

   /// Returns the id.
   int getID() const { return m_id; }
   /// Returns the title
   const MC2String& getName() const { return m_name; }
   /// Returns the requirements bitfield
   uint32 getRequiredBits() const { return m_required; }
   /// Returns the type
   val_type_t getType() const { return m_type; }
   /// Returns the choices if it is a choice_type
   const choice_vect_t& getChoices() const;
   
   /// Adds a new param to the param if it is a multiparam.
   void addChoiceParam( int value,
                        const MC2String& name );
   
   /// Prints the params on the ostream.
   friend ostream& operator<<( ostream& o,
                               const ExternalSearchDescEntry& param ) {
      param.print(o);
      return o;
   }

   /// Saves the entry into a packet.
   int save( Packet* packet, int& pos ) const;
   
private:

   /// Id of the parameter
   int m_id;
   /// Name of the parameter
   MC2String m_name;
   /// Value type
   val_type_t m_type;
   /// Required field
   uint32 m_required;   
   /// Vector of values if multi_type
   choice_vect_t m_choices;
   
   void print( ostream& o ) const;
};

#endif
