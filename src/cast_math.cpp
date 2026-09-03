#include "../include/cast_math.h"



int check_grid(int x, int y){
    int grid_x, grid_y;
    grid_x = (x / BLOCK_SIZE);
    grid_y = (y / BLOCK_SIZE);

    if(grid_x >= WORLD_SIZE || grid_y >= WORLD_SIZE || grid_x < 0 || grid_y < 0){
        return 1;
    }

    return world[grid_y][grid_x];
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
        cot = -cast_abs(a_cot[ray_angle]);
    }
    else{
        cot = cast_abs(a_cot[ray_angle]);
    }

    ax = cast_abs(ay - py) * cot;
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
    int d = cast_abs(dy - py) * cast_abs(a_sin_iv[ray_angle]);
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
        tan = -cast_abs(a_tan[ray_angle]);
    }
    else{
        tan = cast_abs(a_tan[ray_angle]);
    }

    by = cast_abs(px - bx) * tan;
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
    int d = cast_abs(ex - px) * cast_abs(a_cos_iv[ray_angle]);
    d >>= 8;
    return d;
}