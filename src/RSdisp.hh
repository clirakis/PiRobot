/**
 ******************************************************************
 *
 * Module Name : RSdisp.hh
 *
 * Author/Date : C.B. Lirakis / 10-Jan-16
 *
 * Description : display the robot server engine
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
#ifndef __RSDISP_hh__
#define __RSDISP_hh__
#include <time.h>

/* Screen Layout */
/* Row where various messages are displayed. */
#define TOP_BAR        2
#define STATUS_AREA    4
#define STATUS_HEIGHT 11
#define MESSAGE_AREA  20
#define COMMAND_AREA  22

/* Columns */
#define LEFT_AREA     21
#define RIGHT_AREA    LEFT_AREA+21


/* Screen definitions. */
#define MAIN_SCREEN         1
#define HELP_SCREEN         MAIN_SCREEN+1

      void start_display(void);
      void end_display(void);
      int  checkKeys(void);
      void WriteMsgToScreen(char *s);
      void display_message (const char *fmt, ...);
      /* Get the current display screen */
      int  CurrentScreen(void);

      int  DisplayData(void);
      void display_connection( int number, const char *add, int Purpose);
#if 0
      /* Position screen stuff. */
      void display_position(double lat, double lon, double alt, double bias, float time);
      void display_velocity( float *vel_ENU, float freq_offset);
      void display_mode(unsigned char mode);
      void display_rp(unsigned char manual_mode, unsigned char nsvs, 
		      unsigned char ndim, unsigned char *sv_prn,
		      float pdop, float hdop, float vdop, float tdop);
      void display_time(struct timespec *gpstime, double delta);
      void display_options(unsigned char pos_code,
			   unsigned char vel_code,
			   unsigned char time_code,
			   unsigned char aux_code);
      void display_status(unsigned char status_code, unsigned char error_code);

#endif
      /* Mainpulate command area. */
      void DisplayCommandChar(unsigned char c);
      void ClearCommandArea(void);

      /* Options screen stuff */
      void OptionsScreen(void);
      void ParseOptionsKeys(char c);

#endif

