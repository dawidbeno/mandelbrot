
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

#include "mandel.h"
#include <xmmintrin.h>
#include <immintrin.h>

void mandel_basic(unsigned char *image, const struct spec *s){
    float xdiff = s->xlim[1] - s->xlim[0];
    float ydiff = s->ylim[1] - s->ylim[0];
    float iter_scale = 1.0f / s->iterations;
    float depth_scale = s->depth - 1;
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x++) {
            float cr = x * xdiff / s->width  + s->xlim[0];
            float ci = y * ydiff / s->height + s->ylim[0];
            float zr = cr;
            float zi = ci;
            int k = 0;
            float mk = 0.0f;
            while (++k < s->iterations) {
                float zr1 = zr * zr - zi * zi + cr;
                float zi1 = zr * zi + zr * zi + ci;
                zr = zr1;
                zi = zi1;
                mk += 1.0f;
                if (zr * zr + zi * zi >= 4.0f)
                    break;
            }
            mk *= iter_scale;
            mk = sqrtf(mk);
            mk *= depth_scale;
            int pixel = mk;
            image[y * s->width * 3 + x * 3 + 0] = pixel;
            image[y * s->width * 3 + x * 3 + 1] = pixel;
            image[y * s->width * 3 + x * 3 + 2] = pixel;
            
            
        }
    }
}

void mandel_sse2(unsigned char *image, const struct spec *s){
    __m128 xmin = _mm_set_ps1(s->xlim[0]);
    __m128 ymin = _mm_set_ps1(s->ylim[0]);
    __m128 xscale = _mm_set_ps1((s->xlim[1] - s->xlim[0]) / s->width);
    __m128 yscale = _mm_set_ps1((s->ylim[1] - s->ylim[0]) / s->height);
    __m128 threshold = _mm_set_ps1(4);
    __m128 one = _mm_set_ps1(1);
    __m128 iter_scale = _mm_set_ps1(1.0f / s->iterations);
    __m128 depth_scale = _mm_set_ps1(s->depth - 1);
    
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x += 4) {
            __m128 mx = _mm_set_ps(x + 3, x + 2, x + 1, x + 0);
            __m128 my = _mm_set_ps1(y);
            __m128 cr = _mm_add_ps(_mm_mul_ps(mx, xscale), xmin);
            __m128 ci = _mm_add_ps(_mm_mul_ps(my, yscale), ymin);
            __m128 zr = cr;
            __m128 zi = ci;
            int k = 1;
            __m128 mk = _mm_set_ps1(k);
            while (++k < s->iterations) {
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
            mk = _mm_mul_ps(mk, iter_scale);
            mk = _mm_sqrt_ps(mk);
            mk = _mm_mul_ps(mk, depth_scale);
            __m128i pixels = _mm_cvtps_epi32(mk);
            unsigned char *dst = image + y * s->width * 3 + x * 3;
            unsigned char *src = (unsigned char *)&pixels;
            for (int i = 0; i < 4; i++) {
                dst[i * 3 + 0] = src[i * 4];
                dst[i * 3 + 1] = src[i * 4];
                dst[i * 3 + 2] = src[i * 4];
            }
        }
    }
}

void mandel_avx(unsigned char *image, const struct spec *s){
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
            int k = 1;
            __m256 mk = _mm256_set1_ps(k);
            while (++k < s->iterations) {
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
            mk = _mm256_mul_ps(mk, iter_scale);
            mk = _mm256_sqrt_ps(mk);
            mk = _mm256_mul_ps(mk, depth_scale);
            __m256i pixels = _mm256_cvtps_epi32(mk);
            unsigned char *dst = image + y * s->width * 3 + x * 3;
            unsigned char *src = (unsigned char *)&pixels;
            for (int i = 0; i < 8; i++) {
                dst[i * 3 + 0] = src[i * 4];
                dst[i * 3 + 1] = src[i * 4];
                dst[i * 3 + 2] = src[i * 4];
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

void printImageMeta(const struct spec *s){
    printf("Image metadata:\n");
    printf("Width: %d\n", s->width);
    printf("Height: %d\n", s->height);
}

int main(int argc, const char * argv[]) {
    
    /* Config */
    struct spec spec = {
        .width = 1024,
        .height = 1024,
        .depth = 256,
        .xlim = {-2.5, 1.5},
        .ylim = {-1.5, 1.5},
        .iterations = 256
    };
    
    int use_avx = 0;
    int use_sse = 0;
    int use_basic = 0;
    int test = 0;
    
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
        if(!strcmp(argv[option], "-B")){
            use_basic = 1;
            option++;
            continue;
        }
        if(!strcmp(argv[option], "-test")){
            test = 1;
            option++;
            continue;
        }
        if(!strcmp(argv[option], "-xy")){
            option++;
            spec.width = atoi(argv[option]);
            
            option++;
            spec.height = atoi(argv[option]);
            
            option++;
            continue;
        }
    }
    
    /* TEST */
    if(test){
        use_basic = 1;
        use_sse = 1;
        use_avx = 1;
        printf("Obrazok %dx%d px\n", spec.width, spec.height);
    }
    
    if(!use_basic && !use_sse && !use_avx && !test){
        printf("Zadaj moznosti: -A -S -B -test\n");
        return 0;
    }
    
    /* AVX */
    if(use_avx){
        unsigned char *image_avx = (unsigned char*)malloc(spec.width * spec.height * 3);
        clock_t begin_avx=clock();
        mandel_avx(image_avx, &spec);
        clock_t end_avx=clock();
        
        printImageMeta(&spec);
        printf("AVX time: %lf ms\n", diffclock(end_avx, begin_avx));
        
        FILE *fp_avx = fopen("fractal_AVX.png", "wb");
        fprintf(fp_avx, "P6\n%d %d\n%d\n", spec.width, spec.height, spec.depth - 1);
        fwrite(image_avx, spec.width * spec.height, 3, fp_avx);
        free(image_avx);
        fclose(fp_avx);
    }
    
    
    
    /* SSE */
    if(use_sse){
        unsigned char *image_sse = (unsigned char*)malloc(spec.width * spec.height * 3);
        clock_t begin_sse=clock();
        mandel_sse2(image_sse, &spec);
        clock_t end_sse=clock();
        
        printImageMeta(&spec);
        printf("SSE time: %lf ms\n", diffclock(end_sse, begin_sse));
        
        FILE *fp_sse = fopen("fractal_SSE.png", "wb");
        fprintf(fp_sse, "P6\n%d %d\n%d\n", spec.width, spec.height, spec.depth - 1);
        fwrite(image_sse, spec.width * spec.height, 3, fp_sse);
        free(image_sse);
        fclose(fp_sse);
    }
    
    
    /* Basic */
    if(use_basic){
        unsigned char *image = (unsigned char*)malloc(spec.width * spec.height * 3);
        clock_t begin=clock();
        mandel_basic(image, &spec);
        clock_t end=clock();
        
        printImageMeta(&spec);
        printf("Basic time: %lf ms\n", diffclock(end,begin));
        
        FILE *fp = fopen("fractal_Basic.png", "wb");
        fprintf(fp, "P6\n%d %d\n%d\n", spec.width, spec.height, spec.depth - 1);
        fwrite(image, spec.width * spec.height, 3, fp);
        free(image);
        fclose(fp);
    }
    
    return 0;
}