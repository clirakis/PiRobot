/**
 ******************************************************************
 *
 * Module Name : Commands.h
 *
 * Author/Date : C.B. Lirakis / 11-Jan-16
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
#ifndef __COMMANDS_h_
#define __COMMANDS_h_

class TCPConnection;

/// Module documentation here. 
class Commands {
public:
    /// Default Constructor
    Commands(TCPConnection*);
    /// Default destructor
    ~Commands();
    bool Parse (const char*);
    inline bool Run(void) {return fRun;};

    static Commands* GetThis(void) {return fCommands;};

private:
    bool ProgramControl(const char*);
    void Help();

    bool           fRun;
    TCPConnection *fRx;

    static Commands* fCommands;
};
#endif
