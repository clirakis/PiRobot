/**
 ******************************************************************
 *
 * Module Name : serial.cpp
 *
 * Author/Date : C.B. Lirakis / 24-Dec-05
 *
 * Description :
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * 19-Jan-26    CBL    Made into class
 *
 * Classification : Unclassified
 *
 * References :
 * 
 * https://www.msweet.org/serial/serial.html
 *
 *******************************************************************
 */
// System includes.
#include <iostream>
using namespace std;
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

/// Local Includes.
#include "serial.hh"
#include "debug.h"
#include "CLogger.hh"
/**
 ******************************************************************
 *
 * Function Name :
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
RobotSerial::RobotSerial(const char *port, speed_t BaudRate, bool Blocking) :
    CObject()
{
    SET_DEBUG_STACK;
    SetName("RobotSerial");
    SetError(ENONE, __LINE__);
    if (port)
	fPortName     = port;
    else
	fPortName     = string("NONE");

    fSerialPortFd = SerialOpen(port, BaudRate, Blocking);
}

/**
 ******************************************************************
 *
 * Function Name :
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
RobotSerial::~RobotSerial(void)
{
    SET_DEBUG_STACK;
    close(fSerialPortFd);
}
/**
 ******************************************************************
 *
 * Function Name :
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
int RobotSerial::SerialOpen(const char *port, speed_t BaudRate, bool Blocking)
{
    CLogger *pLog = CLogger::GetThis();
    int rc = -1;
    struct termios newtio;


    if (port != NULL)
    {
	/*
	 * Flags for Open
	 * O_RDWR - read write open. 
	 * O_NOCTTY - not a control terminal, all control signals 
	 *            propigate fully through like CTRL-C
	 * O_NDELAY - Don't care what the stat of the DCD signal is. 
	 * 
	 */
	rc = open( port, O_RDWR | O_NOCTTY | O_NDELAY);	
	if (rc <0)
	{
	    //cerr<< strerror(errno) << endl;
	    pLog->LogTime("Serial Open Error: %s\n", strerror(errno));
	    SetError(-1, __LINE__);
	}
	else
	{
	    /* 
	     * See page 354 of Steven's Advanced UNIX Programming book. 
	     * Get existing termios properties. 
	     * 
	     * Alternative reference is:
	     * Posix Programmer's Guide - Donald A. Lewine
	     * c 1991 O'Oreily and Associates
	     * page 153  pertains  to Termial  I/O
	     */

	    /* clear struct for new port settings */
	    memset(&newtio,0, sizeof(newtio)); 
	    newtio.c_ispeed = BaudRate;
	    newtio.c_ospeed = BaudRate;
	    /*
	     * Control flag
	     *
	     * CRTSCTS : output hardware flow control (only used if 
	     * the cable has all necessary lines. See sect. 7 of Serial-HOWTO)
	     * CS8     : 8n1 (8bit,no parity,1 stopbit)
	     * CLOCAL  : local connection, no modem contol
	     * CREAD   : enable receiving characters
	     * 28-Dec-15
	     * PARENB  : enable partiy on output
	     * PARODD  : odd parity. 
	     */
	    //newtio.c_cflag = CS8 | CLOCAL | CREAD | PARENB | PARODD;
	    newtio.c_cflag = CS8 | CLOCAL | CREAD;

	    /*
	     * Input modes.
	     * IGNPAR  : ignore bytes with parity errors
	     * ICRNL   : map CR to NL (otherwise a CR input on the other 
	     *           computer will not terminate input)
	     *         otherwise make device raw (no other input processing)
	     */
	    newtio.c_iflag = IGNPAR | ICRNL | IGNBRK;
	    /*
	     * Output modes
	     * Raw output.
	     */
	    newtio.c_oflag = 0;
	    /*
	     * ICANON  : enable canonical input
	     *           disable all echo functionality, and don't 
	     *           send signals to the calling program.
	     */
	    newtio.c_lflag = ~ICANON;

	    /*
	     * Initialize all control characters
	     * default values can be found in /usr/include/termios.h, 
	     * and are given in the comments, but we don't need them here
	     */
	    newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
	    newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	    newtio.c_cc[VERASE]   = 0;     /* del */
	    newtio.c_cc[VKILL]    = 0;     /* @ */
	    newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
	    newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	    newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	    //    newtio.c_cc[VSWTC]    = 0;     /* '\0' */
	    newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
	    newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	    newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	    newtio.c_cc[VEOL]     = 0;     /* '\0' */
	    newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	    newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	    newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	    newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	    newtio.c_cc[VEOL2]    = 0;     /* '\0' */

	    /*
	     * now clean the modem line and activate the settings for the port
	     */
	    tcflush(rc, TCIFLUSH);
	    tcsetattr(rc, TCSANOW, &newtio);
	    if(Blocking)
	    {
		/* 
		 * this makes the read() function wait until it has stuff to 
		 * read before reading, Blocking Read. 
		 */
		fcntl( rc, F_SETFL, 0);
		pLog->Log("# Serial open blocking\n");
	    }
	    else
	    {
		/* return immediately. */
		fcntl( rc, F_SETFL, FNDELAY);
		pLog->Log("# Serial open non-blocking.\n");
	    }
	}
    }
    return rc;
}

/**
 ******************************************************************
 *
 * Function Name : Write
 *
 * Description : Write data to open serial port. 
 *
 * Inputs : string to write out. Should be text to communicate with
 *          the system. 
 *
 * Returns : NONE
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
size_t RobotSerial::Write(const string& value)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();

    if (fSerialPortFd>=0)
    {
	if (pLog->CheckVerbose(0))
	{
	    pLog->LogTime("Robot::Write %s", value.c_str());
	}
        // Returns number of bytes written. Not currently used. 
	return write(fSerialPortFd, value.c_str(), value.length());
    }
    return 0;
}

/**
 ******************************************************************
 *
 * Function Name : Read
 *
 * Description : Read any data on the open serial port. 
 *
 * Inputs : none
 *
 * Returns : NONE
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool RobotSerial::Read(string& value)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();
    char input[512];
    int rv;

    if (fSerialPortFd>=0)
    {
	memset(input, 0, sizeof(input));
	/*
	 * rv is the number of bytes read. 
	 * it is very odd that while the return value is specified as
	 * size_t, error is given by -1. Seems at odds with each other. 
	 * I've given rv the cast of int to deal with this. 
	 */
	rv = read(fSerialPortFd, input, sizeof(input));
	if (rv>0)
	{
	    value = input;
	    if (pLog->CheckVerbose(0))
	    {
		pLog->LogTime("Robot::Read - %d, %s", rv, value.c_str());
	    }
	    return true;
	}
	else
	{
	    value.clear();
	}
    }
    return false;
}

/**
 ******************************************************************
 *
 * Function Name :
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */

