#include "LedControl.h"
#include "math.h"
#include "SevSeg.h"

LedControl screen = LedControl(12, 11, 10, 2);  // 12 = DIn | 11 = Clk | 10 = cs
SevSeg scoreScreen;


int yPin = A0;
int xPin = 1;  // not in use
int swPin = 7;

int xValue = 2;  // doesnt change
int newYValue = 0;
int oldYValue = 0;
int display = 0;

int row = 7;         // place of obstacle
int rowDisplay = 1;  // display of obstacle
int gap = 0xFF;      // place of Gap in obstacle

bool firstFrame = true;
int gameSpeed = 500;
int playerSpeed = gameSpeed / 5;

unsigned long oldGameTime;
unsigned long oldPlayerTime;
int mode = 1;  // 0 = idle 1 = game
int score = 0;

int buzzer = 32;  // buzzer pin
int melody = 0;
int soundlength = 0;
unsigned long soundTime;
bool soundOn = false;


void clearDisplay() {
  for (int matrix = 0; matrix < screen.getDeviceCount(); matrix++)
    screen.clearDisplay(matrix);
}


void displayDot() {
  //screen.setLed(display, xValue, yValue - 1, false);
  //screen.setLed(display, xValue, yValue + 1, false);
  screen.setLed(display, xValue, oldYValue, true);
}

/* Liest X Input des Joysticks !! Kabel nach Oben !!
  gibt zurück:
  Links -> -1
  Mittig -> 0
  Rechts -> 1
*/
int readX() {  // Links = 0 | mitte = 517 | rechts = 1023
  //return analogRead(0);
  int x = analogRead(xPin);
  if (x >= 500 && x <= 600) return 0;
  else return x - 600 <= 0 ? 1 : -1;
}

/* List Y Input des Joysticks !! Kabel nach Oben !!
  gibt zurück:
  Unten -> -1
  mittig -> 0
  Oben -> 1
*/
int readY() {  // unten = 1023 | mitte = 526 | oben = 0
  //return analogRead(1);
  int y = analogRead(yPin);
  if (y >= 500 && y <= 600) return 0;
  else return y - 600 <= 0 ? 1 : -1;
}

int readButton() {
  return !digitalRead(swPin);
}

void resetRow() {
  gap = random(6);
}


int scrollLeft(int row) {
  int newPos = row - 1;
  if (newPos == -1) {
    if (rowDisplay == 1) {
      rowDisplay = 0;
      newPos = 7;
    } else {
      newPos = 7;
      rowDisplay = 1;
      resetRow();
    }
  } else if (newPos == 2 && rowDisplay == 0) checkGameOver();
  return newPos;
}

void setGap() {
  screen.setLed(rowDisplay, row, gap, false);
  screen.setLed(rowDisplay, row, gap + 1, false);
}

void playScoreSound() {
  if (soundOn) {
    int newTime = millis();
    if (newTime - soundTime >= soundlength) {
      switch (melody) {
        case 0:
          tone(buzzer, 587);
          melody++;
          soundlength = 100;
          break;
        case 1:
          tone(buzzer, 698);
          melody++;
          soundlength = 100;
          break;
        case 2:
          tone(buzzer, 784);
          melody++;
          soundlength = 200;
          break;

        default:
          noTone(buzzer);
          melody = 0;
          soundOn = false;
          soundlength = 0;
      }
      soundTime = newTime;
    }
  }
}

void restartGame() {
  row = 7;
  rowDisplay = 1;
  resetRow();
  gameSpeed = 500;
  score = 0;
  mode = 1;
}

void checkGameOver() {
  if (oldYValue != gap && oldYValue != gap + 1) {
    mode = 0;
    clearDisplay();
  } else {
    scoreUp();
  }
}

void scoreUp() {
  score++;
  soundOn = true;
  if (score % 2 == 0) {
    if (gameSpeed >= 50) {
      gameSpeed -= gameSpeed / ;
    }
  }
}

void moveDot() {
  if (firstFrame) {
    firstFrame = false;
    newYValue += readY();
    if (newYValue != oldYValue) {
      screen.setLed(display, xValue, oldYValue, false);
      oldYValue = newYValue;
    }
  }
  if (newYValue <= -1) newYValue = 7;  // looping around
  if (newYValue >= 8) newYValue = 0;
}





void setup() {
  Serial.begin(9600);

  randomSeed(analogRead(4));

  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(swPin, INPUT_PULLUP);

  pinMode(buzzer, OUTPUT);

  for (int matrix = 0; matrix < screen.getDeviceCount(); matrix++) {
    screen.shutdown(matrix, false);
    screen.setIntensity(matrix, 8);
    screen.clearDisplay(matrix);
  }

  byte numDigits = 4;
  byte digitPins[] = { 2, 3, 4, 5 };
  byte segmentPins[] = { 52, 53, 46, 44, 40, 48, 50, 42 };
  bool resistorsOnSegments = 0;
  // variable above indicates that 4 resistors were placed on the digit pins.
  // set variable to 1 if you want to use 8 resistors on the segment pins.
  scoreScreen.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins, resistorsOnSegments);
  scoreScreen.setBrightness(90);


  resetRow();
  oldGameTime = millis();
  oldPlayerTime = millis();
}

void loop() {
  Serial.print("Y = ");
  Serial.println(oldYValue);


  if (mode) {
    // =====================
    // ----- GAME MODE -----
    // =====================

    scoreScreen.setNumber(score);

    setGap();
    moveDot();
    displayDot();

    unsigned long newGameTime = millis();  // delay for moving bar
    if (newGameTime - oldGameTime >= gameSpeed) {
      oldGameTime = newGameTime;
      row = scrollLeft(row);
      clearDisplay();
      screen.setRow(rowDisplay, row, 0xFF);
    }

    unsigned long newPlayerTime = millis();  // delay for moving player
    if (newPlayerTime - oldPlayerTime >= playerSpeed) {
      firstFrame = true;
      oldPlayerTime = newPlayerTime;
    }

    playScoreSound();

  } else {
    //=====================
    // ----- Stand-by -----
    //=====================

    scoreScreen.setNumber(score);

    if (readButton()) restartGame();
  }

  scoreScreen.refreshDisplay();
}
