/**
 ******************************************************************
 *
 * Module Name : Robot.hh
 *
 * Author/Date : C.B. Lirakis / 04-Jan-26
 *
 * Description : Entry point for overall Robot control program. 
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
#ifndef __ROBOT_hh_
#define __ROBOT_hh_
#  include "CObject.hh" // Base class with all kinds of intermediate

class Robot : public CObject
{
public:
    /** 
     * Build on CObject error codes. 
     */
    enum {ENO_FILE=1, ECONFIG_READ_FAIL, ECONFIG_WRITE_FAIL};
    /**
     * Constructor the lassen SK8 subsystem.
     * All inputs are in configuration file. 
     */
    Robot(const char *ConfigFile);

    /**
     * Destructor for SK8. 
     */
    ~Robot(void);

    /*! Access the This pointer. */
    static Robot* GetThis(void) {return fRobot;};

    /**
     * Main Module DO
     * 
     */
    void Do(void);

    /**
     * Tell the program to stop. 
     */
    void Stop(void);

    /**
     * Control bits - control verbosity of output
     */
    static const unsigned int kVerboseBasic    = 0x0001;
    static const unsigned int kVerboseMSG      = 0x0002;
    static const unsigned int kVerboseFrame    = 0x0010;
    static const unsigned int kVerbosePosition = 0x0020;
    static const unsigned int kVerboseHexDump  = 0x0040;
    static const unsigned int kVerboseCharDump = 0x0080;
    static const unsigned int kVerboseMax      = 0x8000;
 
private:

    bool fRun;

    /*! 
     * Configuration file name. 
     */
    char   *fConfigFileName;


    /* Private functions. ==============================  */


    /*!
     * Read the configuration file. 
     */
    bool ReadConfiguration(void);
    /*!
     * Write the configuration file. 
     */
    bool WriteConfiguration(void);


    /*! The static 'this' pointer. */
    static Robot *fRobot;
};
#endif
