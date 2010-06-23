/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSSIGNPOSTELM_H
#define GMSSIGNPOSTELM_H

#include "config.h"
#include "ItemTypes.h"
#include "LangTypes.h"
#include "MC2String.h"
//#include "OldGenericMap.h"
class DataBuffer;
class OldGenericMap;

/**
 *   Class representing a sign post element. Such an element is typically 
 *   stored in a sign post set (GMSSignPostSet). An element contains one 
 *   item of information such as a destination description, a route number or
 *   a pictogram.
 *
 */
class GMSSignPostElm {
   friend class SignPostTable;

public:
   typedef uint32 ElementClass;
   /// An invalid value for an element class.
   static const ElementClass INVALID_CLASS;

   /**
    * Defines the nature of what this sign post element poins at.
    *
    * Should be possible to store in a byte
    */
   enum connectionType_t {
      undefined = 0,  // undefined in data
      connection = 1, // E.g. a route number that takes the driver to a 
                      // destination.
      destination = 2,// The actual destination information.
      exit = 3,       // Exit information, e.g. exit number or name.

      unsetConnectionType = 0xff // Highest value
   };

   /**
    * Tells what kind of information this sign post holds.
    * 
    * Should be possible to store in a byte
    */
   enum elementType_t {
      invalidElementType = 0,
      routeNumber = 1,
      routeName = 2,
      placeName = 3,
      exitNumber = 4,
      exitName = 5,
      otherDestination = 6,
      pictogram = 7
   };

   // Member methods   /////////////////////////////////////////////////////

   GMSSignPostElm();

   
   /**
    * @name Set methods for individual member variables.
    */
   //@{
   void setTextStringCode(uint32 stringCode);
   void setType(elementType_t type);
   void setClass(ElementClass elementClass);
   void setConnectionType(connectionType_t connectionType);
   void setAmbigousInfo(bool ambInfoValue);
   //@}

   /**
    * @name Get methods for individual member variables.
    */
   //@{
      uint32 getTextStringCode() const;
      connectionType_t getConnectionType() const;
      bool getAmbigousInfo() const;
      /**
       * @return The element type of this element. Element type determines
       *         the type of information stored in this element, for example:
       *         route number, place name, exit number etc.
       */
      elementType_t getType() const;
      ElementClass getElementClass() const;
   //@}



   /**
    * @param theMap The map in which string table the the text of this sign 
    *               post part is stored.
    *
    * @return Returns the text of this sign post element.
    */
   MC2String getTextString(const OldGenericMap& theMap) const;

   /** @name Get information about the text of this sign post element.
    */
   //@{
   LangTypes::language_t getTextLang(const OldGenericMap& theMap) const;
   ItemTypes::name_t getTextNameType(const OldGenericMap& theMap) const;
   //@}        


   /**
    * For debug printing to the log.
    */
   friend ostream& operator<< ( ostream& stream, 
                                const GMSSignPostElm& spElm );
   ostream& debugPrint( ostream& stream, 
                        const OldGenericMap* theMap );

   /**
    * @name Comparison operators.
    */
   //@{
   bool operator==( const GMSSignPostElm& other ) const;
   bool operator!=( const GMSSignPostElm& other ) const;
   //@}

   /**
    * @return Returns true if any of the data of this element differs from its
    *         unset default values.
    */ 
   bool isSet() const;
   

   /**
    * Methods for saving and loading from and to DataBuffer.
    */
   //@{
   void save(DataBuffer& dataBuffer) const;
   void load(DataBuffer& dataBuffer);

   /**
    * Returns the maximum number of bytes this object will occupy when saved 
    * in  a data buffer.
    */
   static uint32 sizeInDataBuffer();
   
   //@}


 private:
   // Member methods   /////////////////////////////////////////////////////
   
   uint32* getTextStringCodePointer();

   // Member variables /////////////////////////////////////////////////////

   /** String code of the text of this signpost entry. 
    *
    *  In case of a pictogram, this is its ID, as defined by the Tele Atlas
    *  MultiNet specification.
    */
   uint32 m_text;


   /** 
    *  Gives indication of what color to paint this entry in.
    *
    *  The colors to use of each value are defined by the Tele Atlas
    *  MultiNet specification.
    */
   ElementClass m_elementClass;

   /** Tells what kind of information this sign post set holds.
    *
    *  Is stored in a byte in the map.
    */
   elementType_t m_elementType;
   
   /** Indicates that the destination of this GMSSignPostElm is used on also 
    *  another sign at the same crossing, but using another route. E.g. a sign
    *  pointing left towards Stockholm via the highway, and at the same 
    *  location a sign pointing towards Stockholm via a smaller road. 
    *
    *  If set to true, this element should be avoided when displaying the 
    *  information of this sign post.
    */
   bool m_ambigousInfo;

   /** Tells what kind of feature this sign post points at.
    *
    *  Is stored in a byte in the map.
    *  The value GMSSignPostElm::unsetConnectionType indicates unset value.
    */
   connectionType_t m_connectionType;


   
}; // GMSSignPostElm


// =======================================================================
//                                     Implementation of inlined methods =


inline uint32
GMSSignPostElm::getTextStringCode() const {
   return m_text;
}

inline GMSSignPostElm::connectionType_t
GMSSignPostElm::getConnectionType() const {
   return m_connectionType;
}

inline bool
GMSSignPostElm::getAmbigousInfo() const {
   return m_ambigousInfo;
}


#endif // GMSSIGNPOSTELM_H


