/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// This unit test tests the SearchMatches
//


#include "MC2UnitTestMain.h"
#include "UserData.h"


// Test country match
MC2_UNIT_TEST_FUNCTION( cellularPhoneModelsTest  ) {
   CellularPhoneModels models;

   CellularPhoneModel* cpm1 = new CellularPhoneModel();
   cpm1->setName("Iphone");
   cpm1->setManufacturer("Apple");
   MC2_TEST_REQUIRED( models.addPhoneModel ( cpm1 ) );

   CellularPhoneModel* cpm2 = new CellularPhoneModel();
   cpm1->setName("Hero");
   cpm2->setManufacturer("HTC");
   MC2_TEST_REQUIRED( models.addPhoneModel ( cpm2 ) );

   CellularPhoneModel* cpm3 = new CellularPhoneModel();
   cpm3->setName("T610");
   cpm3->setManufacturer("SonyEricsson");
   MC2_TEST_REQUIRED( models.addPhoneModel ( cpm3 ) );

   CellularPhoneModel* cpm4 = new CellularPhoneModel();
   cpm4->setName("AX75");
   cpm4->setManufacturer("Siemens");
   MC2_TEST_REQUIRED( models.addPhoneModel ( cpm4 ) );

   CellularPhoneModel* cmRes = models.findModel( "T610" );
   MC2_TEST_REQUIRED(cmRes != NULL);
   MC2_TEST_REQUIRED(strcmp( cmRes->getName(), "T610" ) == 0);

   MC2_TEST_REQUIRED( models.getModel( 77 ) == NULL );
   MC2_TEST_REQUIRED( models.getModel( 2 ) != NULL );

   MC2_TEST_REQUIRED( strcmp( models.getModel( 2 )->getName(), "Hero" ) == 0 );

}
