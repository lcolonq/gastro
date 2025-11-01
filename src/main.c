#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <raylib.h>

#include <qoi.h>

#include "gastro.h"

#include "../converted_model.h"

#define GF gastro_fix_from_double

static i64 off = 0;

qoi_color *TEXTURE;
i64 TEXTURE_WIDTH;
i64 TEXTURE_HEIGHT;

#define QUADIDX(p0, p1, p2, p3) p0, p1, p2, p2, p3, p0
#define COLOR(r, g, b) gastro_fix_new(r), gastro_fix_new(g), gastro_fix_new(b)
static void draw_quad(gastro_ctx *ctx, gastro_program *p) {
    gastro_fix64 sz = GASTRO_FIX_HALF;
    gastro_fix64 verts[] = {
        /* front */
        -sz, -sz, -sz, 0, 0,
        +sz, -sz, -sz, GASTRO_FIX_ONE, 0,
        +sz, +sz, -sz, GASTRO_FIX_ONE, GASTRO_FIX_ONE,
        -sz, +sz, -sz, 0, GASTRO_FIX_ONE
    };
    i64 idxs[] = {
        QUADIDX(0, 1, 2, 3), /* front */
    };
    gastro_render_triangles(ctx, p, verts, 5, idxs, 2);
}
static void draw_cube(gastro_ctx *ctx, gastro_program *p) {
    gastro_fix64 sz = GASTRO_FIX_HALF;
    gastro_fix64 verts[] = {
        /* front */
        -sz, -sz, -sz, 0, 0,
        +sz, -sz, -sz, GASTRO_FIX_ONE, 0,
        +sz, +sz, -sz, GASTRO_FIX_ONE, GASTRO_FIX_ONE,
        -sz, +sz, -sz, 0, GASTRO_FIX_ONE,
        /* right */
        +sz, -sz, -sz, 0, 0,
        +sz, -sz, +sz, GASTRO_FIX_ONE, 0,
        +sz, +sz, +sz, GASTRO_FIX_ONE, GASTRO_FIX_ONE,
        +sz, +sz, -sz, 0, GASTRO_FIX_ONE,
        /* left */
        -sz, -sz, +sz, 0, 0,
        -sz, -sz, -sz, GASTRO_FIX_ONE, 0,
        -sz, +sz, -sz, GASTRO_FIX_ONE, GASTRO_FIX_ONE,
        -sz, +sz, +sz, 0, GASTRO_FIX_ONE,
        /* top */
        -sz, -sz, +sz, 0, 0,
        +sz, -sz, +sz, GASTRO_FIX_ONE, 0,
        +sz, -sz, -sz, GASTRO_FIX_ONE, GASTRO_FIX_ONE,
        -sz, -sz, -sz, 0, GASTRO_FIX_ONE,
        /* bottom */
        -sz, +sz, -sz, 0, 0,
        +sz, +sz, -sz, GASTRO_FIX_ONE, 0,
        +sz, +sz, +sz, GASTRO_FIX_ONE, GASTRO_FIX_ONE,
        -sz, +sz, +sz, 0, GASTRO_FIX_ONE,
        /* back */
        +sz, -sz, +sz, 0, 0,
        -sz, -sz, +sz, GASTRO_FIX_ONE, 0,
        -sz, +sz, +sz, GASTRO_FIX_ONE, GASTRO_FIX_ONE,
        +sz, +sz, +sz, 0, GASTRO_FIX_ONE
    };
    i64 idxs[] = {
        QUADIDX(0, 1, 2, 3), /* front */
        QUADIDX(4, 5, 6, 7), /* right */
        QUADIDX(8, 9, 10, 11), /* left */
        QUADIDX(12, 13, 14, 15), /* top */
        QUADIDX(16, 17, 18, 19), /* bottom */
        QUADIDX(20, 21, 22, 23) /* back */
    };
    gastro_render_triangles(ctx, p, verts, 5, idxs, 12);
}

static gastro_vec4 vertex_shader(gastro_ctx *ctx, gastro_vec3 in, gastro_fix64 *attrs, i64 attrs_len) {
    gastro_fix64 x = gastro_fix_from_double((double) off / 1000.0);
    gastro_vec4 v = gastro_vec4_new(in.x, in.y, in.z, GASTRO_FIX_ONE);
    gastro_mat4x4 scale = gastro_mat4x4_new(
        gastro_fix_from_double(0.05), 0, 0, 0,
        0, gastro_fix_from_double(-0.05), 0, 0,
        0, 0, gastro_fix_from_double(0.05), 0,
        0, 0, 0, GASTRO_FIX_ONE
    );
    u8 theta = (u8) (off / 2);
    /* u8 theta = 13; */
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
    return v;
}

static gastro_color fragment_shader_texture(gastro_ctx *ctx, gastro_vec3 in, gastro_fix64 *attrs, i64 attrs_len) {
    gastro_fix64 u = attrs[0];
    gastro_fix64 v = attrs[1];
    i64 x = gastro_fix_to_i64(gastro_fix_mul(gastro_fix_new(TEXTURE_WIDTH), u));
    i64 y = gastro_fix_to_i64(gastro_fix_mul(gastro_fix_new(TEXTURE_HEIGHT), v));
    qoi_color c = TEXTURE[y * TEXTURE_WIDTH + x];
    return gastro_color_new(c.r, c.g, c.b, c.a);
}

static gastro_color fragment_shader_teapot(gastro_ctx *ctx, gastro_vec3 in, gastro_fix64 *attrs, i64 attrs_len) {
    u8 col = (u8) gastro_fix_to_i64(
        gastro_fix_mul(
            gastro_fix_new(0xff),
            gastro_fix_div(in.z, gastro_fix_new(8))
        )
    );
    return gastro_color_new(0xff - col, 0xff - col, 0xff - col, 0xff);
}

void load_image(char *path) {
    FILE *f;
    u8 *buf;
    i64 len;
    qoi_decoder q;
    qoi_header h;
    qoi_color *pixels;
    if (!(f = fopen(path, "r"))) {
        fprintf(stderr, "failed to open file: %s\n", path);
        return;
    }
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = calloc((size_t) len, sizeof(u8));
    fread(buf, sizeof(u8), (size_t) len, f);
    qoi_decoder_new(&q, buf, len);
    if (!qoi_decode_header(&q, &h)) {
        fprintf(stderr, "failed to decode header\n");
        return;
    }
    pixels = calloc(h.width * h.height, sizeof(qoi_color));
    qoi_decode(&q, &h, pixels);
    TEXTURE = pixels;
    TEXTURE_WIDTH = h.width;
    TEXTURE_HEIGHT = h.height;
}

int main(int argc, char **argv) {
    gastro_ctx ctx;
    u32 width = 256, height = 256;
    gastro_color *pixels = calloc(width * height, sizeof(gastro_color));
    gastro_fix64 *depth = calloc(width * height, sizeof(gastro_fix64));
    gastro_program p;
    gastro_ctx_init(&ctx, width, height, pixels, depth);
    p.vertex = vertex_shader;
    p.fragment = fragment_shader_teapot;

    load_image("sphaero.qoi");

    gastro_draw_clear(&ctx, gastro_color_new(0, 0, 0, 0xff));
    draw_model(&ctx, &p);

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
        SetTargetFPS(300);
        while (!WindowShouldClose()) {
            i64 i;
            Vector2 pos;
            pos.x = 10; pos.y = 10;
            /* gastro_draw_clear(&ctx, gastro_color_new(0, 0, 0, 0xff)); */
            /* draw_cube(&ctx, &p); */
            /* draw_quad(&ctx, &p); */
            /* draw_model(&ctx, &p); */
            off += 1;
            off %= 256 * 64;
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
