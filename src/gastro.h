#ifndef GASTRO_H
#define GASTRO_H

#include "utils.h"

#define GASTRO_FIXED_POINT 16
typedef i64 gastro_fix64;
#define GASTRO_FIX_ONE (1 << GASTRO_FIXED_POINT)
#define GASTRO_FIX_HALF 0x8000
gastro_fix64 gastro_fix_new(i64 x);
i64 gastro_fix_to_i64(gastro_fix64 x);
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
void gastro_vec3_print(gastro_vec3 v);
gastro_vec2 gastro_vec3_xy(gastro_vec3 v);
gastro_vec3 gastro_vec3_perspective(gastro_fix64 nearz, gastro_vec3 p);
gastro_fix64 gastro_vec3_bary_interpolate(gastro_vec3 bary, gastro_fix64 x, gastro_fix64 y, gastro_fix64 z);
typedef struct { gastro_fix64 x, y, z, w; } gastro_vec4;
gastro_vec4 gastro_vec4_new(gastro_fix64 x, gastro_fix64 y, gastro_fix64 z, gastro_fix64 w);
void gastro_vec4_print(gastro_vec4 v);
gastro_vec3 gastro_vec4_xyz(gastro_vec4 v);
gastro_fix64 gastro_vec4_idx(gastro_vec4 v, i64 idx);
gastro_fix64 gastro_vec4_dot(gastro_vec4 a, gastro_vec4 b);

typedef struct { gastro_vec4 rows[4]; } gastro_mat4x4;
gastro_mat4x4 gastro_mat4x4_new(
    gastro_fix64  e0, gastro_fix64  e1, gastro_fix64  e2, gastro_fix64  e3,
    gastro_fix64  e4, gastro_fix64  e5, gastro_fix64  e6, gastro_fix64  e7,
    gastro_fix64  e8, gastro_fix64  e9, gastro_fix64 e10, gastro_fix64 e11,
    gastro_fix64 e12, gastro_fix64 e13, gastro_fix64 e14, gastro_fix64 e15);
void gastro_mat4x4_print(gastro_mat4x4 m);
gastro_vec4 gastro_mat4x4_mul_vec4(gastro_mat4x4 m, gastro_vec4 v);
gastro_mat4x4 gastro_mat4x4_mul_mat4x4(gastro_mat4x4 m, gastro_mat4x4 n);

typedef struct {
    u8 r, g, b, a;
} gastro_color;
gastro_color gastro_color_new(u8 r, u8 g, u8 b, u8 a);

typedef struct {
    u32 width, height;
    gastro_color *pixels;
    gastro_fix64 *depth;
} gastro_ctx;

gastro_vec3 gastro_vec3_denormalize(gastro_ctx *ctx, gastro_vec3 p);

void gastro_ctx_init(gastro_ctx *ret, u32 width, u32 height, gastro_color *pixels, gastro_fix64 *depth);
void gastro_draw_pixel(gastro_ctx *ctx, gastro_color col, gastro_fix64 z, i64 x, i64 y);
void gastro_draw_clear(gastro_ctx *ctx, gastro_color col);

typedef struct {
    gastro_vec3 v;
    gastro_fix64 *attrs;
} gastro_vertex;
typedef gastro_vec4 (*gastro_shader_vertex)(gastro_ctx *ctx, gastro_vec3 v, gastro_fix64 *attrs, i64 attrs_len);
typedef gastro_color (*gastro_shader_fragment)(gastro_ctx *ctx, gastro_vec3 v, gastro_fix64 *attrs, i64 attrs_len);
typedef struct {
    gastro_shader_vertex vertex;
    gastro_shader_fragment fragment;
} gastro_program;
gastro_vertex gastro_program_vertex(gastro_ctx *ctx, gastro_program *p, gastro_vec3 v, gastro_fix64 *attrs, i64 attrs_len);
gastro_color gastro_program_fragment(gastro_ctx *ctx, gastro_program *p, gastro_vec3 v, gastro_fix64 *attrs, i64 attrs_len);

void gastro_render_triangle(gastro_ctx *ctx, gastro_program *p, gastro_vertex p0, gastro_vertex p1, gastro_vertex p2, i64 attrs_len);
void gastro_render_triangles(gastro_ctx *ctx, gastro_program *p, gastro_fix64 *vs, i64 elem_size, i64 *idxs, i64 num_triangles);

#include "trig.h"

#endif
