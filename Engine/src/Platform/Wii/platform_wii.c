#include "raylib.h"
#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>

#define GX_FIFO_SIZE (256 * 1024)

static void* xfb = NULL;
static GXRModeObj* rmode = NULL;
static bool s_shouldClose = false;

void SYS_Report(const char* fmt, ...) { (void)fmt; }

int InitPlatform(void) {
    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    void* gp_fifo = memalign(32, GX_FIFO_SIZE);
    memset(gp_fifo, 0, GX_FIFO_SIZE);
    GX_Init(gp_fifo, GX_FIFO_SIZE);

    GXColor bg = { 0, 0, 0, 255 };
    GX_SetCopyClear(bg, GX_MAX_Z24);
    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    f32 yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    u32 xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
    GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering,
        ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
    GX_SetCullMode(GX_CULL_NONE);
    GX_CopyDisp(xfb, GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);

    extern void ogx_initialize(void);
    ogx_initialize();
    return 0;
}

void ClosePlatform(void) {}
void PollInputEvents(void) {}

void SwapScreenBuffer(void) {
    GX_CopyDisp(xfb, GX_FALSE);
    GX_DrawDone();
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_Flush();
    VIDEO_WaitVSync();
}

double GetTime(void) {
    return (double)ticks_to_microsecs(gettime()) / 1000000.0;
}

bool WindowShouldClose(void) { return s_shouldClose; }
Vector2 GetWindowScaleDPI(void) { return (Vector2) { 1.0f, 1.0f }; }
void MaximizeWindow(void) {}
void MinimizeWindow(void) {}
void SetWindowSize(int w, int h) { (void)w; (void)h; }

// glGetTexImage is not supported on GX hardware
void glGetTexImage(unsigned int target, int level, unsigned int format,
    unsigned int type, void* pixels) {
    (void)target; (void)level; (void)format; (void)type; (void)pixels;
}