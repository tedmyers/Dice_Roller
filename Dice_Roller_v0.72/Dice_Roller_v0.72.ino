
/**********************************
          Dice Roller v0.72
              Ted Myers
              5/25/2015
              
    Modified 8/19/2015 for readability

Program originally canabilized from Adafruit screen
interface default program

This program interfaces
with the SSD1306 display with I2C protocols 
and the Arduino Uno to roll and display electronic 
dice rolls. Uses a rotary encoder and simple 
buttons for input.


*************************************/


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// stuff
volatile int nRolls = 1;
volatile int nSides = 20;
volatile int nModifier = 0;
volatile int nResult;

// limits max/min values of nRolls, nSides, and nModifier
#define MAX_VALUE 100

//for Roll
int nSum = 0;
long timer_seed;

// Rotary integrated button
#define BUTTON_PIN_0 4

// button 1 - roll
#define BUTTON_PIN_1 5

// button 2 - add
#define BUTTON_PIN_2 6

// Interrupt encoder function
#define PIN_A  2
#define PIN_B  3
volatile int counter = 20*2;
volatile boolean screenRefresh;
//
int Mode = 0;
// 0: no selection
// 1: nRolls
// 2: nSides
// 3: nModifier
// 

// For screensaver
unsigned long screenTimer;
#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 

// Splash screen
boolean showSplash;

void setup()   
{   

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  
  // Set splash screen to start
  showSplash = 1;
  
  //Initialize text
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  
  // for temporary button
  pinMode(BUTTON_PIN_0, INPUT);
  //digitalWrite(BUTTON_PIN_0, 1); // Enable internal pull-up - why doesn't this work?

  // Encoder
  pinMode(PIN_A, INPUT); 
  digitalWrite(PIN_A, HIGH); // enable pullup resistor
  pinMode(PIN_B, INPUT); 
  digitalWrite(PIN_B, HIGH); // enable pullup resistor
  attachInterrupt(0, doEncoder1, CHANGE);  // encoder pin on interrupt 0 - pin 2
  
  // Start screensaver timer
  screenTimer = millis();
}


void loop() 
{
  
  // Display Splash Screen until action
  if (showSplash)
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Ted's");
    display.println("Dice      Roller");
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    display.println("Press button to roll!");
    display.display();
    
    // wait until button press
    while ( digitalRead(BUTTON_PIN_0) == LOW
          & digitalRead(BUTTON_PIN_1) == LOW
          & digitalRead(BUTTON_PIN_2) == LOW ) {}
    
    showSplash = 0;
  }

  // "Roll" button pressed
  if ( digitalRead(BUTTON_PIN_1) == HIGH )
  {
    nResult = Roll(nRolls, nSides) + nModifier;
    displayScreen(1);
    /*
    Serial.print("Rolled "); Serial.print(nRolls);
      Serial.print("d"); Serial.print(nSides);
      Serial.print(": "); Serial.println(nResult);
    */
    delay(200); // leaves some space after press
    
    // Restart screensaver timer
    screenTimer = millis();
  }
  
  // "Add" button pressed
  if ( digitalRead(BUTTON_PIN_2) == HIGH )
  {
    nResult += Roll(nRolls, nSides) + nModifier; // show rolls?
    displayScreen(1);
    
    delay(200);
    
    // Problem: freezed after ~20 presses
    screenTimer = millis();
  }
  
  // Rotary button pressed
  if ( digitalRead(BUTTON_PIN_0) == HIGH )
  {
    // debounce?
    delay(10); 
    
    // Rotary button pressed
    if ( digitalRead(BUTTON_PIN_0) == HIGH )
    {
      if ( Mode < 3 )
        Mode++;
      else
        Mode = 0;
        
      displayScreen(0);
      
      delay(200);
    }
  }
  
  // Refresh the screen if knob moved
  if (screenRefresh)
  {
    displayScreen(0);
    screenRefresh = 0;
    
    // Restart screensaver timer
    screenTimer = millis();
  }
  
}


// Functions


// Displays nSides, nRolls, and nResult
// Note: will only display three digits of nResult
void displayScreen(boolean flash) 
{
  // int Mode
  // 0: no selection
  // 1: nRolls
  // 2: nSides
  // 3: nModifier
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  // Highlight nRolls if on that mode
  if ( Mode == 1)
  {
    nRolls = (nRolls > MAX_VALUE ) ? MAX_VALUE: nRolls; // sets nRolls to 1000 if is greater than 1000, else keeps its value
    nRolls = (nRolls < 0 ) ? 0: nRolls;
    display.setTextColor(BLACK, WHITE);
    display.print(nRolls); 
    display.setTextColor(WHITE);
  }
  else
    display.print(nRolls);
    
  display.print("d");
  
  // Highlight nSies if on that mode
  if ( Mode == 2)
  {
    nSides = (nSides > MAX_VALUE ) ? MAX_VALUE: nSides; // sets nRolls to 1000 if is greater than 1000, else keeps its value
    nSides = (nSides < 0 ) ? 0: nSides;
    display.setTextColor(BLACK, WHITE);
    display.print(nSides); 
    display.setTextColor(WHITE);
  }
  else
    display.print(nSides);
  
  // Add a modifier, nModifier
  if ( Mode == 3)
  {
    nModifier = (nModifier < -MAX_VALUE ) ? -MAX_VALUE: nModifier; // sets nRolls to 1000 if is greater than 1000, else keeps its value
    nModifier = (nModifier > MAX_VALUE ) ? MAX_VALUE: nModifier;
    if ( nModifier < 0 )
      display.print("-");
    else
      display.print("+");
      
    display.setTextColor(BLACK, WHITE);
    display.println( abs(nModifier) ); // abs because "-" or "+" is already printed
    display.setTextColor(WHITE);
  }
  else
  {
    if ( nModifier < 0 )
      display.print("-");
    else
      display.print("+");
    display.println( abs(nModifier) );
  }
  
  display.setTextSize(7);
  if ( nResult  < 1000 ) // Set to MAX_VALUE?
    display.println(nResult);
  else
    display.println("ERR");
  
  if (flash)
  {
    display.invertDisplay(true);
    display.display();
    delay(100);
    display.invertDisplay(false);
  }
  
  display.display();
  
}





// Generates nRolls number of random numbers of <= nSides and 
// sums then, generating a simulation of rolling dice
int Roll(unsigned int nRolls, unsigned int nSides)
{
  nSum = 0;
  
  if ( (nRolls <= 0) || (nSides <= 0) )
    return 0; // ERROR
    
  if (timer_seed != millis() ) // Only change the seed when seed is different
  {
    randomSeed( millis() ); // Sets a hopefully unique random seed each time
    timer_seed = millis();
  }
    
  for (int iii=0; iii<nRolls; iii++)
    nSum += random(1, nSides+1);
    
  return nSum;

}


// Executes when interrupt pin changes
void doEncoder1() {
  /* If pinA and pinB are both high or both low, it is spinning
   * forward. If they're different, it's going backward.
   */
   
   /*
  if (digitalRead(PIN_A) == digitalRead(PIN_B)) 
    counter++;
  else 
    counter--;
    */
  
  int incrementer = 0;
  
  if (digitalRead(PIN_A) == digitalRead(PIN_B)) //increment
  {
    counter++;
    if ( counter % 2 ) // if even (every 2 times counter increments)
    incrementer++;
  }
  else //decrement
  {
    counter--;
    if ( counter % 2 ) // if even (every 2 times counter increments)
    incrementer--;
  }
    
  /* (current doEncoder function uses one interrupt to 
      measure two amounts of rotation per indent, using
      two could allow for 4 points of resolution per indent)
  */
  
  // 0: no selection
  // 1: nRolls
  // 2: nSides
  // 3: nModifier
    
  //cool if by default cycled through dice animation - not very feasible
    
  if ( Mode == 0 ) {}
  else if ( Mode == 1 )
    nRolls += incrementer;
  else if ( Mode == 2 )
    nSides += incrementer;
  else if ( Mode == 3 )
    nModifier += incrementer;

  screenRefresh = 1;
  
}


