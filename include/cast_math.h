#include "lut.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define BLOCK_SIZE 32

#define WORLD_SIZE 15
#define FOV_DEGREES 60

// Could be 128?
#define DISTANCE_TO_PROJECTION 110 // = SCREEN_WIDTH / tan(30deg)

// 60 / 128 * 256
#define RAY_STEP 120


const char world[WORLD_SIZE][WORLD_SIZE] = {
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

static inline int normalise_angle_degrees(int degree){
    if(degree > 360){
        return degree - 360;
    }
    if(degree < 0){
        return 360 + degree;
    }
    return degree;
}

static inline int cast_abs(int x){
    if(x < 0)
        return -x;

    return x;
}

static inline int cast_min(int a, int b){
    if(a < b) return a;
    return b;
}


static inline int cast_max(int a, int b){
    if(a > b) return a;
    return b;
}

int get_horizontal_collision(int px, int py, int ray_angle, int* horizontal_hit);
int get_vertical_collision(int px, int py, int ray_angle, int* vertical_hit);
int check_grid(int x, int y);