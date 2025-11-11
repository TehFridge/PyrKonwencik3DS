#include <3ds.h>
#include <citro2d.h>
#include <math.h>
#include <string.h> 
#include "scene_pass.h"
#include "scene_manager.h"
#include "buttons.h"
#include "cwav_shit.h"
#include "global_draws.h"
#include "sprites.h"
#include "quirc.h"
#include "logs.h"

#define CAM_WIDTH  400
#define CAM_HEIGHT 240
#define CAM_BUFSZ  (CAM_WIDTH * CAM_HEIGHT * 2)
#define WAIT_TIMEOUT 1000000000ULL

extern Button buttonsy[2000];

static C3D_Tex cam_tex;
static C2D_Image cam_image;

static void* cam_tex_buf = NULL; 


static Tex3DS_SubTexture cam_subtex_data;

static struct quirc *qr = NULL;
static uint8_t *graybuf = NULL;

static uint8_t *camBuffer = NULL;      
static Handle camEvent[4] = {0};
static bool captureInterrupted = false;
static s32 eventIndex = 0;

static inline void rgb565_to_gray(
    const uint8_t *src,
    uint8_t *dst,
    int width,
    int height
) {
    const uint16_t *pix = (const uint16_t*)src;
    for (int i = 0; i < width * height; i++) {
        uint16_t p = pix[i];
        uint8_t r = ((p >> 11) & 0x1F) << 3;
        uint8_t g = ((p >> 5) & 0x3F) << 2;
        uint8_t b = (p & 0x1F) << 3;
        dst[i] = (uint8_t)(0.299f*r + 0.587f*g + 0.114f*b);
    }
}

void scenePassInit(void) {
    init_logger();

        log_to_file("Allocating VRAM texture buffer");
    
    cam_tex_buf = linearAlloc(CAM_BUFSZ); 
    if (!cam_tex_buf) svcBreak(USERBREAK_PANIC);

    
    C3D_TexInit(&cam_tex, CAM_WIDTH, CAM_HEIGHT, GPU_RGB565);
    C3D_TexSetFilter(&cam_tex, GPU_LINEAR, GPU_LINEAR); 

    
    cam_image.tex = &cam_tex;
    
    
    cam_subtex_data.width = CAM_WIDTH;
    cam_subtex_data.height = CAM_HEIGHT;
    cam_subtex_data.left = 0.0f;
    cam_subtex_data.top = 0.0f;
    cam_subtex_data.right = 1.0f;
    cam_subtex_data.bottom = 1.0f;
    
    
    cam_image.subtex = &cam_subtex_data;
    
    C3D_TexUpload(&cam_tex, cam_tex_buf); 

        log_to_file("Init camera buffers");
    camBuffer = malloc(CAM_BUFSZ * 2); 
    if (!camBuffer) svcBreak(USERBREAK_PANIC);

        log_to_file("Init quirc");
    qr = quirc_new();
    if (!qr) svcBreak(USERBREAK_PANIC);
    if (quirc_resize(qr, CAM_WIDTH, CAM_HEIGHT) < 0)
        svcBreak(USERBREAK_PANIC);

    graybuf = malloc(CAM_WIDTH * CAM_HEIGHT);
    if (!graybuf) svcBreak(USERBREAK_PANIC);

        log_to_file("/-- Init camera --/");
    camInit();

    log_to_file("/-- CAMU_SetSize --/");
    CAMU_SetSize(SELECT_OUT1_OUT2, SIZE_CTR_TOP_LCD, CONTEXT_A);
    log_to_file("/-- CAMU_SetOutputFormat --/");
    CAMU_SetOutputFormat(SELECT_OUT1_OUT2, OUTPUT_RGB_565, CONTEXT_A);
    log_to_file("/-- CAMU_SetFPS --/");
    CAMU_SetFrameRate(SELECT_OUT1_OUT2, FRAME_RATE_30);

    log_to_file("/-- CAMU_SetNoiseFilter --/");
    CAMU_SetNoiseFilter(SELECT_OUT1_OUT2, true);
    log_to_file("/-- CAMU_SetAutoExp --/");
    CAMU_SetAutoExposure(SELECT_OUT1_OUT2, true);
    log_to_file("/-- CAMU_SetAutoWhiteBalance --/");
    CAMU_SetAutoWhiteBalance(SELECT_OUT1_OUT2, true);

    log_to_file("/-- CAMU_SetTrim --/");
    CAMU_SetTrimming(PORT_CAM1, false);
    CAMU_SetTrimming(PORT_CAM2, false);

    u32 maxBytes;
    log_to_file("/-- CAMU_GetMaxBytes --/");
    CAMU_GetMaxBytes(&maxBytes, CAM_WIDTH, CAM_HEIGHT);
    log_to_file("/-- CAMU_SetTransferBytes --/");
    CAMU_SetTransferBytes(PORT_BOTH, maxBytes, CAM_WIDTH, CAM_HEIGHT);

    log_to_file("/-- CAMU_Activate --/");
    CAMU_Activate(SELECT_OUT1_OUT2);

    log_to_file("/-- CAMU_GetBufferErrorInterrutEvent --/");
    CAMU_GetBufferErrorInterruptEvent(&camEvent[0], PORT_CAM1);
    CAMU_GetBufferErrorInterruptEvent(&camEvent[1], PORT_CAM2);

    log_to_file("/-- CAMU_ClearBuffer --/");
    CAMU_ClearBuffer(PORT_BOTH);
    log_to_file("/-- CAMU_VSync --/");
    CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2);
    log_to_file("/-- CAMU_StartCapture --/");
    CAMU_StartCapture(PORT_BOTH);
}


void scenePassUpdate(uint32_t kDown, uint32_t kHeld) {

        if (camEvent[2] == 0)
        CAMU_SetReceiving(&camEvent[2], camBuffer, PORT_CAM1,
                          CAM_BUFSZ, (s16)CAM_BUFSZ);

    if (camEvent[3] == 0)
        CAMU_SetReceiving(&camEvent[3], camBuffer + CAM_BUFSZ,
                          PORT_CAM2, CAM_BUFSZ, (s16)CAM_BUFSZ);

        if (captureInterrupted) {
        CAMU_StartCapture(PORT_BOTH);
        captureInterrupted = false;
    }

        Result res = svcWaitSynchronizationN(&eventIndex, camEvent, 4,
                                        false, WAIT_TIMEOUT);
    
    
    if (R_FAILED(res) || eventIndex < 0) {
        return; 
    }

    switch (eventIndex) {

                case 0:
            log_to_file("/-- CAM1 error --/");
            svcCloseHandle(camEvent[2]);
            camEvent[2] = 0;
            captureInterrupted = true;
            return;

                case 1:
            log_to_file("/-- CAM2 error --/");
            svcCloseHandle(camEvent[3]);
            camEvent[3] = 0;
            captureInterrupted = true;
            return;

                case 2:
            log_to_file("/-- CAM1 OK --/");
            svcCloseHandle(camEvent[2]);
            camEvent[2] = 0;
            
            

                        rgb565_to_gray(camBuffer, graybuf, CAM_WIDTH, CAM_HEIGHT);

                        int w, h;
            uint8_t *qbuf = quirc_begin(qr, &w, &h);
            memcpy(qbuf, graybuf, w*h);
            quirc_end(qr);

            int count = quirc_count(qr);
            for (int i = 0; i < count; i++) {
                struct quirc_code code;
                struct quirc_data data;

                quirc_extract(qr, i, &code);
                if (!quirc_decode(&code, &data)) {
                    
                    printf("QR: %.*s\n", data.payload_len, data.payload);
                }
            }

                        
            memcpy(cam_tex_buf, camBuffer, CAM_BUFSZ);

            
            GSPGPU_FlushDataCache(cam_tex_buf, CAM_BUFSZ);

            
            C3D_TexUpload(&cam_tex, cam_tex_buf);
            
            break;

                case 3:
            log_to_file("/-- CAM2 OK --/");
            svcCloseHandle(camEvent[3]);
            camEvent[3] = 0;
            
            break; 
    }
}


void scenePassRender(void) {
    
    
    

    C2D_DrawParams params = {0}; 

    
    params.pos.x = 0.0f;
    params.pos.y = 0.0f;

    
    
    C2D_DrawImage(cam_image, &params, NULL); 
    
    }


void scenePassExit(void) {

    CAMU_StopCapture(PORT_BOTH);
    CAMU_Activate(SELECT_NONE);

    for (int i = 0; i < 4; i++)
        if (camEvent[i]) svcCloseHandle(camEvent[i]);

    camExit();

    if (graybuf) free(graybuf);
    if (camBuffer) free(camBuffer);
    if (qr) quirc_destroy(qr);

    
    C3D_TexDelete(&cam_tex);
    if (cam_tex_buf) linearFree(cam_tex_buf);
    cam_tex_buf = NULL;
}