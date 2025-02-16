//------------------------------------------------------------------------------
//  sgl-sapp.c
//  Rendering via sokol_gl.h
//------------------------------------------------------------------------------
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#define SOKOL_GL_IMPL
#include "sokol_gl.h"

static struct {
    sg_pass_action pass_action;
    sg_image img;
    sg_sampler smp;
    sgl_pipeline pip_3d;
} state;

static void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    // setup sokol-gl
    sgl_setup(&(sgl_desc_t){
        .logger.func = slog_func,
    });

    // a checkerboard texture
    uint32_t pixels[8][8];
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            pixels[y][x] = ((y ^ x) & 1) ? 0xFFFFFFFF : 0xFF000000;
        }
    }
    state.img = sg_make_image(&(sg_image_desc){
        .width = 8,
        .height = 8,
        .data.subimage[0][0] = SG_RANGE(pixels)
    });

    // ... and a sampler
    state.smp = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
    });

    /* create a pipeline object for 3d rendering, with less-equal
       depth-test and cull-face enabled, note that we don't provide
       a shader, vertex-layout, pixel formats and sample count here,
       these are all filled in by sokol-gl
    */
    state.pip_3d = sgl_make_pipeline(&(sg_pipeline_desc){
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            .write_enabled = true,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
        },
    });

    // default pass action
    state.pass_action = (sg_pass_action) {
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f }
        }
    };
}

static void draw_triangle(void) {
    sgl_defaults();
    sgl_begin_triangles();
    sgl_v2f_c3b( 0.0f,  0.5f, 255, 0, 0);
    sgl_v2f_c3b(-0.5f, -0.5f, 0, 0, 255);
    sgl_v2f_c3b( 0.5f, -0.5f, 0, 255, 0);
    sgl_end();
}

static void draw_quad(float t) {
    static float angle_deg = 0.0f;
    float scale = 1.0f + sinf(sgl_rad(angle_deg)) * 0.5f;
    angle_deg += 0.0f * t;
    sgl_defaults();

    sgl_load_pipeline(state.pip_3d);

    sgl_enable_texture();
    sgl_texture(state.img, state.smp);

    sgl_rotate(sgl_rad(angle_deg), 0.0f, 0.0f, 1.0f);
    sgl_scale(scale, scale, 1.0f);
    sgl_begin_quads();
    sgl_v2f_c3b( -0.5f, -0.5f,  255, 255, 0);
    sgl_v2f_c3b(  0.5f, -0.5f,  0, 255, 0);
    sgl_v2f_c3b(  0.5f,  0.5f,  0, 0, 255);
    sgl_v2f_c3b( -0.5f,  0.5f,  255, 0, 0);
    sgl_end();
}

static void draw_tex_cube(const float t) {
    static float frame_count = 0.0f;
    frame_count += 1.0f * t;
    float a = sgl_rad(frame_count);

    // texture matrix rotation and scale
    float tex_rot = 0.5f * a;
    const float tex_scale = 1.0f + sinf(a) * 0.5f;

    // compute an orbiting eye-position for testing sgl_lookat()
    float eye_x = sinf(a) * 6.0f;
    float eye_z = cosf(a) * 6.0f;
    float eye_y = sinf(a) * 3.0f;

    sgl_defaults();
    sgl_load_pipeline(state.pip_3d);

    sgl_enable_texture();
    sgl_texture(state.img, state.smp);

    sgl_matrix_mode_projection();
    sgl_perspective(sgl_rad(45.0f), 1.0f, 0.1f, 100.0f);
    sgl_matrix_mode_modelview();
    sgl_lookat(eye_x, eye_y, eye_z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    sgl_matrix_mode_texture();
    sgl_rotate(tex_rot, 0.0f, 0.0f, 1.0f);
    sgl_scale(tex_scale, tex_scale, 1.0f);
    // cube();
}

static void frame(void) {
    // frame time multiplier (normalized for 60fps)
    const float t = (float)(sapp_frame_duration() * 60.0);

    /* compute viewport rectangles so that the views are horizontally
       centered and keep a 1:1 aspect ratio
    */
    const int dw = sapp_width();
    const int dh = sapp_height();
    const int ww = dh/2; // not a bug
    const int hh = dh/2;
    const int x0 = dw/2 - hh;
    const int x1 = dw/2;
    const int y0 = 0;
    const int y1 = dh/2;
    // all sokol-gl functions except sgl_draw() can be called anywhere in the frame
    // sgl_viewport(x0, y0, ww, hh, true);
    // draw_triangle();
    sgl_viewport(0, 0, ww, hh, true);
    draw_quad(t);
    // sgl_viewport(x0, y1, ww, hh, true);
    // draw_cubes(t);
    // sgl_viewport(x1, y1, ww, hh, true);
    // draw_tex_cube(t);
    // sgl_viewport(0, 0, dw, dh, true);

    /* Render the sokol-gfx default pass, all sokol-gl commands
       that happened so far are rendered inside sgl_draw(), and this
       is the only sokol-gl function that must be called inside
       a sokol-gfx begin/end pass pair.
       sgl_draw() also 'rewinds' sokol-gl for the next frame.
    */
    sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = sglue_swapchain() });
    sgl_draw();
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    sgl_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = 512,
        .height = 512,
        .sample_count = 4,
        .window_title = "sokol_gl.h (sokol-app)",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}
