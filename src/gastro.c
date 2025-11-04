#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "gastro.h"
#include "utils.h"

#define MAX_ATTRS 16

static i64 min2(i64 x, i64 y) { return x < y ? x : y; }
static i64 min3(i64 x, i64 y, i64 z) { return min2(min2(x, y), z); }
static i64 max2(i64 x, i64 y) { return x > y ? x : y; }
static i64 max3(i64 x, i64 y, i64 z) { return max2(max2(x, y), z); }

gastro_fix gastro_fix_new(i64 x) {
    assert(x < ((i64) 1 << (sizeof(gastro_fix) * 8 - GASTRO_FIXED_POINT)));
    return x << GASTRO_FIXED_POINT;
}
i64 gastro_fix_to_i64(gastro_fix x) {
    return x >> GASTRO_FIXED_POINT;
}
gastro_fix gastro_fix_from_double(double x) {
    double shift = (double) (1 << GASTRO_FIXED_POINT);
    return (gastro_fix) (x * shift);
}
double gastro_fix_to_double(gastro_fix x) {
    double shift = (double) (1 << GASTRO_FIXED_POINT);
    return (double) x / shift;
}
gastro_fix gastro_fix_mul(gastro_fix x, gastro_fix y) {
    return (gastro_fix) (x * y) >> GASTRO_FIXED_POINT;
}
gastro_fix gastro_fix_div(gastro_fix x, gastro_fix y) {
    return (x * (1 << GASTRO_FIXED_POINT)) / y;
}
gastro_fix gastro_fix_sqrt(gastro_fix s) {
    gastro_fix x = GASTRO_FIX_ONE;
    i64 i;
    for (i = 0; i < 5; ++i) {
        x = gastro_fix_mul(GASTRO_FIX_HALF, x + gastro_fix_div(s, x));
    }
    return x;
}

gastro_vec2 gastro_vec2_new(gastro_fix x, gastro_fix y) {
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
gastro_fix gastro_vec2_length(gastro_vec2 v) {
    gastro_fix sq = gastro_fix_mul(v.x, v.x) + gastro_fix_mul(v.y, v.y);
    return gastro_fix_sqrt((gastro_fix) sq);
}
gastro_fix gastro_vec2_cross(gastro_vec2 v, gastro_vec2 u) {
    return gastro_fix_mul(v.x, u.y) - gastro_fix_mul(u.x, v.y);
}
gastro_vec3 gastro_vec3_new(gastro_fix x, gastro_fix y, gastro_fix z) {
    gastro_vec3 ret;
    ret.x = x; ret.y = y; ret.z = z;
    return ret;
}
void gastro_vec3_print(gastro_vec3 v) {
    printf("<%f, %f, %f>\n", gastro_fix_to_double(v.x), gastro_fix_to_double(v.y), gastro_fix_to_double(v.z));
}
gastro_vec2 gastro_vec3_xy(gastro_vec3 v) { return gastro_vec2_new(v.x, v.y); }
gastro_vec3 gastro_vec3_perspective(gastro_fix nearz, gastro_vec3 p) {
    gastro_vec3 ret;
    ret.x = gastro_fix_mul(gastro_fix_div(nearz, p.z), p.x);
    ret.y = gastro_fix_mul(gastro_fix_div(nearz, p.z), p.y);
    ret.z = p.z;
    return ret;
}
gastro_fix gastro_vec3_bary_interpolate(gastro_vec3 bary, gastro_fix x, gastro_fix y, gastro_fix z) {
    return gastro_fix_mul(bary.x, x) + gastro_fix_mul(bary.y, y) + gastro_fix_mul(bary.z, z);
}
gastro_fix gastro_vec3_bary_interpolate_inv(gastro_vec3 bary, gastro_fix x, gastro_fix y, gastro_fix z) {
    return gastro_fix_div(GASTRO_FIX_ONE, 
        gastro_vec3_bary_interpolate(bary,
            gastro_fix_div(GASTRO_FIX_ONE, x),
            gastro_fix_div(GASTRO_FIX_ONE, y),
            gastro_fix_div(GASTRO_FIX_ONE, z)
        )
    );
}

gastro_vec4 gastro_vec4_new(gastro_fix x, gastro_fix y, gastro_fix z, gastro_fix w) {
    gastro_vec4 ret;
    ret.x = x; ret.y = y; ret.z = z; ret.w = w;
    return ret;
}
void gastro_vec4_print(gastro_vec4 v) {
    printf("<%f, %f, %f, %f>\n", gastro_fix_to_double(v.x), gastro_fix_to_double(v.y), gastro_fix_to_double(v.z), gastro_fix_to_double(v.w));
}
gastro_vec3 gastro_vec4_xyz(gastro_vec4 v) { return gastro_vec3_new(v.x, v.y, v.z); }
gastro_fix gastro_vec4_idx(gastro_vec4 v, i64 idx) {
    assert(idx >= 0 && idx < 4);
    switch (idx) {
    case 0: return v.x;
    case 1: return v.y;
    case 2: return v.z;
    case 3: return v.w;
    default: return 0;
    }
}
gastro_fix gastro_vec4_dot(gastro_vec4 a, gastro_vec4 b) {
    return gastro_fix_mul(a.x, b.x)
        + gastro_fix_mul(a.y, b.y)
        + gastro_fix_mul(a.z, b.z)
        + gastro_fix_mul(a.w, b.w);
}

gastro_mat4x4 gastro_mat4x4_new(
    gastro_fix  e0, gastro_fix  e1, gastro_fix  e2, gastro_fix  e3,
    gastro_fix  e4, gastro_fix  e5, gastro_fix  e6, gastro_fix  e7,
    gastro_fix  e8, gastro_fix  e9, gastro_fix e10, gastro_fix e11,
    gastro_fix e12, gastro_fix e13, gastro_fix e14, gastro_fix e15) {
    gastro_mat4x4 ret;
    ret.rows[0] = gastro_vec4_new( e0,  e1,  e2,  e3);
    ret.rows[1] = gastro_vec4_new( e4,  e5,  e6,  e7);
    ret.rows[2] = gastro_vec4_new( e8,  e9, e10, e11);
    ret.rows[3] = gastro_vec4_new(e12, e13, e14, e15);
    return ret;
}
void gastro_mat4x4_print(gastro_mat4x4 m) {
    i64 row;
    for (row = 0; row < 4; ++row) {
        printf("%c %14f, %14f, %14f, %14f, %c\n",
            row == 0 ? '[' : ' ',
            gastro_fix_to_double(m.rows[row].x),
            gastro_fix_to_double(m.rows[row].y),
            gastro_fix_to_double(m.rows[row].z),
            gastro_fix_to_double(m.rows[row].w),
            row == 3 ? ']' : ' ');
    }
}
gastro_vec4 gastro_mat4x4_mul_vec4(gastro_mat4x4 m, gastro_vec4 v) {
    return gastro_vec4_new(
        gastro_vec4_dot(m.rows[0], v),
        gastro_vec4_dot(m.rows[1], v),
        gastro_vec4_dot(m.rows[2], v),
        gastro_vec4_dot(m.rows[3], v));
}
gastro_mat4x4 gastro_mat4x4_mul_mat4x4(gastro_mat4x4 n, gastro_mat4x4 m) {
    gastro_mat4x4 ret;
    i64 row, col;
    for (row = 0; row < 4; ++row) {
        gastro_fix comps[4];
        for (col = 0; col < 4; ++col) {
            comps[col] = gastro_vec4_dot(
                gastro_vec4_new(
                    gastro_vec4_idx(m.rows[0], col),
                    gastro_vec4_idx(m.rows[1], col),
                    gastro_vec4_idx(m.rows[2], col),
                    gastro_vec4_idx(m.rows[3], col)),
                n.rows[row]);
        }
        ret.rows[row] = gastro_vec4_new(comps[0], comps[1], comps[2], comps[3]);
    }
    return ret;
}

gastro_color gastro_color_new(u8 r, u8 g, u8 b, u8 a) {
    gastro_color ret;
    ret.r = r; ret.g = g; ret.b = b; ret.a = a;
    return ret;
}

void gastro_ctx_init(gastro_ctx *ret, u32 width, u32 height, gastro_color *pixels, gastro_fix *depth) {
    ret->width = width;
    ret->height = height;
    ret->pixels = pixels;
    ret->depth = depth;
}

gastro_vec3 gastro_vec3_denormalize(gastro_ctx *ctx, gastro_vec3 p) {
    gastro_vec3 ret;
    ret.x = gastro_fix_mul(p.x + GASTRO_FIX_ONE, gastro_fix_new(ctx->width / 2));
    ret.y = gastro_fix_mul(p.y + GASTRO_FIX_ONE, gastro_fix_new(ctx->height / 2));
    ret.z = p.z;
    return ret;
}

void gastro_draw_pixel(gastro_ctx *ctx, gastro_color col, gastro_fix z, i64 x, i64 y) {
    i64 idx = y * ctx->width + x;
    if (x < 0 || x >= ctx->width || y < 0 || y >= ctx->height) return;
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

gastro_vertex gastro_program_vertex(gastro_ctx *ctx, gastro_program *p, gastro_vec3 v, gastro_fix *attrs, i64 attrs_len) {
    gastro_vertex ret;
    gastro_vec4 res = p->vertex(ctx, v, attrs, attrs_len);
    res.x = gastro_fix_div(res.x, res.w);
    res.y = gastro_fix_div(res.y, res.w);
    /* res.z = gastro_fix_div(res.z, res.w); */
    ret.v = gastro_vec4_xyz(res);
    ret.attrs = attrs;
    return ret;
}
gastro_color gastro_program_fragment(gastro_ctx *ctx, gastro_program *p, gastro_vec3 v, gastro_fix *attrs, i64 attrs_len) {
    gastro_color ret;
    ret = p->fragment(ctx, v, attrs, attrs_len);
    return ret;
}

/* return negative if x is to the right of the line from p0 to p1 */
static gastro_fix compare_edge(gastro_vec2 x, gastro_vec2 p0, gastro_vec2 p1) {
    gastro_vec2 pd = gastro_vec2_sub(p1, p0);
    gastro_vec2 xd = gastro_vec2_sub(x, p0);
    gastro_fix cross = gastro_vec2_cross(xd, pd);
    return cross;
}
static bool triangle_contains(gastro_vec3 *bary, gastro_vec2 x, gastro_vec2 p0, gastro_vec2 p1, gastro_vec2 p2) {
    gastro_fix p0p1 = compare_edge(x, p0, p1);
    gastro_fix p1p2 = compare_edge(x, p1, p2);
    gastro_fix p2p0 = compare_edge(x, p2, p0);
    gastro_fix area = compare_edge(p2, p0, p1);
    if (area == 0) return false;
    bary->x = gastro_fix_div(p1p2, area);
    bary->y = gastro_fix_div(p2p0, area);
    bary->z = gastro_fix_div(p0p1, area);
    return p0p1 <= 0 && p1p2 <= 0 && p2p0 <= 0;
}
void gastro_render_triangle(gastro_ctx *ctx, gastro_program *p, gastro_vertex n0, gastro_vertex n1, gastro_vertex n2, i64 attrs_len) {
    /* assume clockwise winding order of points */
    /* p0 -> p1, p1 -> p2, p2 -> p0 */
    i64 x, y, minx, miny, maxx, maxy;
    gastro_fix attrs[MAX_ATTRS];
    /* convert all points from -1 to 1 range (normalized) into pixels */
    gastro_vec3 p0 = gastro_vec3_denormalize(ctx, n0.v); gastro_vec2 v0 = gastro_vec3_xy(p0);
    gastro_vec3 p1 = gastro_vec3_denormalize(ctx, n1.v); gastro_vec2 v1 = gastro_vec3_xy(p1);
    gastro_vec3 p2 = gastro_vec3_denormalize(ctx, n2.v); gastro_vec2 v2 = gastro_vec3_xy(p2);
    /* compute a bounding box for the triangle - we know only these pixels could possible be drawn */
    minx = min3(gastro_fix_to_i64(p0.x), gastro_fix_to_i64(p1.x), gastro_fix_to_i64(p2.x));
    maxx = max3(gastro_fix_to_i64(p0.x), gastro_fix_to_i64(p1.x), gastro_fix_to_i64(p2.x));
    miny = min3(gastro_fix_to_i64(p0.y), gastro_fix_to_i64(p1.y), gastro_fix_to_i64(p2.y));
    maxy = max3(gastro_fix_to_i64(p0.y), gastro_fix_to_i64(p1.y), gastro_fix_to_i64(p2.y));
    for (x = minx; x <= maxx; ++x) { /* for each pixel in the bounding box: */
        for (y = miny; y <= maxy; ++y) {
            i64 idx = y * ctx->width + x;
            /* consider the center of the pixel, not the top left corner */
            gastro_vec2 centered = gastro_vec2_new(
                gastro_fix_new(x) + GASTRO_FIX_HALF,
                gastro_fix_new(y) + GASTRO_FIX_HALF
            );
            gastro_vec3 bary;
            /* if the center of the pixel is in the triangle */
            if (triangle_contains(&bary, centered, v0, v1, v2)) {
                i64 a;
                gastro_color col;
                /* find the depth of this pixel by using the barycentric coordinates */
                /* we must interpolate the reciprocals to be perspective-correct, since p0 etc. are already projected  */
                /* i don't understand why this works very well yet */
                gastro_fix z = gastro_vec3_bary_interpolate_inv(bary, p0.z, p1.z, p2.z);
                if (z > ctx->depth[idx]) continue;
                gastro_vec3 interp_v = gastro_vec3_new(
                    gastro_vec3_bary_interpolate(bary, p0.x, p1.x, p2.x),
                    gastro_vec3_bary_interpolate(bary, p0.y, p1.y, p2.y),
                    z
                );
                /* interpolate all of the attributes as well */
                /* again, to be perspective correct, we do some funny math */
                for (a = 0; a < attrs_len; ++a) {
                    gastro_fix n0z = gastro_fix_div(n0.attrs[a], p0.z);
                    gastro_fix n1z = gastro_fix_div(n1.attrs[a], p1.z);
                    gastro_fix n2z = gastro_fix_div(n2.attrs[a], p2.z);
                    attrs[a] = gastro_fix_mul(
                        z,
                        gastro_vec3_bary_interpolate(bary, n0z, n1z, n2z)
                    );
                }
                /* actually compute the color for and write the pixel */
                col = gastro_program_fragment(ctx, p, interp_v, attrs, attrs_len);
                gastro_draw_pixel(ctx, col, z, x, y);
                ctx->pixels[idx] = col;
                ctx->depth[idx] = z;
            }
        }
    }
}

static gastro_vertex get_vertex(gastro_ctx *ctx, gastro_program *p, gastro_fix *vs, i64 elem_size, i64 idx) {
    i64 i = idx * elem_size;
    return gastro_program_vertex(ctx, p, gastro_vec3_new(vs[i], vs[i+1], vs[i+2]), &vs[i+3], elem_size - 3);
}
void gastro_render_triangles(gastro_ctx *ctx, gastro_program *p, gastro_fix *vs, i64 elem_size, i64 *idxs, i64 num_triangles) {
    i64 i;
    for (i = 0; i < 3 * num_triangles; i += 3) {
        gastro_vertex v0 = get_vertex(ctx, p, vs, elem_size, idxs[i]);
        gastro_vertex v1 = get_vertex(ctx, p, vs, elem_size, idxs[i + 1]);
        gastro_vertex v2 = get_vertex(ctx, p, vs, elem_size, idxs[i + 2]);
        gastro_render_triangle(ctx, p, v0, v1, v2, elem_size - 3);
    }
}
