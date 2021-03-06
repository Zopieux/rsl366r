// SPDX-License-Identifier: GPL-3.0-or-later

#include <Arduino.h>
#include <RCSwitch.h>

namespace {

constexpr unsigned long kSerialBaud = 9600;
constexpr unsigned int kMessageLengthBits = 24;
constexpr unsigned long k24BitMask = 0xffffff;

struct __attribute__((packed)) Config {
  uint8_t transmitPin;
  uint8_t repeatTx;
  uint16_t pulseLength;
};
constexpr int kConfigSize = 4;

union ConfigU {
  Config config{};
  uint8_t in[kConfigSize];
};

enum class Command { INIT = 0xff, CONFIGURE = 0, SEND = 1 };
constexpr int kCommandSize = 1;
union CommandU {
  Command command = Command::INIT;
  uint8_t in[kCommandSize];
};

using CodeT = unsigned long;
constexpr int kCodeSize = 4;
union CodeToSendU {
  CodeT code;
  uint8_t in[kCodeSize];
};

RCSwitch rcSwitch;
ConfigU config{};
CommandU command{};
CodeToSendU codeToSend{};

void printConfig() {
  Serial.print("transmitPin=");
  Serial.print((int)config.config.transmitPin);
  Serial.write(";");
  Serial.print("repeatTx=");
  Serial.print((int)config.config.repeatTx);
  Serial.write(";");
  Serial.print("pulseLength=");
  Serial.print((int)config.config.pulseLength);
  Serial.write(";\n");
}

void printCode() {
  Serial.print("codeToSend=");
  Serial.print(codeToSend.code);
  Serial.write(";\n");
}


void ledFlash() {
  constexpr uint32_t kOn = LOW, kOff = HIGH;
  constexpr int kDelay = 4;
  digitalWrite(LED_BUILTIN, kOn);
  delay(kDelay);
  digitalWrite(LED_BUILTIN, kOff);
  delay(kDelay);
}

}  // namespace

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(kSerialBaud);
  ledFlash();
}

void loop() {
  switch (command.command) {
    case Command::INIT:
      if (Serial.available() < kCommandSize) return;
      if (Serial.readBytes(reinterpret_cast<char*>(command.in), kCommandSize) !=
          kCommandSize)
        return;
      break;

    case Command::CONFIGURE:
      if (Serial.available() < kConfigSize) return;
      if (Serial.readBytes(reinterpret_cast<char*>(config.in), kConfigSize) !=
          kConfigSize)
        return;
      ledFlash();
      printConfig();
      rcSwitch.enableTransmit((int)config.config.transmitPin);
      rcSwitch.setPulseLength((int)config.config.pulseLength);
      rcSwitch.setRepeatTransmit((int)config.config.repeatTx);
      command.command = Command::INIT;
      break;

    case Command::SEND:
      if (Serial.available() < kCodeSize) return;
      if (Serial.readBytes(reinterpret_cast<char*>(codeToSend.in), kCodeSize) !=
          kCodeSize)
        return;
      ledFlash();
      codeToSend.code &= k24BitMask;
      printCode();
      rcSwitch.send(codeToSend.code, kMessageLengthBits);
      command.command = Command::INIT;
      break;

    default:
      Serial.write("unsupported\n");
      break;
  }
}
