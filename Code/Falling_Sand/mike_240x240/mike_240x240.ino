//
// Mike Rankin 240x240 ESP32 board demo
//
#include <bb_spi_lcd.h>
#include <BitBang_I2C.h>
//#include "um_logo.h"
#include "ez16color.h"

BBI2C i2c;
SPILCD lcd;
uint8_t pBitmap[240*240];

#define IMU_ADDR 0x6A

// Display size
#define WIDTH 240
#define HEIGHT 240

#define TFT_CS         4
#define TFT_RST        22
#define TFT_DC         21
#define TFT_LED        26
#define TFT_MOSI       23
#define TFT_MISO       19
#define TFT_SCK        18

//
// Load the UM logo
void PrepBitmap(void)
{

 uint8_t *s, *d;

 memset(pBitmap, 0, sizeof(pBitmap));
 for (int y=0; y<240; y++) 
 {
  uint8_t ucMask = 0xf0;
   //s = (uint8_t *)&um_logo[1078 + (239-y) * 240];
     s = (uint8_t *)&ez16color[158 + (239-y) * 120]; // 4-bpp
  d = &pBitmap[240*y];
  for (int x=0; x<240; x++) 
  {
   if (s[x/2] & ucMask)
   // if (s[x])
   d[x] = 9; // special color in our palette
   ucMask ^= 0xff;
  }
 }
}



// Number of grains of sand - this is how many pixels in the first line of text
#define N_GRAINS     720  //360, 720
// The 'sand' grains exist in an integer coordinate space that's 256X
// the scale of the pixel grid, allowing them to move and interact at
// less than whole-pixel increments.
#define MAX_X (WIDTH  * 256 - 1) // Maximum X coordinate in grain space
#define MAX_Y (HEIGHT * 256 - 1) // Maximum Y coordinate
typedef struct tag_grain {
  uint16_t  x,  y; // Position
  int16_t vx, vy; // Velocity
  uint8_t color; // pixel color
} GRAIN;

static GRAIN grain[N_GRAINS];
void ResetGrains(int bRandom)
{
int i, j, x, y;
//uint16_t *pBitmap = spilcdGetBuffer(&lcd);

//  memset(pBitmap, 0, sizeof(pBitmap));
  PrepBitmap();
  spilcdFill(&lcd, 0, DRAW_TO_LCD); // | DRAW_TO_RAM);
//  if (bRandom)
  {
    for(i=0; i<N_GRAINS; i++) {  // For each sand grain...
      do {
        grain[i].x = random(WIDTH  * 256); // Assign random position within
        grain[i].y = random(HEIGHT * 256); // the 'grain' coordinate space
        x = grain[i].x >> 8; y = grain[i].y >> 8;
        if (pBitmap[(y * WIDTH) + x] != 0) continue; // occupied
        
        // Check if corresponding pixel position is already occupied...
        for(j=0; (j<i) && (((grain[i].x / 256) != (grain[j].x / 256)) ||
                           ((grain[i].y / 256) != (grain[j].y / 256))); j++);
      } while(j < i); // Keep retrying until a clear spot is found
      x = grain[i].x / 256; y = grain[i].y / 256;
      grain[i].vx = grain[i].vy = 0; // Initial velocity is zero
      grain[i].color = 2; // let's try all red for contrast /*Pal[*/random(7)+1 /*]*/;
      pBitmap[(y*WIDTH) + x] = grain[i].color; // Mark it
    }
  } // random
//  else
//  {
//    spilcdWriteString(&lcd, 40,28,(char *)"C",0xf800,0,FONT_16x32,DRAW_TO_LCD | DRAW_TO_RAM);
//    spilcdWriteString(&lcd, 56,28,(char *)"O",0x6e0,0,FONT_16x32,DRAW_TO_LCD | DRAW_TO_RAM);
//    spilcdWriteString(&lcd, 72,28,(char *)"L",0x1f,0,FONT_16x32,DRAW_TO_LCD | DRAW_TO_RAM);
//    spilcdWriteString(&lcd, 88,28,(char *)"O",0xf81f,0,FONT_16x32,DRAW_TO_LCD | DRAW_TO_RAM);
//    spilcdWriteString(&lcd, 104,28,(char *)"R",0xffe0,0,FONT_16x32,DRAW_TO_LCD | DRAW_TO_RAM);
//    i = 0;
//    for (y=0; y<HEIGHT; y++)
//    {
//      for (x=0; x<WIDTH; x++)
//      {
//        color = pBitmap[(y*WIDTH)+x];
//        if (color != 0) // pixel set?
//        {
//          grain[i].x = x*256; grain[i].y = y*256;
//          grain[i].vx = grain[i].vy = 0; // Initial velocity is zero
//          grain[i].color = color;
//          i++;
//          if (i == N_GRAINS) return;
//        }
//      } // for x
//    } // for y
//    Serial.println(i, DEC);  
//  }
  
} /* ResetGrains() */

void ShowFrame(void)
{
int x, y;
const uint16_t Pal[] = {0x0000,0x00f8,0xffff,0xe0ff,0x1ff8,0x1f00,0xe006,0xff06,0xaaaa, 0x3f2a};

  spilcdSetPosition(&lcd, 0,0,WIDTH,HEIGHT, DRAW_TO_LCD);
  for (y=0; y<HEIGHT; y++) {
    uint16_t usTemp[WIDTH];
    uint8_t *s = &pBitmap[y * WIDTH];
    for (x=0; x<WIDTH; x++) {
      usTemp[x] = Pal[*s++];
    }
    spilcdWriteDataBlock(&lcd, (uint8_t *)usTemp, WIDTH*2, DRAW_TO_LCD | DRAW_WITH_DMA);
  }
} /* ShowFrame() */
void IMUTest(void)
{
int16_t x, y, z;
int32_t v2; // Velocity squared
int16_t ax, ay, az;
//signed int        oldidx, newidx;
signed int        newx, newy;
signed int        x1, y1, x2, y2;
int i;
//uint16_t *pBitmap = spilcdGetBuffer(&lcd);
//uint16_t u16Flags[5]; // divide the display into 16x16 blocks for quicker refresh
int iFrame = 0;

  ResetGrains(0);
  spilcdFill(&lcd, 0, DRAW_TO_LCD);
  spilcdWriteString(&lcd, 72,0,(char *)"IMU Test",0x7e0,0,FONT_12x16, DRAW_TO_LCD);
  spilcdWriteString(&lcd, 28,32,(char *)"Press 4 buttons to exit", 0xffe0,0,FONT_8x8, DRAW_TO_LCD);
  delay(2000);
  ShowFrame();
  GetButtons(); // clear old bits
//  memset(u16Flags,0,sizeof(u16Flags));
  while (1)
  {
    iFrame++;
    if ((iFrame & 7) == 0)
    {
      if (__builtin_popcount(GetButtons()) >= 4)
       return;
    }
    if (iFrame & 1) // update display if we didn't check the buttons
    {
      ShowFrame();
    }
//    if (bConnected) // use the left analog stick to simulate gravity
//    {
//      ax = gp.iLJoyX / 4;
//      ay = gp.iLJoyY / 4;
//    }
//    else // use the accelerometer
    {
      IMUReadAccel(&x, &y, &z);
      az = x / 512; // Transform accelerometer axes    //ax
      ay = y / 512;      // to grain coordinate space  //ay
      ax = abs(-z) / 2048; // Random motion factor     //az
      az = (az >= 3) ? 1 : 4 - az;      // Clip & invert
      ax -= az;                         // Subtract motion factor from X, Y
      ay -= az;
    }
  // Apply 2D accelerometer vector to grain velocities...
  //
  // Theory of operation:
  // if the 2D vector of the new velocity is too big (sqrt is > 256), this means it might jump
  // over pixels. We want to limit the velocity to 1 pixel as a maximum.
  // To avoid using floating point math (sqrt + 2 multiplies + 2 divides)
  // Instead of normalizing the velocity to keep the same direction, we can trim the new
  // velocity to 5/8 of it's value. This is a reasonable approximation since the maximum
  // velocity impulse from the accelerometer is +/-64 (16384 / 256) and it gets added every frame
  //
  for(i=0; i<N_GRAINS; i++) {
    grain[i].vx += ax;// + random(5); // Add a little random impulse to each grain
    grain[i].vy += ay;// + random(5);
    v2 = (int32_t)(grain[i].vx*grain[i].vx) + (int32_t)(grain[i].vy*grain[i].vy);
    if (v2 >= 65536) // too big, trim it
    {
      grain[i].vx = (grain[i].vx * 5)/8; // quick and dirty way to avoid doing a 'real' divide
      grain[i].vy = (grain[i].vy * 5)/8;
    }
  } // for i
  // Update the position of each grain, one at a time, checking for
  // collisions and having them react.  This really seems like it shouldn't
  // work, as only one grain is considered at a time while the rest are
  // regarded as stationary.  Yet this naive algorithm, taking many not-
  // technically-quite-correct steps, and repeated quickly enough,
  // visually integrates into something that somewhat resembles physics.
  // (I'd initially tried implementing this as a bunch of concurrent and
  // "realistic" elastic collisions among circular grains, but the
  // calculations and volument of code quickly got out of hand for both
  // the tiny 8-bit AVR microcontroller and my tiny dinosaur brain.)
  //
  // (x,y) to bytes mapping:
  // The SSD1306 has 8 rows of 128 bytes with the LSB of each byte at the top
  // In other words, bytes are oriented vertically with bit 0 as the top pixel
  // Part of my optimizations were writing the pixels into memory the same way they'll be
  // written to the display. This means calculating an offset and bit to test/set each pixel
  //
  for(i=0; i<N_GRAINS; i++) {
    newx = grain[i].x + grain[i].vx; // New position in grain space
    newy = grain[i].y + grain[i].vy;
    if(newx > MAX_X) {               // If grain would go out of bounds
      newx         = MAX_X;          // keep it inside, and
      grain[i].vx /= -2;             // give a slight bounce off the wall
    } else if(newx < 0) {
      newx         = 0;
      grain[i].vx /= -2;
    }
    if(newy > MAX_Y) {
      newy         = MAX_Y;
      grain[i].vy /= -2;
    } else if(newy < 0) {
      newy         = 0;
      grain[i].vy /= -2;
    }

    x1 = grain[i].x / 256; y1 = grain[i].y / 256; // old position
    x2 = newx / 256; y2 = newy / 256;
    if((x1 != x2 || y1 != y2) && // If grain is moving to a new pixel...
        (pBitmap[(y2*WIDTH)+x2] != 0)) {       // but if that pixel is already occupied...
        // Try skidding along just one axis of motion if possible (start w/faster axis)
        if(abs(grain[i].vx) > abs(grain[i].vy)) { // X axis is faster
          y2 = grain[i].y / 256;
          if(pBitmap[(y2*WIDTH)+x2] == 0) { // That pixel's free!  Take it!  But...
            newy         = grain[i].y; // Cancel Y motion
            grain[i].vy = (grain[i].vy /-2) + random(8);         // and bounce Y velocity
          } else { // X pixel is taken, so try Y...
            y2 = newy / 256; x2 = grain[i].x / 256;
            if(pBitmap[(y2*WIDTH)+x2] == 0) { // Pixel is free, take it, but first...
              newx         = grain[i].x; // Cancel X motion
              grain[i].vx = (grain[i].vx /-2) + random(8);         // and bounce X velocity
            } else { // Both spots are occupied
              newx         = grain[i].x; // Cancel X & Y motion
              newy         = grain[i].y;
              grain[i].vx = (grain[i].vx /-2) + random(8);         // Bounce X & Y velocity
              grain[i].vy = (grain[i].vy /-2) + random(8);
            }
          }
        } else { // Y axis is faster
          y2 = newy / 256; x2 = grain[i].x / 256;
          if(pBitmap[(y2*WIDTH)+x2] == 0) { // Pixel's free!  Take it!  But...
            newx         = grain[i].x; // Cancel X motion
            grain[i].vx = (grain[i].vx /-2) + random(8);        // and bounce X velocity
          } else { // Y pixel is taken, so try X...
            y2 = grain[i].y / 256; x2 = newx / 256;
            if(pBitmap[(y2*WIDTH)+x2] == 0) { // Pixel is free, take it, but first...
              newy         = grain[i].y; // Cancel Y motion
              grain[i].vy = (grain[i].vy /-2) + random(8);        // and bounce Y velocity
            } else { // Both spots are occupied
              newx         = grain[i].x; // Cancel X & Y motion
              newy         = grain[i].y;
              grain[i].vx = (grain[i].vx /-2) + random(8);         // Bounce X & Y velocity
              grain[i].vy = (grain[i].vy /-2) + random(8);
            }
          }
        }
    }
    grain[i].x  = newx; // Update grain position
    grain[i].y  = newy; // possibly only a fractional change
    y2 = newy / 256; x2 = newx / 256;
    if (x1 != x2 || y1 != y2)
    {
      pBitmap[(y1*WIDTH)+x1] = 0; // erase old pixel
      pBitmap[(y2*WIDTH)+x2] = grain[i].color;  // Set new pixel
    }
  } // for i
  } // while (1)
} /* IMUTest() */

//
// Read the current touch button state
// and return flags indicating if they have just been pressed
// 
uint32_t GetButtons(void)
{
  return 0; // DEBUG - add your button logic
} /* GetButtons() */

void IMUInit(void)
{
uint8_t ucTemp[4];

   ucTemp[0] = 0x10; // CTRL1_XL
   ucTemp[1] = 0x20; // 26hz (low power) accelerometer enable
   I2CWrite(&i2c, IMU_ADDR, ucTemp, 2);
   ucTemp[0] = 0x11; // CTRL2_G
   ucTemp[1] = 0x34; // 52hz (low power) gyroscope enable and 500 dps full scale range
//   ucTemp[1] = 0x24; // 26hz (low power) gyroscope enable and 500 dps full scale range
   I2CWrite(&i2c, IMU_ADDR, ucTemp, 2);
   ucTemp[0] = 0x16; // CTR7_G - power mode
   ucTemp[1] = 0x40; // Disable low power mode, enable high pass filter
//   ucTemp[1] = 0x80; // Enable low power mode
   I2CWrite(&i2c, IMU_ADDR, ucTemp, 2);
} /* IMUInit() */

//
// Read the accelerometer register values
//
int IMUReadAccel(short *X, short *Y, short *Z)
{
unsigned char ucTemp[8];
signed short x, y, z;
uint8_t start_reg;

// LSM6DS3
  start_reg = 0x28;

  x = I2CReadRegister(&i2c, IMU_ADDR, start_reg, ucTemp, 6);
  if (x != 0)
  {
    x = (ucTemp[1] << 8) + ucTemp[0]; // little endian
    y = (ucTemp[3] << 8) + ucTemp[2];
    z = (ucTemp[5] << 8) + ucTemp[4];
    if (X != NULL) *X = x;
    if (Y != NULL) *Y = y;
    if (Z != NULL) *Z = z;
    return 0;
  }
  else
     return -1;
} /* IMUReadAccel() */

void setup() {
  Serial.begin(115200);
  i2c.iSDA = 13;
  i2c.iSCL = 14;
  i2c.bWire = 1;
  I2CInit(&i2c, 400000L);

  spilcdInit(&lcd, LCD_ST7789_240, FLAGS_NONE, 40000000, TFT_CS, TFT_DC, TFT_RST, TFT_LED, TFT_MISO, TFT_MOSI, TFT_SCK);
  spilcdSetOrientation(&lcd, LCD_ORIENTATION_180);
  spilcdFill(&lcd, 0, DRAW_TO_LCD);
  spilcdWriteString(&lcd, 0,0,(char *)"Test", 0xf800, 0xffff, FONT_8x8, DRAW_TO_LCD);
  IMUInit();
} /* setup() */

void loop()
{
  delay(2000);
  spilcdFill(&lcd, 0, DRAW_TO_LCD);
  IMUTest();
}
