```markdown
# BluePill USB Wake Device

Auto-USB device that sends a single keystroke after power-up. Used to trigger USB wake-on-keyboard for headless servers.

## Hardware
- STM32F103C8T6 "BluePill" board
- Black Magic Probe (BMP) for programming

## Connections
BMP → BluePill:
- SWDIO → PA13  
- SWCLK → PA14
- GND → GND

## Build/Flash
```bash
pio run -t upload
```

## Operation
- Power LED flashes at 1Hz
- After 10s delay, sends single space keystroke
- Long startup delay prevents repeated triggers from power fluctuations
- Uses libopencm3 framework for stable USB implementation

## Configuration
- `STARTUP_DELAY` - Power-up delay in ms (default: 10000)
- `PERIODIC_TX_INTERVAL` - LED blink rate in ms (default: 1000)
```