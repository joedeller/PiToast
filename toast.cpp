/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x32|64 size display using SPI or I2C to communicate
4 or 5 pins are required to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution

02/18/2013 	Charles-Henri Hallard (http://hallard.me)
						Modified for compiling and use on Raspberry ArduiPi Board
						LCD size and connection are now passed as arguments on 
						the command line (no more #define on compilation needed)
						ArduiPi project documentation http://hallard.me/arduipi

9th Sept 2015 Joe Deller
                  Take the Adafruit Phillip Burgess Flying Toaster Jewelry code and port to
                  Raspberry Pi

*********************************************************************/

#include "ArduiPi_OLED_lib.h"
#include "ArduiPi_OLED.h"
#include <time.h>
#include <getopt.h>
#include "bitmaps.h"
#define PRG_NAME        "toaster"
#define PRG_VERSION     "1.0"

// Instantiate the display
ArduiPi_OLED display;

// Config Option
struct s_opts
{
	int oled;
	int verbose;
} ;

unsigned int iseed = (unsigned int)time(NULL);

// default options values
s_opts opts = {
	//OLED_ADAFRUIT_SPI_128x32,
        OLED_ADAFRUIT_I2C_128x64,  // Default is now type 3
  false										// Not verbose
};

#define N_FLYERS   5 // Number of flying things
#define PIX_SCALE 16  // flyer coordinates are scaled to allow "sub pixel" movements for different speeds

struct Flyer {       // Array of flying things
  int16_t x, y;      // Top-left position * 16 (for subpixel pos updates)
  int8_t  depth;     // Stacking order is also speed, 12-24 subpixels/frame
  uint8_t frame;     // Animation frame; Toasters cycle 0-3, Toast=255
} flyer[N_FLYERS];
 

// Flyer depth comparison function for qsort()
static int compare(const void *a, const void *b) {
  return ((struct Flyer *)a)->depth - ((struct Flyer *)b)->depth;
}


void loop() {
  uint8_t i, f;
  int16_t x, y;
  boolean resort = false;     // By default, don't re-sort depths
 
  display.display();          // Update screen to show current positions
  display.clearDisplay();     // Start drawing next frame
 
  for(i=0; i<N_FLYERS; i++) { // For each flyer...
    // First draw each item...
    f = (flyer[i].frame == 255) ? 4 : (flyer[i].frame++ & 3); // Frame #
    x = flyer[i].x / PIX_SCALE;
    y = flyer[i].y / PIX_SCALE;
    display.drawBitmap(x, y, mask[f], 32, 32, BLACK);
    display.drawBitmap(x, y, img[f], 32, 32, WHITE);

 
    // Then update position, checking if item moved off screen...
    flyer[i].x -= flyer[i].depth * 2; // Update position based on depth,
    flyer[i].y += flyer[i].depth;     // for a sort of pseudo-parallax effect.
    if((flyer[i].y >= (64* PIX_SCALE)) || (flyer[i].x <= (-32*PIX_SCALE))) { // Off screen?
      if((rand()*7) < 5) {         // Pick random edge; 0-4 = top
        flyer[i].x = (rand()%160) * PIX_SCALE;
        flyer[i].y = -32 * PIX_SCALE;
      } else {                    // 5-6 = right
        flyer[i].x = 128 * PIX_SCALE;
        flyer[i].y = (rand()% 64) * PIX_SCALE;
      }
      flyer[i].frame = rand() % 3 ? rand() %4 : 255; // 66% toaster, else toast
      flyer[i].depth = 10 + rand() % PIX_SCALE;
      resort = true;
    }
  }
  // If any items were 'rebooted' to new position, re-sort all depths
  if(resort) qsort(flyer, N_FLYERS, sizeof(struct Flyer), compare);
}


/* ======================================================================
Function: usage
Purpose : display usage
Input 	: program name
Output	: -
Comments: 
====================================================================== */
void usage( char * name)
{
	printf("%s\n", name );
	printf("Usage is: %s --oled type [options]\n", name);
	printf("  --<o>led type\nOLED type are:\n");
	for (int i=0; i<OLED_LAST_OLED;i++)
		printf("  %1d %s\n", i, oled_type_str[i]);
	
	printf("Options are:\n");
	printf("  --<v>erbose  : speak more to user\n");
	printf("  --<h>elp\n");
	printf("<?> indicates the equivalent short option.\n");
	printf("Short options are prefixed by \"-\" instead of by \"--\".\n");
	printf("Example :\n");
	printf( "%s -o 1 use a %s OLED\n\n", name, oled_type_str[1]);
	printf( "%s -o 4 -v use a %s OLED being verbose\n", name, oled_type_str[4]);
}


/* ======================================================================
Function: parse_args
Purpose : parse argument passed to the program
Input 	: -
Output	: -
Comments: 
====================================================================== */
void parse_args(int argc, char *argv[])
{
	static struct option longOptions[] =
	{
		{"oled"	  , required_argument,0, 'o'},
		{"verbose", no_argument, 0, 'v'},
		{"help"		, no_argument,	0, 'h'},
		{0, 0, 0, 0}
	};

	int optionIndex = 0;
	int c;

	while (1) 
	{
		/* no default error messages printed. */
		opterr = 0;

    c = getopt_long(argc, argv, "vho:", longOptions, &optionIndex);

		if (c < 0)
			break;

		switch (c) 
		{
			case 'v': opts.verbose = true	;	break;

			case 'o':
				opts.oled = (int) atoi(optarg);
				
				if (opts.oled < 0 || opts.oled >= OLED_LAST_OLED )
				{
						fprintf(stderr, "--oled %d ignored must be 0 to %d.\n", opts.oled, OLED_LAST_OLED-1);
						fprintf(stderr, "--oled set to 0 now\n");
						opts.oled = 3;
				}
			break;

			case 'h':
				usage(argv[0]);
				exit(EXIT_SUCCESS);
			break;
			
			case '?':
			default:
				fprintf(stderr, "Unrecognized option.\n");
				fprintf(stderr, "Run with '--help'.\n");
				exit(EXIT_FAILURE);
		}
	} /* while */

	if (opts.verbose)
	{
		printf("%s v%s\n", PRG_NAME, PRG_VERSION);
		printf("-- OLED params -- \n");
		printf("Oled is    : %s\n", oled_type_str[opts.oled]);
		printf("-- Other Stuff -- \n");
		printf("verbose is : %s\n", opts.verbose? "yes" : "no");
		printf("\n");
	}	
}



/* ======================================================================
Function: main
Purpose : Main entry Point
Input 	: -
Output	: -
Comments: 
====================================================================== */
int main(int argc, char **argv)
{
	
	// Oled supported display in ArduiPi_SSD1306.h
	// Get OLED type
	parse_args(argc, argv);

	// SPI
	if (display.oled_is_spi_proto(opts.oled))
	{
		// SPI change parameters to fit to your LCD
		if ( !display.init(OLED_SPI_DC,OLED_SPI_RESET,OLED_SPI_CS, opts.oled) )
			exit(EXIT_FAILURE);
	}
	else
	{
		// I2C change parameters to fit to your LCD
		if ( !display.init(OLED_I2C_RESET,opts.oled) )
			exit(EXIT_FAILURE);
	}

	display.begin();
   srand(iseed); // Set the random number seed so the positions aren't always the same	
  // init done
  display.clearDisplay();   // clears the screen  buffer
  display.display();   		// display it (clear display)


  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Mmmm Toast!\n");
  display.display();
  sleep(2);

  display.clearDisplay();
  for(uint8_t i=0; i<N_FLYERS; i++) {  // Randomize initial flyer states
    flyer[i].x     = ((rand() %160) -32) * PIX_SCALE;
    flyer[i].y     = ((rand() %96) -32) * PIX_SCALE;
    flyer[i].frame = rand() %3 ? rand() %4 : 255; // 66% toaster, else toast
    flyer[i].depth = 10 + rand() %PIX_SCALE;             // Speed / stacking order
  }
  qsort(flyer, N_FLYERS, sizeof(struct Flyer), compare); // Sort depths
  int y = 0;
  for (int x=60;x>2;x-=2){
     display.drawBitmap(x,y,mask[x % 4],32,32,BLACK);
     display.drawBitmap(x,y,img[x % 4],32,32,WHITE);
     display.display();
     display.drawBitmap(x,y,img[x % 4],32,32,BLACK);

      y += 1;
      if(y>30){
         y=0;
        }
     }

  for (int i =0;i<1000;i++) {
  loop();
  }

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Mmmm Toast!\n");
  // Free PI GPIO ports
  display.close();

}


