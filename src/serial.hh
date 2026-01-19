/**
 ******************************************************************
 *
 * Module Name : serial.hh
 *
 * Author/Date : C.B. Lirakis / 24-Dec-05
 *
 * Description :
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * 19-Jan-26   CBL   Opening multiple connections, making into a class. 
 *
 * Classification : Unclassified
 *
 * References :
 *
 *******************************************************************
 */
#ifndef __SERIAL_hh_
#define __SERIAL_hh_
#include <string>
#include <termios.h>
#include "CObject.hh"

class RobotSerial : public CObject
{
public:
    RobotSerial(const char *port, speed_t BaudRate=B9600, bool Blocking=false);
    ~RobotSerial(void);

    inline string PortName(void) const {return fPortName;};
    size_t Write(const std::string& outbound);
    bool   Read(std::string &inbound);


private:
    int SerialOpen( const char *port, speed_t BaudRate, bool Blocking);

    /* Private variables */
    string   fPortName;        // Name of current port opened. 
    int      fSerialPortFd; 
};
#endif
