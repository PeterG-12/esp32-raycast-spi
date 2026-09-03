#include "cast_math.h"
#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include <Arduino.h>
#include <cstdint>
#include <cstdlib>

#define OLED_MOSI 23
#define OLED_CLK 18
#define OLED_DC 2
#define OLED_CS 5
#define OLED_RESET 4

#define UP 33
#define RIGHT 34
#define DOWN 35
#define LEFT 32

#define DEBUG 0

#define BENCHMARK

#ifdef USE_HARDWARE_SPI
    #include <SPI.h>
#endif

uint8_t screen_buffer[SCREEN_HEIGHT / 8][SCREEN_WIDTH] = {0};

static inline void clockUp() { digitalWrite(OLED_CLK, HIGH); }

static inline void clockDown() { digitalWrite(OLED_CLK, LOW); }

void send_byte(char byte, int is_data) {

    int DC_VALUE;
    if (is_data) {
        DC_VALUE = HIGH;
    } else {
        DC_VALUE = LOW;
    }
    digitalWrite(OLED_CS, LOW);

    digitalWrite(OLED_DC, DC_VALUE);

    #ifdef USE_HARDWARE_SPI
        SPI.beginTransaction(SPISettings(15000000, MSBFIRST, SPI_MODE0));
        SPI.transfer(&byte, sizeof(byte));
        SPI.endTransaction();
    #endif

    #ifndef USE_HARDWARE_SPI
        for (int i = 0; i < 8; i++) {
            if (byte & 0x80) {
                digitalWrite(OLED_MOSI, HIGH);
            } else {
                digitalWrite(OLED_MOSI, LOW);
            }
            clockUp();
            clockDown();
            byte = byte << 1;
        }
    #endif
    
    digitalWrite(OLED_DC, !DC_VALUE);
    digitalWrite(OLED_CS, HIGH);

}

void print_buf() {

    
    #ifndef USE_HARDWARE_SPI
        for (int i = 0; i < SCREEN_HEIGHT / 8; i++) {
            for (int j = 0; j < SCREEN_WIDTH; j++) {
                send_byte(screen_buffer[i][j], 1);
            }
        }
    #endif
    
    #ifdef USE_HARDWARE_SPI
        digitalWrite(OLED_DC, HIGH);
        digitalWrite(OLED_CS, LOW);
        SPI.beginTransaction(SPISettings(15000000, MSBFIRST, SPI_MODE0));
        SPI.transfer(screen_buffer, sizeof(screen_buffer));
        SPI.endTransaction();

        digitalWrite(OLED_CS, HIGH);
    #endif

}

void draw_screen(int px, int py, int view_angle) {
    int ray_angle = view_angle - FOV_DEGREES / 2;
    ray_angle <<= 8;

    int beta = ray_angle;

    int switch_var = 0;

    int horizontal_hit = 0;
    int vertical_hit = 0;
    for (int i = 0; i < 128; i++) {

        int horizontal_distance =
            get_horizontal_collision(px, py, ray_angle, &horizontal_hit);

        int vertical_distance =
            get_vertical_collision(px, py, ray_angle, &vertical_hit);
        int min_dist = cast_min(horizontal_distance, vertical_distance);

        int beta = (ray_angle >> 8) - view_angle;
        beta = normalise_angle_degrees(beta);

        long long corrected_distance = a_cos[beta] * min_dist;
        corrected_distance >>= 8;

        if (corrected_distance <= 0)
            corrected_distance++;

        int projected_height =
            (BLOCK_SIZE * DISTANCE_TO_PROJECTION) / corrected_distance;
        int range_start = SCREEN_HEIGHT / 2 - projected_height / 2;
        int range_end = SCREEN_HEIGHT / 2 + projected_height / 2;

        int hit_block;

        if (min_dist == vertical_distance) {
            hit_block = vertical_hit;
        } else {
            hit_block = horizontal_hit;
        }
        for (int16_t j = 0; j < SCREEN_HEIGHT / 8; j++) {
            unsigned char mask = 0x00;
            int page_base_y = j * 8;

            for (int bit = 0; bit < 8; bit++) {
                int pixel_y = page_base_y + bit;

                if (pixel_y >= range_start && pixel_y <= range_end) {
                    mask |= (1 << bit);
                }
            }
            screen_buffer[j][i] = mask;
        }
        ray_angle += RAY_STEP;
    }
    print_buf();
}

void setup() {

    #ifdef USE_HARDWARE_SPI
        SPI.begin(OLED_CLK, -1, OLED_MOSI, -1);
    #endif

    #ifndef USE_HARDWARE_SPI
        pinMode(OLED_MOSI, OUTPUT);
        pinMode(OLED_CLK, OUTPUT);
    #endif



    pinMode(OLED_DC, OUTPUT);
    pinMode(OLED_CS, OUTPUT);
    pinMode(OLED_RESET, OUTPUT);

    digitalWrite(OLED_MOSI, LOW);
    digitalWrite(OLED_CLK, LOW);

    digitalWrite(OLED_RESET, LOW);
    delay(500);
    digitalWrite(OLED_RESET, HIGH);

    digitalWrite(OLED_DC, LOW);
    digitalWrite(OLED_CS, LOW);

    pinMode(UP, INPUT_PULLUP);
    pinMode(LEFT, INPUT_PULLUP);
    pinMode(DOWN, INPUT_PULLUP);
    pinMode(RIGHT, INPUT_PULLUP);

    delay(1000);
    send_byte(0x8d, 0);
    send_byte(0x14, 0);
    send_byte(0xAF, 0);

    send_byte(0x20, 0);
    send_byte(0x00, 0);

    send_byte(0x21, 0);
    send_byte(0, 0);
    send_byte(127, 0);

    send_byte(0x22, 0);
    send_byte(0, 0);
    send_byte(7, 0);

    
    #ifdef BENCHMARK
        Serial.begin(115200);
    #endif
}
    

int px = 36;
int py = 36;
int angle = 0;


#ifdef BENCHMARK
int avg_i = 0;
int avg_frame_time = 0;
#endif

void loop() {


    int before = micros();
    angle = normalise_angle_degrees(angle);
    int cos_mov = a_cos[angle] / 64;
    int sin_mov = a_sin[angle] / 64;

    int prev_px = px;
    int prev_py = py;

    if (digitalRead(UP) == LOW) {
        px += cos_mov;
        py -= sin_mov;
    } else if (digitalRead(RIGHT) == LOW) {
        angle = normalise_angle_degrees(angle - 2);
    } else if (digitalRead(LEFT) == LOW) {
        angle = normalise_angle_degrees(angle + 2);
    } else if (digitalRead(DOWN) == LOW) {
        px -= cos_mov;
        py += sin_mov;
    }

    if (check_grid(px, py)) {
        px = prev_px;
        py = prev_py;
    }

    draw_screen(px, py, angle);
    int after = micros();

    #ifdef BENCHMARK
        avg_frame_time += (after - before);
        avg_i++;
        if(avg_i % 1000 == 0){
            Serial.printf("FPS: %f\n", 1.0/((avg_frame_time/1000.0)) * 1000000.0);
            avg_frame_time = 0;
            avg_i = 1;
        }
    #endif
    

}
