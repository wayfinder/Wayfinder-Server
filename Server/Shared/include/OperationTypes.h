/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OPERATION_TYPE
#define OPERATION_TYPE

namespace OperationType {

/// OperationType
enum operationType {
   /// UNKNOWN
   UNKNOWN = 0,
   /// SEARCH
   SEARCH = 1,
   /// SEARCH_CONTINUED
   SEARCH_CONTINUED = 2,
   /// ROUTE
   ROUTE = 3,
   /// ROUTE_CONTINUED
   ROUTE_CONTINUED = 4,
   /// INFORMATION
   INFORMATION = 5,
   /// INFORMATION_CONTINUED
   INFORMATION_CONTINUED = 6,
   /// MESSAGE
   MESSAGE = 7,
   /// MESSAGE_CONTINUED
   MESSAGE_CONTINUED = 8,
   /// LOCALIZATION
   LOCALIZATION = 9,
   /// LOCALIZATION_CONTINUED
   LOCALIZATION_CONTINUED = 10,
   /// SORT_DIST
   SORT_DIST = 11,
   /// SORT_DIST_CONTINUED
   SORT_DIST_CONTINUED = 12,
   /// PROXIMITY
   PROXIMITY = 13,
   /// PROXIMITY_CONTINUED
   PROXIMITY_CONTINUED = 14,
   /// MAP
   MAP = 15,
   /// SEARCH_RIGHT. For checkService search access check.
   SEARCH_RIGHT = 16,
   /// ROUTE_RIGHT. For checkService route access check.
   ROUTE_RIGHT = 17,
   /// ROUTE_RIGHT. For checkService purchase.
   PURCHASE_RIGHT = 18,
   /// APP_STORE_VERIFY. For App Store purchase.
   APP_STORE_VERIFY = 19,

   NBR // The number of operations not an operation
};

}

#endif // OPERATION_TYPE
