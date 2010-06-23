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
ifdef CDIR
export CDIR := $(shell echo $(CDIR) | sed -e 's/\(^.*\/\).*$$/\1/g' -e 's/\/$$//')
else
export CDIR := $(shell echo $(CURDIR) | sed -e 's/\(^.*\/\).*$$/\1/g' -e 's/\/$$//')
endif
include	${CDIR}/Makefile.common

OBJSUFFIX=o
CXXFLAGS +=-I../include
LDFLAGS += -L$(MC2DIR)/Shared/lib$(LIBSUFFIX) -L$(MC2DIR)/Server/lib$(LIBSUFFIX)

TRAFFIC_HANDLER_TEST_TARGET = TrafficHandlerTest

all: all-targets

all-targets: compile execute

execute:
#	@./${TRAFFIC_HANDLER_TEST_TARGET}

compile: ${TRAFFIC_HANDLER_TEST_TARGET}


TRAFFIC_HANDLER_TEST_OBJ=TrafficHandlerTest.${OBJSUFFIX} ../src/.obj*/TrafficHandler.${OBJSUFFIX}
TRAFFIC_HANDLER_TEST_LDFLAGS=${LDFLAGS}
TRAFFIC_HANDLER_TEST_CXXFLAGS=${CXXFLAGS} -I../include -I../../../../../Shared/include
TRAFFIC_HANDLER_TEST_LIBS=${LIBS}

CLEAN_FILES=${TRAFFIC_HANDLER_TEST_OBJ}

DisturbanceChangesetPacketTest.${OBJSUFFIX}: TrafficHandlerTest.cpp ../include/*.h
	${CXX} ${TRAFFIC_HANDLER_TEST_CXXFLAGS} -c TrafficHandlerTest.cpp  -o $@

${TRAFFIC_HANDLER_TEST_TARGET}: ${TRAFFIC_HANDLER_TEST_OBJ}
	${CXX} ${TRAFFIC_HANDLER_TEST_OBJ} ${TRAFFIC_HANDLER_TEST_LDFLAGS} ${TRAFFIC_HANDLER_TEST_LIBS} -o $@


clean:
	@rm -rf ${CLEAN_FILES}

