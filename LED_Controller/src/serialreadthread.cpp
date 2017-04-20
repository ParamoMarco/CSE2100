#include <glib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "global.h"
/** Sets the sleep duration for the thread to 1 second**/
#define READ_THREAD_SLEEP_DURATION_US 100000
/** Sets the time between characters sent to 1ms **/
#define BETWEEN_CHARACTERS_TIMEOUT_US 1000
/** Sets the max voltage variable to 3.3 Volts **/
#define MAX_VOLTAGE 3.3
/** Sets the MAX_ADC_VALUE to 1024.0 */
#define MAX_ADC_VALUE 1024.0
/** Defines the byte which will be checked for the potentiometer**/
#define POT_COMMAND 'P'

/** Defines the start byte of the packet**/
#define PACKET_START_BYTE  0xAA
/** Defines the number of overhead bytes of the packet**/
#define PACKET_OVERHEAD_BYTES  3
/** Defines the minimum number of bytes of the packet**/
#define PACKET_MIN_BYTES  PACKET_OVERHEAD_BYTES + 1
/** Defines the maximum bytes of the packet**/
#define PACKET_MAX_BYTES  255
/** This function validates the packets received **/
int validatePacket(unsigned int packetSize, unsigned char *packet)
{
    // check the packet size
    if(packetSize < PACKET_MIN_BYTES || packetSize > PACKET_MAX_BYTES)
    {
        return 0;
    }

    // check the start unsigned char
    if(packet[0] != PACKET_START_BYTE)
    {
        return 0;
    }

    // check the length unsigned char
    if(packet[1] != packetSize)
    {
        return 0;
    }

    // compute the checksum
    unsigned char checksum = 0x00;
    for(unsigned int i = 0; i < packetSize - 1; i++)
    {
        checksum = checksum ^ packet[i];
    }

    // check to see if the computed checksum and packet checksum are equal
    if(packet[packetSize - 1] != checksum)
    {
        return 0;
    }

    // all validation checks passed, the packet is valid
    return 1;
}

gpointer Serial_Read_Thread()
{
  ssize_t r_res;
  char ob[50];
  unsigned int count=0;
  static unsigned char buffer[PACKET_MAX_BYTES];
  unsigned int packetSize = PACKET_MIN_BYTES;
  double voltage_disp;

  while(!kill_all_threads)
    {
      if(ser_dev!=-1)
	{
	  r_res = read(ser_dev,ob,1);
	  //	  cout<<(int)ob[0]<<endl;
	  if(r_res==0)
	    {
	      usleep(BETWEEN_CHARACTERS_TIMEOUT_US);
	    }
	  else if(r_res<0)
	    {
	      cerr<<"Read error:"<<(int)errno<<" ("<<strerror(errno)<<")"<<endl;
	    }
	  else
	    {
	      //this means we have received a unsigned char;
	      //the unsigned char is in ob[0]

	      //TODO: receive an entire packet and validate it.
		if(count == 0 && ob[0] == PACKET_START_BYTE)
            {
                // this unsigned char signals the beginning of a new packet
                buffer[count] = ob[0];
                count++;
                continue;
            }
            else if(count == 0)
            {
                // the first unsigned char is not valid, ignore it and continue
                continue;
            }
            else if(count == 1)
            {
                // this unsigned char contains the overall packet length
                buffer[count] = ob[0];

                // reset the count if the packet length is not in range
                if(packetSize < PACKET_MIN_BYTES || packetSize > PACKET_MAX_BYTES)
                {
                    count = 0;
                }
                else
                {
                    packetSize = ob[0];
                    count++;
                }
                continue;
            }
            else if(count < packetSize)
            {
                // store the unsigned char
                buffer[count] = ob[0];
                count++;
            }

            // check to see if we have acquired enough bytes for a full packet
            if(count >= packetSize)
            {
                // validate the packet
                if(validatePacket(packetSize, buffer)){
			if(buffer[2] == 'P'){

			voltage_disp = (double) ((buffer[3]<<8) +buffer[4]) * (3.3/1024);
	      g_mutex_lock(mutex_to_protect_voltage_display);
	      sprintf(c_voltage_value,"%1.3f",voltage_disp);
	      g_mutex_unlock(mutex_to_protect_voltage_display);
			
			}
		}
             }   
	      
	      //TODO: Once validated, check if the third unsigned char is a 'P'
	      //decode the payload into a single int between 0 and MAX_ADC_VALUE
	      //convert it into a voltage 0->0V  MAX_ADC_VALUE->MAX_VOLTAGE
	      //load that converted value into voltage_disp
	      //the next three lines need to be there for the value to be
	      //displayed (once you loaded voltage_value with the received
	      //voltage value

	      //once you have voltage_disp calculated us the following three
	      //lines of code to change the global variable c_voltage_value
	      //this is the variable that the main thread periodically displays
	      //in the GUI
	      //g_mutex_lock(mutex_to_protect_voltage_display);
	      //sprintf(c_voltage_value,"%1.3f",voltage_disp);
	      //g_mutex_unlock(mutex_to_protect_voltage_display);
	
	    }
	}
      else
	usleep(READ_THREAD_SLEEP_DURATION_US);
    }

  return NULL;
}
