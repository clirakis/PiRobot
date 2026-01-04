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
#include <cstdio>
#include <cmath>
#include <strings.h>

// Local Includes.
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
Commands::Commands ()
{
    fCommands = this;
    fRun      = true;
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
