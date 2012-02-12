#include "Lite.h"

Lite::Lite(byte pin, byte count)
{
	_pin = 0;
	mix(pin);  				// This will set _pin to the proper PORT value
	DDRB |= _pin;			// This will set the pin to OUTPUT
	PORTB &= (~_pin);		// This will set the pin to LOW
	_count = count;
	_intensity = default_intensity;
	//for(int x=0;x<_count;x++)  //Shouldn't be necessary
	//	for(int y=0;y<3;y++)
	//		state[x][y] = 0;
	fill_color(0,0,0);
}

Lite::~Lite(){}

//packetstart initializes the packet being sent on the data line
// PORTB |= _pin; turns the pin HIGH without touching the other pins
// PORTB &= ~_pin; turns the pin LOW without touching the other pins
void Lite::packetstart()
{
	PORTB |= _pin;
	delayMicroseconds(10);
	PORTB &= (~_pin); 
}

//packetone sends a bit representing a '1' down the data line
// '1' is 20us low, 10us high.
void Lite::packetone()
{
	PORTB &= (~_pin);
	delayMicroseconds(20);
	PORTB |= _pin;
	delayMicroseconds(10);
	PORTB &= (~_pin);
}

//packetzero sends a bit representing a '0' down the data line
// '0' is 10us low, 20us high.
void Lite::packetzero()
{
	PORTB &= (~_pin);
	delayMicroseconds(10);
	PORTB |= _pin;
	delayMicroseconds(20);
	PORTB &= (~_pin);
}

//packetend signals the end of the packet, 30us of quiet.
void Lite::packetend()
{
	PORTB &= (~_pin);
	delayMicroseconds(30);
}                                               

//set_color sets a single light to a given color
void Lite::set_color(byte n, int r, int g, int b)
{
	byte intensity = _intensity;
	if(n<_count)
	{
		state[n][0] = r;
		state[n][1] = g;
		state[n][2] = b;
	}
	int color = (b<<8) + (g<<4) + r;
	int i;
	packetstart();  
   for(i=6;i;i--,(n<<=1))  
        if(n&(1<<5))  
             packetone();  
        else  
             packetzero();  
   for(i=8;i;i--,(intensity<<=1))  
        if(intensity&(1<<7))  
             packetone();  
        else  
             packetzero();  
   for(i=12;i;i--,(color<<=1))  
        if(color&(1<<11))  
             packetone();  
        else  
             packetzero();  
   packetend();  
}


// fill_color sets all of the lights to the same color.
void Lite::fill_color(int r, int g, int b)
{
	for(byte x=0;x<_count;x++)
		set_color(x,r,g,b);
}

// set_intensity changes the intensity of all lights to the value passed to it,
// stepping by one at a time because of a limitation of the lighting hardware.
// time is the length of time, in ms, that the fade should take (default 0)

// NOTE:  Due to the speed of the data line, even with a "time" of 0, it will 
// take at least 170ms to change the lights from zero to full or vice versa.
void Lite::set_intensity(byte intensity, int time)
{
	if(intensity>max_intensity) 	intensity=max_intensity;  	// Capped at 0xCC
	if(intensity<0) 					intensity=0;					// Can't be negative
	if(intensity == _intensity) 	return;
	
	int wait = time / abs(intensity-_intensity); 				// set the delay
	
	while (_intensity!=intensity)
	{
	 	_intensity>intensity?_intensity--:_intensity++;			// shift by one
	 	set_color(63, 0, 0, 0);											// broadcast change
	 	delay(wait);
	}
}

// The mix function can be called to control multiple pins simultaneously,
// in the event that you have multiple strands of lights hooked up.  By
// mixing two or more pins together, you will always transmit exactly the 
// same data to both strands.  This is desirable because it takes less time
// to update both light strands if they are mixed than if they are updated
// separately. 
void Lite::mix(byte pin)
{
	switch(pin)	{
		case 8: _pin |= B000001; break;
		case 9: _pin |= B000010; break;
		case 10: _pin |= B000100; break;
		case 11: _pin |= B001000; break;
		case 12: _pin |= B010000; break;
		case 13: _pin |= B100000; break;
	};
}    

// Unmix will remove a pin from the mix.
void Lite::unmix(byte pin)      
{
	switch(pin)	{
		case 8: _pin &= B111110; break;
		case 9: _pin &= B111101; break;
		case 10: _pin &= B111011; break;
		case 11: _pin &= B110111; break;
		case 12: _pin &= B101111; break;
		case 13: _pin &= B011111; break;
	};
}                    

boolean Lite::isActive()
{	return boolean(_pin); }