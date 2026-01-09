/**
 ******************************************************************
 *
 * Module Name : Robot.cpp
 *
 * Author/Date : C.B. Lirakis / 02-Jan-26
 *
 * Description : Robot control entry points. 
 *
 * Restrictions/Limitations : none
 *
 * Change Descriptions : 
 *
 * Classification : Unclassified
 *
 * References : 
 *
 *
 *******************************************************************
 */  
// System includes.
#include <iostream>
using namespace std;

#include <cstring>
#include <libconfig.h++>
using namespace libconfig;
#include <unistd.h>

/// Local Includes.
#include "Robot.hh"
#include "CLogger.hh"
#include "tools.h"
#include "debug.h"
#include "RSdisp.hh"
#include "tcpserver.hh"
#include "serial.hh"


Robot* Robot::fRobot;

static pthread_t       tcp_thread;
static ThreadControl_t tcpControl;

/**
 ******************************************************************
 *
 * Function Name : Robot constructor
 *
 * Description : initialize CObject variables
 *
 * Inputs : currently none. 
 *
 * Returns : none
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
Robot::Robot(const char* ConfigFile) : CObject()
{
    CLogger *Logger = CLogger::GetThis();

    /* Store the this pointer. */
    fRobot = this;
    SetName("Robot");
    SetError(); // No error.

    fRun  = true;
    tcpControl.Port = 9999;  // default connection port.
    fSerialPort = "NONE"; 

    if(!ConfigFile)
    {
	SetError(ENO_FILE,__LINE__);
	return;
    }

    fConfigFileName = ConfigFile;
    if(!ReadConfiguration())
    {
	SetError(ECONFIG_READ_FAIL,__LINE__);
	return;
    }

    /* USER POST CONFIGURATION STUFF. */
    start_display();

    tcpControl.Run = 1;

    if( pthread_create(&tcp_thread, NULL, TCP_Server,(void *)&tcpControl) == 0)
    {
	display_message(" RX Thread successfully created.\n");
    }
    else
    {
	SET_DEBUG_STACK;
	display_message(" RX Thread failed.\n");
	Logger->Log("# RX Thread create failed. \n");
	return;
    }

    /* open up the serial port to the Raspberry Pi */
    fSerialPortFd = SerialOpen(fSerialPort.c_str(), B115200);
    if (fSerialPortFd < 0)
    {
	Logger->Log("# Error opening serial port %s\n", fSerialPort.c_str());
    }

    Logger->Log("# Robot constructed.\n");

    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Robot Destructor
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
Robot::~Robot(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();

    // Do some other stuff as well. 
    if(!WriteConfiguration())
    {
	SetError(ECONFIG_WRITE_FAIL,__LINE__);
	Logger->LogError(__FILE__,__LINE__, 'W', 
			 "Failed to write config file.\n");
    }

    /* Clean up */
    // Shutdown the TCP server
    tcpControl.Run = 0;
    TCPClose();
    if (fSerialPortFd>=0)
    {
	close(fSerialPortFd);
    }
    end_display();

    // Make sure all file streams are closed
    Logger->Log("# Robot closed.\n");
    SET_DEBUG_STACK;
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
size_t Robot::Write(const string& value)
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
bool Robot::Read(string& value)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();
    char input[256];
    size_t rv;

    if (fSerialPortFd>=0)
    {
	memset(input, 0, sizeof(input));
	// rv is the number of bytes read. 
	rv = read(fSerialPortFd, input, sizeof(input));
	if (rv>0)
	{
	    value = input;
	    if (pLog->CheckVerbose(0))
	    {
		pLog->LogTime("Robot::Read - %d, %s", rv, value.c_str());
		return true;
	    }
	    else
	    {
		value.clear();
	    }
	}
    }
    return false;
}

/**
 ******************************************************************
 *
 * Function Name : Do
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
void Robot::Do(void)
{
    SET_DEBUG_STACK;
    const struct timespec sleeptime = {0L, 200000000L};
    int rv;

    fRun = true;
    while(fRun)
    {
	/*
	 * Check to see if the user has requested
	 * special changes in the setup
	 */
	rv = checkKeys();
	if (rv>0)
	{
	    fRun = false;
	}
	nanosleep( &sleeptime, NULL);
    }
    SET_DEBUG_STACK;
}


/**
 ******************************************************************
 *
 * Function Name : ReadConfiguration
 *
 * Description : Open read the configuration file. 
 *
 * Inputs : none
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on:  
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool Robot::ReadConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();

    /*
     * Open the configuragtion file. 
     */
    try{
	pCFG->readFile(fConfigFileName.c_str());
    }
    catch( const FileIOException &fioex)
    {
	pLog->LogError(__FILE__,__LINE__,'F',
			 "I/O error while reading configuration file.\n");
	return false;
    }
    catch (const ParseException &pex)
    {
	pLog->Log("# Parse error at: %s : %d - %s\n",
		    pex.getFile(), pex.getLine(), pex.getError());
	return false;
    }

    /*
     * Start at the top. 
     */
    const Setting& root = pCFG->getRoot();

    // USER TO FILL IN
    // Output a list of all books in the inventory.
    try
    {
	int    Debug;
	/*
	 * index into group Robot
	 */
	const Setting &MM = root["Robot"];
	MM.lookupValue("Debug",     Debug);
	MM.lookupValue("Port",      tcpControl.Port);
	MM.lookupValue("SerialPort", fSerialPort);
	SetDebug(Debug); 
	pLog->SetVerbose(Debug);
	pLog->Log("# Debug value set to: %d\n", Debug);
	pLog->Log("# Port value set to: %d\n",  tcpControl.Port);
    }
    catch(const SettingNotFoundException &nfex)
    {
	// Ignore.
    }

    delete pCFG;
    pCFG = 0;
    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : WriteConfigurationFile
 *
 * Description : Write out final configuration
 *
 * Inputs : none
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on:  
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool Robot::WriteConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();

    Setting &root = pCFG->getRoot();

    // USER TO FILL IN
    // Add some settings to the configuration.
    Setting &MM = root.add("Robot", Setting::TypeGroup);
    MM.add("Debug",     Setting::TypeInt) = (int) pLog->GetVerbose();
    MM.add("Port",      Setting::TypeInt) = tcpControl.Port;
    MM.add("SerialPort", Setting::TypeString) = fSerialPort;

    // Write out the new configuration.
    try
    {
	pCFG->writeFile(fConfigFileName.c_str());
	pLog->Log("# New configuration successfully written to: %s\n",
		  fConfigFileName.c_str());

    }
    catch(const FileIOException &fioex)
    {
	pLog->Log("# I/O error while writing file: %s \n",
		  fConfigFileName.c_str());
	delete pCFG;
	return(false);
    }
    delete pCFG;

    SET_DEBUG_STACK;
    return true;
}
void Robot::Stop(void) 
{
    fRun=false;
    tcpControl.Run = 0;
}

