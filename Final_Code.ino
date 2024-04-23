#include <Adafruit_ILI9341.h>

#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

/////////////////////////////////////////////////////////////////////
//The Following is the Code Necessary for Implementing the GUI
//On Preliminary Testing for Verification
//Last Updated 02/10/24
/////////////////////////////////////////////////////////////////////
#include "SPI.h" //Include SPI for Serial Communication to Arduino Uno
//#include "C:\Users\nicho\Documents\Arduino\audio_code\Final_Code\Adafruit_GFX_Library\Adafruit_GFX.h" //Include Adafruit GFX Library
//#include "C:\Users\nicho\Documents\Arduino\audio_code\Final_Code\Adafruit_ILI9341\Adafruit_ILI9341.h" //Include Adafruit ILI9341 Library

//For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

//Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//Global variables for each of the test screens for Block Checkoff
#define SCREEN_1 0
#define SCREEN_2 1
#define SCREEN_3 2
#define SCREEN_4 3
#define SCREEN_5 4
#define SCREEN_6 5
#define SCREEN_7 6
#define SCREEN_8 7

uint8_t selectedLineScreen1; //Set the line to highlight for screen 1
uint8_t selectedLineScreen2; //Set the line to highlight for screen 2
uint8_t selectedLineScreen3; //Set the line to highlight for screen 3

uint8_t currentScreen = SCREEN_1; //Initialize currentScreen with Screen 1

#include "SoftwareSerial.h"

//#include "DFPlayer_Mini_MP3.h"
SoftwareSerial portTwo(6, 7);
# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]

int reverb_effect = 0;
int distortion_effect = 1;
int echo_effect = 0;
int preset = 1;
int previous_preset = 1;
int volume = 15;

#define POTENTIOMETER_PIN A0

//constants won't change. They're used here to set pin numbers:
const int buttonPinUp = 2;    //the number of the pushbutton pin for counting up
const int buttonPinDown = 3;  //the number of the pushbutton pin for counting down
const int selectButtonPin = 4; //the number of the select button pin

int buttonStateUp = 0;    //variable for reading the pushbutton status for counting up
int buttonStateDown = 0;  //variable for reading the pushbutton status for counting down
int selectButtonState = 0; //variable for reading the select button status
int count = 0;            //variable to store the count
//Variable to store the previous percentage value
int previousvalue = 0;


//Setup function just to read preliminary Diagnostics
//Given through Adafruit Test Code from Datasheet (NOT VITAL FOR FINAL CODE IMPLEMENTATION)
void setup() {
  Serial.begin(9600); //READ SPI
  Serial.println("ILI9341 Test!"); //PRINT SPI
 
  tft.begin(); //Start Adafruit Screen

  //Read Diagnostics (optional but can help debug problems)
  //Again given through Adafruit TFT Datasheet
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);

  setVolume(volume);
  choose_mp3();
  repeat_play();


  //initialize the pushbutton pins as inputs:
  pinMode(buttonPinUp, INPUT);
  pinMode(buttonPinDown, INPUT);
  pinMode(selectButtonPin, INPUT);
}


void loop() {
  
  countUp();
  countDown();
  checkSelectButton();
  readPotentiometer();
  
  if (preset <= 8) {
    selectedLineScreen1 = preset;
    PRESET1_WINDOW_MAIN(); //Display Preset Screen Window 1
    
    if(preset != previous_preset) {
      choose_mp3();
      previous_preset = preset;
    }
  }
  else if (preset > 8) {
    selectedLineScreen2 = preset; //15th Preset Highlighted of Preset Window 2
    PRESET2_WINDOW_MAIN(); //Display Preset Screen Window 2

    if(preset != previous_preset) {
      choose_mp3();
      previous_preset = preset;
    }
  }
  

}

void countUp() {
  //read the state of the pushbutton for counting up:
  buttonStateUp = digitalRead(buttonPinUp);

  //check if the pushbutton for counting up is pressed:
  if (buttonStateUp == HIGH) {
    //turn LED on:
   
    //Increase count only if it wasn't already pressed and count is less than 16
    if (digitalRead(buttonPinUp) == HIGH && count < 15) {
      count++;
      preset = count;
      Serial.print("Count: ");
      Serial.println(count);
      // Wait until the button is released
      while (digitalRead(buttonPinUp) == HIGH) {
        delay(10);
      }
    }
  } else {
    //turn LED off:
 
  }
}

void countDown() {
  //read the state of the pushbutton for counting down:
  buttonStateDown = digitalRead(buttonPinDown);

  //check if the pushbutton for counting down is pressed:
  if (buttonStateDown == HIGH) {
    //turn LED on:
  
    //Decrease count only if it wasn't already pressed and count is greater than 0
    if (digitalRead(buttonPinDown) == HIGH && count > 0) {
      count--;
      preset = count;
      Serial.print("Count: ");
      Serial.println(count);
      //Wait until the button is released
      while (digitalRead(buttonPinDown) == HIGH) {
        delay(10);
      }
    }
  } else {
    //turn LED off:
  
  }
}

void checkSelectButton() {
  //read the state of the select button:
  selectButtonState = digitalRead(selectButtonPin);

  //check if the select button is pressed:
  if (selectButtonState == HIGH) {
    Serial.println("Select Button has been pressed");
    delay(50); //debounce delay
    while (digitalRead(selectButtonPin) == HIGH) {
      delay(10);
    }
  }
}

void readPotentiometer() {
  //Read the potentiometer value
  int data = analogRead(POTENTIOMETER_PIN);
  //Map the potentiometer value to a value 0-30
  int value = map(data, 0, 1023, 0, 30);

  volume = value;
  //Check if the current percentage value is different from the previous one
  if (value != previousvalue) {
    //Print the pententiometer count
    Serial.print("Potentiometer Count At: ");
    Serial.println(value);
    
    //Update the previous percentage value
    previousvalue = value;
  }

  //Delay before reading again
  delay(100);
}


void setVolume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(2000);
}

void repeat_play() {
   execute_CMD(0x08,0,2); 
}

void mp3_play_folder ( uint8_t folder, uint8_t num) {
	execute_CMD (0x0F, folder, num);
}

void choose_mp3() {
        // OG
        if(reverb_effect == 0 & echo_effect == 0 & distortion_effect == 0) {
          mp3_play_folder(preset, 1);
        }
        // Reverb
        if(reverb_effect == 1 & echo_effect == 0 & distortion_effect == 0) {
          mp3_play_folder(preset, 2);
        }
        // Echo
        else if(reverb_effect == 0 & echo_effect == 1 & distortion_effect == 0) {
          mp3_play_folder(preset, 3);
        }
        // Distortion
        else if(reverb_effect == 0 & echo_effect == 0 & distortion_effect == 1) {
          mp3_play_folder(preset, 4);
        }
        // Reverb & Echo
        else if(reverb_effect == 1 & echo_effect == 1 & distortion_effect == 0) {
          mp3_play_folder(preset, 5);
        }
        // Reverb & Distortion
        else if(reverb_effect == 1 & echo_effect == 0 & distortion_effect == 1) {
          mp3_play_folder(preset, 6);
        }
        // Echo & Distortion
        else if(reverb_effect == 0 & echo_effect == 1 & distortion_effect == 1) {
          mp3_play_folder(preset, 7);
        }
        // All Effects
        else if(reverb_effect == 1 & echo_effect == 1 & distortion_effect == 1) {
          mp3_play_folder(preset, 8);
      }

}


//Main Function for Preset Window 1
unsigned long PRESET1_WINDOW_MAIN(){
  uint8_t rotation = 3; //Set Proper Orientation fo Screen
  tft.setRotation(rotation); //Set Rotation
  tft.fillScreen(ILI9341_WHITE); //Fill Screen with White

  //Display title
  tft.fillRect(10, 0, 300, 25, ILI9341_BLUE); //Blue background for the title, set at Top of Screen
  tft.setTextColor(ILI9341_WHITE); //Title color set to white
  tft.setTextSize(2); //Text Size Set
  tft.setCursor(70, 5); //Cursor Set to Start Text
  tft.println("PRESET WINDOW 1"); //Print PRESET WINDOW 1 as Title

  unsigned long start = micros(); //Record Current Time in Microseconds (For Diagnostics)

  //Display each line of text on top of a rectangle
  for (uint8_t i = 0; i < 8; i++) { //Loop through Presets 1 to 8
    String presetText = "PRESET " + String(i + 1); //Create String Containing The Preset Number
    INIT_PRESET1_WINDOW(presetText.c_str(), i, (i == selectedLineScreen1)); //Call Function to Display Preset Window 1, Preset Text, Line Number, and Highlight Function
  }

  //Determine highlight condition for "GO TO PRESET 2 WINDOW"
  bool highlightGoToPreset2 = (selectedLineScreen1 == 8); //Check if Selected Line is Go To Preset 2

  //Display "GO TO PRESET 2 WINDOW" at the bottom
  tft.fillRect(10, 220, 300, 25, highlightGoToPreset2 ? ILI9341_YELLOW : ILI9341_GREEN); //Fill Rectangle with Yellow if True, otherwise stay green
  tft.setTextColor(ILI9341_BLUE); //Set text color to blue
  tft.setTextSize(2); //Set text size to 2
  tft.setCursor(35, 225); //Set cursor position
  tft.println("GO TO PRESET 2 WINDOW"); //Print "GO TO PRESET WINDOW 2" on label

  return micros() - start; //Calculate time elapsed since 'start' and return (For Diagnostics)
}

//Main Function for Preset Window 2
unsigned long PRESET2_WINDOW_MAIN() {
  uint8_t rotation = 3; //Set Proper Orientation fo Screen
  tft.setRotation(rotation); //Set Rotation
  tft.fillScreen(ILI9341_WHITE); //Fill Screen with White

  //Display title
  tft.fillRect(10, 0, 300, 25, ILI9341_BLUE); //Blue background for the title, set at Top of Screen
  tft.setTextColor(ILI9341_WHITE); //Title color set to white
  tft.setTextSize(2); //Text Size Set
  tft.setCursor(70, 5); //Cursor Set to Start Text
  tft.println("PRESET WINDOW 2"); //Print PRESET WINDOW 2 as Title

  unsigned long start = micros(); //Record Current Time in Microseconds (For Diagnostics)

  //Display each line of text on top of a rectangle
  for (uint8_t i = 0; i < 7; i++) { //Display presets 9 to 15
    String presetText = "PRESET " + String(i + 9); //Starting from Preset 9, Create String Containing Preset Number
    //Call Function to Display Preset Window 2, Preset Text, Line Number, and Highlight Function
    INIT_PRESET2_WINDOW(presetText.c_str(), i, ((currentScreen == SCREEN_4 || currentScreen == SCREEN_5) && (i + 8) == selectedLineScreen2)); //Adjusted selected line
  }

  //Determine highlight condition for "GO TO PRESET 1 WINDOW"
  bool highlightGoToPreset1 = ((currentScreen == SCREEN_4 || currentScreen == SCREEN_5) && selectedLineScreen2 == 15);

  //Display "GO TO PRESET 1 WINDOW" at the bottom
  tft.fillRect(10, 220, 300, 25, highlightGoToPreset1 ? ILI9341_YELLOW : ILI9341_GREEN); //Fill Rectangle with Yellow if True, otherwise stay green
  tft.setTextColor(ILI9341_BLUE); //Set text color to blue
  tft.setTextSize(2); //Set text size to 2
  tft.setCursor(35, 225); //Set cursor position
  tft.println("GO TO PRESET 1 WINDOW"); //Print "GO TO PRESET WINDOW 1" on label

  return micros() - start; //Calculate time elapsed since 'start' and return (For Diagnostics)
}

//Main Function for Audio Effects Window
unsigned long AUDIO_EFFECTS_WINDOW_MAIN() {
  uint8_t rotation = 3; //Set Proper Orientation fo Screen
  tft.setRotation(rotation); //Set Rotation
  tft.fillScreen(ILI9341_WHITE); //Fill Screen with White

  //Display title for Audio Effects Screen
  String title = "AUDIO EFFECTS: PRESET " + String(selectedLineScreen1 + 1); // Dynamically change the title
  tft.fillRect(10, 0, 300, 25, ILI9341_BLUE); //Blue background for the title, set at Top of Screen
  tft.setTextColor(ILI9341_WHITE); //Title color set to white
  tft.setTextSize(2); //Text Size Set
  tft.setCursor(20, 5); //Cursor Set to Start Text
  tft.println(title); //Print AUDIO EFFECTS: PRESET [] as Title

  unsigned long start = micros(); //Record Current Time in Microseconds (For Diagnostics)

  //Display each line of text on top of a rectangle for Audio Effects
  //Using const char* pointer array for proper storage of strings
  const char* effectLabels[] = {
    "No Effects",
    "Reverb",
    "Distortion",
    "Echo",
    "Reverb and Distortion",
    "Reverb and Echo",
    "Distortion and Echo",
    "Reverb, Distortion, Echo"
  };

  //Display each line of text on top of a rectangle
  for (uint8_t i = 0; i < 8; i++) { //Display all 8 Audio Effects
    //Call Function to Display Audio Effects Window, Audio Effects Text, Line Number, and Highlight Function
    INIT_AUDIO_EFFECTS_WINDOW(effectLabels[i], i, (i == selectedLineScreen3));
  }

  // Determine highlight condition for "GO BACK TO PRESET WINDOW"
  bool highlightGoToPresets = (currentScreen == SCREEN_8 && selectedLineScreen3 == 8);

  //Display "GO BACK TO PRESETS WINDOW" at the bottom
  tft.fillRect(10, 220, 300, 25, highlightGoToPresets ? ILI9341_YELLOW : ILI9341_GREEN); //Fill Rectangle with Yellow if True, otherwise stay green
  tft.setTextColor(ILI9341_BLUE); //Set text color to blue
  tft.setTextSize(2); //Set text size to 2
  tft.setCursor(18, 225); //Set cursor position
  tft.println("GO BACK TO PRESET WINDOW"); //Print "GO TO PRESET WINDOW" on label

  return micros() - start; //Calculate time elapsed since 'start' and return (For Diagnostics)
}

//The following three functions are apart of the main window functions, but are just seperated to optimize code

//Setting Rectangle and Text Properties for Preset Window 1
//The char pointer text, line number, and highlight boolean are all passed in
void INIT_PRESET1_WINDOW(const char* text, uint8_t lineNumber, bool highlight) {
  //Set rectangle and text properties
  uint16_t rectX = 10; //Define the X coordinate of the rectangle
  uint16_t rectY = lineNumber * 24 + 28; //Calculate the Y coordinate of the rectangle based on line number
  uint16_t rectWidth = 300; //Set width of rectangle
  uint16_t rectHeight = 20; //Set height of rectangle
  
  //Draw rectangle with color based on highlight condition
  //If highlight is True, set rectangle color to yellow, otherwise set to green
  uint16_t rectColor = highlight ? ILI9341_YELLOW : ILI9341_GREEN;
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, rectColor); //Draw rectangle based on parameters

  //Set text properties
  tft.setTextColor(ILI9341_BLUE); //Set text color to blue
  tft.setTextSize(2); //Set text size to 2

  //Set cursor position for text inside the rectangle
  uint16_t textX = rectX + 100; //Calculate the X coordinate of the text to center it horizontally within the rectangle
  uint16_t textY = rectY + 3; //Calculate the Y coordinate of the text to vertically center it within the rectangle

  tft.setCursor(textX, textY); //Set cursor position for printing text
  tft.print(text); //Print the text inside the rectange
}

//Setting Rectangle and Text Properties for Preset Window 2
//The char pointer text, line number, and highlight boolean are all passed in
void INIT_PRESET2_WINDOW(const char* text, uint8_t lineNumber, bool highlight) {
  //Set rectangle and text properties
  uint16_t rectX = 10; //Define the X coordinate of the rectangle
  uint16_t rectY = lineNumber * 24 + 28; //Calculate the Y coordinate of the rectangle based on line number
  uint16_t rectWidth = 300; //Set width of rectangle
  uint16_t rectHeight = 20; //Set height of rectangle
  
  //Draw rectangle with color based on highlight condition
  //If highlight is True, set rectangle color to yellow, otherwise set to green
  uint16_t rectColor = highlight ? ILI9341_YELLOW : ILI9341_GREEN;
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, rectColor); //Draw rectangle based on parameters

  //Set text properties
  tft.setTextColor(ILI9341_BLUE); //Set text color to blue
  tft.setTextSize(2); //Set text size to 2

  //Set cursor position for text inside the rectangle
  uint16_t textX = rectX + 100; //Calculate the X coordinate of the text to center it horizontally within the rectangle
  uint16_t textY = rectY + 3; //Calculate the Y coordinate of the text to vertically center it within the rectangle

  tft.setCursor(textX, textY); //Set cursor position for printing text
  tft.print(text); //Print the text inside the rectange
}

//Setting Rectangle and Text Properties for Audio Effects Window
//The char pointer text, line number, and highlight boolean are all passed in
void INIT_AUDIO_EFFECTS_WINDOW(const char* text, uint8_t lineNumber, bool highlight) {
  //Set rectangle and text properties
  uint16_t rectX = 10; //Define the X coordinate of the rectangle
  uint16_t rectY = lineNumber * 24 + 28; //Calculate the Y coordinate of the rectangle based on line number
  uint16_t rectWidth = 300; //Set width of rectangle
  uint16_t rectHeight = 20; //Set height of rectangle
  
  //Draw rectangle with color based on highlight condition
  //If highlight is True, set rectangle color to yellow, otherwise set to green
  uint16_t rectColor = highlight ? ILI9341_YELLOW : ILI9341_GREEN;
  tft.fillRect(rectX, rectY, rectWidth, rectHeight, rectColor); //Draw rectangle based on parameters

  // Set text properties
  tft.setTextColor(ILI9341_BLUE); //Set text color to blue
  tft.setTextSize(2); //Set text size to 2

  //Calculate text width and height
  int16_t textWidth, textHeight;
  uint16_t textX, textY; //Declare text position variables

  //Get text bounds
  tft.getTextBounds(text, 0, 0, &textX, &textY, &textWidth, &textHeight);

  // Calculate text position to center it within the rectangle
  textX = rectX + (rectWidth - textWidth) / 2;
  textY = rectY + (rectHeight - textHeight) / 2;

  tft.setCursor(textX, textY); //Set cursor position for printing text
  tft.print(text); //Print the text inside the rectange
}




void execute_CMD(byte CMD, byte Par1, byte Par2)
// Excecute the command and parameters
{
// Calculate the checksum (2 bytes)
word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
// Build the command line
byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};
//Send the command line to the module
for (byte k=0; k<10; k++)
{
portTwo.write( Command_line[k]);
}
}

