#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <raylib.h>

#include "gastro.h"

#define GF gastro_fix_from_double

#define QUAD(p0, p1, p2, p3) p0, p1, p2, p2, p3, p0
#define COLOR(r, g, b) gastro_fix_new(r), gastro_fix_new(g), gastro_fix_new(b)
static void draw_cube(gastro_ctx *ctx, gastro_program *p, gastro_color front) {
    gastro_fix64 sz = GASTRO_FIX_HALF;
    gastro_fix64 verts[] = {
        - sz, - sz, - sz, COLOR(0xff, 0, 0),
        + sz, - sz, - sz, COLOR(0, 0xff, 0),
        + sz, + sz, - sz, COLOR(0, 0, 0xff),
        - sz, + sz, - sz, COLOR(0xff, 0xff, 0),
        - sz, - sz, + sz, COLOR(0xff, 0, 0xff),
        + sz, - sz, + sz, COLOR(0, 0xff, 0xff),
        + sz, + sz, + sz, COLOR(0, 0, 0),
        - sz, + sz, + sz, COLOR(0xff, 0xff, 0xff),
    };
    i64 idxs[] = {
        QUAD(0, 1, 2, 3), /* front */
        QUAD(1, 5, 6, 2), /* right */
        QUAD(4, 0, 3, 7), /* left */
        QUAD(4, 5, 1, 0), /* top */
        QUAD(3, 2, 6, 7), /* bottom */
        QUAD(5, 4, 7, 6) /* back */
    };
    gastro_render_triangles(ctx, p, verts, 6, idxs, 12);
}

static gastro_vec4 vertex_shader(gastro_ctx *ctx, gastro_vec3 in, gastro_fix64 *attrs, i64 attrs_len) {
    static i64 off = 0;
    gastro_fix64 x = gastro_fix_from_double((double) off / 1000.0);
    gastro_vec4 v = gastro_vec4_new(in.x, in.y, in.z, GASTRO_FIX_ONE);
    gastro_mat4x4 scale = gastro_mat4x4_new(
        gastro_fix_from_double(0.5), 0, 0, 0,
        0, GASTRO_FIX_ONE, 0, 0,
        0, 0, gastro_fix_from_double(1.0), 0,
        0, 0, 0, GASTRO_FIX_ONE
    );
    u8 theta = (u8) (off / 32);
    gastro_mat4x4 rot = gastro_mat4x4_new(
        GASTRO_FIX_ONE, 0, 0, 0,
        0, gastro_cos(theta), -gastro_sin(theta), 0,
        0, gastro_sin(theta), gastro_cos(theta), 0,
        0, 0, 0, GASTRO_FIX_ONE
    );
    gastro_mat4x4 trans = gastro_mat4x4_new(
        GASTRO_FIX_ONE, 0, 0, gastro_fix_new(-1), /* x,*/
        0, GASTRO_FIX_ONE, 0, gastro_fix_new(-1),
        0, 0, GASTRO_FIX_ONE, gastro_fix_new(3),
        0, 0, 0, GASTRO_FIX_ONE
    );
    gastro_mat4x4 proj = gastro_mat4x4_new(
        GASTRO_FIX_ONE, 0, 0, 0,
        0, GASTRO_FIX_ONE, 0, 0,
        0, 0, GASTRO_FIX_ONE, 0,
        0, 0, GASTRO_FIX_ONE, 0
    );
    v = gastro_mat4x4_mul_vec4(scale, v);
    v = gastro_mat4x4_mul_vec4(rot, v);
    v = gastro_mat4x4_mul_vec4(trans, v);
    v = gastro_mat4x4_mul_vec4(proj, v);
    off += 1;
    off %= 256 * 64;
    return v;
}

static gastro_color fragment_shader(gastro_ctx *ctx, gastro_vec3 in, gastro_fix64 *attrs, i64 attrs_len) {
    return gastro_color_new(attrs[0] >> GASTRO_FIXED_POINT, attrs[1] >> GASTRO_FIXED_POINT, attrs[2] >> GASTRO_FIXED_POINT, 0xff);
}

int main(int argc, char **argv) {
    gastro_ctx ctx;
    u32 width = 256, height = 256;
    gastro_color *pixels = calloc(width * height, sizeof(gastro_color));
    gastro_fix64 *depth = calloc(width * height, sizeof(gastro_fix64));
    gastro_program p;
    gastro_ctx_init(&ctx, width, height, pixels, depth);
    p.vertex = vertex_shader;
    p.fragment = fragment_shader;

    InitWindow(800, 600, "test");
    {
        Image im;
        Texture2D tex;
        im.data = ctx.pixels;
        im.width = (int) ctx.width;
        im.height = (int) ctx.height;
        im.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        im.mipmaps = 1;
        tex = LoadTextureFromImage(im);
        SetTargetFPS(60);
        while (!WindowShouldClose()) {
            Vector2 pos;
            pos.x = 10; pos.y = 10;
            gastro_draw_clear(&ctx, gastro_color_new(0, 0, 0, 0xff));
            draw_cube(&ctx, &p, gastro_color_new(0xff, 0, 0, 0xff));
            BeginDrawing();
            ClearBackground(RED);
            UpdateTexture(tex, ctx.pixels);
            DrawTextureEx(tex, pos, 0.0, 4.0, WHITE);
            DrawFPS(10, 10);
            EndDrawing();
        }
    }
    CloseWindow();
    (void) argc; (void) argv;
    return 0;
}
