#include <Arduino.h>
#include <cstdlib>
#include <cstdint>
#include "lut.h"

#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC     2
#define OLED_CS     5
#define OLED_RESET  4

#define UP 33
#define RIGHT 34
#define DOWN 35
#define LEFT 32

#define DEBUG 0

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define BLOCK_SIZE 32

#define WORLD_SIZE 15
#define FOV_DEGREES 60

// Could be 128?
#define DISTANCE_TO_PROJECTION 110 // = SCREEN_WIDTH / tan(30deg)

// 60 / 128 * 256
#define RAY_STEP 120

char screen_buffer[SCREEN_HEIGHT / 8][SCREEN_WIDTH] = {0};

char world[WORLD_SIZE][WORLD_SIZE] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,1,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,1,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},

    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};



void clockUp(){
  digitalWrite(OLED_CLK, HIGH);
}

void clockDown(){
  digitalWrite(OLED_CLK, LOW);
}


void send_byte(char byte, int is_data){

  int DC_VALUE;
  if(is_data){
    DC_VALUE = HIGH;
  }
  else{
    DC_VALUE = LOW;
  }

  digitalWrite(OLED_DC, DC_VALUE);
  digitalWrite(OLED_CS, LOW);
  for(int i = 0; i < 8; i++){
    if(byte & 0x80){
      digitalWrite(OLED_MOSI, HIGH);
    }
    else{
      digitalWrite(OLED_MOSI, LOW);
    }
    clockUp();
    clockDown();
    byte = byte << 1;
  }
  digitalWrite(OLED_DC, !DC_VALUE);
  digitalWrite(OLED_CS, HIGH);

}

int check_grid(int x, int y){
    int grid_x, grid_y;
    grid_x = (x / BLOCK_SIZE);
    grid_y = (y / BLOCK_SIZE);

    if(grid_x >= WORLD_SIZE || grid_y >= WORLD_SIZE || grid_x < 0 || grid_y < 0){
        return 1;
    }

    return world[grid_y][grid_x];
}


int normalise_angle_degrees(int degree){
    if(degree > 360){
        return degree - 360;
    }
    if(degree < 0){
        return 360 + degree;
    }
    return degree;
}

int abs(int x){
    if(x < 0)
        return -x;

    return x;
}

int min(int a, int b){
    if(a < b) return a;
    return b;
}


int max(int a, int b){
    if(a > b) return a;
    return b;
}

// Returns the wall distance
int get_horizontal_collision(int px, int py, int ray_angle, int* horizontal_hit){
    // First horizontal contact
    int ax, ay;

    // Last horizontal contact
    int dx, dy;

    int delta_x, delta_y;

    // Check horizontal distance
    ray_angle >>= 8;
    ray_angle = normalise_angle_degrees(ray_angle);

    if((ray_angle < 180) && (ray_angle >= 0)){
        ay = ((py / BLOCK_SIZE)) * BLOCK_SIZE - 1;
        delta_y = -BLOCK_SIZE;
    }
    else{
        ay = ((py / BLOCK_SIZE) + 1) * BLOCK_SIZE;
        delta_y = BLOCK_SIZE;
    }

    int cot = 0;
    if((ray_angle > 90) && (ray_angle <= 270)){
        cot = -abs(a_cot[ray_angle]);
    }
    else{
        cot = abs(a_cot[ray_angle]);
    }

    ax = abs(ay - py) * cot;
    ax >>= 8;
    ax += px;
    delta_x = BLOCK_SIZE * cot;
    delta_x >>= 8;



    dx = ax;
    dy = ay;
    *horizontal_hit = 0;

    while(*horizontal_hit == 0){
        *horizontal_hit = check_grid(dx, dy);
        dx += delta_x;
        dy += delta_y;
    }

    //Distance (distorted)
    int d = abs(dy - py) * abs(a_sin_iv[ray_angle]);
    d >>= 8;
    return d;
}

// Returns the wall distance
int get_vertical_collision(int px, int py, int ray_angle, int* vertical_hit){
    // First vertical contact
    int bx, by;

    // Last vertical contact
    int ex, ey;

    int delta_x, delta_y;

    ray_angle >>= 8;
    ray_angle = normalise_angle_degrees(ray_angle);

    // Check vertical distance


    if((ray_angle < 90) || (ray_angle >= 270)){
        bx = ((px / BLOCK_SIZE) + 1) * BLOCK_SIZE;
        delta_x = BLOCK_SIZE;

    }
    else{
        bx = ((px / BLOCK_SIZE)) * BLOCK_SIZE - 1;
        delta_x = -BLOCK_SIZE;

    }



    int tan = 0;
    if((ray_angle > 0) && (ray_angle <= 180)){
        tan = -abs(a_tan[ray_angle]);
    }
    else{
        tan = abs(a_tan[ray_angle]);
    }

    by = abs(px - bx) * tan;
    by >>= 8;
    by += py;
    delta_y = BLOCK_SIZE * tan;
    delta_y >>= 8;



    ex = bx;
    ey = by;
    //Search until collision

    *vertical_hit = 0;
    while(*vertical_hit == 0){
        *vertical_hit = check_grid(ex, ey);
        ex += delta_x;
        ey += delta_y;
    }

    //Distance (distorted)
    int d = abs(ex - px) * abs(a_cos_iv[ray_angle]);
    d >>= 8;
    return d;
}

void print_buf(){

    for(int i = 0; i < SCREEN_HEIGHT / 8; i++){
        for(int j = 0; j < SCREEN_WIDTH; j++){
            send_byte(screen_buffer[i][j], 1);
        }
    }
}

void draw_screen(int px, int py, int view_angle){
    int ray_angle = view_angle - FOV_DEGREES / 2;
    ray_angle <<= 8;

    int beta = ray_angle;

    int switch_var = 0;

    int* horizontal_hit = (int*)malloc(sizeof(int));
    int* vertical_hit = (int*)malloc(sizeof(int));
    for(int i = 0; i < 128; i++){

        int horizontal_distance = get_horizontal_collision(px, py, ray_angle, horizontal_hit);

        int vertical_distance = get_vertical_collision(px, py, ray_angle, vertical_hit);
        int min_dist = min(horizontal_distance, vertical_distance);


        int beta = (ray_angle >> 8) - view_angle;
        beta = normalise_angle_degrees(beta);

        long long corrected_distance = a_cos[beta] * min_dist;
        corrected_distance >>= 8;

        if(corrected_distance <= 0)
            corrected_distance++;

        int projected_height = (BLOCK_SIZE * DISTANCE_TO_PROJECTION) / corrected_distance;
        int range_start = SCREEN_HEIGHT / 2 - projected_height / 2;
        int range_end = SCREEN_HEIGHT / 2 + projected_height / 2;

        int hit_block;

        if(min_dist == vertical_distance){
            hit_block = *vertical_hit;
        }
        else{
            hit_block = *horizontal_hit;
        }
        for(int16_t j = 0; j < SCREEN_HEIGHT / 8; j++){
            unsigned char mask = 0x00;
            int page_base_y = j * 8;

            for(int bit = 0; bit < 8; bit++){
                int pixel_y = page_base_y + bit;
                
                if(pixel_y >= range_start && pixel_y <= range_end){
                    mask |= (1 << bit); 
                }
            }
            screen_buffer[j][i] = mask;
          }
        ray_angle += RAY_STEP;
    }
    print_buf();
    free(vertical_hit);
    free(horizontal_hit);
}


void setup() {
  pinMode(OLED_MOSI, OUTPUT);
  pinMode(OLED_CLK, OUTPUT);

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

}



int px = 36;
int py = 36;
int angle = 0;

void loop() {

    angle = normalise_angle_degrees(angle);
    int cos_mov = a_cos[angle] / 64;
    int sin_mov = a_sin[angle] / 64;

    int prev_px = px;
    int prev_py = py;

    if(digitalRead(UP) == LOW){
        px += cos_mov;
        py -= sin_mov;
    }
    else if(digitalRead(RIGHT) == LOW){
        angle = normalise_angle_degrees(angle - 2);
    }
    else if(digitalRead(LEFT) == LOW){
        angle = normalise_angle_degrees(angle + 2);
    }
    else if(digitalRead(DOWN) == LOW){
        px -= cos_mov;
        py += sin_mov;
    }

    if(check_grid(px, py)){
        px = prev_px;
        py = prev_py;
    }

    draw_screen(px, py, angle);

    delay(10);
}
