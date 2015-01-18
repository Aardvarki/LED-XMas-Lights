#include "Lite.h"

Lite::Lite(byte count)
{
	_pinB = 0;
	_pinD = 0;
	_count = count;
	_intensity = default_intensity;
}

Lite::~Lite(){}

void Lite::init()
{
	DDRB |= _pinB;			// This will set the pin to OUTPUT
	PORTB &= (~_pinB);		// This will set the pin to LOW
	DDRD |= _pinD;			// This will set the pin to OUTPUT
	PORTD &= (~_pinD);		// This will set the pin to LOW
	
	fill_color(0,0,0);		// Sets all lights to OFF position (enumerating)
}

void Lite::_low()
{
	PORTB &= (~_pinB); 
	PORTD &= (~_pinD);
}

void Lite::_high()
{
	PORTB |= _pinB;
	PORTD |= _pinD;
}

//packetstart initializes the packet being sent on the data line
// 10us high
void Lite::packetstart()
{
	_high();
	delayMicroseconds(10);
	_low();
}

//packetone sends a bit representing a '1' down the data line
// '1' is 20us low, 10us high.
void Lite::packetone()
{
	_low();
	delayMicroseconds(20);
	_high();
	delayMicroseconds(10);
	_low();
}

//packetzero sends a bit representing a '0' down the data line
// '0' is 10us low, 20us high.
void Lite::packetzero()
{
	_low();
	delayMicroseconds(10);
	_high();
	delayMicroseconds(20);
	_low();
}

//packetend signals the end of the packet, 30us of quiet.
void Lite::packetend()
{
	_low();
	delayMicroseconds(30);
}                                               

//set_color sets a single light to a given color
void Lite::set_color(byte n, int r, int g, int b)
{
	byte intensity = _intensity;
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
	if(intensity>max_intensity) 	
		intensity=max_intensity;  		// Capped at 0xCC
	if(intensity<0) 
		intensity=0;					// Can't be negative
	if(intensity == _intensity) 	
		return;
	
	int wait = time / abs(intensity-_intensity); 	// set the delay
	
	while (_intensity!=intensity)
	{
	 	_intensity>intensity?_intensity--:_intensity++;		// shift by one
	 	set_color(63, 0, 0, 0);								// broadcast change
	 	delay(wait);
	}
}

// The mix function can be called to control multiple pins simultaneously,
// in the event that you have multiple strands of lights hooked up.  By
// mixing two or more pins together, you will always transmit exactly the 
// same data to all pins.  This is desirable because it takes less time
// to update multiple strands if they are mixed than if they are updated
// separately. 
void Lite::mix(byte pin)
{
	switch(pin)	{
		case 2: _pinD |= B00000100; break;
		case 3: _pinD |= B00001000; break;
		case 4: _pinD |= B00010000; break;
		case 5: _pinD |= B00100000; break;
		case 6: _pinD |= B01000000; break;
		case 7: _pinD |= B10000000; break;
		
		case 8: _pinB |= B000001; break;
		case 9: _pinB |= B000010; break;
		case 10: _pinB |= B000100; break;
		case 11: _pinB |= B001000; break;
		case 12: _pinB |= B010000; break;
		case 13: _pinB |= B100000; break;
	};
}    

// Unmix will remove a pin from the mix.
void Lite::unmix(byte pin)      
{
	switch(pin)	{
		case 2: _pinD &= B11111011; break;
		case 3: _pinD &= B11110111; break;
		case 4: _pinD &= B11101111; break;
		case 5: _pinD &= B11011111; break;
		case 6: _pinD &= B10111111; break;
		case 7: _pinD &= B01111111; break;
		
		case 8: _pinB &= B111110; break;
		case 9: _pinB &= B111101; break;
		case 10: _pinB &= B111011; break;
		case 11: _pinB &= B110111; break;
		case 12: _pinB &= B101111; break;
		case 13: _pinB &= B011111; break;
	};
}                    

boolean Lite::isActive()
{	return boolean(_pinD | _pinB); }
