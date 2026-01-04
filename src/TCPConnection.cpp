/**
 ******************************************************************
 *
 * Module Name : ThreadControl.cpp
 *
 * Author/Date : C.B. Lirakis / 06-Jul-10
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
#include <semaphore.h>
#include <sys/resource.h>
#include <fstream>
#include <string>

#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h> 
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>          /* hostent struct, gethostbyname() */
#include <arpa/inet.h>      /* inet_ntoa() to format IP address */
#include <netinet/in.h>     /* in_addr structure */
#include <string.h>         /* bzero */
#include <errno.h>          /* error processing. */
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>       /* waitpid */
#include <time.h>
#include <pthread.h>
#include <limits.h>

/** Local Includes. */
#include "debug.h"
#include "TCPConnection.hh"

TCPConnection* TCPConnection::fConnection;

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
TCPConnection::TCPConnection(int  Conn, int Listen, const char *rxaddr) 
{
    int    rv;
    time_t now;

    fConnection = this;

    fError    = NO_ERROR;
    // Use the time in seconds from the epoch as the unique ID. 
    time(&now);
    fRun       = true;
    fIsRunning = true;
    fListen_fd = Listen;
    fConnection_fd = Conn;
    fParentAddress = strdup(rxaddr);
    fPurpose   = NONE;

    rv = fcntl(Conn, F_SETFL, O_NONBLOCK);	
    if (rv == -1)
    {
	fError = ERROR_MODIFYING_ATTRIBUTES;
    }
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
TCPConnection::~TCPConnection()
{
    close(fConnection_fd);
    fIsRunning = false;
    delete fParentAddress;
    printf("TCPConnection delete.\n");
}

/**
 ******************************************************************
 *
 * Function Name : Write
 *
 * Description : Write "n" bytes to a descriptor.
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
long TCPConnection::Write(const void *vptr, size_t n)
{
    SET_DEBUG_STACK;
    size_t	nleft;
    int	        nwritten;
    const char	*ptr;

    fError = NO_ERROR;
    ptr    = (const char *)vptr;
    nleft  = n;
    while (nleft > 0) 
    {
        nwritten = write(fConnection_fd, ptr, nleft);
#if 0
        cout << __FILE__ << " " << __LINE__ 
             << " " << nwritten << " " << errno << endl;
#endif
        if ( nwritten < 0) 
        {
            /*
             * EINTR   4
             * EIO     5
             * EBADF   9
             * EAGAIN 11
             * EFAULT 14
             * EINVAL 22
             * ENOSPC 28
             * EPIPE  32
             * ECONNRESET 104
             * http://www.ioplex.com/~miallen/errcmp.html
             */
            switch( errno)
            {
            case EINTR:
                nwritten = 0;		/* and call write() again */
                break;
            case ECONNRESET:
                return -1;
                break;
            case EPIPE:
                return -2;
                break;
            default:
                return 0;			/* error */
                break;
            }
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    SET_DEBUG_STACK;
    return n;
}

/********************************************************************
 *
 * Function Name : Read
 *
 * Description : Get any data being sent to us. 0 is ok, nothing 
 * available. 
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/
int TCPConnection::Read(void *data, size_t NumberBytes)
{
    int            count;
    size_t         ReadCount;
    unsigned char* ReadPointer = (unsigned char *)data;

    SET_DEBUG_STACK;
    fError = NO_ERROR;
    
    ReadCount = 0;
    if (data == NULL)
    {
        return ReadCount;
    }
    while (ReadCount < NumberBytes)
    {
        count = read(fConnection_fd, ReadPointer, (NumberBytes-ReadCount));
        if ( count < 0) 
        {
            /*
             * EINTR   4
             * EIO     5
             * EBADF   9
             * EAGAIN 11
             * EFAULT 14
             * EINVAL 22
             * ENOSPC 28
             * EPIPE  32
             * ECONNRESET 104
             * http://www.ioplex.com/~miallen/errcmp.html
             */
            switch( errno)
            {
            case EAGAIN:
                // For the non-blocking I/O case, there is no data at this time
                return ReadCount;
            case EINTR:
                // Keep going. more to read. 
                break;
            case ECONNRESET:
                fError = ERROR_READ;
                SET_DEBUG_STACK;
                return ReadCount;
                break;
            case EPIPE:
                fError = ERROR_READ;
                SET_DEBUG_STACK;
                return ReadCount;
                break;
            default:
                fError = ERROR_READ;
                SET_DEBUG_STACK;
                return ReadCount;
                break;
            }

        }
        else if ( count == 0 )
        {
            fError = NO_READDATA;
            SET_DEBUG_STACK;
            return ReadCount;
        }
        else
        {
            ReadCount    += count;
            ReadPointer  += count;
        }
    }
    SET_DEBUG_STACK;
    return ReadCount;
}
void TCPConnection::Close(void)
{
    close(fConnection_fd);
}
void TCPConnection::Done(void)
{
    fPurpose = DISCONNECT;
    fIsRunning = false;
}
