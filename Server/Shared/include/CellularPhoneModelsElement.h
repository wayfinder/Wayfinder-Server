/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CELLULARPHONEMODELSELEMENT_H
#define CELLULARPHONEMODELSELEMENT_H

#include "config.h"
#include "CacheElement.h"
#include "UserData.h"


/**
 * CacheElement container for CellularPhoneModels.
 *
 */
class CellularPhoneModelsElement : public CacheElement {
   public:
      /**
       * Constructs a new CellularPhoneModelsElement.
       * @param models The CellularPhoneModels that this object now
       *               should cache. This class now handles deletion
       *               of models.
       */
      CellularPhoneModelsElement( CellularPhoneModels* models );

      
      /**
       * Destructor, deletes models.
       */
      virtual ~CellularPhoneModelsElement();


      /**
       *  Finds a phone model named name, returns NULL if none found.
       */
      inline CellularPhoneModel* findModel( const char* name );
   

      /**
       * Returns the model with index index, returns NULL if no such index.
       */
      inline CellularPhoneModel* getModel( uint32 index );
   

      /**
       * Returns the different manufacturer's names.
       * If no manufacturers exists the function returns empty 
       * StringVector.
       */
      inline const StringVector& getManufacturer();


      /**
       * Returns the number different manufacturers.
       */
      inline uint32 getNbrManufacturers();
      

      /**
       * Returns the number of CellularPhoneModel.
       */
      inline uint32 getNbrCellularPhoneModels();


   private:
      /**
       * The CellularPhoneModels to cache.
       */
      CellularPhoneModels* m_models;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline CellularPhoneModel* 
CellularPhoneModelsElement::findModel( const char* name ) {
   return m_models->findModel( name );
}


inline CellularPhoneModel* 
CellularPhoneModelsElement::getModel( uint32 index ) {
   return m_models->getModel( index );
}


inline const StringVector&
CellularPhoneModelsElement::getManufacturer() {
   return m_models->getManufacturer();
}


inline uint32 
CellularPhoneModelsElement::getNbrManufacturers() {
   return m_models->getNbrManufacturers();
}


inline uint32 
CellularPhoneModelsElement::getNbrCellularPhoneModels() {
   return m_models->size();
}


#endif // CELLULARPHONEMODELSELEMENT_H

