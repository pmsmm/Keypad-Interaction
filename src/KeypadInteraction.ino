#include <LiquidCrystal_I2C.h>

const uint8_t redLED = 10;
const uint8_t greenLED = 11;

byte rows[] = {6, 7, 8, 9};
const int rowCount = sizeof(rows) / sizeof(rows[0]);

byte cols[] = {2, 3, 4, 5};
const int colCount = sizeof(cols) / sizeof(cols[0]);

byte keys[colCount][rowCount];
char numbers[colCount][rowCount] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', 'D', '#', 'D'}
};

LiquidCrystal_I2C lcd(0x27, 16, 2);

char newSecretCode[8];
char introducedSecretCode[8];

int secretCodeIndex = 0;

bool INTERACTION_SOLVED, INTERACTION_RUNNING;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.blink();
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
}

void loop() {
  if (!Serial) {
    Serial.begin(9600);
  }
  if (Serial.available()) {
    processSerialMessage();
  }
  if (INTERACTION_SOLVED == false && INTERACTION_RUNNING == true) {
    gameLoop();
  }
}

void gameLoop() {
  char pressedKey = readMatrixAndReturnPressedKey();
  if (pressedKey != '\n') {
    if (secretCodeIndex < 8) {
      lcd.setCursor((4 + secretCodeIndex), 1);
      lcd.print(pressedKey);
      introducedSecretCode[secretCodeIndex] = pressedKey;
      secretCodeIndex += 1;
      if (secretCodeIndex == 8 and checkWinning()) {
        Serial.println("COM:INTERACTION_SOLVED;MSG:User Guessed Code Correctly;PNT:2000");
        Serial.flush();
      }
    } else {
      secretCodeIndex = 0;
    }
  }
  delay(225);
}

char readMatrixAndReturnPressedKey() {
  char pressedKey = '\n';
  while (pressedKey == '\n') {
    // iterate the columns
    for (int colIndex = 0; colIndex < colCount; colIndex++) {
      // col: set to output to low
      byte curCol = cols[colIndex];
      pinMode(curCol, OUTPUT);
      digitalWrite(curCol, LOW);

      // row: interate through the rows
      for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
        byte rowCol = rows[rowIndex];
        pinMode(rowCol, INPUT_PULLUP);
        keys[colIndex][rowIndex] = digitalRead(rowCol);
        pinMode(rowCol, INPUT);
      }
      // disable the column
      pinMode(curCol, INPUT);
    }

    for (int colIndex = 0; colIndex < colCount; colIndex++) {
      for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
        if (keys[colIndex][rowIndex] == LOW) {
          pressedKey = numbers[colIndex][rowIndex];
        }
      }
    }

    if (Serial.available()) {
      processSerialMessage();
      return '\n';
    }

  }

  return pressedKey;

}

void processSerialMessage() {
  const int BUFF_SIZE = 64; // make it big enough to hold your longest command
  static char buffer[BUFF_SIZE + 1]; // +1 allows space for the null terminator
  static int length = 0; // number of characters currently in the buffer

  char c = Serial.read();
  if ((c == '\r') || (c == '\n')) {
    // end-of-line received
    if (length > 0) {
      tokenizeReceivedMessage(buffer);
    }
    length = 0;
  } else {
    if (length < BUFF_SIZE) {
      buffer[length++] = c; // append the received character to the array
      buffer[length] = 0; // append the null terminator
    }
  }
}

void tokenizeReceivedMessage(char *msg) {
  const int COMMAND_PAIRS = 10;
  char* tokenizedString[COMMAND_PAIRS + 1];
  int index = 0;

  char* command = strtok(msg, ";");
  while (command != 0) {
    char* separator = strchr(command, ':');
    if (separator != 0) {
      *separator = 0;
      tokenizedString[index++] = command;
      ++separator;
      tokenizedString[index++] = separator;
    }
    command = strtok(0, ";");
  }
  tokenizedString[index] = 0;

  processReceivedMessage(tokenizedString);
}

void processReceivedMessage(char** command) {
  if (strcmp(command[1], "START") == 0) {
    startSequence(command[3]);
  } else if (strcmp(command[1], "PAUSE") == 0) {
    pauseSequence(command[3]);
  } else if (strcmp(command[1], "STOP") == 0) {
    stopSequence(command[3]);
  } else if (strcmp(command[1], "INTERACTION_SOLVED_ACK") == 0) {
    setInteractionSolved();
  } else if (strcmp(command[1], "PING") == 0) {
    ping(command[3]);
  } else if (strcmp(command[1], "BAUD") == 0) {
    setBaudRate(atoi(command[3]), command[5]);
  } else if (strcmp(command[1], "SETUP") == 0) {
    Serial.println("COM:SETUP;INT_NAME:Keypad Interaction;BAUD:9600");
    Serial.flush();
  }
}

void startSequence(char* TIMESTAMP) {
  for (int i = 0; i < 8; i++) {
    newSecretCode[i] = numbers[random(0, rowCount)][random(0, colCount)];
  }
  INTERACTION_SOLVED = false;
  INTERACTION_RUNNING = true;
  secretCodeIndex = 0;
  Serial.print("COM:START_ACK;MSG:");
  for (int i = 0; i < 8; i++) {
    Serial.print(newSecretCode[i]);
  }
  Serial.print(";ID:");
  Serial.print(TIMESTAMP);
  Serial.print("\r\n");
  Serial.flush();
  lcd.setCursor(0, 0);
  lcd.print("#Introduce Code#");
  lcd.setCursor(4, 1);
}

void pauseSequence(char* TIMESTAMP) {
  INTERACTION_RUNNING = !INTERACTION_RUNNING;
  if (INTERACTION_RUNNING) {
    Serial.print("COM:PAUSE_ACK;MSG:Device is now running;ID:");
  } else {
    Serial.print("COM:PAUSE_ACK;MSG:Device is now paused;ID:");
  }
  Serial.print(TIMESTAMP);
  Serial.print("\r\n");
  Serial.flush();
}

void stopSequence(char* TIMESTAMP) {
  INTERACTION_RUNNING = false;
  Serial.print("COM:STOP_ACK;MSG:Device is now stopped;ID:");
  Serial.print(TIMESTAMP);
  Serial.print("\r\n");
  Serial.flush();
}

void setInteractionSolved() {
  INTERACTION_SOLVED = true;
  INTERACTION_RUNNING = false;
}

void ping(char* TIMESTAMP) {
  Serial.print("COM:PING;MSG:PING;ID:");
  Serial.print(TIMESTAMP);
  Serial.print("\r\n");
  Serial.flush();
}

void setBaudRate(int baudRate, char* TIMESTAMP) {
  Serial.flush();
  Serial.begin(baudRate);
  Serial.print("COM:BAUD_ACK;MSG:The Baud Rate was set to ");
  Serial.print(baudRate);
  Serial.print(";ID:");
  Serial.print(TIMESTAMP);
  Serial.print("\r\n");
  Serial.flush();
}

bool checkWinning() {
  for (int i = 0; i < 8; i++) {
    if (introducedSecretCode[i] != newSecretCode[i]) {
      digitalWrite(redLED, HIGH);
      lcd.init();
      lcd.setCursor(0, 0);
      lcd.print("#Access Denied#");
      delay(1500);
      lcd.setCursor(0, 0);
      lcd.print("#Introduce Code#");
      lcd.setCursor(4, 1);
      lcd.backlight();
      digitalWrite(redLED, LOW);
      return false;
    }
  }
  digitalWrite(greenLED, HIGH);
  delay(1000);
  lcd.init();
  lcd.setCursor(0, 0);
  lcd.print("#Access Granted#");
  lcd.setCursor(4, 1);
  lcd.backlight();
  digitalWrite(greenLED, LOW);
  INTERACTION_SOLVED = true;
  return true;
}
