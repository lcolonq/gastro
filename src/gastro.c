#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "gastro.h"
#include "utils.h"

gastro_fix64 gastro_fix_new(i64 x) {
    assert(x < ((i64) 1 << (sizeof(gastro_fix64) * 8 - GASTRO_FIXED_POINT)));
    return x << GASTRO_FIXED_POINT;
}

gastro_fix64 gastro_fix_from_double(double x) {
    double shift = (double) (1 << GASTRO_FIXED_POINT);
    return (gastro_fix64) (x * shift);
}
double gastro_fix_to_double(gastro_fix64 x) {
    double shift = (double) (1 << GASTRO_FIXED_POINT);
    return (double) x / shift;
}
gastro_fix64 gastro_fix_mul(gastro_fix64 x, gastro_fix64 y) {
    return (x * y) >> GASTRO_FIXED_POINT;
}
gastro_fix64 gastro_fix_div(gastro_fix64 x, gastro_fix64 y) {
    return (x * (1 << GASTRO_FIXED_POINT)) / y;
}
gastro_fix64 gastro_fix_sqrt(gastro_fix64 s) {
    gastro_fix64 x = GASTRO_FIX_ONE;
    i64 i;
    for (i = 0; i < 5; ++i) {
        x = gastro_fix_mul(GASTRO_FIX_HALF, x + gastro_fix_div(s, x));
    }
    return x;
}

gastro_vec2 gastro_vec2_new(gastro_fix64 x, gastro_fix64 y) {
    gastro_vec2 ret;
    ret.x = x; ret.y = y;
    return ret;
}
gastro_vec2 gastro_vec2_add(gastro_vec2 x, gastro_vec2 y) {
    return gastro_vec2_new(x.x + y.x, x.y + y.y);
}
gastro_vec2 gastro_vec2_sub(gastro_vec2 x, gastro_vec2 y) {
    return gastro_vec2_new(x.x - y.x, x.y - y.y);
}
gastro_fix64 gastro_vec2_length(gastro_vec2 v) {
    gastro_fix64 sq = gastro_fix_mul(v.x, v.x) + gastro_fix_mul(v.y, v.y);
    return gastro_fix_sqrt(sq);
}
gastro_fix64 gastro_vec2_cross(gastro_vec2 v, gastro_vec2 u) {
    return gastro_fix_mul(v.x, u.y) - gastro_fix_mul(u.x, v.y);
}
gastro_vec3 gastro_vec3_new(gastro_fix64 x, gastro_fix64 y, gastro_fix64 z) {
    gastro_vec3 ret;
    ret.x = x; ret.y = y; ret.z = z;
    return ret;
}
gastro_vec2 gastro_vec3_xy(gastro_vec3 v) { return gastro_vec2_new(v.x, v.y); }
gastro_vec3 gastro_vec3_perspective(gastro_fix64 nearz, gastro_vec3 p) {
    gastro_vec3 ret;
    ret.x = gastro_fix_mul(gastro_fix_div(nearz, p.z), p.x);
    ret.y = gastro_fix_mul(gastro_fix_div(nearz, p.z), p.y);
    ret.z = p.z;
    return ret;
}
gastro_fix64 gastro_vec3_bary_interpolate(gastro_vec3 bary, gastro_fix64 x, gastro_fix64 y, gastro_fix64 z) {
    return gastro_fix_mul(bary.x, x) + gastro_fix_mul(bary.y, y) + gastro_fix_mul(bary.z, z);
}

gastro_color gastro_color_new(u8 r, u8 g, u8 b, u8 a) {
    gastro_color ret;
    ret.r = r; ret.g = g; ret.b = b; ret.a = a;
    return ret;
}

void gastro_ctx_init(gastro_ctx *ret, u32 width, u32 height, gastro_color *pixels, gastro_fix64 *depth) {
    ret->width = width;
    ret->height = height;
    ret->pixels = pixels;
    ret->depth = depth;
}

void gastro_draw_point(gastro_ctx *ctx, gastro_color col, gastro_fix64 z, i64 x, i64 y) {
    i64 idx = y * ctx->width + x;
    if(x < 0 || x >= ctx->width || y < 0 || y >= ctx->height) return;
    if (z > ctx->depth[idx]) return;
    ctx->pixels[idx] = col;
    ctx->depth[idx] = z;
}

void gastro_draw_clear(gastro_ctx *ctx, gastro_color col) {
    i64 x, y;
    for (x = 0; x < ctx->width; ++x) {
        for (y = 0; y < ctx->height; ++y) {
            i64 idx = y * ctx->width + x;
            ctx->pixels[idx] = col;
            ctx->depth[idx] = INT64_MAX;
        }
    }
}

/* return negative if x is to the right of the line from p0 to p1 */
static gastro_fix64 compare_edge(gastro_vec2 x, gastro_vec2 p0, gastro_vec2 p1) {
    gastro_vec2 pd = gastro_vec2_sub(p1, p0);
    gastro_vec2 xd = gastro_vec2_sub(x, p0);
    gastro_fix64 cross = gastro_vec2_cross(xd, pd);
    return cross;
}
static bool triangle_contains(gastro_vec3 *bary, gastro_vec2 x, gastro_vec2 p0, gastro_vec2 p1, gastro_vec2 p2) {
    gastro_fix64 p0p1 = compare_edge(x, p0, p1);
    gastro_fix64 p1p2 = compare_edge(x, p1, p2);
    gastro_fix64 p2p0 = compare_edge(x, p2, p0);
    gastro_fix64 area = compare_edge(p2, p0, p1);
    if (area == 0) return false;
    bary->x = gastro_fix_div(p1p2, area);
    bary->y = gastro_fix_div(p2p0, area);
    bary->z = gastro_fix_div(p0p1, area);
    return p0p1 < 0 && p1p2 < 0 && p2p0 < 0;
}
void gastro_draw_triangle(gastro_ctx *ctx, gastro_color col, gastro_vec3 p0, gastro_vec3 p1, gastro_vec3 p2) {
    /* assume clockwise winding order of points */
    /* p0 -> p1, p1 -> p2, p2 -> p0 */
    i64 x, y;
    gastro_vec2 v0, v1, v2;
    v0 = gastro_vec3_xy(p0);
    v1 = gastro_vec3_xy(p1);
    v2 = gastro_vec3_xy(p2);
    gastro_draw_point(ctx, gastro_color_new(0xff, 0, 0, 0xff), 0, p0.x >> GASTRO_FIXED_POINT, p0.y >> GASTRO_FIXED_POINT);
    gastro_draw_point(ctx, gastro_color_new(0xff, 0, 0, 0xff), 0, p1.x >> GASTRO_FIXED_POINT, p1.y >> GASTRO_FIXED_POINT);
    gastro_draw_point(ctx, gastro_color_new(0xff, 0, 0, 0xff), 0, p2.x >> GASTRO_FIXED_POINT, p2.y >> GASTRO_FIXED_POINT);
    for (x = 0; x < ctx->width; ++x) {
        for (y = 0; y < ctx->height; ++y) {
            gastro_vec2 p = gastro_vec2_new(
                gastro_fix_new(x) + GASTRO_FIX_HALF,
                gastro_fix_new(y) + GASTRO_FIX_HALF
            );
            gastro_vec3 bary;
            if (triangle_contains(&bary, p, v0, v1, v2)) {
                gastro_fix64 z = gastro_fix_div(GASTRO_FIX_ONE, 
                    gastro_vec3_bary_interpolate(bary,
                        gastro_fix_div(GASTRO_FIX_ONE, p0.z),
                        gastro_fix_div(GASTRO_FIX_ONE, p1.z),
                        gastro_fix_div(GASTRO_FIX_ONE, p2.z)
                    )
                );
                /* col.r = (u8) (gastro_vec3_bary_interpolate(bary, gastro_fix_new(255), 0, 0) >> GASTRO_FIXED_POINT); */
                /* col.g = (u8) (gastro_vec3_bary_interpolate(bary, 0, gastro_fix_new(255), 0) >> GASTRO_FIXED_POINT); */
                /* col.b = (u8) (gastro_vec3_bary_interpolate(bary, 0, 0, gastro_fix_new(255)) >> GASTRO_FIXED_POINT); */
                /* col.a = 0xff; */
                gastro_draw_point(ctx, col, z, x, y);
            }
        }
    }
}
void gastro_draw_quad(gastro_ctx *ctx, gastro_color col, gastro_vec3 p0, gastro_vec3 p1, gastro_vec3 p2, gastro_vec3 p3) {
    gastro_draw_triangle(ctx, col, p0, p1, p2);
    gastro_draw_triangle(ctx, col, p2, p3, p0);
}

void gastro_draw_cube(gastro_ctx *ctx, gastro_vec3 pos) {
    gastro_fix64 sz = gastro_fix_new(40);
    gastro_fix64 zsz = gastro_fix_from_double(0.25);
    gastro_vec3 p0 = gastro_vec3_perspective(GASTRO_FIX_ONE, gastro_vec3_new(pos.x - sz, pos.y - sz, pos.z - zsz));
    gastro_vec3 p1 = gastro_vec3_perspective(GASTRO_FIX_ONE, gastro_vec3_new(pos.x + sz, pos.y - sz, pos.z - zsz));
    gastro_vec3 p2 = gastro_vec3_perspective(GASTRO_FIX_ONE, gastro_vec3_new(pos.x + sz, pos.y + sz, pos.z - zsz));
    gastro_vec3 p3 = gastro_vec3_perspective(GASTRO_FIX_ONE, gastro_vec3_new(pos.x - sz, pos.y + sz, pos.z - zsz));
    gastro_vec3 p4 = gastro_vec3_perspective(GASTRO_FIX_ONE, gastro_vec3_new(pos.x - sz, pos.y - sz, pos.z + zsz));
    gastro_vec3 p5 = gastro_vec3_perspective(GASTRO_FIX_ONE, gastro_vec3_new(pos.x + sz, pos.y - sz, pos.z + zsz));
    gastro_vec3 p6 = gastro_vec3_perspective(GASTRO_FIX_ONE, gastro_vec3_new(pos.x + sz, pos.y + sz, pos.z + zsz));
    gastro_vec3 p7 = gastro_vec3_perspective(GASTRO_FIX_ONE, gastro_vec3_new(pos.x - sz, pos.y + sz, pos.z + zsz));
    /* gastro_vec3 p0 = gastro_vec3_new(pos.x - sz, pos.y - sz, pos.z - sz); */
    /* gastro_vec3 p1 = gastro_vec3_new(pos.x + sz, pos.y - sz, pos.z - sz); */
    /* gastro_vec3 p2 = gastro_vec3_new(pos.x + sz, pos.y + sz, pos.z - sz); */
    /* gastro_vec3 p3 = gastro_vec3_new(pos.x - sz, pos.y + sz, pos.z - sz); */
    /* gastro_vec3 p4 = gastro_vec3_new(pos.x - sz, pos.y - sz, pos.z + sz); */
    /* gastro_vec3 p5 = gastro_vec3_new(pos.x + sz, pos.y - sz, pos.z + sz); */
    /* gastro_vec3 p6 = gastro_vec3_new(pos.x + sz, pos.y + sz, pos.z + sz); */
    /* gastro_vec3 p7 = gastro_vec3_new(pos.x - sz, pos.y + sz, pos.z + sz); */
    gastro_draw_quad(ctx, gastro_color_new(0xff, 0, 0, 0xff), p0, p1, p2, p3); /* front */
    gastro_draw_quad(ctx, gastro_color_new(0, 0xff, 0, 0xff), p1, p5, p6, p2); /* right */
    gastro_draw_quad(ctx, gastro_color_new(0, 0, 0xff, 0xff), p4, p0, p3, p7); /* left */
    gastro_draw_quad(ctx, gastro_color_new(0xff, 0, 0xff, 0xff), p4, p5, p1, p0); /* top */
    gastro_draw_quad(ctx, gastro_color_new(0xff, 0xff, 0, 0xff), p3, p2, p6, p7); /* bottom */
    gastro_draw_quad(ctx, gastro_color_new(0xff, 0xff, 0xff, 0xff), p5, p4, p7, p6); /* back */
}
