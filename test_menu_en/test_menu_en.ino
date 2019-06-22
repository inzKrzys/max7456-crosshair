
  #include <SPI.h>
  #include <MAX7456.h>
  #include <EEPROM.h>
  #include <stdlib.h> 

  const byte osdChipSelect             =            6; 
  const byte masterOutSlaveIn          =                      MOSI;
  const byte masterInSlaveOut          =                      MISO;
  const byte slaveClock                =                      SCK;
  const byte osdReset                  =            0;

// Global Constants ////////////////////////////////////////////////////////////

  const unsigned long debugBaud = 9600;         // Initial serial baud rate for 
                                                //   Debug PC interface

// Global Variables ////////////////////////////////////////////////////////////
  
  HardwareSerial Debug = Serial;                // Set debug connection
  MAX7456 OSD( osdChipSelect );
	

int analogPin = A0;             //BAT2 on board
//int analogPin = A1;             //for initial testing

uint8_t x;                            //co-ordinates for crosshair
uint8_t y;
int xoff;
int yoff;
uint8_t menu;                         //number of menu page
uint8_t menuItem;                     //number of menu item
uint8_t maxmenuItem=4;                //max menu item number including 0
bool kolor;                           //color of text, 0-white, 1-black
bool bat_stat = 0;                    //0-voltage in V, 1-capacity in %
uint16_t bat_icon;

#define ADC_RANGE 50
uint8_t keys[5];
const uint16_t adc_kx[5] PROGMEM = {190, 367, 533, 687, 833 };
//////////////////////// Keys      Right,Up, Down, Left, Select 

int kv, wzr; //variable for keys


uint16_t wynik; 
uint8_t BAT1 = A2;
uint8_t i_cel = 0;      //number of battery serial cells
uint32_t srednia = 0;   //average voltage
uint32_t napiecie;
uint8_t dt = 5;         //number of measurments
char cz_dz[3];          //decimal part of voltage
char cz_u[3];           //fractional part of voltage
uint16_t min_cel = 315; //minimum cell voltage *100, this is when low battery alarm triggers
uint32_t alarm;  
uint16_t min_v = 300;   //empty cell voltage (3,0V)
uint16_t max_v = 420;   //full cell voltage (4,2V)
uint8_t proc;
uint32_t proc_v;
// Hardware Setup //////////////////////////////////////////////////////////////

  void setup() 
  {
	  
	  
	  analogReference(INTERNAL);
    unsigned char system_video_in=NULL;  
    // Initialize the Serial connection to the debug computer:
    Debug.begin( debugBaud );
    
    
    // Initialize the SPI connection:
    SPI.begin();
    SPI.setClockDivider( SPI_CLOCK_DIV2 );      // Must be less than 10MHz.
    
    // Initialize the MAX7456 OSD:
    OSD.begin();                                // Use PAL with default area.
    OSD.setSwitchingTime( 5 );       
    
    system_video_in=OSD.videoSystem();
    if(NULL!=system_video_in)
    {
        OSD.setDefaultSystem(system_video_in) ;
    }
    else
    {
        OSD.setDefaultSystem(MAX7456_PAL) ;
    }
    
    OSD.display();                              // Activate the text display.
	  OSD.setCharEncoding(MAX7456_ASCII);         // Use non-decoded access.
    OSD.noLineWrap();                           // Set wrap behaviour.
    OSD.noPageWrap();

    x = EEPROM.read(1);	                        //write eeprom value to variables
    y = EEPROM.read(2);  
    xoff = EEPROM.read(3);
    yoff = EEPROM.read(4);
    
    
    Serial.begin(9600);


    menu=0;
    menuItem=0;
    kolor=EEPROM.read(5);
    bat_stat=EEPROM.read(7);
    i_cel=EEPROM.read(6);
  }
 
// Main Code ///////////////////////////////////////////////////////////////////

  void loop() 
  {
    
    while (OSD.notInVSync());           // Wait for VSync to start to
    if (kolor==1){OSD.invertColor();}   //0-white 1-black
    else {OSD.normalColor();}
    OSD.setTextOffset(xoff, yoff);      //moving entire screen in 1 pixel increments
    
    switch(menu)
    {

      //////////normalna praca/////////////
      case 0:
      OSD.clear();
      CrossPrint();   
      
      do
        { 
          BatCheck();
          KeyCheck();
          if( keys[4]==4 ) menu=1;
        } while( menu ==0 );            //cheking button select
 
      break;
      
      //////////menu///////////////////////
      case 1:
        switch(menuItem)
        {
          case 0:                       //position
          OSD.clear();
          MenuPrint();
          OSD.setCursor(4,4);
          OSD.write(0x1A);              //cursor
          OSD.display();
          KeyCheck();
          if( keys[1]==4 ) menuItem = 4;  //up
          if( keys[2]==4 ) menuItem = 1 ; //down
          if( keys[4]==4 ) menu = 2;      //select
          break;

          case 1:                       //black
          OSD.clear();
          MenuPrint();
          OSD.setCursor(4,5);
          OSD.write(0x1A);
          OSD.display();
          KeyCheck();
          if( keys[1]==4 ) menuItem = 0 ;   //up
          if( keys[2]==4 ) menuItem = 2 ;   //down
          if( keys[4]==4 ) kolor = !kolor;  //select
          break;

          case 2:                           //number of serial cells
          OSD.clear();
          MenuPrint();
          OSD.setCursor(4,6);
          OSD.write(0x1A); 
          OSD.display();
          KeyCheck();
          i_cel=constrain(i_cel, 1, 4);
          if( keys[1]==4 ) menuItem = 1 ;   //up
          if( keys[2]==4 ) menuItem = 3 ;   //down
          if( keys[0]==4 ) ++i_cel;         //right
          if( keys[3]==4 ) --i_cel;         //left
          break;

          case 3: // display state of battery in V or %
          OSD.clear();
          MenuPrint();
          OSD.setCursor(4,7);
          OSD.write(0x1A);
          OSD.display();
          KeyCheck();
          if( keys[1]==4 ) menuItem = 2 ;           //up
          if( keys[2]==4 ) menuItem = 4 ;           //down
          if( keys[4]==4 ) bat_stat = !bat_stat;    //select        
          break;

          case 4: //exit
          OSD.clear();
          MenuPrint();
          OSD.setCursor(4,8);
          OSD.write(0x1A);
          OSD.display();
          KeyCheck();
          if( keys[1]==4 ) menuItem = 3 ;           //up
          if( keys[2]==4 ) menuItem = 0 ;           //down
          if( keys[4]==4 )                          //select
            {EEPROM.update(5, kolor);
             EEPROM.update(6, i_cel);
             EEPROM.update(7, bat_stat); 
             menu = 0;
            }
          break;
        }
        break;

      ///////////setting coarse position//////////
      case 2:
      OSD.clear();
      xyPrint();
      CrossPrint();
      KeyCheck();
      x=constrain(x, 2, 15);
      y=constrain(y, 1, 8);
      if( keys[0]==4 )++x; //right
      if( keys[1]==4 )--y; //up
      if( keys[2]==4 )++y; //down
      if( keys[3]==4 )--x; //left
      if( keys[4]==4 )     //select
        {
          EEPROM.update(1, x);
          EEPROM.update(2, y);
          menu = 3;  
        }
      
      break;

      //////////setting precise position//////////
      case 3:
      OSD.clear();
      xyOffPrint();
      CrossPrint();
      KeyCheck();
      xoff=constrain(xoff, -32, 31);  
      yoff=constrain(yoff, -15, 15);
      if( keys[0]==4 )++xoff; //right
      if( keys[1]==4 )--yoff; //up
      if( keys[2]==4 )++yoff; //down
      if( keys[3]==4 )--xoff; //left
      if( keys[4]==4 )        //select
        {
          EEPROM.update(3, xoff);
          EEPROM.update(4, yoff);
          menu = 1;  
        }
       OSD.setTextOffset(xoff, yoff);
      break;
      

    }


     
    
  /*if( keys[0]==4 ) Serial.println("right");
  if( keys[1]==4 ) Serial.println("up");
  if( keys[2]==4 ) Serial.println("down");
  if( keys[3]==4 ) Serial.println("left");
  if( keys[4]==4 ) Serial.println("select"); 

  //only for initial testing 
*/
  } 

//*********************************************************************
//BATTERY VOLTAGE
//*********************************************************************

void BatCheck() {
 
  OSD.setBlinkingTime(3);
  delay(1);
  wynik = analogRead(BAT1);
  
  srednia = srednia * dt;
  srednia = srednia + wynik;
  srednia = srednia / (dt+1);

  napiecie = ( srednia * 990UL ) / 579UL; 
  //napiecie = (rolling average from adc * middle voltage from range 1s-4s, in this case 9,9V) / adc value for 9,9V

  div_t divmod = div( napiecie, 100 );
  itoa( divmod.quot, cz_dz, 10 );
  itoa( divmod.rem, cz_u, 10 );
  if( divmod.rem <10 ) {
    cz_u[0]='0';
    cz_u[1]=divmod.rem+'0';
    }

  proc_v = napiecie;  
  if( (i_cel == 0) || (i_cel > 4) ){i_cel = 3;}               //if number of cells = 0, set 3 as default
  if(proc_v < (min_v * i_cel)) proc_v = (min_v * i_cel);
  if(proc_v > (max_v * i_cel)) proc_v = (max_v * i_cel);
  proc = (((proc_v - (min_v * i_cel)) * 100)/((max_v * i_cel) - (min_v * i_cel))); // calculation of battery %

  if(proc > 85){bat_icon = 0x0e;}
  else if( 85 >= proc && proc > 72 ){bat_icon = 0x0d;}
  else if( 72 >= proc && proc > 57 ){bat_icon = 0x0c;}
  else if( 57 >= proc && proc > 42 ){bat_icon = 0x0b;}
  else if( 42 >= proc && proc > 27 ){bat_icon = 0x0a;}
  else if( 27 >= proc && proc > 12 ){bat_icon = 0x09;}
  else if( 12 >= proc ){bat_icon = 0x08;}
  
  delay(1);
  alarm = ( min_cel * i_cel );
  if(alarm >= napiecie) OSD.blink();
  else OSD.noBlink();
 
  delay(1);
  OSD.setCursor(1,0);
  if(bat_stat == false)
    {
      OSD.write( bat_icon ); OSD.display();
      OSD.print( cz_dz );
      OSD.print( "." );
      OSD.print( cz_u );
      OSD.print("V ");   
      OSD.noBlink();
    }
   else 
    {
      OSD.write( bat_icon ); OSD.display();
      OSD.print(proc);
      OSD.print("%     ");  
      OSD.noBlink();
    }
}

  
//************************************************************************
//ANALOG KEYPAD
//************************************************************************

 void KeyCheck() {
	int i;
  
	kv=analogRead(analogPin);  
  for(i=0; i<5; i++) {
    wzr = pgm_read_word( &adc_kx[i] );
    if( kv > wzr-ADC_RANGE && kv < wzr+ADC_RANGE  ) {
      if( keys[i]<5 ) keys[i]++;
    } else keys[i]=0;
  }
 }


 
//************************************************************************
//CO-ORDINATES DISPLAY
//************************************************************************  
  
void xyPrint() {
delay(1);
OSD.setCursor(1,1);
OSD.print("X=");OSD.print(x);
OSD.setCursor(10,1);
OSD.print("Y=");OSD.print(y);	
}

void xyOffPrint() {
delay(1);
OSD.setCursor(1,1);
OSD.print("Xoff=");OSD.print(xoff);
OSD.setCursor(10,1);
OSD.print("Yoff=");OSD.print(yoff);  
}

//************************************************************************
//CROSSHAIR DRAWING
//************************************************************************

void CrossPrint() {
  int x4=x+6;
  int y1=y;
  delay(1);
    OSD.setCursor( x4, y );
    OSD.write(0xfd);                                            
    OSD.setCursor( x4, ++y1 );                                            
    OSD.write(0xfd);    
    OSD.setCursor( x, ++y1 );             
    OSD.write(0xe2);OSD.write(0xde);OSD.write(0xe5);OSD.write(0xfc);OSD.write(0xfc);OSD.write(0xfc);OSD.write(0xe9);OSD.write(0xfc);OSD.write(0xfc);OSD.write(0xfc);OSD.write(0xe4);OSD.write(0xde);OSD.write(0xe3);// "----+----" );                   
    OSD.setCursor( x4, ++y1 );                                            
    OSD.write(0xfd);
    OSD.setCursor( x4, ++y1 );                                            
    OSD.write(0xfd);
    OSD.setCursor( x4, ++y1 );                                            
    OSD.write(0xe1); 
    OSD.setCursor( x4, ++y1 );  
    OSD.write(0xdf);  
    OSD.display();
}

//*******************************************
//MENU DRAWING
//*******************************************
void MenuPrint() 
{
  delay(1);
  OSD.setCursor(5,3);
  OSD.print("MENU");
  OSD.setCursor(5,4);
  OSD.print("Position");
  OSD.setCursor(5,5);
  OSD.print("Black [");
  OSD.print(kolor);
  OSD.print("]");
  OSD.setCursor(5,6);
  OSD.print("Cells number ");
  OSD.print(i_cel);
  OSD.print("S");
  OSD.setCursor(5,7);
  OSD.print("Battery indicator");
  if(bat_stat == false) OSD.print(" [V]");
  else OSD.print(" [%]");
  OSD.setCursor(5,8);
  OSD.print("Exit");
}
