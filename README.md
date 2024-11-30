# BluePill USB HID Keyboard

Auto-keyboard for headless server startup.

## Connections:
- BMP → BluePill:
  - SWDIO → PA13
  - SWCLK → PA14
  - GND → GND
  - 3.3V optional

## Usage:
\`\`\`bash
pio run -t upload
\`\`\`

LED on PC13 will flash when key is pressed.
# powerloss-restart
