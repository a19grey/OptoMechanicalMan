#include "Adafruit_WS2801.h"
#include "SPI.h" // Comment out this line if using Trinket or Gemma
#ifdef __AVR_ATtiny85__
 #include <avr/power.h>
#endif

/*****************************************************************************
  For explanation of some of  the low-level functions, see the example sketch for driving Adafruit WS2801 pixels!
  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
*****************************************************************************/

/* Choose which 2 pins you will use for output.
 Can be any valid output pins.
 The colors of the wires may be totally different so
 BE SURE TO CHECK YOUR PIXELS TO SEE WHICH WIRES TO USE! */
 // Note that on some systems the 0 & 1 pins are reserved for communication so don't use these.
uint8_t dataPin  = 2;    // Yellow wire on Adafruit WS2801 Pixels
uint8_t clockPin = 3;    // Green wire on Adafruit WS2801 Pixels
uint8_t dataPin2  = 4;   // Yellow wire on Adafruit WS2801 Pixels
uint8_t clockPin2 = 5;   // Green wire on Adafruit WS2801 Pixels

// Don't forget to connect the ground wire to Arduino ground,
// and the +5V wire to a +5V supply

// Set the first variable to the NUMBER of pixels. 25 = 25 pixels in a row
uint8_t numpixels = 25;
const int numpixels_c = 25;
Adafruit_WS2801 strip = Adafruit_WS2801(numpixels, dataPin, clockPin);
Adafruit_WS2801 strip2 = Adafruit_WS2801(numpixels, dataPin2, clockPin2);

////////////////////////////////////////////////////////////////////////
// Initialize variables for later use:
uint8_t setcolor_r = 50;
uint8_t setcolor_g = 200;
uint8_t setcolor_b = 50;

// These variables set which analog inputs to read in the X,Y,Z acceleration data 
const int xInput = A0; const int x2Input = A3;
const int yInput = A1; const int y2Input = A4;
const int zInput = A2; const int z2Input = A5;

//The raw analog input signal received from the accelerometer
int xRaw; int yRaw; int zRaw;
int x2Raw; int y2Raw; int z2Raw;

//The converted 0-255 integer signal used for a color
int x255; int y255; int z255; 
int x2255; int y2255; int z2255;

//These variables are used when finding the largest/smallest of the three X,Y,Z values returned from each accelerometer
int mostmin; int mostmax; int mostmin2; int mostmax2;

//These variabls store the color state of all pixels in the RGB string.
int stringstate[numpixels_c][3]; int stringstate2[numpixels_c][3];

//The number range of the color space -- typically 0-255
int max_number = 255; int minimum_number = 0;

// Take multiple analog data samples to reduce noise
const int sampleSize = 5;
////////////////////////////////////////////////////////////////////////


void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  // Initialize the LED strips. 
  strip.begin(); strip2.begin();

  // Update LED contents, to start they are all 'off'
  strip.show(); strip2.show();
  
  //Use external analog reference when using the ADXL sensor for inputs
  analogReference(EXTERNAL);
  Serial.begin(9600);
}


void loop() {
  // Read in the axis values
  xRaw = ReadAxis(xInput); x2Raw = ReadAxis(x2Input);
  yRaw = ReadAxis(yInput); y2Raw = ReadAxis(y2Input);
  zRaw = ReadAxis(zInput); z2Raw = ReadAxis(z2Input);

  //Convert the axis values where the mid-point of the range is actually 0 acceleration therefore after adjusting the range to 0-255 I subtract '127'
  //Ideally, this '127 == 0' value would be calibrated for each sensor axis, but I haven't bothered :)
  //Then I take the absolute value because I don't care if it's up or down motion in that direction (negative colors don't exist... yet)
  x255 = abs(map(xRaw,0,1023,0,255)-127); x2255 = abs(map(x2Raw,0,1023,0,255)-127);
  y255 = abs(map(yRaw,0,1023,0,255)-127); y2255 = abs(map(y2Raw,0,1023,0,255)-127);
  z255 = abs(map(zRaw,0,1023,0,255)-127); z2255 = abs(map(z2Raw,0,1023,0,255)-127);

  //A variable to define which pixels to set to the value of the ADXL inputs. My C programming skills are low...
  //Depending on the application you may wish to launch the 'pulse' of light from the end of the string (my case) or from the beginning
  uint8_t position1 = numpixels_c;
  uint8_t position2 = numpixels_c-1;
  
  /* Now here's the algorithm magic! You could just set R=X, G=Y, and B=Z, but first, I noticed that then the lights are never off because there's always some noise
   and errors in calibration. Second, fucking gravity is always on, so whichever way gravity is pointing, the string always lights up. 
   So, there's like an infinite number of ways to map accelerations to colors, but I've chosen a simple start.
   1. If the accelerations are big enough that they swamp gravity, just show the raw color (R=X, G=Y, B=Z)
   2. If the accelerations are moderate-values, ignore the smallest-value color (basically treating motions as in a 2D plane).
      -- when I didn't ignore the smallest value I got a lot of 'muddy' colors that weren't very pretty.
   3. If the accelerations are too small, launch a pulse of lights set to 'off' */
  if ((x255 + y255 + z255) > 500){
   //implements condition #1 above
   setcolor_r = x255;
   setcolor_g = y255;
   setcolor_b = z255;
  }else if ((x255 + y255 + z255) > 50){
   //implements condition #2 above
   mostmin = min(min(x255,y255),z255);
   setcolor_r = (mostmin==x255)?0:x255;
   setcolor_g = (mostmin==y255)?0:y255;
   setcolor_b = (mostmin==z255)?0:z255;
  } else{
   //implements condition #3 above
   setcolor_r = 0; setcolor_g = 0; setcolor_b = 0;
  }

  // Set first two LED values to the chosen color
  stringstate[position1][0] = setcolor_r;
  stringstate[position1][1] = setcolor_g;
  stringstate[position1][2] = setcolor_b;
  stringstate[position2][0] = setcolor_r;
  stringstate[position2][1] = setcolor_g;
  stringstate[position2][2] = setcolor_b;

// Repeat all the same math/calcularions for the second set of LEDs connected (typically two arms or two legs)
if ((x2255 + y2255 + z2255) > 500){
   setcolor_r = x2255;
   setcolor_g = y2255;
   setcolor_b = z2255;
  }else if ((x2255 + y2255 + z2255) > 50){
   mostmin2 = min(min(x2255,y2255),z2255);
   setcolor_r = (mostmin2==x2255)?0:x2255;
   setcolor_g = (mostmin2==y2255)?0:y2255;
   setcolor_b = (mostmin2==z2255)?0:z2255;
  } else{
   setcolor_r = 0; setcolor_g = 0; setcolor_b = 0;
   //mostmax = max(max(x255,y255),z255);
   //setcolor_r = (mostmax==x255)?0:x255/10;
   //setcolor_g = (mostmax==y255)?0:y255/10;
   //setcolor_b = (mostmax==z255)?0:z255/10;
  }
// Set first two values to the chosen color
  stringstate2[position1][0] = setcolor_r;
  stringstate2[position1][1] = setcolor_g;
  stringstate2[position1][2] = setcolor_b;
  stringstate2[position2][0] = setcolor_r;
  stringstate2[position2][1] = setcolor_g;
  stringstate2[position2][2] = setcolor_b;

  // Set LED arrays to the proper color state
  // The past lines of code just calculated the colors to set. These lines actually tell the LED array to light up according to the RGB values saved in StringState
  SetColorState(stringstate); SetColorState2(stringstate2);

  /* Shift array 'left' or from the end (last pixel) towards the first pixel
  Basically, move the lighting pattern one position from the end of the array to the front of the array
  These are the lines that tell the lights to move 'up' the arm/leg. 
  If you used shift_right here, the lights would travel from the back down towards the hands/legs */
  shift_left(stringstate,numpixels_c);
  shift_left(stringstate,numpixels_c); // Move one more position left
  shift_left(stringstate2,numpixels_c); //Repeat the same shifting on the second string
  shift_left(stringstate2,numpixels_c);
  
  delay(100); //Pause briefly to limit the speed at which the light travels up the string. Without this pause, things move way too quickly and the light pattern is confusing.
}

void SetColorState(int stringstate[][3]) {
  int ii;
    for (ii=0; ii<numpixels; ii++) {
        strip.setPixelColor(ii,Color(stringstate[ii][0],stringstate[ii][1],stringstate[ii][2]));  
    }
    strip.show();
}

void SetColorState2(int stringstate[][3]) {
  int ii;
    for (ii=0; ii<numpixels; ii++) {
        strip2.setPixelColor(ii,Color(stringstate[ii][0],stringstate[ii][1],stringstate[ii][2]));  
    }
    strip2.show();
}

// Read in analog values for a number of times (sampleSize) and return the averaged result
int ReadAxis(int axisPin)
{
  long reading = 0;
  analogRead(axisPin);
  for (int i = 0; i < sampleSize; i++)
  {
    reading += analogRead(axisPin);
  }
  return reading/sampleSize;
}

// Launch a single pulse of light along the LED string, with the given RGB color and wait time between the dot of light moving.
// Written for testing purposes.
void colorlaunch(uint8_t wait,uint8_t setcolor_r,uint8_t setcolor_g,uint8_t setcolor_b) {
  int ii,jj;
 
  for (jj=0; jj<25; jj++) {
    for (ii=0; ii<25; ii++) {
      if (ii==jj){
        strip.setPixelColor(ii,Color(setcolor_r,setcolor_g,setcolor_b));  
      } else{
        strip.setPixelColor(ii,Color(0,0,0));
      }
    }
    strip.show();
    delay(wait);
  }
}


/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

// Shift whatever N by 3 array right by one (towards the array start). The first value in array is ignored/removed
// If you don't set the value of a[end] somewhere else in the code, the memory address value can be random and a random color will be displayed by the corresponding LED
void shift_right(int a[][3], int n) {
   int i;
   for(i = n-1; i != -1; i--){
      a[i][0] = a[i-1][0];
      a[i][1] = a[i-1][1];
      a[i][2] = a[i-1][2];
   }
}

// Shift whatever N by 3 array left by one (towards the array end). The last value in array is ignored/removed
// If you don't set the value of a[0] somewhere else in the code, the memory address value can be random and a random color will be displayed by the corresponding LED
void shift_left(int a[][3], int n) {
   int i;
   for(i = 0; i != n; i++){
      a[i][0] = a[i+1][0];
      a[i][1] = a[i+1][1];
      a[i][2] = a[i+1][2];
   }
}
