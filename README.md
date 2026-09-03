# Optimized minimalist raycast engine for ESP32

Written originally in C, I ported and optimized my raycast engine for the constrained resources of the ESP32.

## Technical details

1. Low-level SPI protocol implementation (Bit-banging):
  - I have taken the hex commands directly from the datasheet and transmitted them through simple bit-banging.
  - Manual SCLK clocking adheres to SSD1306 display requirements.
2. Computational efficiency
  - Fixed-point arithmetic using a signed Q8.8 / 16-bit integer scheme to eliminate floating-point unit (FPU) bottlenecks
  - Values are shifted by 8 bits to achieve adequate resolution.
  - Lookup tables store precalculated trigonometric results for quick access.
3. Simple interface
  - GPIO-based 4 direction digital joystick ensures no input latency.
4. Benchmarking, comparison to hardware SPI
  - The project demonstrates that, as expected when using the hardware SPI even without careful optimisation, speedups of up to 10x can be experienced
  - According to the benchmarks:
    | Driver | Transfer Strategy | Observed Frame Rate |
    | --- | --- | --- |
    | **Software SPI (Bit-banging)** | Byte-by-byte GPIO toggling | **85 - 95 FPS** |
    | **Hardware SPI (`SPI.h`)** | 1024-byte contiguous bulk push | **750 - 850 FPS FPS** |

![output2](https://github.com/user-attachments/assets/1495274d-0577-4162-a918-e67cd54c5442)
