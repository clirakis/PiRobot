/**
 ******************************************************************
 *
 * Module Name : tcpserver.hh
 *
 * Author/Date : C.B. Lirakis / 10-May-09
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
#ifndef __TCPSERVER_h_
#define __TCPSERVER_h_
#include <semaphore.h>

# ifdef __cplusplus
  extern          "C"
  {
# endif
      /*! structure for threads in the TCP server system */
      struct ThreadControl_t {
	  int Run;
	  int IsRunning;
      };
      /*! The actual server this should be started as a thread. 
       * The TCPConnection class should be overridden and inserted 
       * here to work in the specific
       */
      void* TCP_Server(void *val);
      /*! Method to access the close of the socket opened by the system. */
      void  TCPClose(void);
# ifdef __cplusplus
  }
# endif

#endif
