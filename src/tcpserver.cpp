/**
 ******************************************************************
 *
 * Module Name : tcpserver.cpp
 *
 * Author/Date : C.B. Lirakis / 09-May-09
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
 *
 *******************************************************************
 */
/* System includes. */
#include <iostream>
using namespace std;
// #include <semaphore.h>
// #include <sys/resource.h>
// #include <fstream>
// #include <string>

#include <csignal>
#include <arpa/inet.h>      /* inet_ntoa() to format IP address */
#include <cstring>         /* bzero */
#include <fcntl.h>

/** Local Includes. */
#include "debug.h"
#include "tcpserver.hh"
#include "TCPConnection.hh"
#include "RSdisp.hh"
#include "Commands.hh"
#include "CLogger.hh"

/** Global Variables. */

#define MAXLINE   256
#define SERV_PORT 9879   /* TCP and UDP client-servers */
#define LISTENQ   1024

static int  Verbose        = -1;
static int  listenfd;
static Commands *cmd;

const int MAX_CONNECTIONS = 4;
static bool Connections[MAX_CONNECTIONS];

/**
 ******************************************************************
 *
 * Function Name : Add 
 *
 * Description : keep track of the connections 
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
static int Add(void)
{
    SET_DEBUG_STACK;
    int i = 0;
    while (Connections[i] && (i<MAX_CONNECTIONS)) i++;
    if (i<MAX_CONNECTIONS)
    {
	Connections[i] = true;
	return i;
    }
    return -1;
}
/**
 ******************************************************************
 *
 * Function Name : Delete
 *
 * Description : Mark the connection indicated as not connected. 
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
static void Delete(int i)
{
    SET_DEBUG_STACK;
    Connections[i] = false;
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
void SetVerbose(unsigned char Level)
{
    SET_DEBUG_STACK;
    Verbose = Level;
}
/**
 ******************************************************************
 *
 * Function Name : UserSignal
 *
 * Description : Register SIGPIPE and the action to deal with it. 
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
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();

    switch (sig)
    {
    case SIGPIPE:
	pLog->LogTime("# SIGUSR: %d\n",sig);
	//tcpControl.Run = 0;
	display_message("SIGPIPE.");
	break;
    }
}

/**
 ******************************************************************
 *
 * Function Name : ConnectionThread
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
void* ConnectionThread(void* arg)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();

    char     line[256];
    long     rc;
    int      rv       = 0;
    time_t   now;
    TCPConnection *Rx = (TCPConnection*) arg;

    //struct timespec timeout;
    /* Seconds and microseconds timeout at 100ms */
    struct timespec sleeptime = { 0, 500000000};

    if (pLog->CheckVerbose(0))
    {
        pLog->LogTime("# Thread %d %d\n", Rx->Connection(), Rx->GetListen());
    }
    signal (SIGPIPE, UserSignal); 

    cmd = new Commands();

    while( Rx->Run())
    {
	display_connection( Rx->Number(), Rx->Address(), Rx->Purpose());

        memset( line, 0, sizeof(line));
        // Any inbound data?
        rc = Rx->Read(line, sizeof(line));
        if (rc > 0)
        {
            if(cmd->Parse(line))
	    {
		Rx->Stop();
	    }
        }
        else
        {
            if (Rx->Error() == TCPConnection::NO_ERROR)
            {
                nanosleep(&sleeptime, NULL);
            }
            else
            {
                Rx->Stop();
            }
        }

        rv = 0;
        time(&now);
        memset(line, 0, sizeof(line));
        ctime_r( &now, line);
        if (rv == 0)
        {
            rc = Rx->Write(line, strlen(line));
            if (Verbose>1)
            {
                cout << __FILE__<< " " 
                     << __LINE__ << " " 
                     << line 
                     << " " << rc << endl;
            }
            if (rc < 0)
            {
		display_message("Connection closed");
		Rx->Close();
                Rx->Stop();
            }
            nanosleep( &sleeptime, NULL);
        }
        else
        {
            switch(errno)
            {
            case EINTR:
            case EAGAIN:
            case ESPIPE:
            case ETIMEDOUT:
                nanosleep(&sleeptime, NULL);
                break;
            default:
                perror("trywait");
                cout << errno << endl;
                nanosleep(&sleeptime, NULL);
                break;
            }
        }
    } // End while Rx->Run is true loop.

    Rx->Done();
    display_message("Receive thread exits.");
    display_connection( Rx->Number(), Rx->Address(), Rx->Purpose());

    SET_DEBUG_STACK;
    Delete(Rx->Number());
    return 0;
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
void* TCP_Server(void *val)
{
    SET_DEBUG_STACK;
    const int              Port = 9999;
    int			   listenfd, connfd, rv;
    socklen_t	           clilen;
    struct sockaddr_in	   cliaddr, servaddr;
    pthread_t              rx_thread;
    TCPConnection          *Control;
    struct timespec        sleeptime = { 0, 500000000};
    struct ThreadControl_t *TC = (struct ThreadControl_t *) val;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    rv = fcntl(listenfd, F_SETFL, O_NONBLOCK);	
    if (rv == -1)
    {
        cerr << "Error setting non-block on listen socket." << endl;
    }


    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //servaddr.sin_port        = htons(SERV_PORT);
    servaddr.sin_port        = htons(Port);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    if (Verbose > 0)
    {
        cout << "Waiting for connections on port: "
             << Port << endl;
    }
    while(TC->Run)
    {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd < 0)
        {
            switch (errno)
            {
            case EAGAIN:
                nanosleep(&sleeptime, NULL);
                break;
            default:
                perror("Accept");
                break;
            }
        }
        else
        {
            if (Verbose>0)
            {
                cout << "Accept: " <<  __FILE__
                     << " " <<  __LINE__
                     << " " << connfd
                     << " " << listenfd 
                     << endl;
            }
	    display_message("Connected to: %s\n", inet_ntoa(cliaddr.sin_addr));
            Control = new TCPConnection(connfd, listenfd, inet_ntoa(cliaddr.sin_addr));
	    rv = Add();
	    if (rv>=0)
	    {
		Control->SetNumber(rv);
		if (Verbose>0)
		{
		    cout << "Spawning thread: " 
			 << __FILE__
			 << " " <<  __LINE__
			 << " " << Control->Connection()
			 << " " << Control->GetListen()
			 << endl;
		}
		rv = pthread_create( &rx_thread, NULL, ConnectionThread,
				     (void *)Control);
		if( rv == 0)
		{
		    if (Verbose > 0)
		    {
			cout << "RX Thread successfully created." 
			     << " " <<  __FILE__ 
			     << " " <<  __LINE__ << endl;
		    }
		}
		else
		{
		    SET_DEBUG_STACK;
		}
	    }
	    else 
	    {
		delete Control;
		display_message("Connection count exceeded.");
	    }
        }
    }
    return 0;
}
void TCPClose(void)
{
    delete cmd;
    close(listenfd);
}
