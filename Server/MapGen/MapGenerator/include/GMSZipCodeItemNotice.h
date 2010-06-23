/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSZIPCODEITEMNOTICE_H
#define GMSZIPCODEITEMNOTICE_H

#include "VectorElement.h"
#include "GMSZipCodeItem.h"

/**
  *   Notice object for GMSZipCodeItem.
  *   
  */
class GMSZipCodeItemNotice : public VectorElement {
   public:
     
      /** 
        *   Constructor 
        */
      GMSZipCodeItemNotice(const char* zipCode, const char* zipArea,
                           const char *alphaSortingName) {
         m_zipCode = StringUtility::newStrDup(zipCode);
         m_zipArea = StringUtility::newStrDup(zipArea);
         m_alphaSortingName = StringUtility::newStrDup(alphaSortingName);
      }

  
      /** 
        *   Destructor 
        */
      virtual ~GMSZipCodeItemNotice() {
         delete m_zipCode;
         delete m_zipArea;
         delete m_alphaSortingName;
      }

      const char* getZipCode() const { return m_zipCode; };
      const char* getZipArea() const { return m_zipArea; };
      const char *getAlphaSortingName() const { return m_alphaSortingName; }

      ///   equal
      bool virtual operator == (const VectorElement& elm) const {
         GMSZipCodeItemNotice &zcin = (GMSZipCodeItemNotice &) elm;
         return (( strcmp( getAlphaSortingName(),
                           zcin.getAlphaSortingName() ) == 0 ));
      }
   
      ///   not equal
      bool virtual operator != (const VectorElement& elm) const {
         GMSZipCodeItemNotice &zcin = (GMSZipCodeItemNotice &) elm;
         return (( strcmp( getAlphaSortingName(),
                           zcin.getAlphaSortingName() ) != 0 ));
      }
   
      ///   greater
      bool virtual operator > (const VectorElement& elm) const {
         GMSZipCodeItemNotice &zcin = (GMSZipCodeItemNotice &) elm;
         return (( strcmp( getAlphaSortingName(),
                           zcin.getAlphaSortingName() ) > 0 ));
      }
   
      ///   less
      bool virtual operator < (const VectorElement& elm) const {
         GMSZipCodeItemNotice &zcin = (GMSZipCodeItemNotice &) elm;
         return (( strcmp( getAlphaSortingName(),
                           zcin.getAlphaSortingName() ) < 0 ));
      }
   
   private:
      GMSZipCodeItemNotice() {}; // private default constructor.
      
      char* m_zipCode;
      char* m_zipArea;
      char *m_alphaSortingName;
};

#endif
