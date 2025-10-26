#ifndef GASTRO_H
#define GASTRO_H

#include "utils.h"

#define GASTRO_FIXED_POINT 16
typedef i64 gastro_fix64;
#define GASTRO_FIX_ONE (1 << GASTRO_FIXED_POINT)
#define GASTRO_FIX_HALF 0x8000
gastro_fix64 gastro_fix_new(i64 x);
gastro_fix64 gastro_fix_from_double(double x);
double gastro_fix_to_double(gastro_fix64 x);
gastro_fix64 gastro_fix_mul(gastro_fix64 x, gastro_fix64 y);
gastro_fix64 gastro_fix_div(gastro_fix64 x, gastro_fix64 y);
gastro_fix64 gastro_fix_sqrt(gastro_fix64 s);

typedef struct { gastro_fix64 x, y; } gastro_vec2;
gastro_vec2 gastro_vec2_new(gastro_fix64 x, gastro_fix64 y);
gastro_vec2 gastro_vec2_add(gastro_vec2 x, gastro_vec2 y);
gastro_vec2 gastro_vec2_sub(gastro_vec2 x, gastro_vec2 y);
gastro_fix64 gastro_vec2_length(gastro_vec2 v);
gastro_fix64 gastro_vec2_cross(gastro_vec2 v, gastro_vec2 u);
typedef struct { gastro_fix64 x, y, z; } gastro_vec3;
gastro_vec3 gastro_vec3_new(gastro_fix64 x, gastro_fix64 y, gastro_fix64 z);
gastro_vec2 gastro_vec3_xy(gastro_vec3 v);
gastro_vec3 gastro_vec3_perspective(gastro_fix64 nearz, gastro_vec3 p);
gastro_fix64 gastro_vec3_bary_interpolate(gastro_vec3 bary, gastro_fix64 x, gastro_fix64 y, gastro_fix64 z);

typedef struct {
    u8 r, g, b, a;
} gastro_color;
gastro_color gastro_color_new(u8 r, u8 g, u8 b, u8 a);

typedef struct {
    u32 width, height;
    gastro_color *pixels;
    gastro_fix64 *depth;
} gastro_ctx;

void gastro_ctx_init(gastro_ctx *ret, u32 width, u32 height, gastro_color *pixels, gastro_fix64 *depth);
void gastro_draw_point(gastro_ctx *ctx, gastro_color col, gastro_fix64 z, i64 x, i64 y);
void gastro_draw_clear(gastro_ctx *ctx, gastro_color col);
void gastro_draw_triangle(gastro_ctx *ctx, gastro_color col, gastro_vec3 p0, gastro_vec3 p1, gastro_vec3 p2);
void gastro_draw_quad(gastro_ctx *ctx, gastro_color col, gastro_vec3 p0, gastro_vec3 p1, gastro_vec3 p2, gastro_vec3 p3);
void gastro_draw_cube(gastro_ctx *ctx, gastro_vec3 pos);

#endif
