# Optimized minimalist raycast engine for ESP32

Written originally in C, I ported and optimized my raycast engine for the constrained resources of the ESP32.


## Technical details

1. Low-level SPI protocol implementation (Bit Banging):
  - To circumvent SPI library overhead I have taken the hex commands directly from the datasheet and transmitted it trough simple bit-banging.
  - Manual SCLK clocking adheres to SSD1306 display requirments.
2. Computational efficiency
  - To achieve a high framerate I used fixed-point arithmetic to represent real numbers in the integer primitive data type.
  - Values are shifted by 8 bits to achiece adequate resolution.
  - Lookup tables store precalculated trigonometric results for quick access.
3. Simple interface
  - GPIO-based 4 direction digital joystick ensures no input latency. 
![output2](https://github.com/user-attachments/assets/1495274d-0577-4162-a918-e67cd54c5442)
