#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <raylib.h>

#include "gastro.h"

int main(int argc, char **argv) {
    gastro_ctx ctx;
    u32 width = 256, height = 256;
    gastro_color *pixels = calloc(width * height, sizeof(gastro_color));
    gastro_fix64 *depth = calloc(width * height, sizeof(gastro_fix64));
    gastro_ctx_init(&ctx, width, height, pixels, depth);

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
        while (!WindowShouldClose()) {
            Vector2 pos;
            pos.x = 10; pos.y = 10;
            gastro_draw_clear(&ctx, gastro_color_new(0, 0, 0, 0xff));
            /* gastro_draw_quad(&ctx, gastro_color_new(0, 0xff, 0, 0xff), */
            /*     gastro_vec3_new(gastro_fix_new(1) + GASTRO_FIX_HALF, gastro_fix_new(1) + GASTRO_FIX_HALF, gastro_fix_new(10)), */
            /*     gastro_vec3_new(gastro_fix_new(100) + GASTRO_FIX_HALF, gastro_fix_new(30) + GASTRO_FIX_HALF, GASTRO_FIX_ONE), */
            /*     gastro_vec3_new(gastro_fix_new(70) + GASTRO_FIX_HALF, gastro_fix_new(80) + GASTRO_FIX_HALF, GASTRO_FIX_ONE), */
            /*     gastro_vec3_new(gastro_fix_new(10) + GASTRO_FIX_HALF, gastro_fix_new(80) + GASTRO_FIX_HALF, GASTRO_FIX_ONE) */
            /* ); */
            gastro_draw_cube(&ctx, gastro_vec3_new(gastro_fix_new(50), gastro_fix_new(50), gastro_fix_new(50)));
            BeginDrawing();
            ClearBackground(RED);
            UpdateTexture(tex, ctx.pixels);
            DrawTextureEx(tex, pos, 0.0, 8.0, WHITE);
            EndDrawing();
        }
    }
    CloseWindow();
    (void) argc; (void) argv;
    return 0;
}
