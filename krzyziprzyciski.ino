
// Included Libraries //////////////////////////////////////////////////////////

  #include <SPI.h>
  #include <MAX7456.h>
  #include <EEPROM.h>




// Pin Mapping /////////////////////////////////////////////////////////////////
  
  // pinValue = 0 means "not connected"

  //  FDTI Basic 5V                   ---  Arduino  VCC      (AVCC,VCC)
  //  FDTI Basic GND                  ---  Arduino  GND      (AGND,GND)
  //  FDTI Basic CTS                  ---  Arduino  GND      (AGND,GND)
  //  FDTI Basic DTR                  ---  Arduino  GRN
  //  FDTI Basic TXO                  ---> Arduino  TXO [PD0](RXD)
  //  FDTI Basic RXI                 <---  Arduino  RXI [PD1](TXD)
  
  
  //  Max7456 +5V   [DVDD,AVDD,PVDD]  ---  Arduino  VCC      (AVCC,VCC)
  //  Max7456 GND   [DGND,AGND,PGND]  ---  Arduino  GND      (AGND,GND)
  //  Max7456 CS    [~CS]            <---  Arduino  10  [PB2](SS/OC1B)
  //  Max7456 CS    [~CS]            <---  Mega2560 43  [PL6]
  const byte osdChipSelect             =            6; //6 for micro minimosd board
  //  Max7456 DIN   [SDIN]           <---  Arduino  11  [PB3](MOSI/OC2)
  //  Max7456 DIN   [SDIN]           <---  Mega2560 51  [PB2](MOSI)
  const byte masterOutSlaveIn          =                      MOSI;
  //  Max7456 DOUT  [SDOUT]           ---> Arduino  12  [PB4](MISO)
  //  Max7456 DOUT  [SDOUT]           ---> Mega2560 50  [PB3](MISO)
  const byte masterInSlaveOut          =                      MISO;
  //  Max7456 SCK   [SCLK]           <---  Arduino  13  [PB5](SCK)
  //  Max7456 SCK   [SCLK]           <---  Mega2560 52  [PB1](SCK)
  const byte slaveClock                =                      SCK;
  //  Max7456 RST   [~RESET]          ---  Arduino  RST      (RESET)
  const byte osdReset                  =            0;
  //  Max7456 VSYNC [~VSYNC]          -X-
  //  Max7456 HSYNC [~HSYNC]          -X-
  //  Max7456 LOS   [LOS]             -X-





// Global Macros ///////////////////////////////////////////////////////////////





// Global Constants ////////////////////////////////////////////////////////////

  const unsigned long debugBaud = 9600;         // Initial serial baud rate for 
                                                //   Debug PC interface
  




// Global Variables ////////////////////////////////////////////////////////////
  
  HardwareSerial Debug = Serial;                // Set debug connection
  
  MAX7456 OSD( osdChipSelect );
	

#define key_left 10
#define key_right 9
#define key_up 8
#define key_down 7
#define key_mode 5
int x;                            //wspolrzedne
int y;
boolean ustawienia=0;             //flaga ustawien

// Hardware Setup //////////////////////////////////////////////////////////////

  void setup() 
  {
    unsigned char system_video_in=NULL;  
    // Initialize the Serial connection to the debug computer:
    Debug.begin( debugBaud );
    
    
    // Initialize the SPI connection:
    SPI.begin();
    SPI.setClockDivider( SPI_CLOCK_DIV2 );      // Must be less than 10MHz.
    
    // Initialize the MAX7456 OSD:
    OSD.begin();                                // Use NTSC with default area.
    OSD.setSwitchingTime( 5 );       
    
    system_video_in=OSD.videoSystem();
    if(NULL!=system_video_in)
    {
        OSD.setDefaultSystem(system_video_in) ;
    }
    else
    {
        OSD.setDefaultSystem(MAX7456_NTSC) ;
    }
    
    OSD.display();                              // Activate the text display.
	  OSD.setCharEncoding(MAX7456_ASCII);         // Use non-decoded access.
    OSD.noLineWrap();                           // Set wrap behaviour.
    OSD.noPageWrap();

    x = EEPROM.read(1);                //zapis do zmiennej wartosci z eeprom
    y = EEPROM.read(2);  
    //Serial.begin(9600);
    pinMode(key_left, INPUT_PULLUP);
    pinMode(key_right, INPUT_PULLUP);
    pinMode(key_up, INPUT_PULLUP);
    pinMode(key_down, INPUT_PULLUP);
    pinMode(key_mode, INPUT_PULLUP);

  }
  // setup()





// Main Code ///////////////////////////////////////////////////////////////////

  void loop() 
  {
    
    while (OSD.notInVSync());                   // Wait for VSync to start to 
      CrossPrint();                                          
    //while (true);
    if (digitalRead(key_mode) == LOW) 
    {
		ustawienia ^= 1;  //zmiana stanu flagi ustawien
		//Serial.println("key_mode");         // do testow w terminalu
		EEPROM.update(1, x);                  //zapis do eeprom wartosci zmiennej
		EEPROM.update(2, y);
		while (digitalRead(key_mode) == LOW) { delay(100);}
    }
    if ((digitalRead(key_left) == LOW) && (ustawienia==1))  //jesli stan na key_left niski i flaga ustawien = 1 (wcisniety)
    {
		//Serial.println("key_left");      //wyswietl "key_left"
		x-=1;                            //odejmij 1 i zapisz wynik do zmiennej x
		while (OSD.notInVSync());
		OSD.clear();
		CrossPrint();               //wyswietl wartosc zmiennej x
		while (digitalRead(key_left) == LOW) { delay(100);} //dopuki key_left wcisniety pozostan w petli bezczynnej. dzieki temu jedno wcisniecie = zmiana x o jedna jednostke
    }
    if ((digitalRead(key_right) == LOW) && (ustawienia==1))
    {
		//Serial.println("key_right");
		x+=1;
		while (OSD.notInVSync());
		OSD.clear();
		CrossPrint();
		while (digitalRead(key_right) == LOW) { delay(100);}
    }
    if ((digitalRead(key_up) == LOW) && (ustawienia==1)) 
    {
		//Serial.println("key_up");
		y-=1;
		while (OSD.notInVSync());
		OSD.clear();
		CrossPrint();
		while (digitalRead(key_up) == LOW) { delay(100);}
    }
    if ((digitalRead(key_down) == LOW) && (ustawienia==1)) 
    {
		//Serial.println("key_down");
		y+=1;
		while (OSD.notInVSync());
		OSD.clear();
		CrossPrint();
		while (digitalRead(key_down) == LOW) { delay(100);}
    }
  
			  
		
  } 

void CrossPrint()
{
    
	  OSD.setCursor( (x+4), y );                                            
    OSD.print( "/" );  
    OSD.setCursor( (x+4), (y+1) );                                            
    OSD.print( "/" );    
    OSD.setCursor( x, (y+2) );             
    OSD.print( "----+----" );                   
    OSD.setCursor( (x+4), (y+3) );                                            
    OSD.print( "/" );
    OSD.setCursor( (x+4), (y+4) );                                            
    OSD.print( "/" );  	
}
