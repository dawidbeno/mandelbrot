#pragma once

typedef struct imgspec {
    /* Image Specification */
    int width;
    int height;
    int depth;
    /* Fractal Specification */
    float xlim[2];
    float ylim[2];
    int iterations;
} IMGSPEC;

/* KONSTANTY */
#define TEST_LENGHT 10
#define IMG_PATH_AVX "/Users/dejvid/Codes/SKOLA/APS/mandelbrot_zadanie/fractal_AVX.png"
#define IMG_PATH_SSE "/Users/dejvid/Codes/SKOLA/APS/mandelbrot_zadanie/fractal_SSE.png"
#define IMG_PATH_BASIC "/Users/dejvid/Codes/SKOLA/APS/mandelbrot_zadanie/fractal_Basic.png"

