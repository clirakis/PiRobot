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
#include <string>
#include <sstream>
#include <vector> 

#include <csignal>
#include <arpa/inet.h>      /* inet_ntoa() to format IP address */
#include <cstring>          /* bzero */
#include <fcntl.h>

/** Local Includes. */
#include "debug.h"
#include "tcpserver.hh"
#include "TCPConnection.hh"
#include "RSdisp.hh"
#include "Commands.hh"
#include "CLogger.hh"
#include "Robot.hh"

/** Global Variables. */

#define MAXLINE   256
#define SERV_PORT 9879   /* TCP and UDP client-servers */
#define LISTENQ   1024

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
    Robot   *pRobot = Robot::GetThis();

    switch (sig)
    {
    case SIGPIPE:
	pLog->LogTime("# SIGUSR: %d\n",sig);
	//tcpControl.Run = 0;
	if (pRobot->DisplayOn())
	{
	    display_message("SIGPIPE.");
	}
	break;
    }
}
/**
 ******************************************************************
 *
 * Function Name : DisplayMessages
 *
 * Description : Break up the inbound serial messages into
 *               chunks to display on the screen if active. 
 *
 * Inputs : std::string to parse
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
static void DisplayMessages(const string &inbound)
{
    SET_DEBUG_STACK;
    istringstream sstream(inbound);
    string token;
    vector<string> tokens;

    while(getline(sstream, token, '\n'))
    {
	tokens.push_back(token);
    }
    for(const string& token : tokens)
    {
	display_message("GPS: %s\n", token.c_str());
    }
}
/**
 ******************************************************************
 *
 * Function Name : TCPWrite
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

/**
 ******************************************************************
 *
 * Function Name : SendHeartBeat
 *
 * Description : send a time tag as a heartbeat to know the 
 *               connection is open and the times are reasonable. 
 *               allows to check for latency too. 
 *
 * Inputs : TCPConnection parameters
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
static bool WriteTCP(TCPConnection *Rx, const char *line)
{
    SET_DEBUG_STACK;
    const struct timespec sleeptime = { 0, 100000000};
    CLogger   *pLog = CLogger::GetThis();
    Robot     *pR   = Robot::GetThis();
    bool      rv = true;
    long      rc;

    if (rv)
    {
	// Send a heartbeat to the originating connection. 
	rc = Rx->Write(line, strlen(line));
	if (pLog->CheckVerbose(1))
	{
	    pLog->LogTime("Write rc %s %d %s %d \n",
			  __FILE__, __LINE__, line, rc);
	}
	
	if (rc < 0)
	{
	    if (pR->DisplayOn())
	    {
		display_message("Connection closed");
	    }
	    pLog->LogTime(" Connection Closed!\n");
	    Rx->Close();
	    Rx->Stop();
	    rv = false;
        }
        else
        {
	    if (pLog->CheckVerbose(1))
	    {
		pLog->Log("# Connection thread, errno: %s\n", strerror(errno));
	    }
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
		pLog->LogTime(" TCP connection error: %s\n", strerror(errno));
                nanosleep(&sleeptime, NULL);
                break;
            }
        }
    }
    return rv;
}
/**
 ******************************************************************
 *
 * Function Name : SendHeartBeat
 *
 * Description : send a time tag as a heartbeat to know the 
 *               connection is open and the times are reasonable. 
 *               allows to check for latency too. 
 *
 * Inputs : TCPConnection parameters
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
static bool SendHeartBeat(TCPConnection *Rx)
{
    SET_DEBUG_STACK;
    time_t    now;
    struct    tm *tmnow;
    char      line[128],tmsg[64];

    time(&now);
    memset(tmsg, 0, sizeof(tmsg));
    tmnow = localtime(&now);
    strftime(line, sizeof(line), "H: %F %T\n", tmnow); 
    return WriteTCP(Rx, line);
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
    const   uint32_t  nanosec = 200000000;

    TCPConnection *Rx    = (TCPConnection*) arg;
    CLogger       *pLog  = CLogger::GetThis();
    Robot         *pR    = Robot::GetThis();
    int           rc;
    string        inbound;
    char          line[512];
    time_t        now, prev;

    /* 
     * Seconds and microseconds timeout 
     * Only send heartbeat every second. 
     */
    const struct timespec sleeptime = { 0, nanosec};

    pLog->LogTime("Connection Thread Starts, %d %d\n", 
		  Rx->Connection(), Rx->GetListen());

    // register signal to end connection when it drops. 
    signal (SIGPIPE, UserSignal); 

    // Setup command structure .
    cmd = new Commands(Rx);

    // count seconds between loops. 
    time(&now);
    prev = now; 
    // Loop until a quit occurs. 
    while( Rx->Run())
    {
	if (pR->DisplayOn())
	{
	    display_connection( Rx->Number(), Rx->Address(), Rx->Purpose());
	}
        /* 
	 *-------------------------------------------
	 * INBOUND TCP TRAFFIC 
	 * -------------------------------------------
	 */
        memset( line, 0, sizeof(line));
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

	/* 
	 * -----------------------------------------------------------
	 *  OUTBOUND TCP data ----------------------------------------
	 * -----------------------------------------------------------
	 */
	if ((now-prev)>0)
	{
	    SendHeartBeat(Rx);
	    prev = now;
	}

	/*
	 * Any inbound traffic from the GPS
	 */
	if (pR->Read(inbound))
	{
	    if (pR->DisplayOn())
	    {
		DisplayMessages(inbound);
	    }
	    WriteTCP(Rx, inbound.c_str());
	    if (pLog->CheckVerbose(1))
	    {
		pLog->Log("# Inbound: %s\n", inbound.c_str());
	    }
	}
	/* Arduino inbound */

	// FIXME

	nanosleep( &sleeptime, NULL);
	time(&now);
    } // End while Rx->Run is true loop.

    Rx->Done();
    if (pR->DisplayOn())
    {
	display_message("TCP Receive thread exits.");
	display_connection( Rx->Number(), Rx->Address(), Rx->Purpose());
    }
    pLog->LogTime("TCP Receive thread exits.\n");
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
    CLogger *pLog = CLogger::GetThis();
    Robot   *pRobot = Robot::GetThis();

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
	pLog->LogTime("# Error setting non-block on listen socket.\n");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(TC->Port);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    pLog->LogTime("Waiting for connections on port: %d \n", TC->Port);

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
            if (pLog->CheckVerbose(0))
            {
		pLog->LogTime("# Accept: %s %d connfd: %d listenfd: %d \n",
			      __FILE__, __LINE__, connfd, listenfd);
		pLog->Log("# client: %s\n", inet_ntoa(cliaddr.sin_addr));
            }
	    if (pRobot->DisplayOn())
	    {
		display_message("Connected to: %s\n", 
				inet_ntoa(cliaddr.sin_addr));
	    }
            Control = new TCPConnection(connfd, listenfd, inet_ntoa(cliaddr.sin_addr));
	    rv = Add();
	    if (rv>=0)
	    {
		Control->SetNumber(rv);
		if (pLog->CheckVerbose(0))
		{
		    pLog->LogTime("Spawning thread: %s %d Connection: %d, Listen: %d\n", 
				  __FILE__, __LINE__, Control->Connection(),
				  Control->GetListen());
		}
		rv = pthread_create( &rx_thread, NULL, ConnectionThread,
				     (void *)Control);
		if( rv == 0)
		{
		    if (pLog->CheckVerbose(0))
		    {
			pLog->LogTime("RX Thread successfully created.\n"); 
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
		if (pRobot->DisplayOn())
		{
		    display_message("Connection count exceeded.");
		}
		pLog->LogTime("# ERROR - Connection count exceeded.\n");
	    }
        }
    }
    pLog->LogTime("TCP_Server thread exits.\n");
    return 0;
}
void TCPClose(void)
{
    delete cmd;
    close(listenfd);
}
