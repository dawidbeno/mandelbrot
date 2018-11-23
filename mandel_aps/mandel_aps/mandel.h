//
//  mandel.h
//  mandel_aps
//
//  Created by Dávid Beňo on 21/11/2018.
//  Copyright © 2018 Dávid Beňo. All rights reserved.
//

#ifndef mandel_h
#define mandel_h
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


#endif /* mandel_h */
