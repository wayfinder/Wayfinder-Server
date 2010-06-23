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
#SUBDIRS=src
#include	../Makefile.common

ifdef CDIR
export CDIR := $(shell echo $(CDIR) | sed -e 's/\(^.*\/\).*$$/\1/g' -e 's/\/$$//')
else
export CDIR := $(shell echo $(CURDIR) | sed -e 's/\(^.*\/\).*$$/\1/g' -e 's/\/$$//')
endif
#MC2DIR=../../../
#CDIR=${MC2DIR}
include	${CDIR}/Makefile.common

CXXFLAGS +=-I../include
LDFLAGS += -L$(MC2DIR)/Shared/lib$(LIBSUFFIX) -L$(MC2DIR)/Server/lib$(LIBSUFFIX)

DISTURBRANCE_CHANGESET_PACKET_TARGET = DisturbanceChangesetPacketTest

all: all-targets

all-targets: compile execute

#subdir:
#	@for i in $(SUBDIRS); do echo ${i}; cd ${i}; make; done
#	echo "==== DisturbanceChangesetPacket ===="


execute:
#	@./${DISTURBRANCE_CHANGESET_PACKET_TARGET}

compile: ${DISTURBRANCE_CHANGESET_PACKET_TARGET}


DISTURBRANCE_CHANGESET_PACKET_OBJ=DisturbanceChangesetPacketTest.o
DISTURBRANCE_CHANGESET_PACKET_LDFLAGS=${LDFLAGS}
DISTURBRANCE_CHANGESET_PACKET_CXXFLAGS=${CXXFLAGS} -Iinclude -I../../../Shared/include
DISTURBRANCE_CHANGESET_PACKET_LIBS=${LIBS}

CLEAN_FILES=${DISTURBRANCE_CHANGESET_PACKET_OBJ}

DisturbanceChangesetPacketTest.o: DisturbanceChangesetPacketTest.cpp ../include/*.h ../../../Shared/include/*.h 
	${CXX} ${DISTURBRANCE_CHANGESET_PACKET_CXXFLAGS} -c DisturbanceChangesetPacketTest.cpp  -o $@

${DISTURBRANCE_CHANGESET_PACKET_TARGET}: ${DISTURBRANCE_CHANGESET_PACKET_OBJ}
	${CXX} ${DISTURBRANCE_CHANGESET_PACKET_OBJ} ${DISTURBRANCE_CHANGESET_PACKET_LDFLAGS} ${DISTURBRANCE_CHANGESET_PACKET_LIBS} -o $@


clean:
	@rm -rf ${CLEAN_FILES}








