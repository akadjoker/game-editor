#****************************************************************************
#
# Makefile for Kyra.
# Lee Thomason and David Leimbach
# www.grinninglizard.com
# www.sourceforge.net/projects/kyra
#
# This is a GNU make (gmake) makefile
#****************************************************************************

# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          := NO

# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO

#****************************************************************************
ifeq (RELEASE, ${MAKE_KYRA})
	DEBUG   := NO
	PROFILE := NO
endif

ifeq (DEBUG, ${MAKE_KYRA})
	DEBUG   := YES
	PROFILE := NO
endif

ifeq (PROFILE, ${MAKE_KYRA})
	DEBUG   := NO
	PROFILE := YES
endif

CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib

DEBUG_CFLAGS     := -Wall -Wno-unknown-pragmas -Wno-format -g -DDEBUG
RELEASE_CFLAGS   := -Wall -Wno-unknown-pragmas -Wno-format

LIBS		 :=   $(shell sdl-config --libs) ../engine/libkyra.a

DEBUG_CXXFLAGS   := ${DEBUG_CFLAGS}
RELEASE_CXXFLAGS := ${RELEASE_CFLAGS}

DEBUG_LDFLAGS    := -g
RELEASE_LDFLAGS  :=

ifeq (YES, ${DEBUG})
   CFLAGS       := ${DEBUG_CFLAGS}
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CFLAGS       := ${RELEASE_CFLAGS}
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

ifeq (YES, ${PROFILE})
   CFLAGS   := ${CFLAGS} -pg
   CXXFLAGS := ${CXXFLAGS} -pg
   LDFLAGS  := ${LDFLAGS} -pg
else
   ifeq (NO, ${DEBUG})
     CFLAGS   := ${CFLAGS} -O3
     CXXFLAGS := ${CXXFLAGS} -O3
   endif
endif

#****************************************************************************
# Preprocessor directives
#****************************************************************************

ifeq (YES, ${PROFILE})
  DEFS :=
else
  DEFS :=
endif

#****************************************************************************
# Include paths
#****************************************************************************

INCS := $(shell sdl-config --cflags)


#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************

OUTPUT := tutorial1

all: ${OUTPUT}


#****************************************************************************
# Source files
#****************************************************************************

SRCS :=	code_d.cpp

# Add on the sources for libraries
SRCS := ${SRCS}

OBJS := $(addsuffix .o,$(basename ${SRCS}))

#****************************************************************************
# Output
#****************************************************************************

${OUTPUT}: ${OBJS} ../engine/libkyra.a
	${LD} -o $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}

#****************************************************************************
# common rules
#****************************************************************************

# Rules for compiling source files to object files
%.o : %.cpp
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

%.o : %.c
	${CC} -c ${CFLAGS} ${INCS} $< -o $@

clean:
	-rm -f core ${OBJS} ${OUTPUT}

#depend:
#	makedepend ${INCS} ${SRCS}

# Dependencies. Badly over included, but there were just too many problems
# using makedepend and sharing the makefiles.

include ../makefiledefines
${OBJS} :  $(addprefix ../engine/, ${ENGINE_HEADERS}) $(addprefix ../util/, ${UTIL_HEADERS}) $(addprefix ../gui/, ${GUI_HEADERS}) \
           tutorial1.h

