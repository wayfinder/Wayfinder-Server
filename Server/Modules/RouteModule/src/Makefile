#
# Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
# 
#     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#     * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

TARGET   = RouteModule

# debug level
CXXFLAGS	+=	-DDEBUG_LEVEL_1
#CXXFLAGS	+=	-DDEBUG_LEVEL_2
#CXXFLAGS	+=	-DDEBUG_LEVEL_4
#CXXFLAGS	+=	-DDEBUG_LEVEL_8
#CXXFLAGS	+=	-DDEBUG_LEVEL_DB
#CXXFLAGS	+=	-DDEBUG_LEVEL_RM
#CXXFLAGS	+=	-DDEBUG_LEVEL_SPAM
#CXXFLAGS	+=	-DSPAM_ME_HARD
#OPTFLAGS	=	-O2 

ifeq ($(MAIN_METHOD), noderand)
   CXXFLAGS += -DMAIN_METHOD_NODERAND
   TARGET = NodeRandomizer
else
   ifeq ($(MAIN_METHOD), randtest)
      CXXFLAGS += -DMAIN_METHOD_RANDTEST
      TARGET = RandomNodesTest
   else
      ifeq ($(MAIN_METHOD), crashtest)
         CXXFLAGS += -DMAIN_METHOD_CRASHTEST
         TARGET = CrashTest
      else
         ifeq ($(MAIN_METHOD), readresult)
            CXXFLAGS += -DMAIN_METHOD_READ_ROUTING_RESULT
	         TARGET = ReadRoutingResult
         else
            ifeq ($(MAIN_METHOD), multitest)
              CXXFLAGS += -DMAIN_METHOD_ROUTEPROFILING
              TARGET = MultiMapTest
            else 
              CXXFLAGS += -DMAIN_METHOD_ROUTEMODULE
            endif
         endif
      endif
   endif
endif

export CDIR := $(shell echo $(CURDIR) | sed -e 's/\(^.*\/\).*$$/\1/g' -e 's/\/$$//')
include	$(CDIR)/Makefile.common
