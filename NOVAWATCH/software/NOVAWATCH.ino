#include <Wire.h>

// ============================================================
// NOVAWATCH
// Arduino Nano + MAX7219 + DS3231 + 7x74HC595 + 7xULN2803A
// Alimentation principale : 12 V DC
// Affichage : 114 LED
// Contour : 40 groupes x 4 LED = 160 LED
// ============================================================

const byte PIN_BUTTON_POWER = 2;
const byte PIN_BUTTON_MODE  = 3;
const byte PIN_BUTTON_PLUS  = 4;
const byte PIN_BUTTON_MINUS = 5;
const byte PIN_BUZZER       = 6;
const byte PIN_COLON        = 7;

const byte PIN_595_DATA  = 8;
const byte PIN_595_LATCH = 9;
const byte PIN_MAX_CS    = 10;
const byte PIN_MAX_DATA  = 11;
const byte PIN_CLOCK     = 13;

const byte RTC_ADDRESS = 0x68;

// -------------------- MAX7219 -------------------------------
const byte MAX_REG_DECODE      = 0x09;
const byte MAX_REG_INTENSITY   = 0x0A;
const byte MAX_REG_SCANLIMIT   = 0x0B;
const byte MAX_REG_SHUTDOWN    = 0x0C;
const byte MAX_REG_DISPLAYTEST = 0x0F;

const byte SEG_A = 0x01;
const byte SEG_B = 0x02;
const byte SEG_C = 0x04;
const byte SEG_D = 0x08;
const byte SEG_E = 0x10;
const byte SEG_F = 0x20;
const byte SEG_G = 0x40;

const byte DIGIT_MASK[10] = {
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
const byte SHIFT_REG_COUNT = 7;
const byte CONTOUR_GROUPS = 40;
byte contourData[SHIFT_REG_COUNT] = {0};
byte contourGroup = 0;
unsigned long lastContourUpdate = 0;
const unsigned long CONTOUR_INTERVAL = 100;

// -------------------- RTC -----------------------------------
byte hourNow = 0;
byte minuteNow = 0;
byte secondNow = 0;
unsigned long lastRTCRead = 0;
const unsigned long RTC_INTERVAL = 500;

// -------------------- Etats ---------------------------------
bool watchOn = false;
bool startupActive = false;
unsigned long startupStart = 0;
unsigned long lastStartupFrame = 0;
byte startupFrame = 0;
const unsigned long STARTUP_DURATION = 2300;
const unsigned long STARTUP_FRAME_INTERVAL = 120;

enum EditField { EDIT_HOUR, EDIT_MINUTE };
bool editMode = false;
EditField editField = EDIT_HOUR;
byte editHour = 0;
byte editMinute = 0;
bool editVisible = true;
unsigned long lastBlink = 0;
const unsigned long BLINK_INTERVAL = 350;

// -------------------- Boutons -------------------------------
struct Button {
  byte pin;
  bool raw;
  bool stable;
  unsigned long changedAt;
};

Button buttonPower = {PIN_BUTTON_POWER, HIGH, HIGH, 0};
Button buttonMode  = {PIN_BUTTON_MODE, HIGH, HIGH, 0};
Button buttonPlus  = {PIN_BUTTON_PLUS, HIGH, HIGH, 0};
Button buttonMinus = {PIN_BUTTON_MINUS, HIGH, HIGH, 0};

const unsigned long DEBOUNCE_MS = 35;
const unsigned long MODE_WINDOW_MS = 1000;
byte modeClicks = 0;
unsigned long modeDeadline = 0;

// -------------------- Buzzer --------------------------------
struct Note {
  unsigned int frequency;
  unsigned int duration;
};

const Note STARTUP_MELODY[] = {
  {523, 120}, {659, 120}, {784, 120}, {1047, 220},
  {784, 120}, {659, 120}, {523, 260}
};
const byte STARTUP_MELODY_COUNT = sizeof(STARTUP_MELODY) / sizeof(STARTUP_MELODY[0]);
byte melodyIndex = 0;
unsigned long melodyNext = 0;

byte bcdToDec(byte value) {
  return ((value >> 4) * 10) + (value & 0x0F);
}

byte decToBcd(byte value) {
  return ((value / 10) << 4) | (value % 10);
}

// ============================================================
// MAX7219
// ============================================================
void maxWrite(byte reg, byte value) {
  digitalWrite(PIN_MAX_CS, LOW);
  shiftOut(PIN_MAX_DATA, PIN_CLOCK, MSBFIRST, reg);
  shiftOut(PIN_MAX_DATA, PIN_CLOCK, MSBFIRST, value);
  digitalWrite(PIN_MAX_CS, HIGH);
}

void maxClear() {
  for (byte reg = 1; reg <= 8; reg++) {
    maxWrite(reg, 0x00);
  }
}

void maxInit() {
  pinMode(PIN_MAX_CS, OUTPUT);
  pinMode(PIN_MAX_DATA, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);

  digitalWrite(PIN_MAX_CS, HIGH);
  digitalWrite(PIN_CLOCK, LOW);

  maxWrite(MAX_REG_DISPLAYTEST, 0x00);
  maxWrite(MAX_REG_DECODE, 0x00);
  maxWrite(MAX_REG_SCANLIMIT, 0x03);
  maxWrite(MAX_REG_INTENSITY, 0x06);
  maxWrite(MAX_REG_SHUTDOWN, 0x01);
  maxClear();
}

void maxDigit(byte index, byte segments) {
  if (index > 3) return;
  maxWrite(index + 1, segments);
}

void displayHHMM(byte h, byte m) {
  maxDigit(0, DIGIT_MASK[h / 10]);
  maxDigit(1, DIGIT_MASK[h % 10]);
  maxDigit(2, DIGIT_MASK[m / 10]);
  maxDigit(3, DIGIT_MASK[m % 10]);
}

void displayEdit() {
  byte hTens = editHour / 10;
  byte hUnits = editHour % 10;
  byte mTens = editMinute / 10;
  byte mUnits = editMinute % 10;

  if (editField == EDIT_HOUR && !editVisible) {
    maxDigit(0, 0x00);
    maxDigit(1, 0x00);
  } else {
    maxDigit(0, DIGIT_MASK[hTens]);
    maxDigit(1, DIGIT_MASK[hUnits]);
  }

  if (editField == EDIT_MINUTE && !editVisible) {
    maxDigit(2, 0x00);
    maxDigit(3, 0x00);
  } else {
    maxDigit(2, DIGIT_MASK[mTens]);
    maxDigit(3, DIGIT_MASK[mUnits]);
  }
}

void setColon(bool on) {
  digitalWrite(PIN_COLON, on ? HIGH : LOW);
}

// ============================================================
// DS3231
// ============================================================
bool rtcReadTime() {
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) return false;

  if (Wire.requestFrom(RTC_ADDRESS, (byte)3) != 3) return false;

  secondNow = bcdToDec(Wire.read() & 0x7F);
  minuteNow = bcdToDec(Wire.read() & 0x7F);
  hourNow = bcdToDec(Wire.read() & 0x3F);

  if (hourNow > 23 || minuteNow > 59 || secondNow > 59) return false;
  return true;
}

bool rtcWriteTime(byte h, byte m, byte s) {
  if (h > 23 || m > 59 || s > 59) return false;

  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0x00);
  Wire.write(decToBcd(s));
  Wire.write(decToBcd(m));
  Wire.write(decToBcd(h));
  return Wire.endTransmission() == 0;
}

// ============================================================
// 74HC595 + ULN2803A + contour
// ============================================================
void clearContour() {
  for (byte i = 0; i < SHIFT_REG_COUNT; i++) {
    contourData[i] = 0x00;
  }
}

void setContourGroup(byte group, bool on) {
  if (group >= CONTOUR_GROUPS) return;

  byte chip = group / 8;
  byte bit = group % 8;

  if (on) contourData[chip] |= (byte)(1 << bit);
  else contourData[chip] &= (byte)~(1 << bit);
}

void writeContour() {
  digitalWrite(PIN_595_LATCH, LOW);

  // IMPORTANT : Q0 doit recevoir le bit 0.
  // Avec un 74HC595, le premier bit envoyé termine sur Q7.
  // On envoie donc MSB first pour que le bit 0 finisse sur Q0.
  // Le dernier CI de la chaîne doit être envoyé en premier.
  for (int chip = SHIFT_REG_COUNT - 1; chip >= 0; chip--) {
    shiftOut(PIN_595_DATA, PIN_CLOCK, MSBFIRST, contourData[chip]);
  }

  digitalWrite(PIN_595_LATCH, HIGH);
}

void contourInit() {
  pinMode(PIN_595_DATA, OUTPUT);
  pinMode(PIN_595_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);

  clearContour();
  writeContour();
}

void updateContour() {
  if (!watchOn) return;
  if (millis() - lastContourUpdate < CONTOUR_INTERVAL) return;

  lastContourUpdate = millis();

  clearContour();
  setContourGroup(contourGroup, true);
  writeContour();

  contourGroup++;
  if (contourGroup >= CONTOUR_GROUPS) contourGroup = 0;
}

// ============================================================
// Buzzer
// ============================================================
void beepAction() {
  tone(PIN_BUZZER, 880, 60);
}

void beepDigit(byte digit) {
  const unsigned int frequencies[10] = {
    262, 294, 330, 349, 392, 440, 494, 523, 587, 659
  };
  tone(PIN_BUZZER, frequencies[digit % 10], 75);
}

void startMelody() {
  melodyIndex = 0;
  melodyNext = 0;
}

void updateMelody() {
  if (!startupActive) return;

  unsigned long now = millis();
  if (now < melodyNext) return;

  if (melodyIndex >= STARTUP_MELODY_COUNT) {
    noTone(PIN_BUZZER);
    melodyNext = now + 100000UL;
    return;
  }

  tone(PIN_BUZZER,
       STARTUP_MELODY[melodyIndex].frequency,
       STARTUP_MELODY[melodyIndex].duration - 10);

  melodyNext = now + STARTUP_MELODY[melodyIndex].duration;
  melodyIndex++;
}

// ============================================================
// Splash / démarrage
// ============================================================
void displaySplash(byte frame) {
  byte mask = 0;

  switch (frame % 8) {
    case 0: mask = SEG_A; break;
    case 1: mask = SEG_B; break;
    case 2: mask = SEG_C; break;
    case 3: mask = SEG_D; break;
    case 4: mask = SEG_E; break;
    case 5: mask = SEG_F; break;
    case 6: mask = SEG_G; break;
    case 7: mask = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G; break;
  }

  maxDigit(0, mask);
  maxDigit(1, mask);
  maxDigit(2, mask);
  maxDigit(3, mask);
  setColon((frame % 2) == 0);
}

void startWatch() {
  watchOn = true;
  editMode = false;
  startupActive = true;
  startupStart = millis();
  lastStartupFrame = 0;
  startupFrame = 0;
  contourGroup = 0;
  startMelody();

  maxWrite(MAX_REG_SHUTDOWN, 0x01);
  maxClear();
  clearContour();
  writeContour();
}

void stopWatch() {
  watchOn = false;
  startupActive = false;
  editMode = false;
  modeClicks = 0;
  noTone(PIN_BUZZER);

  maxClear();
  setColon(false);
  clearContour();
  writeContour();
  maxWrite(MAX_REG_SHUTDOWN, 0x00);
}

void updateStartup() {
  if (!startupActive) return;

  unsigned long now = millis();

  if (now - lastStartupFrame >= STARTUP_FRAME_INTERVAL) {
    lastStartupFrame = now;
    displaySplash(startupFrame);
    startupFrame++;
  }

  if (now - startupStart >= STARTUP_DURATION) {
    startupActive = false;
    noTone(PIN_BUZZER);

    if (!rtcReadTime()) {
      hourNow = 0;
      minuteNow = 0;
      secondNow = 0;
    }

    displayHHMM(hourNow, minuteNow);
    setColon(true);
  }
}

// ============================================================
// Boutons
// ============================================================
bool pressed(Button &button) {
  bool reading = digitalRead(button.pin);

  if (reading != button.raw) {
    button.raw = reading;
    button.changedAt = millis();
  }

  if (millis() - button.changedAt >= DEBOUNCE_MS && reading != button.stable) {
    button.stable = reading;
    if (button.stable == LOW) return true;
  }

  return false;
}

void resetClock() {
  if (rtcWriteTime(0, 0, 0)) {
    hourNow = 0;
    minuteNow = 0;
    secondNow = 0;
  }

  beepAction();
  displayHHMM(hourNow, minuteNow);
}

void enterEditMode() {
  if (!rtcReadTime()) return;

  editHour = hourNow;
  editMinute = minuteNow;
  editField = EDIT_HOUR;
  editVisible = true;
  editMode = true;
  lastBlink = millis();
  displayEdit();
  beepAction();
}

void validateEditMode() {
  if (rtcWriteTime(editHour, editMinute, 0)) {
    hourNow = editHour;
    minuteNow = editMinute;
    secondNow = 0;
  }

  editMode = false;
  editVisible = true;
  displayHHMM(hourNow, minuteNow);
  setColon(true);
  beepAction();
}

void processModeClicks() {
  if (modeClicks == 0) return;
  if (millis() < modeDeadline) return;

  if (!editMode) {
    if (modeClicks == 1) {
      resetClock();
    } else if (modeClicks == 2) {
      enterEditMode();
    }
  } else {
    if (modeClicks == 1) {
      editField = (editField == EDIT_HOUR) ? EDIT_MINUTE : EDIT_HOUR;
      editVisible = true;
      lastBlink = millis();
      displayEdit();
      beepAction();
    } else if (modeClicks == 3) {
      validateEditMode();
    }
  }

  modeClicks = 0;
}

void handleButtons() {
  if (pressed(buttonPower)) {
    if (watchOn) stopWatch();
    else startWatch();
  }

  if (!watchOn || startupActive) return;

  if (pressed(buttonMode)) {
    modeClicks++;
    if (modeClicks > 3) modeClicks = 3;
    modeDeadline = millis() + MODE_WINDOW_MS;
  }

  if (editMode) {
    if (pressed(buttonPlus)) {
      if (editField == EDIT_HOUR) {
        editHour = (editHour + 1) % 24;
        beepDigit(editHour % 10);
      } else {
        editMinute = (editMinute + 1) % 60;
        beepDigit(editMinute % 10);
      }
      editVisible = true;
      lastBlink = millis();
      displayEdit();
    }

    if (pressed(buttonMinus)) {
      if (editField == EDIT_HOUR) {
        editHour = (editHour == 0) ? 23 : editHour - 1;
        beepDigit(editHour % 10);
      } else {
        editMinute = (editMinute == 0) ? 59 : editMinute - 1;
        beepDigit(editMinute % 10);
      }
      editVisible = true;
      lastBlink = millis();
      displayEdit();
    }
  }

  processModeClicks();
}

void updateEditBlink() {
  if (!editMode) return;

  if (millis() - lastBlink >= BLINK_INTERVAL) {
    lastBlink = millis();
    editVisible = !editVisible;
    displayEdit();
  }
}

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
  rtcReadTime();

  watchOn = false;
  startupActive = false;
  editMode = false;
}

void loop() {
  handleButtons();

  if (!watchOn) return;

  updateMelody();
  updateStartup();
  updateContour();

  if (startupActive) return;

  if (!editMode && millis() - lastRTCRead >= RTC_INTERVAL) {
    lastRTCRead = millis();
    if (rtcReadTime()) {
      displayHHMM(hourNow, minuteNow);
    }
  }

  updateEditBlink();
}
