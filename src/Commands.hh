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
/// Module documentation here. 
class Commands {
public:
    /// Default Constructor
    Commands();
    /// Default destructor
    ~Commands();
    bool Parse (const char*);
    inline bool Run(void) {return fRun;};

    static Commands* GetThis(void) {return fCommands;};

private:
    bool ProgramControl(const char*);

    bool fRun;

    static Commands* fCommands;
};
#endif
