#include "global.h"
 /**Structure to keep all the widgets**/
Gui_Window_AppWidgets *gui_app; 

int ser_dev=-1; 

char c_voltage_value[40];

int kill_all_threads;

GMutex *mutex_to_protect_voltage_display;



  

