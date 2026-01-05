/********************************************************************
 *
 * Module Name : Commands.cpp
 *
 * Author/Date : C.B. Lirakis / 11-Feb-16
 *
 * Description : Generic module
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 ********************************************************************/
// System includes.

#include <iostream>
using namespace std;
#include <cstring>

// Local Includes.
#include "debug.h"
#include "Commands.hh"
#include "RSdisp.hh"
#include "Version.hh"
#include "TCPConnection.hh"

Commands* Commands::fCommands;

// defined in main.cpp
void Terminate(int);

/**
 ******************************************************************
 *
 * Function Name : Commands constructor
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
Commands::Commands (TCPConnection *rx)
{
    SET_DEBUG_STACK;
    fCommands = this;
    fRun      = true;
    fRx       = rx;
}

/**
 ******************************************************************
 *
 * Function Name : Commands destructor
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
Commands::~Commands ()
{
}


/**
 ******************************************************************
 *
 * Function Name : Commands function
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
bool Commands::Parse(const char *line)
{
    SET_DEBUG_STACK;
    bool rc = false; 
    display_message("PARSE %s\n", line);
    /*
     * Precursors for determining what the command is.
     * $ - NMEA string in. 
     * % - Program control
     * @ - Robot control
     */
    switch(line[0])
    {
    case '$':
	// parse a NMEA string.
	break;
    case '%':
	// Parse a program string.
	rc = ProgramControl(&line[1]);  
	break;
    case '@':
	// Parse a control line for the robot. 
	break;
    case '?':
	// Help dialog
	Help();
	break;
    }
    return rc;
}
/**
 ******************************************************************
 *
 * Function Name : Commands function
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
bool Commands::ProgramControl(const char *line)
{
    bool rc = false;
    TCPConnection *c = TCPConnection::Get();
    char msg[64];

    if (!strncasecmp( line, "QUIT", 4))
    {
	fRun = false;
	rc   = true; // end the program.
	Terminate(0);
    }
    else if(!strncasecmp( line, "VERSION", 7))
    {
	sprintf(msg,"Version: %d.%d\n", MAJOR_VERSION, MINOR_VERSION);
	c->Write(msg, strlen(msg));
    }
    return rc;
}
/**
 ******************************************************************
 *
 * Function Name : Commands function
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
void Commands::Help(void)
{
    SET_DEBUG_STACK;
    char text[128];

    snprintf( text, sizeof(text), "HELP, available commands:\n");
    fRx->Write(text, strlen(text));

    snprintf( text, sizeof(text), "$ - Parse a NMEA string - INCOMPLETE\n");
    fRx->Write(text, strlen(text));

    snprintf( text, sizeof(text), "%% - Parse a program string\n");
    fRx->Write(text, strlen(text));

    snprintf( text, sizeof(text), "%%QUIT\n");
    fRx->Write(text, strlen(text));

    snprintf( text, sizeof(text), "%%VERSION\n");
    fRx->Write(text, strlen(text));


    snprintf( text, sizeof(text), "@ - Robot control line, pass on to Arduino\n");
    fRx->Write(text, strlen(text));

    snprintf( text, sizeof(text), "\n");
    fRx->Write(text, strlen(text));

}
