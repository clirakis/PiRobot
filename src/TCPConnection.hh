/*
 ******************************************************************
 *
 * Module Name : TCPConnection.hh
 *
 * Author/Date : C.B. Lirakis / 30-Jan-15
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
#ifndef __TCPCONNECTION_hh_
#  define __TCPCONNECTION_hh_
/*! This is a class that is created in it's own thread to handle 
 * a specific connection once it is created. This holds the basics of
 * read and write. 
 */
class TCPConnection 
{
public:

    /*! The intended use is to be able to connect to various devices
     *  for input or I/O. This can be more generic as time goes on
     */
    enum Purposes{NONE = 0, COMMANDS, POSITION, ATTITUTE, GENERAL, DISCONNECT};

    /*! The constructor - pass in the conneciton file descriptor
     * the lisen file descriptor and a character string of the numerical
     * octet that describes where the connection is coming from. 
     */
    TCPConnection( int Conn, int Listen, const char *rxaddr);
    /*! Desctructor - close the connection. */
    ~TCPConnection();

    /*! Inline function to indicate that the thread should stop running. */
    inline void Stop()             {fRun = false;};
    /*! Inline function to indicate the status of the run flag */
    inline bool Run()        const {return fRun;};
    /*! Inline function to determine if the loop is still executing. */
    inline bool Running()    const {return !fIsRunning;};
    /*! Inline funciton to get access to the listen file descriptor */
    inline int  GetListen()  const {return fListen_fd;};
    /*! Inline funciton to get access to the conneciton file descriptor */
    inline int  Connection() const {return fConnection_fd;};
    /*! Inline funciton to get access to the last generated error */
    inline int  Error()      const {return fError;};
    /*! Inline funciton that returns true if the connection fd matches the 
     * input fd*/
    inline bool SameFD(int fd) const {return (fd == fConnection_fd);};
    /*! All the user to set the purpose as shown in the enumerator above. */
    inline void SetPurpose(int p) { fPurpose = p;};
    /*! Return an indicator on the purpose of this connection. */
    inline int  Purpose(void) const {return fPurpose;};
    /*! get an ordinal number for the thread */
    inline void SetNumber(int i) { fNumber = i;};
    /*! Set a number that indicates an operational parameter. */
    inline int  Number(void) {return fNumber;};
    /*! What is the parent address - the address of the Client. */
    inline const char *Address(void) const {return fParentAddress;};

    void Done(void);
    /*! Read function */
    int  Read(void *data, size_t NumberBytes);
    /*! Write function */
    long Write(const void *data, size_t NumberBytes);
    /*! character wrappr for write function */
    inline long Write(const char *d, size_t N) {return Write((void*)d,N);};
    /*! close the connection */
    void Close(void);

    /*! A static method to get to this handle. */
    static TCPConnection* Get(void) {return fConnection;};

    enum Errors {NO_ERROR, NO_READDATA, ERROR_READ, ERROR_WRITE, 
                 ERROR_MODIFYING_ATTRIBUTES};
private:
    /*! The file descriptor of the listener */
    int     fListen_fd;
    /*! The file descriptor of the connection that was given at the listen. */
    int     fConnection_fd;
    /*! Should we continue to run? */
    bool    fRun;
    /*! Are we running. */
    bool    fIsRunning;
    /*! Indicator of the last error */
    int     fError;
    /*! The client address */
    char*   fParentAddress;
    /*! The purpose assigned to this connection */
    int     fPurpose;
    /*! The number assigned by the user */
    int     fNumber;

    /*! A static pointer to 'this' */
    static TCPConnection* fConnection;
};
#endif
