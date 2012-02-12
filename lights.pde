#include <Lite.h>
#include <EEPROM.h>

// Pin definitions
#define LITE_PIN_ONE 8
#define LITE_PIN_TWO 9
#define SOUND_PIN 4
#define BRIGHTNESS A0

// Counter definitions: light count, program count, mode count
#define NUMLITES 50
#define NUMPROGRAMS 5
const byte NUMMODES[NUMPROGRAMS+1] = {0,8,3,10,3,12};

#define wrap(vr,mx) ((vr+mx)%mx)

// Memory location definition, 0x0 through 0x3FF are valid
#define PROGMEM 0x0

// Global variable definitions
volatile int program;
volatile int mode;
volatile boolean update;
byte stats[NUMLITES][3]; 
byte bright;
Lite lites;

void setup()
{
	pinMode(SOUND_PIN, OUTPUT);
	//pinMode(BRIGHTNESS, INPUT);
	tone(SOUND_PIN,400);
	delay(2000);        
	noTone(SOUND_PIN);
	lites.init(LITE_PIN_ONE,NUMLITES);
	lites.mix(LITE_PIN_TWO);
	attachInterrupt(0,green,RISING);
	attachInterrupt(1,red,RISING);
	program = EEPROM.read(PROGMEM);
	if (program > NUMPROGRAMS)
	{
		program=1;
		EEPROM.write(PROGMEM,program);
	}
	mode = EEPROM.read(program);
	if (mode > NUMMODES[program])
	{
		mode=1;
		EEPROM.write(program,mode);
	}
}

void loop()
{
	//bright = map(analogRead(BRIGHTNESS),512,1023,0,204);
	//lites.set_intensity(bright);	
	switch(program){
		case 1: pgm_Constant(); break;
		case 2: pgm_Random(); break;
		case 3: pgm_Rainbow(); break;
		case 4: pgm_Fade(); break;
		case 5: pgm_Fire(); break;
	};
}

void pgm_Constant()
{
	if(update)
		switch(mode){
			case 1: update=false; for(int x=0;x<10;x++) lites.fill_color(0,0,0); break; //off
			case 2: update=false; for(int x=0;x<10;x++)  lites.fill_color(15,0,0); break; //red
			case 3: update=false; for(int x=0;x<10;x++)  lites.fill_color(0,15,0); break; //green
			case 4: update=false; for(int x=0;x<10;x++)  lites.fill_color(0,0,15); break; //blue
			case 5: update=false; for(int x=0;x<10;x++)  lites.fill_color(0,15,15); break; //cyan
			case 6: update=false; for(int x=0;x<10;x++)  lites.fill_color(15,0,15); break; //magenta
			case 7: update=false; for(int x=0;x<10;x++)  lites.fill_color(15,15,0); break; //yellow
			case 8: update=false; for(int x=0;x<10;x++)  lites.fill_color(15,15,15); break; //white
		}
}

void pgm_Random()
{
	byte w, x, y, z;
	switch(mode){
			case 1: update=false; w=0; x=3; y=16; z=0; break; // 1/3 off, 2/3 random
			case 2: update=false;  w=15; x=10; y=16; z=0;break; // 90% random, 10% 15
			case 3: update=false;  w=1; x=2; y=1; z=15; break; // half 1, half 15
		}
 	for(int i=0;i<NUMLITES;i++)
 		lites.set_color(random(NUMLITES),random(x)?random(y)+z:w,random(x)?random(y)+z:w,random(x)?random(y)+z:w);
}

void hue(byte h, byte n)
{
	if(h>=90) return;
	byte r = h % 15;
	byte z = (h-r)/15;
	switch(z){
		case 0: lites.set_color(n,15,r,0); break;
		case 1: lites.set_color(n,15-r,15,0); break;
		case 2: lites.set_color(n,0,15,r); break;
		case 3: lites.set_color(n,0,15-r,15); break;
		case 4: lites.set_color(n,r,0,15); break;
		case 5: lites.set_color(n,15,0,15-r); break;
	}
}

void pgm_Rainbow()
{
	byte last=0;
	if(update)
	{
		update = false;
		if(mode & 1)
		{	for(int x=0;x<NUMLITES;x++)
				stats[x][0] = 0;	
		}
		else
		{	
			last = wrap(last + ((mode+1)/2), 90);
			for(int x=0;x<NUMLITES;x++)
			{
                            stats[x][0] = last + ((mode+1)/2);
                            last = wrap(last + ((mode+1)/2), 90);
                        }
		}
	}
	for(byte y=0;y<NUMLITES;y++)
	{
		stats[y][0] = wrap(stats[y][0]+((mode+1)/2), 90);
		hue(stats[y][0], y);
	}
		
		
	
		
}

void green()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 150)
  {
		tone(SOUND_PIN,2000,60);
		if(++mode > NUMMODES[program])
			mode=1;
		update = true;
	}
  last_interrupt_time = interrupt_time;	
}

void red()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 150)
  {
		tone(SOUND_PIN,1000,60);
		if(++program > NUMPROGRAMS)
			program = 1;
		mode = EEPROM.read(program);
		if (mode > NUMMODES[program])
		{
			mode=1;
			EEPROM.write(program,mode);
		}
		update = true;
  }
  last_interrupt_time = interrupt_time;	
}

void pgm_Fade()
{
	int pos[3];
	int dir[3];
	int cap, bounce;
	boolean sub;
	if(update)
	{
		switch(mode){
			case 1: cap = 15; bounce=0; sub=false; break;
			case 2: cap = 15; bounce=10; sub=false; break;
			case 3: cap = 15; bounce=2; sub=false; break;
	
		}
		for(int x=0;x<NUMLITES;x++)
			for(int y=0;y<3;y++)
				stats[x][y] = 0;	
		for(int y=0;y<3;y++) 
 		{
 			random(2)?dir[y]=1:dir[y]=-1;	//randomly decide what direction each color moves in
 			pos[y] = random(NUMLITES);	//start each color at a random location
 			stats[pos[y]][y] = cap;		//set the color to full
 		}
	}
	for(int x=0;x<NUMLITES;x++)
	{
	 	int top = max(max(stats[x][0],stats[x][1]),stats[x][2]);
	 	int bot = min(min(stats[x][0],stats[x][1]),stats[x][2]);
	 	if(top<=1) continue;
	 	for(int y=0;y<3;y++)	
	 		if(stats[x][y]) 
	 			stats[x][y]--;
	 	if(sub)
	 		lites.set_color(x,15-stats[x][0],15-stats[x][1],15-stats[x][2]);
	 	else{
	 		if(!bot) 
	 			lites.set_color(x,cap-(top-stats[x][0]),cap-(top-stats[x][1]),cap-(top-stats[x][2]));
	 		else
	 			lites.set_color(x,stats[x][0],stats[x][1],stats[x][2]);
	 		}
	}
	for(int y=0;y<3;y++)
	{
		pos[y] = wrap(pos[y]+dir[y],NUMLITES);
		if((pos[y] == 0 || pos[y] == (NUMLITES-1)) && !random(bounce))
			dir[y]*=-1;
		stats[dir[y]][y]=cap;
		if (y==0)	lites.set_color(pos[y],15,0,0);
		if (y==1)	lites.set_color(pos[y],0,15,0);
		if (y==2)	lites.set_color(pos[y],0,0,15);
	}
}

void pgm_Fire()
{
	int heat;
	uint8_t min, avg, y;
	switch(mode){
		case 1: min=1; avg=4; break;
		case 7: min=1; avg=4; break;
		case 2: min=2; avg=6; break;
		case 8: min=2; avg=6; break;
		case 3: min=3; avg=8; break;
		case 9: min=3; avg=8; break;
		case 4: min=5; avg=10; break;
		case 10: min=5; avg=10; break;
		case 5: min=7; avg=12; break;
		case 11: min=7; avg=12; break;
		case 6: min=9; avg=15; break;
		case 12: min=9; avg=15; break;
	}
	if(update)
	{
		update=false;
		for(int x=0;x<NUMLITES;x++)
				stats[x][0]=min;
		for(int x=0;x<10;x++) lites.fill_color(min,0,0);
	}
	
	for(int x=0;x<NUMLITES;x++)
	{
		y=random(NUMLITES);
		if(random(avg)+random(avg)+3>=stats[y][0])		
		{
			stats[y][0]+=random(avg);
			stats[constrain(y-1,0,49)][0]++;
			stats[constrain(y+1,0,49)][0]++;
		}else{
			stats[y][0]-=(random(avg)/2);
		}
		if(mode>6)
			lites.set_color(y,constrain(stats[y][0],min,15),constrain(int(stats[y][0])-15,0,15),constrain(int(stats[y][0])-30,0,15));	
		else
			lites.set_color(y,constrain(int(stats[y][0])-30,0,15),constrain(int(stats[y][0])-15,0,15),constrain(stats[y][0],min,15));				
	}
}	
	
	
