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
#define IMG_PATH_AVX "fractal_AVX.png"
#define IMG_PATH_SSE "fractal_SSE.png"
#define IMG_PATH_BASIC "fractal_Basic.png"


#define TSIZE1 500
#define TSIZE2 1000
#define TSIZE3 2000
#define TSIZE4 3000
#define TSIZE5 4000

#define DEPTH 256
#define ITERATIONS 256
