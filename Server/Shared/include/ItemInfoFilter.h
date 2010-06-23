/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMINFOFILTER_H
#define ITEMINFOFILTER_H
#include "ItemInfoEnums.h"

namespace ItemInfoFilter {

/**
 * Decides if the item info of a certain type shall be filtered
 * @param type The type of the item info.
 * @param filter The filter used.
 * @return true if the info item shall be visible, false if filtered out.
 */
bool includeItemInfo( ItemInfoEnums::InfoType type, 
                      ItemInfoEnums::InfoTypeFilter filter );

/**
 * Decides if the Name item info shall be filtered
 * @param filter The filter used.
 * @return true if the Name shall be added, false if filtered out.
 */
bool includeNameItems( ItemInfoEnums::InfoTypeFilter filter );

/**
 * Decides if the Category item info shall be filtered
 * @param filter The filter used.
 * @return true if the Category shall be added, false if filtered out.
 */
bool includeCategoryItem( ItemInfoEnums::InfoTypeFilter filter );

}

#endif
