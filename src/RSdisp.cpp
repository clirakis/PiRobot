/********************************************************************
 *
 * Module Name : RSdisp.c
 *
 * Author/Date : CBL/10-Jan-16
 *
 * Description : Display data robot server interface
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
 ********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <ncurses.h>
#include <time.h>
#include <math.h>

#include "RSdisp.hh"
#include "RSUtility.hh"

static int display_data = 1;
static int CommandCol   = 2;
static int current_screen;

WINDOW *vin;

static char    *main_frame_window[] =
{
 (char *)"+----------------------------------------------------------------------------+",
 (char *)"|            Robot Server  (h)ome, (r)efresh                                 |",
 (char *)"+----------------------------------------------------------------------------+",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|  Messages -----------------------------------------------------------------+",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|                                                                            |",
 (char *)"|  Command  -----------------------------------------------------------------+",
 (char *)"|                                                                            |",
 (char *)"+----------------------------------------------------------------------------+"
};

#define MAINFRAMEWINSIZ (sizeof(main_frame_window)/sizeof(main_frame_window[0]))

static char *home_strings[] = {
    (char *)   "   Home Screen                                 ",
    (char *)   "IP Connection 1 :                              ",
    (char *)   "IP Conneciton 2 :                              ",
    (char *)   "IP Connection 3 :                              ",
    (char *)   "IP Connection 4 :                              ",
    (char *)   "     Time of Fix:                HDOP:         ",
    (char *)   "      Velocity E:                VDOP:         ",
    (char *)   "      Velocity N:                TDOP:         ",
    (char *)   "      Velocity Z:              Status:         ",
    (char *)   "Frequency Offset:               Error:         ",
    (char *)   "        Fix type:                              ",
};

#define HOME_STR_SIZE sizeof(home_strings) / sizeof(home_strings[0])


static char *help_strings[] = {
    (char *) "                          Help                 ",
    (char *) "                                               ",
    (char *) "  d - toggle display data (Hex codes rec'd)    ",
    (char *) "  f - filter screen                            ",
    (char *) "  h - home                                     ",
    (char *) "  o - options screen                           ",
    (char *) "  q - quit                                     ",
    (char *) "  t - interactive test screen                  ",
    (char *)  "  w - toggle logging                           ",
    (char *) "  ? - help                                     ",
    (char *) "                                               ",
    (char *) "                                               ",
};

#define HELP_STR_SIZE sizeof(help_strings) / sizeof(help_strings[0])

int CurrentScreen(void)
{
    return current_screen;
}
int DisplayData(void)
{
    return display_data;
}
static void main_frame(void)
{
    uint32_t i;
    int x = 0;
    int y = 0;

    werase(vin);

    for (i = 0; i < MAINFRAMEWINSIZ; i++ )
    {
	wmove( vin, x+i, y);
        wprintw(vin, "%s", main_frame_window[i]);
    }

    switch (current_screen)
    {
    case MAIN_SCREEN:
	x = 3;
	y = 2;
        for (i = 0; i < HOME_STR_SIZE; i++ )
        {
            wmove( vin, x+i, y);
	    wprintw( vin, "%s",home_strings[i]);
        }
        break;
    case HELP_SCREEN:
	x = 3;
	y = 2;
        for (i = 0; i < HELP_STR_SIZE; i++ )
        {
            wmove( vin, x+i, y);
	    wprintw( vin, "%s",help_strings[i]);
        }
	break;
    default:
	break;
    }

    /* set background color */
    wbkgd(vin, COLOR_PAIR(1));

    /* push the output to the screen */
    wrefresh(vin);
}

void display_status(unsigned char status_code, unsigned char error_code)
{
    static char *Status[13] = {"Doing position fixes.",
			      "Don't have GPS time. ",
			      "Need initialization. ",
			      "PDOP too hight.      ",
			      "XXX                  ",
			      "XXX                  ",
			      "XXX                  ",
			      "XXX                  ",
			      "No usable satellites.",
			      "Only 1 useable sat.  ",
			      "Only 2 useable sat.  ",
			      "Only 3 useable sat.  ",
			      "Satellite unusuable. ",
    };
    char Error[32];

    int row = STATUS_AREA + 7;
    int col = RIGHT_AREA;

    if (status_code < 13)
    {
	wmove  (vin, row, col);
	wprintw(vin, "%s", Status[status_code]);
    }
    row++;
    
    sprintf( Error, " ");
    if (error_code & 0x01) strcat(Error, "No battery   ");
    if (error_code & 0x10) strcat(Error, "Antenna fault");
    wmove  (vin, row, col);
    wprintw(vin, "%s", Error);
}

void display_time(struct timespec *gpstime, double delta)
{
    int row = STATUS_AREA-1;
    int col = LEFT_AREA;
    char timestr[128], tmp[32];

    strftime( timestr, sizeof(timestr), "%F %H:%M:%S ", 
	      gmtime(&gpstime->tv_sec));
    sprintf(tmp, " %g", delta);
    strcat( timestr, tmp);
    wmove  (vin, row, col);
    wprintw(vin, "%s", timestr);
    row++;
}

void display_rp(unsigned char manual_mode, unsigned char nsvs, 
		unsigned char ndim, unsigned char *sv_prn,
		float pdop, float hdop, float vdop, float tdop)
{
    const char *mode_str[] = {"2D    ", 
			      "3D    ",
			      "Auto  ",
			      "Manual",
    };
    char sv[40], tmp[8];

    int row = STATUS_AREA;
    int col = RIGHT_AREA;
    int i;
    wmove  (vin, row, col);
    wprintw(vin, "%s", mode_str[manual_mode]);
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%2d", nsvs);
    row++;
    
    memset(sv, 0, sizeof(sv));
    sprintf(sv, "%2d ", sv_prn[0]);
    
    for (i=1;i<nsvs;i++)
    {
	sprintf(tmp, "%2d ", sv_prn[i]);
	strcat(sv, tmp);
    }
    wmove  (vin, row, col);
    wprintw(vin, "%s", sv);
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%8.2f", pdop);
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%8.2f", hdop );
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%8.2f", vdop);
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%8.2f", tdop);
    row++;
}

void display_mode( unsigned char mode)
{
    const char *mode_str[] = {"Manual GPS", 
			      "Manual GPD", 
			      "Auto GPS  ",
			      "Auto GPD  "
    };
    int row = STATUS_AREA+9;
    int col = LEFT_AREA;
    wmove  (vin, row, col);
    wprintw(vin, "%s", mode_str[mode]);
    row++;
}
void display_velocity( float *vel_ENU, float freq_offset)
{
    int row, col;

    row = STATUS_AREA+5;
    col = LEFT_AREA;
    
    wmove  (vin, row, col);
    wprintw(vin, "%6.4f", vel_ENU[0]);
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%6.4f", vel_ENU[1]);
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%5.4f", vel_ENU[2]);
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%8.2f", freq_offset);
    row++;
}
static char *str_lat(double latitude)
{
    double degrees, minutes, fracs;
    static char s1[16];

    degrees = fabs(latitude * R2D);
    minutes =  60.0 * modf(degrees, &degrees);
    fracs   = 10000.0 * modf(minutes, &minutes);
    if (latitude < 0.0)
    {
        sprintf(s1, "S  %2.2d %2.2d.%4.4d",
                (int)degrees, (int)minutes, (int)fracs);
    }
    else
    {
        sprintf(s1, "N  %2.2d %2.2d.%4.4d",
                (int)degrees, (int)minutes, (int)fracs);
    }
    return s1;
}

static char *str_lon(double longitude)
{
    double degrees, minutes, fracs;
    static char s2[16];

    degrees = fabs(longitude * R2D);
    minutes =   60.0 * modf(degrees, &degrees);
    fracs   = 10000.0 * modf(minutes, &minutes);
    if (longitude < 0.0)
    {
        sprintf(s2, "W %3.3d %2.2d.%4.4d",
               (int)degrees, (int)minutes, (int)fracs);
    }
    else
    {
        sprintf(s2, "E %3.3d %2.2d.%4.4d",
                (int)degrees, (int)minutes, (int)fracs);
    }
    return s2;
}

void display_position(double lat, double lon, double alt, double bias, float time)
{
    int row, col;

    row = STATUS_AREA;
    col = LEFT_AREA;
    
    wmove  (vin, row, col);
    wprintw(vin, "%s", str_lat(lat));
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%s", str_lon(lon));
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%6.2f", alt);
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%6.2f", bias);
    row++;
    
    wmove  (vin, row, col);
    wprintw(vin, "%6.2f", time);
    row++;
    
    /* set background color */
    wbkgd(vin, COLOR_PAIR(1));
    /* push the output to the screen */
    wrefresh(vin);
}
void display_connection( int number, const char *add, int Purpose)
{
    int row, col;
    static char* cPurpose[5] = {"NONE    ","COMMAND ","POSITION","ATTITUDE ","DISCONNECT"};

    row = STATUS_AREA + number%4;
    col = LEFT_AREA;
    
    wmove  (vin, row, col);
    wprintw(vin, "%s", add);

    col = RIGHT_AREA;
    wmove  (vin, row, col);
    wprintw(vin, "%s", cPurpose[Purpose]);

    /* set background color */
    wbkgd(vin, COLOR_PAIR(1));
    /* push the output to the screen */
    wrefresh(vin);
}

void WriteMsgToScreen(char *s)
{
    static char *cp;
    static char msg[64];
    static int row = MESSAGE_AREA;

    int n = strlen(s);
    if (n<1)
	return;

    if ((cp = strchr( s, '\r' )) != NULL)
    {
        *cp = '\0';
    }

    /* Clear out anything that is residual. */
    memset( msg, 0x20, sizeof(msg));
    memcpy( msg, s, n);

    wmove(vin,row,2);
    row--;
    if (row == (STATUS_AREA + STATUS_HEIGHT)) row = MESSAGE_AREA;
    /* THIS REALLY DOESN'T SCROLL! */

    /* print something to the window. Standard printf like format */
    wprintw(vin, "%s", msg);
    /* set background color */
    wbkgd(vin, COLOR_PAIR(1));

    /* push the output to the screen */
    wrefresh(vin);
}

void display_message (const char *fmt, ...)
{
    va_list p;
    char s[256], *cp;
    // char c[128];
    static int once = FALSE;

    va_start(p, fmt);
    vsprintf(s, fmt, p);
    va_end(p);
    
    if ((!once) && (current_screen == 0))
    {
	once = TRUE;
	main_frame();
    }
    //sprintf(c, "%-65.65s", s);
    if ((cp = strchr(s, '\n' )) != NULL)
    {
	*cp = '\0';
    }
    WriteMsgToScreen(s);
}
void ClearCommandArea(void)
{
    CommandCol = 2;
    wmove( vin, COMMAND_AREA, CommandCol);
    wprintw(vin, "                        ");

    /* set background color */
    wbkgd(vin, COLOR_PAIR(1));

    /* push the output to the screen */
    wrefresh(vin);
}
void DisplayCommandChar(unsigned char c)
{
    wmove( vin, COMMAND_AREA, CommandCol);    
    wprintw(vin, "0x%X ", c);
    CommandCol += 5;

    /* set background color */
    wbkgd(vin, COLOR_PAIR(1));

    /* push the output to the screen */
    wrefresh(vin);

} 
void WriteCmdToScreen(char *s)
{
    static char *cp;
    static char msg[64];

    int n = strlen(s);
    if (n<1)
	return;

    if ((cp = strchr( s, '\r' )) != NULL)
    {
        *cp = '\0';
    }

    /* Clear out anything that is residual. */
    memset( msg, 0x20, sizeof(msg));
    memcpy( msg, s, n-1);

    wmove( vin, COMMAND_AREA, 2);

    /* print something to the window. Standard printf like format */
    wprintw( vin, "%s", msg);

    /* set background color */
    wbkgd(vin, COLOR_PAIR(1));

    /* push the output to the screen */
    wrefresh(vin);
}

/* KeyCommand */
static void ParseHomeKeys( char c)
{
    switch(c)
    {
    default:
	break;
    }
}
/**
 ******************************************************************
 *
 * Function Name : check_keys
 *
 * Description : default screen is position status data. 
 * Changing such that the keys are checked based on which screen is active. 
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
int checkKeys(void)
{
    int rc = 0;

    /* get a character from the window. */
    int c = wgetch(vin);

    if (c != '\0')
    {
	switch (c)
	{
	case 0:
	    break;
	case 'd':
	case 'D':
	    if (display_data>0) 
		display_data = 0;
	    else
		display_data = 1;
	    break;
	case '?':
	    current_screen = HELP_SCREEN;
	    main_frame();
	    break;
	case 'h':
	case 'H':
	    current_screen = MAIN_SCREEN;
	    main_frame();
	    break;
	case 'q':
	case 'Q':
	    /* QUIT */
	    rc = 1;
	    break;
	case 'r':
	case 'R':
	    /* Repaint the screen */
	    main_frame();
	    break;
	default:
	    switch(current_screen) 
	    {
	    default:
		ParseHomeKeys(c);
		break;
	    }
	    break;
	}
    }
    return rc;
}

void start_display(void)
{
    initscr();
    start_color();
    init_pair( 1, COLOR_YELLOW, COLOR_BLUE);
    init_pair( 2, COLOR_BLUE  , COLOR_YELLOW);
    init_pair( 3, COLOR_BLUE  , COLOR_WHITE);
    /* 
     * newwin arguments - 
     * Number lines
     * ncolumns
     * begin_y
     * begin_x
     */
    vin = newwin(24,80,0,0);
    current_screen = MAIN_SCREEN;
    main_frame();
    
    //start_keys(GREEN_ONWHITE, 1, 62);
    WriteMsgToScreen("Start Display....");
}
void end_display(void)
{
    delwin(vin);
    endwin();
}
