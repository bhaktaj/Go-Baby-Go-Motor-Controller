/* 
   Sound Controller for Go Baby Go Conversions
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.

   Authors: CJ Evans, Aya Nakayama, Carmen Quispe, and Jayesh Bhakta (bhaktaj@lacitycollege.edu)
   Institution: Los Angeles City College, Los Angeles, CA 90029
   Date: 10/27/2018 
   Version: 1.1
   
   
   
*/
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"

/* Hardware Connections to Adafruit Sound Effect Board */
#define SFX_TX 2
#define SFX_RX 3
#define SFX_RST 4
#define SFX_ACT 5
#define YELLOW_BTN 6
#define RED_BTN 7
#define GREEN_BTN 8
#define MUSIC_DISABLE 9
#define MUSIC_STOP_BTN 10

/*music file names*/
char MUSIC_FILE_YES[12]= "T0100YESOGG";
char MUSIC_FILE_NOPE[12]= "T01000NOOGG";
#define MUSIC_FILE_LAST_NUMBER 7

int last_music_track_played;

/* function declarations */
void flushInput();
void play_sound_file(int number);
void stop_sfx();


/*software serial*/
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);
boolean retVal;

void setup() 
{
  Serial.begin(115200);
  Serial.println("Adafruit Sound Board!");
  int attempt;
  int volume;
  
  /*softwareserial at 9600 baud*/
  ss.begin(9600);
  //button inputs
  
  pinMode(SFX_ACT,INPUT_PULLUP);
  pinMode(YELLOW_BTN,INPUT_PULLUP); 
  pinMode(RED_BTN,INPUT_PULLUP); 
  pinMode(GREEN_BTN,INPUT_PULLUP); 
  pinMode(MUSIC_DISABLE,INPUT_PULLUP); 
  pinMode(MUSIC_STOP_BTN,INPUT_PULLUP); 
  
  if (!sfx.reset()) 
  {
    Serial.println("Not found");
    while (1);
  }
  Serial.println("SFX board found");

  /*Set volume to max level*/
  attempt=0;
  while (attempt<20)
  { 
      flushInput();
      volume=sfx.volUp();
      if (volume>=204)
        { 
          Serial.print("Volume initially set to :");
          Serial.println(volume);
          break;
        }
      attempt++;  
  }

  last_music_track_played=0;
}


void loop() 
{
    flushInput();
    
      /*check for button presses*/
      if(!digitalRead(YELLOW_BTN))
      {
          Serial.println("Yellow Button Pressed");
          
          if(!digitalRead(SFX_ACT))
          {
            /*music is playing so stop music and play next*/
            stop_sfx();
          }  
          
          if(last_music_track_played>=MUSIC_FILE_LAST_NUMBER)
          {  
            play_sound_file(0);
            last_music_track_played=0;
          }
          else
          {
            play_sound_file(last_music_track_played++);
          }
      }

      if(!digitalRead(RED_BTN))
      {
          Serial.println("Red Button Pressed");
          
          if (!digitalRead(SFX_ACT))
          {
              stop_sfx();    
          }

          /*play "no" sound */
          if (!sfx.playTrack(MUSIC_FILE_NOPE))
          {
             Serial.println("Failed to play NO vocals");
          }
          else
          {
            //wait for the sound to complete
            delay(1000);  
          }
      }

      if(!digitalRead(GREEN_BTN))
      {
          Serial.println("Green Button Pressed");
          
          if (!digitalRead(SFX_ACT))
          {
              stop_sfx();    
          }

          /*play "yes" sound */
          if (!sfx.playTrack(MUSIC_FILE_YES))
          {
             Serial.println("Failed to play YES vocals");
          }
          else
          {
            //wait for the sound to complete
            delay(1000);  
          }
      }

      if(digitalRead(MUSIC_STOP_BTN))
      {
        Serial.println("Stop Button Pressed");
        
        if (!digitalRead(SFX_ACT))
        {
            stop_sfx();  
        }
      }

    delay(250);
}

/************************ MENU HELPERS ***************************/

void flushInput() 
{
  // Read all available serial input to flush pending data.
  uint16_t timeoutloop = 0;
  while (timeoutloop++ < 40) {
    while(ss.available()) {
      ss.read();
      timeoutloop = 0;  // If char was received reset the timer
    }
    delay(1);
  }
}

void play_sound_file(int number)
{
  char filename[12];
  char filenumber_char;
  
  /*check number is valid*/
  if(number>=0 && number<=9 && !digitalRead(MUSIC_DISABLE))
  { 
    flushInput();
    
    strcpy(filename,"T00NEXT");
    filenumber_char=48+number;
    filename[7]=filenumber_char;
    filename[8]=79; //"O"
    filename[9]=71; //"G"
    filename[10]=71; // "G"
    filename[11]=0; // NULL
    if(! sfx.playTrack(filename))
    {
      Serial.println("Failed to play track");
    }
    Serial.print("Now Playing: ");
    Serial.println(filename);
  }
}

void stop_sfx()
{
    if (!sfx.stop())
      {
        Serial.println("Failed to stop device");
      }  
}


