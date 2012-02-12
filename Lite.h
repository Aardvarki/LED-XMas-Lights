/*
  Written by Adam Karki, using code borrowed from the internet, modified to use
  direct port manipulation instead of the slower digitalWrite function.
  
  Due to the use of port manipulation (and my own laziness) this code only functions
  on digital pins 8-13.  
  
  NOTE:
  You should wait at least 500ms before declaring your first Lite instance if you
  are powering your Arduino and your lights on the same plug, the lights should
  be allowed enough time to ensure they are powered on before transmitting to them.
  A simple delay(500); at the top of your setup function accomplishes this well.
  
  TO DOs: 
  Add support for digital pins 2-7
  
  Released into the public domain.
*/
#ifndef Lite_h
#define Lite_h
#include <WProgram.h>

// Define the starting information for the lights
#define default_intensity 0xCC
#define max_intensity 0xCC
#define max_brightness 0xF                       

class Lite{
	public:
		Lite(byte pin, byte count);
		
      ~Lite();
   	void set_color(byte n, byte intensity, int r, int g, int b);
   	void set_color(byte n, int r, int g, int b);
   	void fill_color(int r, int g, int b);
   	void set_intensity(byte intensity, int time = 0);
   	byte state[][3];
   	void mix(byte pin);  
   	void unmix(byte pin);
   	boolean isActive();
   	
   private:
   	void packetstart();
   	void packetone();
   	void packetzero();
   	void packetend();
		byte _pin;
		byte _intensity;
		byte _count;
};
#endif