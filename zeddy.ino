
/* Simon Sings - a memory game for 4 leds, 4 switches and a buzzer.
 "Sings" is the sound-enhanced version of "Fairly Simple Simon".
 No relation to the Singh family :)
 
 Breadboard diagram, video, etc. at http://bit.ly/simonsings
 
 After reset, Simon plays a bar from "Simple Simon says", and
 you start playing a game at level 4.
 
 Playing a game at level N:
 1) Simon picks a random sequence of N LED flashes.
 2) Simon waits for you to press any button when ready.
 3) Simon "says" the sequence. Memorize it.
 4) You should then repeat the sequence on the buttons.
    * If you're right, Simon plays the song, and you start
      a level N+1 game at step 1.
    * If you push a wrong button, Simon plays a sad tune,
      and you go back to step 2.
 
 cc-by-sa by @TheRealDod,
 Nov 23, 2010
 */
#include <Wtv020sd16p.h>
//#include "pitches.h"
int led = 11;
int motorPin = 13;
long randNumber;

int resetPin = 2;  // The pin number of the reset pin.
int clockPin = 3;  // The pin number of the clock pin.
int dataPin = 4;  // The pin number of the data pin.
int busyPin = 5;  // The pin number of the busy pin.
Wtv020sd16p wtv020sd16p(resetPin,clockPin,dataPin,busyPin);


const int NLEDS = 4; 
const int LEDPINS[NLEDS] = {
  6.7,9,10}; // Need to be PWM pins, and we need 3 and 11 free (for tone())
const int SWITCHPINS[NLEDS] = {
  15,16,17,18}; // Analog inputs 1-4
const int SWITCHPRESSED = 1; // 1 or 0, for normally-open/closed switches
//const int SPEAKERPIN = 8;
const int NOTES[NLEDS] = {
  0, 0, 0, 0};

const int FADESTEPS = 8;
const int FADEINDURATION = 200;
const int FADEOUTDURATION = 150;
const int SEQDELAY = 50; // Millis between led flashes.
const int PAUSEB4SEQ = 500; // Millis before starting the sequence.
const int MINLEVEL = 4;
const int MAXLEVEL = 16;
int gameLevel;
int simonSez[MAXLEVEL]; // sequence of 0..NLEDS-1


// -- song-array note fields --
// Tone
const int NOTETONE = 0;
const int SILENCE = 0;
const int ENDOFSONG = -1;
// Duration
const int NOTEDURATION = 1;
const int SINGLEBEAT = 125; // Note duration (millis) is multiplied by this
const float PAUSEFACTOR=0.2; // relative length of silence after playing a note
// LED
const int NOTELED = 2;
const int ALLLEDS = -1;

int LEDSONG[][3] = {
  {SILENCE,2,ALLLEDS}
  ,{SILENCE,1,2}
  ,{SILENCE,1,2}
  ,{SILENCE,1,2}
  ,{SILENCE,1,3}
  ,{SILENCE,1,2}
  ,{SILENCE,3,1}
  ,{SILENCE,1,ALLLEDS}
  ,{SILENCE,1,ALLLEDS}
  ,{SILENCE,1,ALLLEDS}
  ,{SILENCE,2,ALLLEDS}
  ,{SILENCE,5,ALLLEDS}
  ,{ENDOFSONG,ENDOFSONG,ENDOFSONG}
};

//int WINSONG[][3] = {
//  {SILENCE,2,ALLLEDS}
//  ,{SILENCE,1,2}
//  ,{SILENCE,1,2}
//  ,{SILENCE,1,2}
//  ,{SILENCE,1,3}
//  ,{SILENCE,1,2}
//  ,{SILENCE,3,1}
//  ,{SILENCE,1,ALLLEDS}
//  ,{SILENCE,1,ALLLEDS}
//  ,{SILENCE,1,ALLLEDS}
//  ,{SILENCE,2,ALLLEDS}
//  ,{SILENCE,5,ALLLEDS}
//  ,{ENDOFSONG,ENDOFSONG,ENDOFSONG}
//};

int LOSESONG[][3] = {
  {SILENCE,2,3},{SILENCE,2,2},{SILENCE,2,1},{SILENCE,8,ALLLEDS},{ENDOFSONG,ENDOFSONG,ENDOFSONG}
};

void setup() {
  
  //Initializes the module.
  wtv020sd16p.reset();
  
  // Analog in 0 should *not* be connected.
  // It's mama's little PRNG :)
  randomSeed(analogRead(0));
  //pinMode(SPEAKERPIN,OUTPUT);
  //noTone(SPEAKERPIN);
  gameLevel=MINLEVEL;
  for (byte l=0; l<NLEDS; l++) {
    pinMode(LEDPINS[l], OUTPUT);
    pinMode(led, OUTPUT);  
    pinMode(motorPin, OUTPUT);     
   

  }
  //Serial.begin(9600);
  // Visual feedback after reset. Also good as a cable check :)
//  playLedSequence(); 
}

void loop() {
  int done;
  initGameSequence(gameLevel);
  done = 0;
  while (!done) {
    getSwitchStroke();
    delay(PAUSEB4SEQ);
    playGameSequence(gameLevel);
    if (playerGuess(gameLevel)) {
      playWinSequence();
      done = 1;
      if (gameLevel<MAXLEVEL) {
        gameLevel++;
      }
    } 
    else {
      
      //      randNumber = random(0, 3);
       //     wtv020sd16p.playVoice(randNumber); //  Aldo: Lose sequence sounds - Now in Lose Func
            playLoseSequence(); // OG
            eyes(); // Erratic eye blink
      
    }
  }
}

void eyes () {
        for (int i=0; i <= 75; i++){
          
          if (i < 75) 
            randNumber = random(10, 150);

              digitalWrite(led, HIGH);
              digitalWrite(motorPin, HIGH);
              delay(randNumber);
              digitalWrite(led, LOW);
              digitalWrite(motorPin, LOW);
              delay(randNumber);
           }  // Aldo: Erratic eye blink
}

void initGameSequence(int gameLevel) {
  // assertion: gameLevel<=MAXLEVEL
  for (int i=0; i<gameLevel; i++) {
    simonSez[i]=random(NLEDS);
  }
}

void playGameSequence(int gameLevel) {
  for (int i=0; i<gameLevel; i++) {
    playLed(simonSez[i]); // Fade LED and play its note
  }
}

void fadeLed(int theLed,int val,int duration) {
  int fadeStep=256/FADESTEPS;
  int fadeDelay=duration/FADESTEPS;
  for (int i=0; i<256; i+=fadeStep) {
    if (theLed>=0) {
      analogWrite(LEDPINS[theLed],val?i:255-i);
    } 
    else { // ALLLEDS
      for (int j=0; j<NLEDS; j++) {
        analogWrite(LEDPINS[j],val?i:255-i);
      }
    }
    delay(fadeDelay);
  }
  // force val (in case fadeStep doesn't divide 256)
  if (theLed>=0) {
    digitalWrite(LEDPINS[theLed],val); 
  }
  else {
    for (int j=0; j<NLEDS; j++) {
      digitalWrite(LEDPINS[j],val); 
    }
  }
}

void playLed(int theLed) { // Fade LED and play its note
 // tone(SPEAKERPIN,NOTES[theLed]);
  fadeLed(theLed,HIGH,FADEINDURATION); // Fade in
 // noTone(SPEAKERPIN);
  fadeLed(theLed,LOW,FADEOUTDURATION); // Fade out
  wtv020sd16p.playVoice(LEDPINS[theLed]); // Aldo: Audio track # to led pin # 6, 7, 9, 10
  delay(1350);
}

int playerGuess(int gameLevel) {
  for (int i=0 ; i<gameLevel ; i++) {
    int guess=getSwitchStroke();
    //Serial.print(guess,DEC);
    //Serial.print(",");
    //Serial.println(simonSez[i]);
    if (guess!=simonSez[i]) {
      return 0;
    } 
    else {
      playLed(guess); // Fade LED and play its note
    }
  }
  return 1;
}



int playWinSequence() {
            wtv020sd16p.playVoice(8); //  Aldo: Lose sequence sounds

}

int playLoseSequence() {
            randNumber = random(0, 3);

            wtv020sd16p.playVoice(randNumber); //  Aldo: Lose sequence sounds

}

int getSwitchStroke() {
  while (get1stPressedSwitch()>=0) {
    // flush everything until no switch is pressed
    delay(50);
  }
  while (get1stPressedSwitch()<0) {
    // wait for next press
    delay(50);
  }
  return get1stPressedSwitch();
}

int get1stPressedSwitch() {
  for (int i=0; i<NLEDS; i++) {
    if (digitalRead(SWITCHPINS[i])==SWITCHPRESSED) {
      return i;
    }
  }
  return -1;
}









