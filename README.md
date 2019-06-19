# max7456 minimOSD based crosshair

early stage, basic functions works

max7456 library -> https://www.tinyosshop.com/arduino-osd-shield

for compatibility with minimosd board change 10 to 6 in line: const byte osdChipSelect =10;


### /////////uploading fonts////////////////
1. Download zip file from http://www.mwosd.com/ 
2. Unzip Go to MW_OSD folder Delete or rename fontB.h 
3. Paste siatka2.h and rename it to fontB.h 
4. Open MW_OSD.ino 
5. Go to config page Choose board by uncommenting correct line (minimosd or microminimosd) 
6. Find line "//define LOADFONT_BOLD" and uncomment it 
7. Write sketch to your board This may take 2-3 minutes DO NOT unplug power
8. If your board is conneted to lcd/tv you should see all characters on screen Now You can write crosshair code to board.


### /////////removing voltage divider////////// 
To connect 5 way joystick switch You need to remove voltage divider from line A0->BAT2. 
* You will need any soldering gear and multimeter.
1. First find resistor 15kOhm connected with BAT2 output 
2. Next find resistor 1kOhm connected to 15kOhm 
3. You just finded Check if other side of 1kOhm resistor is connected to GND. 
4. If yes, You got the right ones. 
5. Now desolder both of them 
6. Finaly bridge soldering pads, where was 15k resistor, with solder drop or small piece of wire where was 15k resistor 
7. Check if BAT2 output has electrical connection with PC0 input (google "atmega328tqfp pinout") 
 
 Congratulation You did it.
 
 
 #### ////////directions on my joystick are wrong//////////
 1. Write down on paper what directions You get and what You should get
 2. Open test_menu.ino
 3. Find line const uint16_t adc_kx[5] PROGMEM = {190, 367, 533, 687, 833 };
 4. Values from left to right are responsible for directions: Right, Up, Down, Left, Select
 5. Change order acording to your notes
 6. Save and write code to board
