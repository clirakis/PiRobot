##################################################################
#
#	Makefile for RServer using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	10-Jan-16       CBL     Original
#
######################################################################
# Machine specific stuff
#
#
TARGET = RServer
#
# Compile time resolution.
#
INCLUDE =  -I$(DRIVE)/common/utility
LIBS = -lutility -lcurses

# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp RSdisp.cpp TCPConnection.cpp tcpserver.cpp \
	Commands.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = Version.hh RSdisp.hh RSUtility.hh TCPConnection.hh \
	tcpserver.hh Commands.hh

# When we build all, what do we build?
all:      $(TARGET)

include $(DRIVE)/common/makefiles/makefile.inc


#dependencies
include make.depend 
# DO NOT DELETE
