#include <DFRobotDFPlayerMini.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "SPI.h" 

#define TFT_DC 9
#define TFT_CS 10
#define POTENTIOMETER_PIN A0

//Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

uint8_t selectedLineScreen1; //Set the line to highlight for screen 1
uint8_t selectedLineScreen2; //Set the line to highlight for screen 2
uint8_t selectedLineScreen3; //Set the line to highlight for screen 3

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))   // Using a soft serial port
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/6, /*tx =*/7);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

// Declare Variables for keeping track of preset
int reverb_effect = 0;
int distortion_effect = 0;
int echo_effect = 0;
int preset = 0;
int previous_preset = 1;
int volume = 20;
int selected = 0;
int effect_count = 0;
int effect_on = 0;
int effect_select = 0;

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

DFRobotDFPlayerMini myDFPlayer;

void setup() {
  #if (defined ESP32)
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/D3, /*tx =*/D2);
  #else
  FPSerial.begin(9600);
  #endif

  Serial.begin(115200);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms

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

  //initialize the pushbutton pins as inputs:
  pinMode(buttonPinUp, INPUT_PULLUP);
  pinMode(buttonPinDown, INPUT_PULLUP);
  pinMode(selectButtonPin, INPUT_PULLUP);

  selectedLineScreen1 = preset;
  PRESET1_WINDOW_MAIN(); //Display Preset Screen Window 1

  // Attach interrupts for up and down buttons
  attachInterrupt(digitalPinToInterrupt(buttonPinUp), countUp, LOW);
  attachInterrupt(digitalPinToInterrupt(buttonPinDown), countDown, LOW);
  myDFPlayer.volume(volume);  //Set volume value (0~30).
  myDFPlayer.enableLoop();
}

void loop() {
  
  // Check select button and update volume if needed
  checkSelectButton();
  readPotentiometer();
  myDFPlayer.volume(volume);
}



void update_preset () {
  if (preset <= 8) {

    if(effect_on == 0) {
      selectedLineScreen1 = preset;
      PRESET1_WINDOW_MAIN(); //Display Preset Screen Window 1
    }

    if (selected == 1 && preset < 8 && effect_on == 0) {
      AUDIO_EFFECTS_WINDOW_MAIN();
      effect_on = 1;
    }

    if (effect_on == 1 && effect_count < 9) {
      selectedLineScreen3 = effect_count;
      AUDIO_EFFECTS_WINDOW_MAIN();

    }
    if (effect_on == 1 && effect_select == 1) {
      if(effect_count == 8) {
        effect_count = 0;
        effect_on = 0;
        effect_select = 0;
        selected = 0;
        selectedLineScreen1 = preset;
        PRESET1_WINDOW_MAIN();
      }
      else {
        choose_mp3();
        effect_select = 0;
      }
  }

    // Handles next page press
  if (preset == 8 && selected == 1) { 
    preset = 9;
    selected = 0;
  }
  if (preset == 16 && selected == 1) {
    preset = 0;
    selected = 0;
    selectedLineScreen1 = preset;
    PRESET1_WINDOW_MAIN(); //Display Preset Screen Window 1
  }

  if (preset > 8) {

    if(effect_on == 0){
      selectedLineScreen2 = preset - 1;
      PRESET2_WINDOW_MAIN(); //Display Preset Screen Window 2
    }

    if (selected == 1 && preset > 8 && effect_on == 0) {
      AUDIO_EFFECTS_WINDOW_MAIN();
      effect_on = 1;
    }

    if (effect_on == 1 && effect_count < 9) {
      selectedLineScreen3 = effect_count;
      AUDIO_EFFECTS_WINDOW_MAIN();

    }
    if (effect_on == 1 && effect_select == 1) {
      if(effect_count == 8) {
        effect_count = 0;
        effect_on = 0;
        effect_select = 0;
        selected = 0;
        selectedLineScreen1 = preset;
        PRESET2_WINDOW_MAIN();
      }
      else {
        choose_mp3();
        effect_select = 0;
      }
    }
  
  }  
}

void countUp() {
  //check if the pushbutton for counting up is pressed:
  if (buttonStateUp == LOW) {
   
    //Increase count only if it wasn't already pressed and count is less than 16
    if (digitalRead(buttonPinUp) == LOW && preset < 16 && preset != 8 && effect_on != 1) {
      preset++;
      Serial.print("Count: ");
      Serial.println(preset);
      update_preset();
      // Wait until the button is released
      while (digitalRead(buttonPinUp) == LOW) {
        delay(10);
      }
   
    }
    if (digitalRead(buttonPinUp) == HIGH && effect_on == 1 && effect_count < 8) {
   
      effect_count++;
      Serial.print("Effect Count: ");
      Serial.println(effect_count);
      update_preset();
    }
  }   
}

void countDown() {
  //read the state of the pushbutton for counting down:
  buttonStateDown = digitalRead(buttonPinDown);

  //check if the pushbutton for counting down is pressed:
  if (buttonStateDown == LOW) {
    //turn LED on:
  
    //Decrease count only if it wasn't already pressed and count is greater than 0
    if (digitalRead(buttonPinDown) == LOW && preset > 0 && effect_on != 1) {
      preset--;
      Serial.print("Count: ");
      Serial.println(preset);
      update_preset();
      //Wait until the button is released
      while (digitalRead(buttonPinDown) == LOW) {
        delay(10);
      }
     
    }

    if (digitalRead(buttonPinDown) == LOW && effect_on == 1 && effect_count > 0) {
      effect_count--;
       Serial.print("Effect Count: ");
      Serial.println(effect_count);
      update_preset();
    }
  }
}

void checkSelectButton() {
  //read the state of the select button:
  selectButtonState = digitalRead(selectButtonPin);

  //check if the select button is pressed:
  if (selectButtonState == LOW && selected != 1) {
    selected = 1;
    
    Serial.print("Select: ");
    Serial.println(selected);
   // delay(50); //debounce delay
    update_preset();
    while (digitalRead(selectButtonPin) == LOW) {
      delay(10);
    }
    
  }

  if (selectButtonState == LOW && selected == 1) {
    effect_select = 1;
    update_preset();
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

void choose_mp3() {
        // OG
        if(effect_count == 0) {
          myDFPlayer.playFolder(preset, 1);
        }
        // Reverb
        if(effect_count == 1) {
          myDFPlayer.playFolder(preset, 2);
        }
        // Echo
        else if(effect_count == 3) {
          myDFPlayer.playFolder(preset, 3);
        }
        // Distortion
        else if(effect_count == 2) {
          myDFPlayer.playFolder(preset, 4);
        }
        // Reverb & Echo
        else if(effect_count == 5) {
          myDFPlayer.playFolder(preset, 5);
        }
        // Reverb & Distortion
        else if(effect_count == 4) {
          myDFPlayer.playFolder(preset, 6);
        }
        // Echo & Distortion
        else if(effect_count == 6) {
          myDFPlayer.playFolder(preset, 7);
        }
        // All Effects
        else if(effect_count == 7) {
          myDFPlayer.playFolder(preset, 8);
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
    INIT_PRESET2_WINDOW(presetText.c_str(), i,  (i + 8) == selectedLineScreen2); //Adjusted selected line
  }

  //Determine highlight condition for "GO TO PRESET 1 WINDOW"
  bool highlightGoToPreset1 = (selectedLineScreen2 == 15);

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
  bool highlightGoToPresets = ( selectedLineScreen3 == 8);

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
