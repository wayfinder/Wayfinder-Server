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
include	$(CDIR)/Makefile.common

LIBS += $(MC2DIR)/Shared/lib$(LIBSUFFIX)/libMC2Shared.a ReferenceImp/libReferenceImp.a

# This is file uses multiple targets, 
# instead of the normal Makefile.common setup 
# with only support for one TARGET

BITBUFFERTEST_OBJ = BitBufferTest.o
BITBUFFERTEST_TARGET = BitBufferTest
BITBUFFERTEST_CLEAN_FILES = ${BITBUFFERTEST_TARGET} ${BITBUFFERTEST_OBJ}
BITBUFFERTEST_LIBS = ${LIBS}
BITBUFFERTEST_LDFLAGS = ${LDFLAGS}

PIPETEST_OBJ = PipeTest.o
PIPETEST_TARGET = PipeTest
PIPETEST_CLEAN_FILES = ${PIPETEST_OBJ} ${PIPETEST_TARGET}
PIPETEST_LIBS = ${LIBS} -lpthread
PIPETEST_LDFLAGS = ${LDFLAGS}

CLEAN_FILES = ${BITBUFFERTEST_CLEAN_FILES} ${PIPETEST_CLEAN_FILES}
ALL_TARGETS = ${BITBUFFERTEST_TARGET} ${PIPETEST_TARGET}

all: all-subdirs all-targets

all-subdirs:
	@unset CDIR; cd ReferenceImp && ${MAKE}

all-targets: ${ALL_TARGETS}
	@echo "=== BitBufferTest ==="
	@./${BITBUFFERTEST_TARGET} 100000
	@echo "=== PipeTest ==="
	@./${PIPETEST_TARGET}

# meh....fix dependencies....
BitBufferTest.o: BitBufferTest.cpp ../include/BitBuffer.h ../include/SharedBuffer.h ReferenceImp/BitBuffer.h
	@${CXX} -c ${CXXFLAGS} BitBufferTest.cpp -o $@

${BITBUFFERTEST_TARGET}: ${BITBUFFERTEST_OBJ} ${BITBUFFERTEST_LIBS}
	@${CXX} ${BITBUFFERTEST_OBJ} ${BITBUFFERTEST_LDFLAGS} ${BITBUFFERTEST_LIBS} -o $@

PipeTest.o: PipeTest.cpp ../include/Pipe.h Thread.h
	@${CXX} -c ${CXXFLAGS} PipeTest.cpp -o $@

${PIPETEST_TARGET}: ${PIPETEST_OBJ} ${PIPETEST_LIBS}
	@${CXX} ${PIPETEST_OBJ} ${PIPETEST_LDFLAGS} ${PIPETEST_LIBS} -o $@

clean:
	@unset CDIR; cd ReferenceImp && ${MAKE} clean
	@rm -f ${CLEAN_FILES}
