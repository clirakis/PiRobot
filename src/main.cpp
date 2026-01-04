/**
 ******************************************************************
 *
 * Module Name : 
 *
 * Author/Date : C.B. Lirakis / 20-Jan-14
 *
 * Description :
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 *******************************************************************
 */
// System includes.
#include <iostream>
using namespace std;
#include <cstring>
#include <cmath>
#include <csignal>
#include <unistd.h>
#include <time.h>
#include <fstream>

/// Local Includes.
#include "debug.h"
#include "Version.hh"
#include "RSdisp.hh"
#include "tcpserver.hh"

static bool      verbose;
static ofstream* logFile;
static pthread_t       tcp_thread;
static ThreadControl_t tcpControl;

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
static void UserSignal(int sig)
{
    switch (sig)
    {
    case SIGUSR1:
    case SIGUSR2:
	*logFile << "# SIGUSR: " << sig << endl;
	tcpControl.Run = 0;
	break;
    }
}


/**
******************************************************************
*
* Function Name : Terminate
*
* Description : Deal with errors in a clean way!
*               ALL, and I mean ALL exits are brought 
*               through here!
*
* Inputs : Signal causing termination. 
*
* Returns : none
*
* Error Conditions : Well, we got an error to get here. 
*
*******************************************************************
*/ 
void Terminate (int sig) 
{
    static int i=0;
    char msg[128], tmp[64];
    time_t now;
    time(&now);
 
    i++;
    if (i>1) 
    {
        _exit(-1);
    }

    *logFile << "# Program Ends: " << ctime(&now);

    switch (sig)
    {
    case -1: 
      sprintf( msg, "User abnormal termination");
      break;
    case 0:                    // Normal termination
        sprintf( msg, "Normal program termination.");
        break;
    case SIGHUP:
        sprintf( msg, " Hangup");
        break;
    case SIGINT:               // CTRL+C signal 
        sprintf( msg, " SIGINT ");
        break;
    case SIGQUIT:               //QUIT 
        sprintf( msg, " SIGQUIT ");
        break;
    case SIGILL:               // Illegal instruction 
        sprintf( msg, " SIGILL ");
        break;
    case SIGABRT:              // Abnormal termination 
        sprintf( msg, " SIGABRT ");
        break;
    case SIGBUS:               //Bus Error! 
        sprintf( msg, " SIGBUS ");
        break;
    case SIGFPE:               // Floating-point error 
        sprintf( msg, " SIGFPE ");
        break;
    case SIGKILL:               // Kill!!!! 
        sprintf( msg, " SIGKILL");
        break;
    case SIGSEGV:              // Illegal storage access 
        sprintf( msg, " SIGSEGV ");
        break;
    case SIGTERM:              // Termination request 
        sprintf( msg, " SIGTERM ");
        break;
    case SIGSTKFLT:               // Stack fault
        sprintf( msg, " SIGSTKFLT ");
        break;
    case SIGTSTP:               // 
        sprintf( msg, " SIGTSTP");
        break;
    case SIGXCPU:               // 
        sprintf( msg, " SIGXCPU");
        break;
    case SIGXFSZ:               // 
        sprintf( msg, " SIGXFSZ");
        break;
    case SIGSTOP:               // 
        sprintf( msg, " SIGSTOP ");
        break;
    case SIGPWR:               // 
        sprintf( msg, " SIGPWR ");
        break;
    case SIGSYS:               // 
        sprintf( msg, " SIGSYS ");
        break;
    default:
        sprintf( msg, " Uknown signal type: %d", sig);
        break;
    }
    if (sig!=0)
    {
        sprintf ( tmp, " %s %d", LastFile, LastLine);
        strncat ( msg, tmp, sizeof(msg));
        //syslog  ( LOG_ERR, msg);
        *logFile << msg << endl;
    }

    /// User termination here
    logFile->close();
    delete logFile;

    TCPClose();
    end_display();

    if (sig == 0)
    {
        _exit (0);
    }
    else
    {
        _exit (-1);
    }
}

/**
 ******************************************************************
 *
 * Function Name : Help
 *
 * Description : provides user with help if needed.
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 *******************************************************************
 */
static void Help(void)
{
    SET_DEBUG_STACK;
    cout << "********************************************" << endl;
    cout << "* Test file for text Logging.              *" << endl;
    cout << "* Built on "<< __DATE__ << " " << __TIME__ << "*" << endl;
    cout << "* Available options are :                  *" << endl;
    cout << "*                                          *" << endl;
    cout << "********************************************" << endl;
}
/**
 ******************************************************************
 *
 * Function Name :  ProcessCommandLineArgs
 *
 * Description : Loop over all command line arguments
 *               and parse them into useful data.
 *
 * Inputs : command line arguments. 
 *
 * Returns : none
 *
 * Error Conditions : none
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
static void
ProcessCommandLineArgs(int argc, char **argv)
{
    int option;
    SET_DEBUG_STACK;
    do
    {
        option = getopt( argc, argv, "f:hHnv");
        switch(option)
        {
        case 'f':
            break;
        case 'h':
        case 'H':
            Help();
        Terminate(0);
        break;
        case 'v':
            verbose = true;
            break;
        }
    } while(option != -1);
}
/**
 ******************************************************************
 *
 * Function Name : Initialize
 *
 * Description : Initialze the process
 *               - Setup traceback utility
 *               - Connect all signals to route through the terminate 
 *                 method
 *               - Perform any user initialization
 *
 * Inputs : none
 *
 * Returns : true on success. 
 *
 * Error Conditions : depends mostly on user code
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
static bool Initialize(void)
{
    SET_DEBUG_STACK;
    time_t now;
    char   msg[128];
    time(&now);
    strftime (msg, sizeof(msg), "%F %T", gmtime(&now));

    signal (SIGHUP , Terminate);   // Hangup.
    signal (SIGINT , Terminate);   // CTRL+C signal 
    signal (SIGKILL, Terminate);   // 
    signal (SIGQUIT, Terminate);   // 
    signal (SIGILL , Terminate);   // Illegal instruction 
    signal (SIGABRT, Terminate);   // Abnormal termination 
    signal (SIGIOT , Terminate);   // 
    signal (SIGBUS , Terminate);   // 
    signal (SIGFPE , Terminate);   // 
    signal (SIGSEGV, Terminate);   // Illegal storage access 
    signal (SIGTERM, Terminate);   // Termination request 
    signal (SIGSTKFLT, Terminate); // 
    signal (SIGSTOP, Terminate);   // 
    signal (SIGPWR, Terminate);    // 
    signal (SIGSYS, Terminate);    // 
    signal (SIGUSR1, UserSignal);  //
    signal (SIGUSR2, UserSignal);  // 
    // User initialization goes here. 
    logFile = new ofstream();
    *logFile << "# Program Begins: " << msg << endl
	     << "# Compiled on: " << __DATE__ << " " __TIME__ << endl
	     << "# Version: " 
	     << MAJOR_VERSION << "." << MINOR_VERSION
	     << endl;

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
	return 0;
    }

    return true;
}

/**
 ******************************************************************
 *
 * Function Name : main
 *
 * Description : It all starts here:
 *               - Process any command line arguments
 *               - Do any necessary initialization as a result of that
 *               - Do the operations
 *               - Terminate and cleanup
 *
 * Inputs : command line arguments
 *
 * Returns : exit code
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
int main(int argc, char **argv)
{
    int rv;
    //const struct timespec sleeptime = {0L, 200000000L};

    ProcessCommandLineArgs(argc, argv);
    if (Initialize())
    {
	while (tcpControl.Run > 0)
	{
	    /*
	     * Check to see if the user has requested
	     * special changes in the setup
	     */
	    rv = checkKeys();
	    if (rv>0)
	    {
		tcpControl.Run = 0;
	    }
	    //nanosleep( &sleeptime, NULL);
	}
    }
    Terminate(0);
}
