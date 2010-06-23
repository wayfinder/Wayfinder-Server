/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "TileModule.h"
#include "TileProcessor.h"
#include "TileReader.h"
#include "PropertyHelper.h"

#include "ExamplePolys.h"
#include "GfxUtility.h"
#include "Point.h"
#include "Intersect.h"
#include "XMLInit.h"

#include <vector>

#include "PolygonDefects.h"
#include "GnuPlotDump.h"
#include <sstream>

TileModule::TileModule( int argc, char* argv[] )
   : Module( MODULE_TYPE_TILE,
             argc, argv, false )
{
   
}

TileModule::~TileModule()
{

}

int
TileModule::handleCommand(const char* input) 
{      
   // no own commands right now, add later
   return Module::handleCommand(input);
}

void
TileModule::init()
{

   m_processor.reset( new TileProcessor( m_loadedMaps.get() ) );

   mc2dbg2 << "TileModule::init() done, going to Module::init()" << endl;

   Module::init();
}

const char* craziness = "2400 3000 4200 3000 6000 4800 4800 6000 2400 7200 3600 5400 5400 6000 4800 7200 4200 6000 4200 5400 4800 6000 5400 7200 6600 7800 6000 6000 4200 6600 3000 7200 2400 6600 3000 6000 5400 6600 6000 5400 4200 4800 3000 3000 5400 3000 5400 4200 3000 5400 2400 6600 3600 9000 7800 8400 7200 6000 4800 5400 3000 4800 2400 3600 3000 3600 4800 3600 4800 3000 3000 2400 6600 4200 6000 5400 2400 6000 2400 4200 3000 2400 1200 1800 4200 3600 9000 5400 6600 9000 4200 8400 6000 4800 6000 3600 3600 3600 1800 7800 1800 8400 5400 8400 7200 9600 3600 9000 2400 8400 600 6600 1200 6600 4200 9000 5400 9600 7200 9000 6000 7800 2400 7200 600 5400 2400 4800 8400 7800 9000 6600 7800 4200 6600 4200 5400 4200 3000 4800 1800 4800 1800 4200 6600 3000 6600 2400 4800 1800 3000 1800 5400 3600 7200 5400 9000 8400 6000 9000 2400 9000 6600 3000 2400 1200 10200 9000 7800 9600 600 7800 600 4200 6600 6600 5400 5400 4800 7200 7200 7200 7200 5400 4200 4800 1800 3600 2400 3000 1200 3000 1200 2400 1800 2400 1800 3000 1800 4200 600 3600 0 1800 600 1800 600 3600";

const char* star = "3600 7800 6600 600 10200 8400 3600 1200 12000 5400 3000 4800 10800 1800 3600 7800";

const char* karamellen = "0 0 10 10 20 0 20 10 10 0 0 10 0 0";

const char* star5 = "4000 2000 9000 2000 4000 6000 7000 500 9000 6000 4000 2000"; 

const char* blaj = "3000 4200 9000 4200 9000 2400 7200 5400 4800 1800 3000 4200";
const char* flugamedstreck = "4800 3000 7200 3000 7200 4200 4800 1800 4800 4200 7200 1800 7200 3000";

const char* halvklyddig = "3000 6000 6000 1200 7200 4200 3600 1800 6000 600 8400 1800 9600 3000 3000 3600 3000 1200 9000 3600 9000 600 1800 5400 4800 600 9000 5400 7800 600 5400 4800 3600 600 10800 4800 10200 600 6000 6000 3000 6000";


const char* adjacent = "3600 3000 7800 3000 7800 5400 4800 5400 4800 3000 9000 3000 9000 6000 3600 6000 3600 3000 2400 3000 2400 3600 3600 3600 3000 4200 3600 4200";
const char* adjacent2 = "7800 3000 7800 5400 4800 5400 4800 3000 9000 3000 9000 6000 3600 6000 3600 3000 2400 3000 2400 3600 3600 3600 3000 4200 3600 4200 3600 3000";
const char* adjacentsimple = "3600 3000 7800 3000 7800 5400 4800 5400 4800 3000 9000 3000 9000 6000 3600 6000";

const char* touching = "5400 2400 9600 2400 8400 4800 7200 2400 6000 4800 5400 2400";

const char* falu = "3600 2400 3600 3600 3000 3600 3000 4800 4800 4800 4800 3600 3600 3000 4800 2400 5400 2400 5400 5400 2400 5400 2400 2400 3600 2400";

const char* falu2 = "4800 3600 6000 3600 6000 2400 3600 2400 3600 6000 9000 6000 9000 2400 6000 2400 6000 3600 7800 3600 7800 4800 4800 4800 4800 3600";

const char* falu3 = "4200 4200 5400 3600 4200 3000 3000 3000 3000 5400 7800 5400 7800 3000 6000 3000 5400 3600 6000 4200 6000 4800 4200 4800 4200 4200";

const char* nonintersecting = "2400 2400 3600 1200 5400 1200 7800 3000 7800 4800 6000 6600 3600 6600 1800 6000 1800 4200 2400 2400";

const char* nonintersecting2 = "2400 3600 4200 3600 4200 4200 4200 4800 4200 3000 4200 4800 2400 4800 2400 4200 2400 3600";

const char* triangle = "3600 4800 5400 3000 7200 4800 3600 4800";

const char* flugan = "155969834 663413054 155981176 663400856 155981176 663413054 155969834 663400856";
const char* flugan3 = "155969 663413 155981 663400 155981 663413 155969 663400";
const char* flugan2 = "663413054 155969834 663400856 155981176 663413054 155981176 663400856 155969834";

const char* stupid = "100516062 656278606 100680036 656387922 100352088 656715870 100953326 656715870 100078798 656715870 99750850 656278606 100516062 656278606";

const char* verystupid = "10 0 0 0 5 0 6 0 3 0 5 0 7 0 7 5 7 10 7 5 7 0 -10 0 10 10 10 0";

const char* italy = "98384400 552920328 100680036 554232120 100516062 552264432 107730918 546525342 107075022 548329056 110846424 552592380 110682450 554724042 112814112 554724042 113633982 552428406 118717176 553248276 120028968 551280588 121340760 551772510 120356916 556035834 124784214 555215964 124784214 558987366 131507148 557839548 133146888 560135184 145936860 561446976 144461094 560791080 147576600 557347626 163482078 554888016 159546702 552264432 162990156 550788666 160694520 548820978 162662208 548493030 161842338 547017264 165941688 544393680 163646052 543901758 164793870 544065732 161514390 546197394 160202598 544885602 157906962 545377524 147412626 541442148 147084678 537834720 149708262 536358954 147740574 534227292 146592756 535047162 147576600 527832306 151348002 524716800 162498234 519469632 168237324 507827478 180863322 500120700 191521632 500448648 192997398 498316986 189717918 496513272 190045866 494873532 196768800 492085974 154955430 492085974 145772886 497989038 138885978 504384024 132327018 505695816 133474836 507171582 127243824 512254776 125112162 512090802 125440110 515698230 120684864 524716800 104287464 529963968 98548374 526520514 98384400 526192566";

const char* dubbeltratt = "0 2 1 1 2 1 3 2 3 0 2 1 1 1 0 0";

const char* england = "0 638842704 1639740 638022834 3935376 633267588 327948 630971952 4263324 629824134 6394986 631955796 15413556 631463874 20168802 628840290 20988672 626052732 18693036 621297486 15577530 619493772 13773816 620641590 15085608 619821720 12462024 619821720 15249582 619657746 8690622 616542240 10986258 617362110 10986258 615886344 3279480 613754682 16889322 613098786 16397400 609983280 2951532 605228034 0 605883930 0 638842704";

const char* three_crossing = "1200 1200 8400 8400 8400 4800 1200 4800 1200 7800 1200 8400 8400 1200 1200 1200";

const char* crazypart = "1800 4679 1800 4200 600 3600 0 1800 600 1800 600 3600 1800 3200 1800 3000 1200 3000 1200 2400 1800 2400 1800 3000 2400 3000 2800 3000 2867 2800 1200 1800 3000 2400 3900 2700";


void doStuff()
{
   stringstream kalle( crazypart );

   vector<vector<POINT> > stuff;
  
   vector<POINT> polygon;


   while ( kalle ) { 
      int x;
      int y;
      kalle >> x;
      kalle >> y;
      if ( kalle ) {
         polygon.push_back( makePoint( x, y ) );
      }
   }

   stuff.push_back( polygon );
//   std::reverse( polygon.begin(), polygon.end() );
//   stuff.back().insert( stuff.back().end(), polygon.begin(), polygon.end() );
              
   {
      vector<MC2Coordinate> coords;
      for ( int i = 0, n = stuff.back().size(); i < n; ++i ) {
         coords.push_back( MC2Coordinate( stuff.back()[i].y, stuff.back()[i].x ) );
      }
      bool intersect = GfxUtil::selfIntersecting( coords.begin(), coords.end() );
      cout << "intersects self = " << intersect << endl;
   
   }
   {
     vector<POINT> output;   
     GfxUtil::removeSomePolygonDefects( output,
                                        stuff.back().begin(),
                                        stuff.back().end() );
   }
 
   
   
//   GfxUtility::cleanSelfintersecting( stuff );
   cout << GnuPlotDump::octave_dump( stuff.back().begin(),
                                     stuff.back().end() );
   
   cout << "Before touching:" << endl;
  
   GfxUtility::splitSelfTouching( stuff );

   
   
   for ( vector<vector<POINT> >::const_iterator vt = stuff.begin();
         vt != stuff.end();
         ++vt ) {
      cout << GnuPlotDump::octave_dump( vt->begin(), vt->end() );   
   }
   {
      vector<MC2Coordinate> coords;
      for ( int i = 0, n = stuff.back().size(); i < n; ++i ) {
         coords.push_back( MC2Coordinate( stuff.back()[i].y, stuff.back()[i].x ) );
      }
      bool intersect = GfxUtil::selfIntersecting( coords.begin(), coords.end() );
      cout << "intersects self = " << intersect << endl;
   }
  
}


int main(int argc, char *argv[])
try {
//   doStuff();
//   return 0;
   ISABThreadInitialize init;
   PropertyHelper::PropertyInit propInit;
   XMLTool::XMLInit initXML;

   TileModule* TM = new TileModule( argc, argv );
   JTCThreadHandle handle = TM;

   TM->parseCommandLine();

   TM->init();
   TM->start();
   TM->gotoWork(init);

   return 0;
} catch ( const ComponentException& ex ) {
   mc2log << fatal << "Could not initialize TileModule. "
          << ex.what() << endl;
   return 1;
}

