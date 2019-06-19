#early stage, basic functions works

max7456- osd crosshair

max7456 library -> https://www.tinyosshop.com/arduino-osd-shield

for compatibility with minimosd board change 10 to 6 in line: const byte osdChipSelect    =10;


/////////uploading fonts////////////////
Download zip file from http://www.mwosd.com/
Unzip
Go to MW_OSD folder
Delete or rename fontB.h
Paste siatka2.h and rename it to fontB.h
Open MW_OSD.ino 
Go to config page
Choose board by uncommenting correct line (minimosd or microminimosd)
Find line "//define LOADFONT_BOLD" and uncomment it
Write sketch to your board
This may take 2-3 minutes DO NOT unplug power
If your board is conneted to lcd/tv you should see all characters on screen
Now You can write crosshair code to board.

/////////removing voltage divider//////////
To connect 5 way joystick switch You need to remove voltage divider from line A0->BAT2.
You will need any soldering gear and multimeter.
First find resistor 15kOhm connected with BAT2 output
Next find resistor 1kOhm connected to 15kOhm You just finded
Check if other side of 1kOhm resistor is connected to GND. If yes, You got the right ones.
Now desolder both of them
Finaly bridge soldering pads, where was 15k resistor, with solder drop or small piece of wire where was 15k resistor
Check if BAT2 output has electrical connection with PC0 input (google "atmega328tqfp pinout")
Congratulation You did it. 
