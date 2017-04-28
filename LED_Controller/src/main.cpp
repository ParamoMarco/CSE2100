/*!
	* \author	 Marco Paramo 
	* \author Kenny Vo
	* \version 1.0
	* \mainpage The Teensy LED Controller
	* \section intro_sec Introduction
	* This code was developed to finish Lab 9 in CSE2100
	* \section compile_sec Compilation
	* Here are the instructions on how to compile this code.
	* \subsection Step1 Cmake
	* run cmake from the build directory in the LED_Controller with the command "cmake .." (../LED_Controller/build cmake ..)
	* \subsection Step2 make 
	* run the make command from within the build directory
	*/
#include "global.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#define VOLTAGE_DISPLAY_UPDATE_MS 100//!<This variable delays how often voltage is read and displayed on the GUI

 #define GuiappGET(xx) gui_app->xx=GTK_WIDGET(gtk_builder_get_object(p_builder,#xx))   //!<Creates the GUI creating macro using GTK
/*!
  *\brief Connect widgets with code
  *\param p_builder Pointer to the GUI build structure
**/
void ObtainGuiWidgets(GtkBuilder *p_builder)
{
 
  GuiappGET(window1);
  GuiappGET(entry_sd);
  GuiappGET(label_voltage);
  GuiappGET(entry_red);
  GuiappGET(entry_blue);
  GuiappGET(entry_green);
  GuiappGET(scale_red);
  GuiappGET(scale_green);
  GuiappGET(scale_blue);
  GuiappGET(label_tx);

}


//********************************************************************
// GUI handlers
//********************************************************************
/*!
 * \brief This thread runs in the background to read voltage from the Teensy
 * \param p_gptr The voltage recieved from the Teensy to display on the GUI
 */
gboolean  Voltage_Display_Displayer(gpointer p_gptr)
{
  // do not change this function
  g_mutex_lock(mutex_to_protect_voltage_display);
  gtk_label_set_text(GTK_LABEL(gui_app->label_voltage),c_voltage_value);
  g_mutex_unlock(mutex_to_protect_voltage_display);
  return true;
}
/*!
 *\brief Opens up the serial port for the Teensy
 * \param p_wdgt Pointer to the widget
 * \param p_data Pointer to the data for the widget
 */
extern "C" void button_opendevice_clicked(GtkWidget *p_wdgt, gpointer p_data ) 
{
  //do not change  the next few lines
  //they contain the mambo-jumbo to open a serial port
  
  const char *t_device_value; //!< Directory to the device port
  struct termios my_serial; //!< Structure for the seriel port

  t_device_value = gtk_entry_get_text(GTK_ENTRY(gui_app->entry_sd));
  //open serial port with read and write, no controling terminal (we don't
  //want to get killed if serial sends CTRL-C), non-blocking 
  ser_dev = open(t_device_value, O_RDWR | O_NOCTTY );
  
  bzero(&my_serial, sizeof(my_serial)); // clear struct for new port settings 
        
  //B9600: set baud rate to 9600
  //   CS8     : 8n1 (8bit,no parity,1 stopbit)
  //   CLOCAL  : local connection, no modem contol
  //   CREAD   : enable receiving characters  */
  my_serial.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
         
  tcflush(ser_dev, TCIFLUSH);
  tcsetattr(ser_dev,TCSANOW,&my_serial);

  //You can add code beyond this line but do not change anything above this line
  
}
/*!
 *\brief Closes up the serial port for the Teensy
 * \param p_wdgt Pointer to the widget
 * \param p_data Pointer to the data for the widget
 */
extern "C" void button_closedevice_clicked(GtkWidget *p_wdgt, gpointer p_data ) 
{

  gtk_widget_set_sensitive (gui_app->button_closedevice,FALSE);  //!!<This is how you disable a button:
  //this is how you enable a button:
  gtk_widget_set_sensitive (gui_app->button_opendevice,TRUE);  //!!<This is how you enable a button:

  //do not change the next two lines; they close the serial port
  close(ser_dev);
  ser_dev=-1;

}
/*!
 *\brief  Sends the packet created to the Teensy
 * \param p_wdgt Pointer to the widget
 * \param p_data Pointer to the data for the widget
 */
extern "C" void button_send_clicked(GtkWidget *p_wdgt, gpointer p_data ) 
{
  const char *t_red_value;
  unsigned char uc_red_value;
  const char *t_green_value;
  unsigned char uc_green_value;
  const char *t_blue_value;
  unsigned char uc_blue_value;
  char c_cc_value[40];
  char send_buff[7];
  int length_send_buff = 7;


  //getting text from widget:
  t_red_value = gtk_entry_get_text(GTK_ENTRY(gui_app->entry_red));
  uc_red_value = atoi(t_red_value);
  t_green_value = gtk_entry_get_text(GTK_ENTRY(gui_app->entry_green));
  uc_green_value = atoi(t_green_value);
  t_blue_value = gtk_entry_get_text(GTK_ENTRY(gui_app->entry_blue));
  uc_blue_value = atoi(t_blue_value);
  //setting range on scale slider to uc_red_value 
  gtk_range_set_value(GTK_RANGE(gui_app->scale_red),uc_red_value);
gtk_range_set_value(GTK_RANGE(gui_app->scale_green),uc_green_value);
gtk_range_set_value(GTK_RANGE(gui_app->scale_blue),uc_blue_value);		      
  //setting text on label
  unsigned char checksum;
  checksum = 0xAA ^ 0x07 ^ 0x4C ^ uc_red_value ^ uc_green_value ^ uc_blue_value;
  sprintf(c_cc_value,"AA 07 4C %0.2x %0.2x %0.2x %0.2x",uc_red_value, uc_green_value, uc_blue_value, checksum);
  gtk_label_set_text(GTK_LABEL(gui_app->label_tx),c_cc_value);
  send_buff[0] = (char) 0xAA;
  send_buff[1] = (char) 0x07;
  send_buff[2] = 'L';
  send_buff[3] = uc_red_value;
  send_buff[4] = uc_green_value;
  send_buff[5] = uc_blue_value;
  send_buff[6] = checksum;
  //this is how you send an array out on the serial port:
  write(ser_dev,send_buff,length_send_buff);
}
/*!
 *\brief Changes the in the text boxes based on changes in the scale widget
 * \param p_wdgt Pointer to the widget
 * \param p_data Pointer to the data for the widget
 */
extern "C" void scale_rgb_value_changed(GtkWidget *p_wdgt, gpointer p_data ) 
{
  char c_cc_value[40];
  char send_buff[7];
  int length_send_buff = 7;
  //getting the value of the scale slider 
  double g_red_value = gtk_range_get_value(GTK_RANGE(gui_app->scale_red));
  double g_green_value = gtk_range_get_value(GTK_RANGE(gui_app->scale_green));
  double g_blue_value = gtk_range_get_value(GTK_RANGE(gui_app->scale_blue));
 
 
unsigned char uc_red_value = (unsigned char) g_red_value;
unsigned char uc_green_value = (unsigned char) g_green_value;
unsigned char uc_blue_value = (unsigned char) g_blue_value;

//setting text on entry
  sprintf(c_cc_value,"%d",uc_red_value);
  gtk_entry_set_text(GTK_ENTRY(gui_app->entry_red),c_cc_value);
  sprintf(c_cc_value,"%d",uc_green_value);
  gtk_entry_set_text(GTK_ENTRY(gui_app->entry_green),c_cc_value);
  sprintf(c_cc_value,"%d",uc_blue_value);
  gtk_entry_set_text(GTK_ENTRY(gui_app->entry_blue),c_cc_value);
  

  unsigned char checksum;
  checksum = 0xAA ^ 0x07 ^ 0x4C ^ uc_red_value ^ uc_green_value ^ uc_blue_value;
  sprintf(c_cc_value,"AA 07 4C %0.2x %0.2x %0.2x %0.2x",uc_red_value, uc_green_value, uc_blue_value, checksum);
  gtk_label_set_text(GTK_LABEL(gui_app->label_tx),c_cc_value);
  send_buff[0] = (char) 0xAA;
  send_buff[1] = (char) 0x07;
  send_buff[2] = 'L';
  send_buff[3] = uc_red_value;
  send_buff[4] = uc_green_value;
  send_buff[5] = uc_blue_value;
  send_buff[6] = checksum;
  //this is how you send an array out on the serial port:
  write(ser_dev,send_buff,length_send_buff);
  
}

/*!
 *\brief Closes the GUI and ends the program when the Exit button is pressed
 * \param p_wdgt Pointer to the widget
 * \param p_data Pointer to the data for the widget
 */
extern "C" void button_exit_clicked(GtkWidget *p_wdgt, gpointer p_data ) 
{
  gtk_main_quit();

}



//********************************************************************
//********************************************************************
// 
//   Main loop
//
//********************************************************************
//********************************************************************
/** Main loop**/
int main(int argc, char **argv)
{

  GtkBuilder *builder;
  GError *err = NULL;

  GThread *read_thread;

  //this is how you allocate a Glib mutex
  g_assert(mutex_to_protect_voltage_display == NULL);
  mutex_to_protect_voltage_display = new GMutex;
  g_mutex_init(mutex_to_protect_voltage_display);

  // this is used to signal all threads to exit
  kill_all_threads=false;
  
  //spawn the serial read thread
  read_thread = g_thread_new(NULL,(GThreadFunc)Serial_Read_Thread,NULL);
  
  // Now we initialize GTK+ 
  gtk_init(&argc, &argv);
  
  //create gtk_instance for visualization
  gui_app = g_slice_new(Gui_Window_AppWidgets);

  //builder
  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "teensy_control.glade", &err);

  
  //error handling
  if(err)
    {
      g_error(err->message);
      g_error_free(err);
      g_slice_free(Gui_Window_AppWidgets, gui_app);
      exit(-1);
    }

  // Obtain widgets that we need
  ObtainGuiWidgets(builder);

  // Connect signals
  gtk_builder_connect_signals(builder, gui_app);

  // Destroy builder now that we created the infrastructure
  g_object_unref(G_OBJECT(builder));

  //display the gui
  gtk_widget_show(GTK_WIDGET(gui_app->window1));

  //this is going to call the Voltage_Display_Displayer function periodically
  gdk_threads_add_timeout(VOLTAGE_DISPLAY_UPDATE_MS,Voltage_Display_Displayer,NULL);

  //the main loop
  gtk_main();

  //signal all threads to die and wait for them (only one child thread)
  kill_all_threads=true;
  g_thread_join(read_thread);
  
  //destroy gui if it still exists
  if(gui_app)
    g_slice_free(Gui_Window_AppWidgets, gui_app);

  return 0;
}
