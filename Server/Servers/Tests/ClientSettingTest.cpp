/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2UnitTestMain.h"

#include "ImageTable.h"
#include "FileUtils.h"
#include "ClientSettings.h"
#include "MC2String.h"
#include "StringUtility.h"


/**
 * Test of the ClientSetting class
 *
 **/
MC2_UNIT_TEST_FUNCTION( clientSettingTest ) {

   char* data;
   const char* filename ="data/navclientsettings.txt";

   MC2_TEST_REQUIRED_EXT( FileUtils::getFileContent( filename, data ), 
                          "Failed to load required data file." );

   ClientSettingStorage settingStorage( NULL );

   MC2_TEST_REQUIRED( settingStorage.parseSettingsFile( data ) );

   const ClientSetting* setting = settingStorage.getSetting( "wf10-UnitTest1-br", "" );
   MC2_TEST_REQUIRED( setting != NULL );
   MC2_TEST_CHECK( strcmp( setting->getClientType(), "wf10-UnitTest1-br" ) == 0  );
   MC2_TEST_CHECK( strcmp( setting->getClientTypeOptions(), "Client-Type-Options" ) == 0  );
   MC2_TEST_CHECK( setting->getMatrixOfDoomLevel() == 2 );
   MC2_TEST_CHECK( setting->getNotAutoUpgradeToSilver() == false );
   MC2_TEST_CHECK( setting->getSilverRegionID() == 2097173 );
   MC2_TEST_CHECK( setting->getSilverTimeYear() == 0 );
   MC2_TEST_CHECK( setting->getSilverTimeMonth() == 6 );
   MC2_TEST_CHECK( setting->getSilverTimeDay() == 0 );
   MC2_TEST_CHECK( setting->getExplicitSilverTime() == MAX_UINT32 );
   MC2_TEST_CHECK( setting->getBlockDate() == StringUtility::makeDate( "2038-01-01") );
   MC2_TEST_CHECK( setting->getNotCreateWLUser() == false );
   MC2_TEST_CHECK( setting->getCreateLevel() == 0 );
   MC2_TEST_CHECK( setting->getCreateRegionID() == 2097173 );
   MC2_TEST_CHECK( setting->getCreateRegionTimeYear() == 0 );
   MC2_TEST_CHECK( setting->getCreateRegionTimeMonth() == 9 );
   MC2_TEST_CHECK( setting->getCreateRegionTimeDay() == 0 );
   MC2_TEST_CHECK( setting->getExplicitCreateRegionTime() == MAX_UINT32 );
   MC2_TEST_CHECK( strcmp( setting->getPhoneModel(), "K750i" ) == 0 );
   MC2_TEST_CHECK( strcmp( setting->getImageExtension(), "png" ) == 0 );
   MC2_TEST_CHECK( setting->getNoLatestNews() == false );
   MC2_TEST_CHECK( strcmp( setting->getCallCenterList(), "") == 0 );
   MC2_TEST_CHECK( strcmp( setting->getBrand(), "TEST") == 0 );
   MC2_TEST_CHECK( strcmp( setting->getCategoryPrefix(), "test" ) == 0 );
   MC2_TEST_CHECK( setting->getImageSet() == ImageTable::DEFAULT );
   MC2_TEST_CHECK( strcmp( setting->getVersion(), "4.64.0:4.9.0:0") == 0 );
   MC2_TEST_CHECK( setting->getForceUpgrade() == false );
   MC2_TEST_CHECK( strcmp( setting->getServerListName().c_str(), "" ) == 0 );
   MC2_TEST_CHECK( strcmp( setting->getUpgradeId().c_str(), "marketId55" ) == 0 );
   MC2_TEST_CHECK( strcmp( setting->getExtraRights().c_str(), "" ) == 0 );
   
   // one additional version tests
   const ClientSetting* setting2 = settingStorage.getSetting( "wf10-UnitTest2-br", "" );
   MC2_TEST_REQUIRED( setting2 != NULL );
   MC2_TEST_CHECK( strcmp( setting2->getVersion(), "4.65.0:5.9.0:0" ) == 0 );
   MC2_TEST_CHECK( setting2->getForceUpgrade() == true );
   MC2_TEST_CHECK( strcmp( setting2->getUpgradeId().c_str(), "http://market.place.com/download/1" ) == 0 );
   





}

