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
NEEDSSL := yes
NEEDXML := yes

ifdef CDIR
export CDIR := $(shell echo $(CDIR) | sed -e 's/\(^.*\/\).*$$/\1/g' -e 's/\/$$//')
else
export CDIR := $(shell echo $(CURDIR) | sed -e 's/\(^.*\/\).*$$/\1/g' -e 's/\/$$//')
endif
include	$(CDIR)/Makefile.common
LDFLAGS += -L$(MC2DIR)/Shared/lib$(LIBSUFFIX) -L$(MC2DIR)/Server/lib$(LIBSUFFIX)
LIBS += -lMC2Shared -lMC2ServersShared -lMC2ServerModules

# This is file uses multiple targets, 
# instead of the normal Makefile.common setup 
# with only support for one TARGET

# This file needs to be improved when it comes to deps and *.o files.

EVENTTEST_OBJ = EventTest.o
EVENTTEST_TARGET = EventTest
EVENTTEST_CLEAN_FILES = ${EVENTTEST_TARGET} ${EVENTTEST_OBJ}
EVENTTEST_LIBS = ../src/.objects$(LIBSUFFIX)/Event*.o \
	../src/.objects$(LIBSUFFIX)/TalkerUtility.o \
	../src/.objects$(LIBSUFFIX)/StringTalker*.o \
	../src/.objects$(LIBSUFFIX)/Time.o \
	../src/.objects$(LIBSUFFIX)/ExtServiceTalker.o \
        ../src/.objects$(LIBSUFFIX)/WC*.o \
	 ${LIBS}

EVENTTEST_LDFLAGS = ${LDFLAGS}
EVENTTEST_CXXFLGS = -I../include

TALKERTEST_OBJ = TalkerTest.o
TALKERTEST_TARGET = TalkerTest
TALKER_CLEAN_FILES = ${TALKERTEST_TARGET} ${TALKERTEST_OBJ}
TALKERTEST_LIBS =  \
	../src/.objects$(LIBSUFFIX)/TalkerUtility.o \
	../src/.objects$(LIBSUFFIX)/StringTalker*.o \
	../src/.objects$(LIBSUFFIX)/ExtServiceTalker.o \
        ../src/.objects$(LIBSUFFIX)/Yellow*.o \
        ../../../Shared/src/.objects$(LIBSUFFIX)/SearchReplyData.o \
	 ${LIBS}

TALKERTEST_LDFLAGS = ${LDFLAGS}
TALKERTEST_CXXFLGS = -I../include

CLEAN_FILES = ${EVENTTEST_CLEAN_FILES} ${TALKERTEST_CLEAN_FILES}
ALL_TARGETS = ${EVENTTEST_TARGET} ${TALKERTEST_TARGET}
#Enable when memory leak check is needed
#PREPROG=valgrind --leak-check=yes
PREPROG=

all: all-targets

all-targets: ${ALL_TARGETS}
	@echo "=== EventTest ==="
	@${PREPROG} ./${EVENTTEST_TARGET}
#	@echo "=== TalkerTest ==="
#	@${PREPROG} ./${TALKERTEST_TARGET}

# meh....fix dependencies....
EventTest.o: EventTest.cpp ../include/*.h
	${CXX} -c ${CXXFLAGS} ${EVENTTEST_CXXFLAGS} EventTest.cpp -o $@

TalkerTest.o: TalkerTest.cpp ../include/*.h
	${CXX} -c ${CXXFLAGS} ${TALKERTEST_CXXFLAGS} TalkerTest.cpp -o $@

${EVENTTEST_TARGET}: ${EVENTTEST_OBJ} ${EVENTTEST_LIBS}
	@${CXX} ${EVENTTEST_LIBS} ${EVENTTEST_OBJ} ${EVENTTEST_LDFLAGS}  -o $@

${TALKERTEST_TARGET}: ${TALKERTEST_OBJ} ${TALKERTEST_LIBS}
	@${CXX} ${TALKERTEST_LIBS} ${TALKERTEST_OBJ} ${TALKERTEST_LDFLAGS}  -o $@

clean:
	@rm -f ${CLEAN_FILES}

depclean:
	@echo No depclean in ExtServiceModule Tests
