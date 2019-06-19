
  #include <SPI.h>
  #include <MAX7456.h>
  #include <EEPROM.h>
  #include <stdlib.h> 

  const byte osdChipSelect             =            6; //6 for micro minimosd board
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
	

int analogPin = A0;             //BAT2 na plytce
//int analogPin = A1;             //do testow

uint8_t x;                            //wspolrzedne
uint8_t y;
int xoff;
int yoff;
uint8_t menu;                         //nr strony menu
uint8_t menuItem;                     //nr pozycji menu
uint8_t maxmenuItem=4;                //ilosc pozycji menu liczac od 0
bool kolor;
bool bat_stat = 0;                        //0 napiecie w V, 1 pojemnosc w procentach
uint16_t bat_icon;

#define ADC_RANGE 50
uint8_t keys[5];
const uint16_t adc_kx[5] PROGMEM = {190, 367, 533, 687, 833 };
//////////////////////// Przyciski   P,   G,   D,   L,   S 

int kv, wzr; //zmienne do obslugi przyciskow


uint16_t wynik; 
uint8_t BAT1 = A2;
uint8_t i_cel = 0;
uint32_t srednia = 0;
uint32_t napiecie;
uint8_t dt = 5;         //ilosc przebiegow
char cz_dz[3];  //czesc dziesietna
char cz_u[3];  //czesc ulamkowa
uint16_t min_cel = 315; //napiecie celi *100 przy ktorym zalaczany jest alarm
uint32_t alarm;  
uint16_t min_v = 300; //napiecie pustej celi
uint16_t max_v = 420; //napiecie pelnej celi
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

    x = EEPROM.read(1);	//zapis wartosci z eeprom do zmiennej
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
    
    while (OSD.notInVSync()); // Wait for VSync to start to
    if (kolor==1){OSD.invertColor();} //0-bialy 1-czarny
    else {OSD.normalColor();}
    OSD.setTextOffset(xoff, yoff); //przesuniecie wszystkiego o pojedyncze pixele
    
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
        } while( menu ==0 ); //petla sprawdza przycisk 4 az zostanie wcisniety
 
      break;
      
      //////////menu///////////////////////
      case 1:
        switch(menuItem)
        {
          case 0: //pozycja
          OSD.clear();
          MenuPrint();
          OSD.setCursor(4,4);
          OSD.write(0x1A);
          OSD.display();
          //OSD.print(">");  //kursor
          KeyCheck();
          if( keys[1]==4 ) menuItem = 4;//gora
          if( keys[2]==4 ) menuItem = 1 ;//dol
          if( keys[4]==4 ) menu = 2;//srodek
          break;

          case 1: //czarny
          OSD.clear();
          MenuPrint();
          OSD.setCursor(4,5);
          OSD.write(0x1A);
          OSD.display();
          KeyCheck();
          if( keys[1]==4 ) menuItem = 0 ;//gora
          if( keys[2]==4 ) menuItem = 2 ;//dol
          if( keys[4]==4 ) kolor = !kolor;//srodek
          break;

          case 2: //ilosc cel
          OSD.clear();
          MenuPrint();
          OSD.setCursor(4,6);
          OSD.write(0x1A); 
          OSD.display();
          KeyCheck();
          i_cel=constrain(i_cel, 1, 4);
          if( keys[1]==4 ) menuItem = 1 ;//gora
          if( keys[2]==4 ) menuItem = 3 ;//dol
          if( keys[0]==4 ) ++i_cel;//prawo
          if( keys[3]==4 ) --i_cel;//lewo
          break;

          case 3: // wyswietlanie stanu baterii V lub %
          OSD.clear();
          MenuPrint();
          OSD.setCursor(4,7);
          OSD.write(0x1A);
          OSD.display();
          KeyCheck();
          if( keys[1]==4 ) menuItem = 2 ;//gora
          if( keys[2]==4 ) menuItem = 4 ;//dol
          if( keys[4]==4 ) bat_stat = !bat_stat;//srodek          
          break;

          case 4: //wyjscie
          OSD.clear();
          MenuPrint();
          OSD.setCursor(4,8);
          OSD.write(0x1A);
          OSD.display();
          KeyCheck();
          if( keys[1]==4 ) menuItem = 3 ;//gora
          if( keys[2]==4 ) menuItem = 0 ;//dol
          if( keys[4]==4 ) 
            {EEPROM.update(5, kolor);
             EEPROM.update(6, i_cel);
             EEPROM.update(7, bat_stat); 
             menu = 0;
            }//srodek
          break;
        }
        break;

      ///////////ustawienie pozycji zgrubnej//////////
      case 2:
      OSD.clear();
      xyPrint();
      CrossPrint();
      KeyCheck();
      x=constrain(x, 2, 15);
      y=constrain(y, 1, 8);
      if( keys[0]==4 )++x; //prawo
      if( keys[1]==4 )--y; //gora
      if( keys[2]==4 )++y; //dol
      if( keys[3]==4 )--x; //lewo
      if( keys[4]==4 )//srodek
        {
          EEPROM.update(1, x);
          EEPROM.update(2, y);
          menu = 3;  
        }
      
      break;

      //////////ustawienie pozycji dokladnej//////////
      case 3:
      OSD.clear();
      xyOffPrint();
      CrossPrint();
      KeyCheck();
      xoff=constrain(xoff, -32, 31);  //sprawdzic max wartosci
      yoff=constrain(yoff, -15, 15);
      if( keys[0]==4 )++xoff; //prawo
      if( keys[1]==4 )--yoff; //gora
      if( keys[2]==4 )++yoff; //dol
      if( keys[3]==4 )--xoff; //lewo
      if( keys[4]==4 )//srodek
        {
          EEPROM.update(3, xoff);
          EEPROM.update(4, yoff);
          menu = 1;  
        }
       OSD.setTextOffset(xoff, yoff);
      break;
      

    }


     
    
  /*if( keys[0]==4 ) Serial.println("prawy");
  if( keys[1]==4 ) Serial.println("gora");
  if( keys[2]==4 ) Serial.println("dol");
  if( keys[3]==4 ) Serial.println("lewy");
  if( keys[4]==4 ) Serial.println("srodek"); 
*/
  } 

//*********************************************************************
//NAPIECIE BATERII
//*********************************************************************

void BatCheck() {
 
  OSD.setBlinkingTime(3);
  delay(1);
  wynik = analogRead(BAT1);
  
  srednia = srednia * dt;
  srednia = srednia + wynik;
  srednia = srednia / (dt+1);

  napiecie = ( srednia * 990UL ) / 579UL; 
  //napiecie = (srednia kroczaca z adc * srodkowe napiecie z zakresu 1s-4s, czyli 9,9V) / odczytana wartosc adc dla 9,9V

  div_t divmod = div( napiecie, 100 );
  itoa( divmod.quot, cz_dz, 10 );
  itoa( divmod.rem, cz_u, 10 );
  if( divmod.rem <10 ) {
    cz_u[0]='0';
    cz_u[1]=divmod.rem+'0';
    }

  proc_v = napiecie;  
  if( (i_cel == 0) || (i_cel > 4) ){i_cel = 3;} //jesli ilosc cel 0 to ustaw domyslnie 3
  if(proc_v < (min_v * i_cel)) proc_v = (min_v * i_cel);
  if(proc_v > (max_v * i_cel)) proc_v = (max_v * i_cel);
  proc = (((proc_v - (min_v * i_cel)) * 100)/((max_v * i_cel) - (min_v * i_cel))); //obliczenie procent baterii

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
      OSD.print("V ");   //spacja po V jest potrzebna do kasowania smieci przy przejsciu z wyniku 4 znakowego na 3 znakowy
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
//KLAWIATURA ANALOGOWA
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
//WYSWIETLANIE WSPOLRZEDNYCH
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
//RYSOWANIE KRZYZA
//************************************************************************

void CrossPrint() {
  int x4=x+6;
  int y1=y;
  delay(1);
    OSD.setCursor( x4, y );
    OSD.write(0xfd);                                            
    //OSD.print( "/" );  
    OSD.setCursor( x4, ++y1 );                                            
    OSD.write(0xfd);    
    OSD.setCursor( x, ++y1 );             
    //OSD.write(0xe2 0xe5 0xfc 0xfc 0xfc 0xe9 0xfc 0xfc 0xfc 0xe4 0xe3);
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
//RYSOWANIE MENU
//*******************************************
void MenuPrint() 
{
  delay(1);
  OSD.setCursor(5,3);
  OSD.print("MENU");
  OSD.setCursor(5,4);
  OSD.print("Pozycja");
  OSD.setCursor(5,5);
  OSD.print("Czarny [");
  OSD.print(kolor);
  OSD.print("]");
  OSD.setCursor(5,6);
  OSD.print("Ilosc Cel ");
  OSD.print(i_cel);
  OSD.print("S");
  OSD.setCursor(5,7);
  OSD.print("Stan Baterii");
  if(bat_stat == false) OSD.print(" [V]");
  else OSD.print(" [%]");
  OSD.setCursor(5,8);
  OSD.print("Wyjscie");
}
