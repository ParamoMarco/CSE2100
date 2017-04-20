#include <gtk/gtk.h>
#include <stdlib.h>
#include <iostream>
/** Created STDC Format Macros **/
#define __STDC_FORMAT_MACROS


#ifndef _MY__GLOBAL__H
/** Created Global.h link if it is not included already**/
#define _MY__GLOBAL__H 

using namespace std;


/**************************************************************
 * GUI window stuff
 **************************************************************/

typedef struct 
{
  GtkWidget *window1; //visualization window

  GtkWidget *entry_sd; //the text field to contain the name of the serial device
  GtkWidget *label_voltage; // the label on which we display the voltage
  GtkWidget *scale_red;
  GtkWidget *scale_green;
  GtkWidget *scale_blue;
  GtkWidget *entry_red;
  GtkWidget *entry_green;
  GtkWidget *entry_blue;
  GtkWidget *button_send;
  GtkWidget *button_exit;
  GtkWidget *button_closedevice;
  GtkWidget *button_opendevice;
  GtkWidget  *label_tx;

} Gui_Window_AppWidgets;
/** Pointer to the GUI window **/
extern Gui_Window_AppWidgets *gui_app;

/**This is the serial devices handle**/
extern int ser_dev;

/**This is to gracefully shut down threads**/
extern int kill_all_threads;

/**This variable is for communicating the voltage value string**/
extern char c_voltage_value[40];

/**This is the mutex for the above variable**/
extern GMutex *mutex_to_protect_voltage_display;

/**Prototype of function for serial read thread**/
gpointer Serial_Read_Thread();



#endif
