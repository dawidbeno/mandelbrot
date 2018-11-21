//  Created by Dávid Beňo
//  Copyright © 2018 Dávid Beňo. All rights reserved.
//
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <complex.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <getopt.h>



using namespace std;

struct spec {
    /* Image Specification */
    int width;
    int height;
    int depth;
    /* Fractal Specification */
    float xlim[2];
    float ylim[2];
    int iterations;
};

typedef struct {
    int r;
    int g;
    int b;
} Pixel;


void mandel_basic(Pixel **img, const struct spec *s)
{
    float xdiff = s->xlim[1] - s->xlim[0];
    float ydiff = s->ylim[1] - s->ylim[0];
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x++) {
            float cr = x * xdiff / s->width  + s->xlim[0];
            float ci = y * ydiff / s->height + s->ylim[0];
            float zr = 0.0;
            float zi = 0.0;
            int count = 0;
            float mk = 0.0f;
            while (++count < s->iterations) {
                float zr1 = zr * zr - zi * zi + cr;
                float zi1 = zr * zi + zr * zi + ci;
                zr = zr1;
                zi = zi1;
                mk += 1.0f;
                if (zr * zr + zi * zi >= 4.0f)
                    break;
            }
            Pixel pixel;
            if(count == s->iterations){ // BLACK
                pixel.r = 0;
                pixel.g = 0;
                pixel.b = 0;
            }else{
                // COLOR
                double t = (double)count / (double)s->iterations;
                
                pixel.r = (int) (9 * (1 - t) * t * t * t * 255);
                pixel.g = (int) (15 * (1 - t) * (1 - t) * t * t * 255);;
                pixel.b = (int) (8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
            }
            img[y][x] = pixel;
            
        }
    }
}


void mandel_sse2(Pixel **img, const struct spec *s)
{
    __m128 xmin = _mm_set_ps1(s->xlim[0]);
    __m128 ymin = _mm_set_ps1(s->ylim[0]);
    __m128 xscale = _mm_set_ps1((s->xlim[1] - s->xlim[0]) / s->width);
    __m128 yscale = _mm_set_ps1((s->ylim[1] - s->ylim[0]) / s->height);
    __m128 threshold = _mm_set_ps1(4);
    __m128 one = _mm_set_ps1(1);
    
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x += 4) {
            __m128 mx = _mm_set_ps(x + 3, x + 2, x + 1, x + 0);
            __m128 my = _mm_set_ps1(y);
            __m128 cr = _mm_add_ps(_mm_mul_ps(mx, xscale), xmin);
            __m128 ci = _mm_add_ps(_mm_mul_ps(my, yscale), ymin);
            __m128 zr = cr;
            __m128 zi = ci;
            int count = 1;
            __m128 mk = _mm_set_ps1(count);
            while (++count < s->iterations) {
                /* Compute z1 from z0 */
                __m128 zr2 = _mm_mul_ps(zr, zr);
                __m128 zi2 = _mm_mul_ps(zi, zi);
                __m128 zrzi = _mm_mul_ps(zr, zi);
                
                zr = _mm_add_ps(_mm_sub_ps(zr2, zi2), cr);
                zi = _mm_add_ps(_mm_add_ps(zrzi, zrzi), ci);
                
                zr2 = _mm_mul_ps(zr, zr);
                zi2 = _mm_mul_ps(zi, zi);
                __m128 mag2 = _mm_add_ps(zr2, zi2);
                __m128 mask = _mm_cmplt_ps(mag2, threshold);
                mk = _mm_add_ps(_mm_and_ps(mask, one), mk);
                
                if (_mm_movemask_ps(mask) == 0)
                    break;
            }

            __m128i pixels = _mm_cvtps_epi32(mk);

            for (int i = 0; i < 4; i++) {
                Pixel pixel;
                if(pixels[i] == s->iterations){ // BLACK
                    pixel.r = 0;
                    pixel.g = 0;
                    pixel.b = 0;
                }else{
                    // COLOR
                    double t = (double)count / (double)s->iterations;
                    
                    pixel.r = (int) (9 * (1 - t) * t * t * t * 255);
                    pixel.g = (int) (15 * (1 - t) * (1 - t) * t * t * 255);
                    pixel.b = (int) (8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
                }
                img[y][x+i] = pixel;

            }
        }
    }
}

void mandel_avx(Pixel **img, const struct spec *s)
{
    __m256 xmin = _mm256_set1_ps(s->xlim[0]);
    __m256 ymin = _mm256_set1_ps(s->ylim[0]);
    __m256 xscale = _mm256_set1_ps((s->xlim[1] - s->xlim[0]) / s->width);
    __m256 yscale = _mm256_set1_ps((s->ylim[1] - s->ylim[0]) / s->height);
    __m256 threshold = _mm256_set1_ps(4);
    __m256 one = _mm256_set1_ps(1);
    __m256 iter_scale = _mm256_set1_ps(1.0f / s->iterations);
    __m256 depth_scale = _mm256_set1_ps(s->depth - 1);


    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x += 8) {
            __m256 mx = _mm256_set_ps(x + 7, x + 6, x + 5, x + 4,
                                      x + 3, x + 2, x + 1, x + 0);
            __m256 my = _mm256_set1_ps(y);
            __m256 cr = _mm256_add_ps(_mm256_mul_ps(mx, xscale), xmin);
            __m256 ci = _mm256_add_ps(_mm256_mul_ps(my, yscale), ymin);
            __m256 zr = cr;
            __m256 zi = ci;
            int count = 1;
            __m256 mk = _mm256_set1_ps(count);
            while (++count < s->iterations) {
                /* Compute z1 from z0 */
                __m256 zr2 = _mm256_mul_ps(zr, zr);
                __m256 zi2 = _mm256_mul_ps(zi, zi);
                __m256 zrzi = _mm256_mul_ps(zr, zi);

                zr = _mm256_add_ps(_mm256_sub_ps(zr2, zi2), cr);
                zi = _mm256_add_ps(_mm256_add_ps(zrzi, zrzi), ci);

                zr2 = _mm256_mul_ps(zr, zr);
                zi2 = _mm256_mul_ps(zi, zi);
                __m256 mag2 = _mm256_add_ps(zr2, zi2);
                __m256 mask = _mm256_cmp_ps(mag2, threshold, _CMP_LT_OS);
                mk = _mm256_add_ps(_mm256_and_ps(mask, one), mk);

                if (_mm256_testz_ps(mask, _mm256_set1_ps(-1)))
                    break;
            }
//            mk = _mm256_mul_ps(mk, iter_scale);
//            mk = _mm256_sqrt_ps(mk);
//            mk = _mm256_mul_ps(mk, depth_scale);
            __m256i pixels = _mm256_cvtps_epi32(mk);
//            unsigned char *dst = image + y * s->width * 3 + x * 3;
//            unsigned char *src = (unsigned char *)&pixels;
            for (int i = 0; i < 8; i++) {
                Pixel pixel;
                if(pixels[i] == s->iterations){ // BLACK
                    pixel.r = 0;
                    pixel.g = 0;
                    pixel.b = 0;
                }else{
                    // COLOR
                    double t = (double)count / (double)s->iterations;

                    pixel.r = (int) (9 * (1 - t) * t * t * t * 255);
                    pixel.g = (int) (15 * (1 - t) * (1 - t) * t * t * 255);;
                    pixel.b = (int) (8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
                }
                img[y][x+i] = pixel;

            }
        }
    }
}



double diffclock(double clock1,double clock2)
{
    double diffticks=clock1-clock2;
    double diffms=(diffticks*1000)/CLOCKS_PER_SEC;
    return diffms;
}

#ifdef __x86_64__
#include <cpuid.h>

static inline int
is_avx_supported(void)
{
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    return ecx & bit_AVX ? 1 : 0;
}
#endif // __x86_64__


int main(int argc, char * argv[]) {
    
    // Image specifications
    struct spec img_spec = {
        .width = 3000,
        .height = 3000,
        .depth = 250,
        .xlim = {-2.0, 1.0},
        .ylim = {-1.5, 1.5},
        .iterations = 256
    };
    
    int use_avx = 0;
    int use_sse = 0;
    
    /* Parse Options */
    int option = 1;
    while (option < argc) {
        if(!strcmp(argv[option], "-A")){
            use_avx = 1;
            option++;
            continue;
        }
        if(!strcmp(argv[option], "-S")){
            use_sse = 1;
            option++;
            continue;
        }
        if(!strcmp(argv[option], "-xy")){
            option++;
            img_spec.width = atoi(argv[option]);
            
            option++;
            img_spec.height = atoi(argv[option]);
        
            option++;
            continue;
        }
        if(!strcmp(argv[option], "-fname")){
            
        }
        
    }
    
    
    // Allocate image
    Pixel **image;
    image = (Pixel **)malloc(img_spec.height * sizeof(Pixel *));
    for(int i=0; i<img_spec.height; i++){
        image[i] = (Pixel *)malloc(img_spec.width * sizeof(Pixel));
    }
    
    if(use_avx == 1){
        clock_t begin_avx = clock();
        mandel_avx(image, &img_spec);
        clock_t end_avx = clock();
        printf("AVX time: %lf ms\n", diffclock(end_avx, begin_avx));
        
        // opens file and write header
        FILE *fp = fopen("/Users/dejvid/Codes/fractal_AVX.ppm", "wb");
        fprintf(fp, "P3\n%d %d\n%d\n", img_spec.width, img_spec.height, img_spec.depth - 1);
        
        // write image
        for(int j=0; j<img_spec.height; j++){
            for(int i=0; i<img_spec.width; i++){
                fprintf(fp, "%d %d %d ", image[j][i].r, image[j][i].g, image[j][i].b);
            }
        }
        fclose(fp);
    }
    else if (use_sse == 1){
        
        clock_t begin_sse = clock();
        mandel_sse2(image, &img_spec);
        clock_t end_sse = clock();
        printf("SSE time: %lf ms\n", diffclock(end_sse, begin_sse));
        
        // opens file and write header
        FILE *fp = fopen("/Users/dejvid/Codes/fractal_SSE.ppm", "wb");
        fprintf(fp, "P3\n%d %d\n%d\n", img_spec.width, img_spec.height, img_spec.depth - 1);
        
        // write image
        for(int j=0; j<img_spec.height; j++){
            for(int i=0; i<img_spec.width; i++){
                fprintf(fp, "%d %d %d ", image[j][i].r, image[j][i].g, image[j][i].b);
            }
        }
        fclose(fp);
    }
    else{
        clock_t begin_complex = clock();
        mandel_basic(image, &img_spec);
        clock_t end_complex = clock();
        printf("Basic time: %lf ms\n", diffclock(end_complex, begin_complex));
        
        // opens file and write header
        FILE *fp = fopen("/Users/dejvid/Codes/fractal_Basic.ppm", "wb");
        fprintf(fp, "P3\n%d %d\n%d\n", img_spec.width, img_spec.height, img_spec.depth - 1);
        
        // write image
        for(int j=0; j<img_spec.height; j++){
            for(int i=0; i<img_spec.width; i++){
                fprintf(fp, "%d %d %d ", image[j][i].r, image[j][i].g, image[j][i].b);
            }
        }
        fclose(fp);
    }
    
    
    
    
    

    
    return 0;
}
