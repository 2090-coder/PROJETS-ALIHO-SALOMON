#include <Arduino.h>
#include <Wire.h>

// ============================================================
// NOVAWATCH
// Arduino Nano + MAX7219 + DS3231 + 7x74HC595 + 7xULN2803A
// ============================================================

// -------------------- Arduino pins ---------------------------
const uint8_t PIN_BUTTON_POWER = 2;
const uint8_t PIN_BUTTON_MODE  = 3;
const uint8_t PIN_BUTTON_PLUS  = 4;
const uint8_t PIN_BUTTON_MINUS = 5;
const uint8_t PIN_BUZZER       = 6;
const uint8_t PIN_COLON        = 7;

const uint8_t PIN_595_DATA  = 8;   // DS pin 14
const uint8_t PIN_595_LATCH = 9;   // STCP pin 12
const uint8_t PIN_MAX_CS    = 10;  // LOAD/CS pin 12
const uint8_t PIN_MAX_DATA  = 11;  // DIN pin 1
const uint8_t PIN_595_CLOCK = 13;  // SHCP pin 11
const uint8_t PIN_MAX_CLOCK = 13;  // CLK pin 13

const uint8_t RTC_ADDRESS = 0x68;

// -------------------- MAX7219 registers ----------------------
const uint8_t MAX_REG_DECODE      = 0x09;
const uint8_t MAX_REG_INTENSITY   = 0x0A;
const uint8_t MAX_REG_SCANLIMIT   = 0x0B;
const uint8_t MAX_REG_SHUTDOWN    = 0x0C;
const uint8_t MAX_REG_DISPLAYTEST = 0x0F;

// Segment bits: A B C D E F G DP
const uint8_t SEG_A  = 0x01;
const uint8_t SEG_B  = 0x02;
const uint8_t SEG_C  = 0x04;
const uint8_t SEG_D  = 0x08;
const uint8_t SEG_E  = 0x10;
const uint8_t SEG_F  = 0x20;
const uint8_t SEG_G  = 0x40;

const uint8_t DIGIT_MASKS[10] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
  SEG_B | SEG_C,
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,
  SEG_B | SEG_C | SEG_F | SEG_G,
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
  SEG_A | SEG_B | SEG_C,
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G
};

// -------------------- Contour -------------------------------
// 51 groups = 17 red + 17 green + 17 blue.
// Each group controls 4 LEDs of the same color through one ULN2803A channel.
const uint8_t SHIFT_REG_COUNT = 7;
const uint8_t CONTOUR_GROUPS = 51;
uint8_t contourRegisters[SHIFT_REG_COUNT] = {0};

const unsigned long CONTOUR_INTERVAL = 80;
unsigned long lastContourUpdate = 0;
uint8_t contourPosition = 0;

// -------------------- RTC data -------------------------------
uint8_t rtcHour = 0;
uint8_t rtcMinute = 0;
uint8_t rtcSecond = 0;
unsigned long lastRtcRead = 0;

// -------------------- Main states ----------------------------
bool watchOn = false;
bool startupActive = false;
unsigned long startupStarted = 0;
uint8_t startupStep = 0;
unsigned long lastStartupStep = 0;
const unsigned long STARTUP_DURATION = 1600;
const unsigned long STARTUP_STEP_INTERVAL = 100;

// -------------------- Edit mode ------------------------------
enum EditField {
  EDIT_HOUR,
  EDIT_MINUTE
};

bool editMode = false;
EditField editField = EDIT_HOUR;
uint8_t editHour = 0;
uint8_t editMinute = 0;
bool editBlinkVisible = true;
unsigned long lastBlink = 0;
const unsigned long BLINK_INTERVAL = 350;

// -------------------- Button 2 click detection ---------------
// Specification: 1, 2 or 3 presses inside one second.
const unsigned long MULTI_CLICK_WINDOW = 1000;
uint8_t modeClickCount = 0;
unsigned long lastModeClickTime = 0;

const unsigned long DEBOUNCE_MS = 35;

struct ButtonState {
  uint8_t pin;
  bool lastReading;
  bool stableState;
  unsigned long lastChange;
};

ButtonState buttonPower = {PIN_BUTTON_POWER, HIGH, HIGH, 0};
ButtonState buttonPlus  = {PIN_BUTTON_PLUS, HIGH, HIGH, 0};
ButtonState buttonMinus = {PIN_BUTTON_MINUS, HIGH, HIGH, 0};
ButtonState buttonMode  = {PIN_BUTTON_MODE, HIGH, HIGH, 0};

// -------------------- Buzzer melody --------------------------
struct Note {
  uint16_t frequency;
  uint16_t duration;
};

const Note startupMelody[] = {
  {523, 120}, {659, 120}, {784, 120}, {1047, 220},
  {784, 120}, {659, 120}, {523, 250}
};
const uint8_t STARTUP_MELODY_COUNT = sizeof(startupMelody) / sizeof(startupMelody[0]);
uint8_t melodyIndex = 0;
unsigned long melodyNextTime = 0;

// ============================================================
// Utility
// ============================================================
uint8_t bcdToDec(uint8_t value) {
  return ((value >> 4) * 10) + (value & 0x0F);
}

uint8_t decToBcd(uint8_t value) {
  return ((value / 10) << 4) | (value % 10);
}

// ============================================================
// MAX7219
// ============================================================
void maxWrite(uint8_t reg, uint8_t value) {
  digitalWrite(PIN_MAX_CS, LOW);
  shiftOut(PIN_MAX_DATA, PIN_MAX_CLOCK, MSBFIRST, reg);
  shiftOut(PIN_MAX_DATA, PIN_MAX_CLOCK, MSBFIRST, value);
  digitalWrite(PIN_MAX_CS, HIGH);
}

void maxClear() {
  for (uint8_t digit = 1; digit <= 8; digit++) {
    maxWrite(digit, 0x00);
  }
}

void maxInit() {
  pinMode(PIN_MAX_CS, OUTPUT);
  pinMode(PIN_MAX_DATA, OUTPUT);
  pinMode(PIN_MAX_CLOCK, OUTPUT);

  digitalWrite(PIN_MAX_CS, HIGH);

  maxWrite(MAX_REG_DISPLAYTEST, 0x00);
  maxWrite(MAX_REG_DECODE, 0x00);
  maxWrite(MAX_REG_SCANLIMIT, 0x03);
  maxWrite(MAX_REG_INTENSITY, 0x06);
  maxWrite(MAX_REG_SHUTDOWN, 0x01);
  maxClear();
}

void maxSetDigit(uint8_t digitIndex, uint8_t mask) {
  if (digitIndex > 3) return;
  maxWrite(digitIndex + 1, mask);
}

// ============================================================
// Colon
// ============================================================
void setColon(bool state) {
  digitalWrite(PIN_COLON, state ? HIGH : LOW);
}

// ============================================================
// Display
// ============================================================
void displayTime(uint8_t hour, uint8_t minute) {
  uint8_t hTens = hour / 10;
  uint8_t hUnits = hour % 10;
  uint8_t mTens = minute / 10;
  uint8_t mUnits = minute % 10;

  if (!editMode || editField != EDIT_HOUR || editBlinkVisible) {
    maxSetDigit(0, DIGIT_MASKS[hTens]);
    maxSetDigit(1, DIGIT_MASKS[hUnits]);
  } else {
    maxSetDigit(0, 0x00);
    maxSetDigit(1, 0x00);
  }

  if (!editMode || editField != EDIT_MINUTE || editBlinkVisible) {
    maxSetDigit(2, DIGIT_MASKS[mTens]);
    maxSetDigit(3, DIGIT_MASKS[mUnits]);
  } else {
    maxSetDigit(2, 0x00);
    maxSetDigit(3, 0x00);
  }

  setColon(true);
}

void displayStartupFrame(uint8_t step) {
  uint8_t mask = 0;

  if (step > 0) mask |= SEG_A;
  if (step > 1) mask |= SEG_B;
  if (step > 2) mask |= SEG_C;
  if (step > 3) mask |= SEG_D;
  if (step > 4) mask |= SEG_E;
  if (step > 5) mask |= SEG_F;
  if (step > 6) mask |= SEG_G;

  for (uint8_t i = 0; i < 4; i++) {
    maxSetDigit(i, mask);
  }

  setColon(step % 2 == 0);
}

// ============================================================
// DS3231
// ============================================================
void rtcRead() {
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) return;

  if (Wire.requestFrom(RTC_ADDRESS, (uint8_t)3) != 3) return;

  rtcSecond = bcdToDec(Wire.read() & 0x7F);
  rtcMinute = bcdToDec(Wire.read() & 0x7F);
  rtcHour = bcdToDec(Wire.read() & 0x3F);
}

void rtcSetTime(uint8_t hour, uint8_t minute, uint8_t second) {
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0x00);
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.endTransmission();
}

// ============================================================
// 74HC595 contour driver
// ============================================================
void clearContourRegisters() {
  for (uint8_t i = 0; i < SHIFT_REG_COUNT; i++) {
    contourRegisters[i] = 0;
  }
}

void setContourGroup(uint8_t group, bool state) {
  if (group >= CONTOUR_GROUPS) return;

  uint8_t chip = group / 8;
  uint8_t bit = group % 8;

  if (state) {
    contourRegisters[chip] |= (1 << bit);
  } else {
    contourRegisters[chip] &= ~(1 << bit);
  }
}

void writeContourRegisters() {
  digitalWrite(PIN_595_LATCH, LOW);

  // 74HC595 #1 is closest to Arduino DS.
  // Send #7 first so the last 8 bits arrive in #1.
  for (int8_t chip = SHIFT_REG_COUNT - 1; chip >= 0; chip--) {
    shiftOut(PIN_595_DATA, PIN_595_CLOCK, LSBFIRST, contourRegisters[chip]);
  }

  digitalWrite(PIN_595_LATCH, HIGH);
}

void contourInit() {
  pinMode(PIN_595_DATA, OUTPUT);
  pinMode(PIN_595_LATCH, OUTPUT);
  pinMode(PIN_595_CLOCK, OUTPUT);

  digitalWrite(PIN_595_LATCH, LOW);
  clearContourRegisters();
  writeContourRegisters();
}

void updateContourAnimation() {
  if (!watchOn) return;
  if (millis() - lastContourUpdate < CONTOUR_INTERVAL) return;

  lastContourUpdate = millis();

  clearContourRegisters();
  setContourGroup(contourPosition, true);
  writeContourRegisters();

  contourPosition++;
  if (contourPosition >= CONTOUR_GROUPS) {
    contourPosition = 0;
  }
}

// ============================================================
// Buzzer
// ============================================================
void startStartupMelody() {
  melodyIndex = 0;
  melodyNextTime = 0;
}

void updateStartupMelody() {
  if (!startupActive) {
    noTone(PIN_BUZZER);
    return;
  }

  unsigned long now = millis();
  if (now < melodyNextTime) return;

  if (melodyIndex >= STARTUP_MELODY_COUNT) {
    noTone(PIN_BUZZER);
    melodyNextTime = now + 100000UL;
    return;
  }

  tone(PIN_BUZZER, startupMelody[melodyIndex].frequency, startupMelody[melodyIndex].duration - 10);
  melodyNextTime = now + startupMelody[melodyIndex].duration;
  melodyIndex++;
}

void beepForDigit(uint8_t digit) {
  static const uint16_t frequencies[10] = {
    262, 294, 330, 349, 392, 440, 494, 523, 587, 659
  };
  tone(PIN_BUZZER, frequencies[digit % 10], 70);
}

void beepAction() {
  tone(PIN_BUZZER, 880, 55);
}

// ============================================================
// Startup animation
// ============================================================
void startStartup() {
  startupActive = true;
  startupStarted = millis();
  lastStartupStep = 0;
  startupStep = 0;
  contourPosition = 0;
  startStartupMelody();
  maxWrite(MAX_REG_SHUTDOWN, 0x01);
}

void updateStartup() {
  if (!startupActive) return;

  unsigned long now = millis();

  if (now - lastStartupStep >= STARTUP_STEP_INTERVAL) {
    lastStartupStep = now;
    if (startupStep < 7) startupStep++;
    displayStartupFrame(startupStep);
  }

  if (now - startupStarted >= STARTUP_DURATION) {
    startupActive = false;
    noTone(PIN_BUZZER);
    rtcRead();
    displayTime(rtcHour, rtcMinute);
  }
}

// ============================================================
// Buttons
// ============================================================
bool buttonPressed(ButtonState &button) {
  bool reading = digitalRead(button.pin);

  if (reading != button.lastReading) {
    button.lastChange = millis();
    button.lastReading = reading;
  }

  if ((millis() - button.lastChange) > DEBOUNCE_MS && reading != button.stableState) {
    button.stableState = reading;
    if (button.stableState == LOW) {
      return true;
    }
  }

  return false;
}

void resetClock() {
  rtcSetTime(0, 0, 0);
  rtcHour = 0;
  rtcMinute = 0;
  rtcSecond = 0;
  editHour = 0;
  editMinute = 0;
  beepAction();
}

void enterEditMode() {
  rtcRead();
  editHour = rtcHour;
  editMinute = rtcMinute;
  editField = EDIT_HOUR;
  editMode = true;
  editBlinkVisible = true;
  lastBlink = millis();
  noTone(PIN_BUZZER);
  beepAction();
}

void validateEditMode() {
  rtcSetTime(editHour, editMinute, 0);
  rtcHour = editHour;
  rtcMinute = editMinute;
  rtcSecond = 0;
  editMode = false;
  editBlinkVisible = true;
  displayTime(rtcHour, rtcMinute);
  beepAction();
}

void processModeClicks() {
  if (modeClickCount == 0) return;

  unsigned long now = millis();
  if (now - lastModeClickTime < MULTI_CLICK_WINDOW) return;

  if (!editMode) {
    if (modeClickCount == 1) {
      resetClock();
    } else if (modeClickCount == 2) {
      enterEditMode();
    }
  } else {
    if (modeClickCount == 1) {
      // In edit mode, one press switches HOURS <-> MINUTES.
      editField = (editField == EDIT_HOUR) ? EDIT_MINUTE : EDIT_HOUR;
      editBlinkVisible = true;
      lastBlink = now;
      beepAction();
    } else if (modeClickCount == 3) {
      validateEditMode();
    }
  }

  modeClickCount = 0;
}

void handleButtons() {
  if (buttonPressed(buttonPower)) {
    watchOn = !watchOn;

    if (watchOn) {
      startStartup();
    } else {
      startupActive = false;
      editMode = false;
      noTone(PIN_BUZZER);
      maxWrite(MAX_REG_SHUTDOWN, 0x00);
      maxClear();
      setColon(false);
      clearContourRegisters();
      writeContourRegisters();
    }
  }

  if (!watchOn || startupActive) return;

  if (buttonPressed(buttonMode)) {
    unsigned long now = millis();

    modeClickCount++;
    lastModeClickTime = now;
  }

  if (editMode) {
    if (buttonPressed(buttonPlus)) {
      if (editField == EDIT_HOUR) {
        editHour = (editHour + 1) % 24;
        beepForDigit(editHour % 10);
      } else {
        editMinute = (editMinute + 1) % 60;
        beepForDigit(editMinute % 10);
      }
      editBlinkVisible = true;
      lastBlink = millis();
    }

    if (buttonPressed(buttonMinus)) {
      if (editField == EDIT_HOUR) {
        editHour = (editHour == 0) ? 23 : editHour - 1;
        beepForDigit(editHour % 10);
      } else {
        editMinute = (editMinute == 0) ? 59 : editMinute - 1;
        beepForDigit(editMinute % 10);
      }
      editBlinkVisible = true;
      lastBlink = millis();
    }
  }

  processModeClicks();
}

// ============================================================
// Edit blink
// ============================================================
void updateEditBlink() {
  if (!editMode || startupActive) return;

  if (millis() - lastBlink >= BLINK_INTERVAL) {
    lastBlink = millis();
    editBlinkVisible = !editBlinkVisible;
    displayTime(editHour, editMinute);
  }
}

// ============================================================
// Setup
// ============================================================
void setup() {
  pinMode(PIN_BUTTON_POWER, INPUT_PULLUP);
  pinMode(PIN_BUTTON_MODE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_PLUS, INPUT_PULLUP);
  pinMode(PIN_BUTTON_MINUS, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_COLON, OUTPUT);

  digitalWrite(PIN_COLON, LOW);

  Wire.begin();
  maxInit();
  contourInit();
  rtcRead();

  watchOn = false;
  startupActive = false;
  editMode = false;
}

// ============================================================
// Loop
// ============================================================
void loop() {
  handleButtons();

  if (!watchOn) return;

  updateStartupMelody();
  updateStartup();
  updateContourAnimation();

  if (startupActive) return;

  if (millis() - lastRtcRead >= 250) {
    lastRtcRead = millis();

    if (!editMode) {
      rtcRead();
      displayTime(rtcHour, rtcMinute);
    }
  }

  updateEditBlink();
}
