# rsl366r

Control radio-controlled power sockets with any Arduino-compatible board and a cheap 433Mhz emitter.

## Hardware needed

1. Some 5V-enabled board with one digital I/O pin and serial communication (eg. USB). I've tested this code successfully with:
   * A $2 “Blue Pill” STM32F103C8T6 from Aliexpress (`bluepill_f103c8_128k`)
   * A $6 AT MEGA 2560 clone from Aliexpress (`megaatmega2560`)
1. A 433 Mhz RF transmitter module. I often see them referred to as “Superheterodyne”. They are often sold in receiver/transmitter kits, for instance [this $1 one](https://www.aliexpress.com/item/32980820915.html) from Aliexpress. 
1. Radio-controlled (433.95MHz) power sockets. This was tested with two [RSL366R ](https://www.amazon.fr/dp/B00FIMWB6Q), sold under the *Chacon* brand, part number SKU 54660.

You'll possibly need more stuff to upload the program to your device. I used a TTL-232R-3v3 to flash my Blue Pill. Arduino boards like the 2560 are programmed using the built-in USB port and do not need extra cables.

Plug the 433 Mhz transmitter *5V* and *GND* pins to *5V* and *GND* on your board, and its *data* pin to any appropriate digital pin on the board.
Refer to your board pinout to choose an appropriate digital I/O pin and make note of its Arduino pin number. The example below uses Arduino pin number `10`.

## Setup

Refer to the [Platformio documentation](https://docs.platformio.org/en/latest/tutorials/index.html) on how to compile and program your device from this code. 
You might need to tweak [`platformio.ini`](./platformio.ini) to fit your environment, such as serial port names or other boards.

You do not need to change the C++ code for normal usage. All the relevant communication parameters (data pin, pulse length, pulse repeats) are dynamically configurable at runtime. See below.

## Usage

Once flashed, the device should appear as a serial interface (eg. `/dev/ttyACM0`) to your host computer.
It speaks a very simple binary protocol for configuring and sending commands. Please refer to the self-explanatory 
Python code below.

Well-known pulse codes for RSL366R are [documented here](https://gist.github.com/Zopieux/b4a51bca101d59339a839e3cc8e0ec98).

You might want to tweak the pulse length and TX repeats if the values used below are flaky on your setup.
5 TX repeats and a 413 µs pulse length work very reliably for me.

```python
import serial, struct

# Serial speed is 9600 bauds.
# Assuming little-endian platform.
s = serial.Serial("/dev/ttyACM0", 9600)

# First, let's configure the device.
# Size (bytes) | 1            | 1            | 1            | 2
# Index        | 0            | 1            | 2            | 3, 4
# Meaning      | cmd          | transmit pin | pulse repeat | pulse length
CONFIGURE = 0x00
s.write(struct.pack("<BBBH", CONFIGURE, 10, 5, 413))
# Device confirms with an ASCII-encoded LF-terminated line.
print(s.readline())
# b'transmitPin=10;repeatTx=5;pulseLength=413;\n'

# Then let's send a pulse code.
# Size (bytes) | 1            | 4
# Index        | 0            | 1
# Meaning      | cmd          | transmit code
SEND = 0x01
s.write(struct.pack("<BL", SEND, 1381717))
# Device confirms with an ASCII-encoded LF-terminated line.
print(s.readline())
# b'codeToSend=1381717;\n'
```

## License

[GNU General Public License v3.0 or later](https://spdx.org/licenses/GPL-3.0-or-later.html).
