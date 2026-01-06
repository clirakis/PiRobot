##################################################################
#
#	Makefile for RServer using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	10-Jan-16       CBL     Original
#	03-Jan-26	CBL	Well, long time but resuming the project
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
LIBS = -lutility -lcurses -lconfig++

# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp Robot.cpp RSdisp.cpp TCPConnection.cpp tcpserver.cpp \
	Commands.cpp UserSignals.cpp serial.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = Version.hh Robot.hh RSdisp.hh RSUtility.hh TCPConnection.hh \
	tcpserver.hh Commands.hh UserSignals.hh serial.hh

# When we build all, what do we build?
all:      $(TARGET)

include $(DRIVE)/common/makefiles/makefile.inc

