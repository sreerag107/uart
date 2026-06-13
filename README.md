EmbedKit_Sreerag
UART Frame Parser Assignment
Author: Sreerag K S
Description
This project implements a UART Frame Parser using a state machine in C.
The parser processes incoming bytes one at a time and supports:
• Start of Frame (SOF) detection
• Command field parsing
• Length field parsing
• Payload reception
• XOR checksum validation
• Inter-byte timeout detection
• Automatic parser recovery after timeout or checksum failure
Protocol Format
FieldSize
SOF1 Byte
CMD1 Byte
LEN1 Byte
PAYLOADN Bytes
CHECKSUM1 Byte
SOF: 0xAA
Checksum Calculation:
CHECKSUM = CMD ^ LEN ^ PAYLOAD[0] ^ PAYLOAD[1] ... ^ PAYLOAD[N]
Features
• Byte-by-byte state machine parser
• Configurable inter-byte timeout
• Timeout can be disabled
• Checksum verification
• Error handling and parser reset
1• Recovery after corrupted frames
• Clean C99 implementation
• Uses fixed-width integer types from <stdint.h>
Build Instructions
Compile using GCC:
gcc -Wall -Wextra -std=c99 uart_parser.c -o uart_parser
Run
./uart_parser
Test Cases Implemented
1. Clean Valid Frame
2. Timeout Mid-Frame and Recovery
3. Two Valid Frames Back-to-Back
4. Timeout Disabled
File Structure
UARTS/
├── uart.c
└── README.md
Notes
• No external libraries are used.
• Compatible with Linux, macOS and Windows (MinGW/WSL).
• Compiles cleanly with GCC using C99 standard.
2
