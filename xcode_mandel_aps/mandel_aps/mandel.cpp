
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#include "mandel.h"
#include <xmmintrin.h>
#include <immintrin.h>


void mandel_basic(unsigned char *image,  struct imgspec *img){
    float color = 0.0f;
    float cr, ci, zr, zi, zr_pom, zi_pom, z2;
    float pixelmap[2] = {img->xcoord[1] - img->xcoord[0], img->ycoord[1] - img->ycoord[0]};
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            zr = cr = x * pixelmap[0] / img->width  + img->xcoord[0];
            zi = ci = y * pixelmap[1] / img->height + img->ycoord[0];
            color = 0.0f;
            for(int counter=0; counter < ITERATIONS; counter++) {
                color += 1.0f;
                zr_pom = zr * zr - zi * zi + cr;
                zi_pom = zr * zi + zr * zi + ci;
                zr = zr_pom;
                zi = zi_pom;
                z2 = zr * zr + zi * zi;
                if ( z2 >= 4.0f){
                    break;
                }
            }
            color /= ITERATIONS;
            image[y * img->width * 3 + x * 3 + 0] = (int) (9 * (1 - color) * color * color * color * 255);
            image[y * img->width * 3 + x * 3 + 1] = (int) (15 * (1 - color) * (1 - color) * color * color * 255);
            image[y * img->width * 3 + x * 3 + 2] = (int) (8.5 * (1 - color) * (1 - color) * (1 - color) * color * 255);
        }
    }
}


void mandel_sse(unsigned char *image, struct imgspec *s){
    __m128 color, cr, ci, zi, zr;
    
    __m128 xleft = _mm_set_ps1(s->xcoord[0]);
    __m128 ydown = _mm_set_ps1(s->ycoord[0]);
    
    __m128 xdiff = _mm_set_ps1(s->xcoord[1] - s->xcoord[0]);
    __m128 ydiff = _mm_set_ps1(s->ycoord[1] - s->ycoord[0]);
    __m128 width = _mm_set_ps1(s->width);
    __m128 height = _mm_set_ps1(s->height);
    
    __m128 xpixelmap = _mm_div_ps(xdiff, width);
    __m128 ypixelmap = _mm_div_ps(ydiff, height);
    
    __m128 four = _mm_set_ps1(4);
    __m128 one = _mm_set_ps1(1);

    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x += 4) {
            __m128 x_indexes = _mm_set_ps(x + 3, x + 2, x + 1, x + 0); // v cykle skacem o 4, preto tu manualne pracujem so 4 cilami
            __m128 y_indexes = _mm_set_ps1(y);
            cr = zr = _mm_add_ps(_mm_mul_ps(x_indexes, xpixelmap), xleft);
            ci = zi = _mm_add_ps(_mm_mul_ps(y_indexes, ypixelmap), ydown);
            color = _mm_set_ps1(1.0f);
            for(int counter=0; counter < ITERATIONS; counter++) {
                __m128 zrzi = _mm_mul_ps(zr, zi); // zr * zi

                zr = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(zr, zr), _mm_mul_ps(zi, zi)), cr); // zr * zr - zi * zi + cr
                zi = _mm_add_ps(_mm_add_ps(zrzi, zrzi), ci); // 2 * zr * zi + ci;
                
                __m128 z2 = _mm_add_ps(_mm_mul_ps(zr, zr), _mm_mul_ps(zi, zi)); // zr * zr + zi * zi
                
                __m128 mask = _mm_cmplt_ps(z2, four);
                color = _mm_add_ps(_mm_and_ps(mask, one), color);

                if (_mm_movemask_ps(mask) == 0){
                    break;
                }
                
            }
//            __m128 iters = _mm_set_ps1(ITERATIONS);
//            __m128 cols = _mm_div_ps(color, iters);
            
            __m128i pixels = _mm_cvtps_epi32(color); // konverzia float na int
            unsigned char *src = (unsigned char *)&pixels;
            for (int i = 0; i < 4; i++) {
                int srcc = src[i*4];
                image[(y * s->width * 3 + x * 3) + (i * 3) + 0] = (int) (9 * (1 - srcc) * srcc * srcc * srcc * 255);
                image[(y * s->width * 3 + x * 3) + (i * 3) + 1] = (int) (15 * (1 - srcc) * (1 - srcc) * srcc * srcc * 255);
                image[(y * s->width * 3 + x * 3) + (i * 3) + 2] = (int) (8.5 * (1 - srcc) * (1 - srcc) * (1 - srcc) * srcc * 255);
            }
        }
    }
}

void mandel_avx(unsigned char *image, struct imgspec *s){
    __m256 color, cr, ci, zr, zi;
    
    __m256 xleft = _mm256_set1_ps(s->xcoord[0]);
    __m256 ydown = _mm256_set1_ps(s->ycoord[0]);
    
    __m256 xdiff = _mm256_set1_ps(s->xcoord[1] - s->xcoord[0]);
    __m256 ydiff = _mm256_set1_ps(s->ycoord[1] - s->ycoord[0]);
    __m256 width = _mm256_set1_ps(s->width);
    __m256 height = _mm256_set1_ps(s->height);
    
    __m256 xpixelmap = _mm256_div_ps(xdiff, width);
    __m256 ypixelmap = _mm256_div_ps(ydiff, height);
    
    __m256 four = _mm256_set1_ps(4.0f);
    __m256 one = _mm256_set1_ps(1);
    
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x += 8) {
            __m256 mx = _mm256_set_ps(x + 7, x + 6, x + 5, x + 4, x + 3, x + 2, x + 1, x + 0);
            __m256 my = _mm256_set1_ps(y);
            zr = cr = _mm256_add_ps(_mm256_mul_ps(mx, xpixelmap), xleft);
            zi = ci = _mm256_add_ps(_mm256_mul_ps(my, ypixelmap), ydown);

            color = _mm256_set1_ps(1.0f);
            for(int counter=0; counter < ITERATIONS; counter++) {
                __m256 zrzi = _mm256_mul_ps(zr, zi); // zr * zi
                
                zr = _mm256_add_ps(_mm256_sub_ps(_mm256_mul_ps(zr, zr), _mm256_mul_ps(zi, zi)), cr); // (zr * zr - zi * zi) + cr
                zi = _mm256_add_ps(_mm256_add_ps(zrzi, zrzi), ci); // 2 * zr * zi + ci;

                __m256 z2 = _mm256_add_ps(_mm256_mul_ps(zr, zr), _mm256_mul_ps(zi, zi)); // zr * zr + zi * zi
                
                __m256 mask = _mm256_cmp_ps(z2, four, 1);
                color = _mm256_add_ps(_mm256_and_ps(mask, one), color);
                
                if(_mm256_movemask_ps(mask) == 0){
                    break;
                }
            }
            __m256i pixels = _mm256_cvtps_epi32(color);
            unsigned char *src = (unsigned char *)&pixels;
            for (int i = 0; i < 8; i++) {
                int srcc = src[i*4];
                image[(y * s->width * 3 + x * 3) + (i * 3) + 0] = (int) (9 * (1 - srcc) * srcc * srcc * srcc * 255);
                image[(y * s->width * 3 + x * 3) + (i * 3) + 1] = (int) (15 * (1 - srcc) * (1 - srcc) * srcc * srcc * 255);
                image[(y * s->width * 3 + x * 3) + (i * 3) + 2] = (int) (8.5 * (1 - srcc) * (1 - srcc) * (1 - srcc) * srcc * 255);
            }
        }
    }
}


/* UTILS */
int diffclock(double clock1,double clock2)
{
    double diffticks=clock1-clock2;
    double diffms=(diffticks*1000)/CLOCKS_PER_SEC;
    return (int)diffms;
}

int average_time(int *arr, int size){
    int result = 0;
    for(int i=0; i<size; i++){
        result += arr[i];
    }
    result = (result / size);
    return result;
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
#endif


/* run functions */
int run_mandel_avx(struct imgspec *spec, int store_img){
    unsigned char *image_avx = (unsigned char *)malloc(spec->width * spec->height * 3);
    clock_t begin_avx=clock();
    mandel_avx(image_avx, spec);
    clock_t end_avx=clock();
    
    int time_avx = diffclock(end_avx, begin_avx);
    
    if(store_img){
        FILE *fp_avx = fopen(IMG_PATH_AVX, "wb");
        fprintf(fp_avx, "P6\n%d %d\n%d\n", spec->width, spec->height, DEPTH - 1);
        fwrite(image_avx, spec->width * spec->height, 3, fp_avx);
        fclose(fp_avx);
    }
    free(image_avx);
    return time_avx;
}



int run_mandel_sse(struct imgspec *spec, int store_img){
    unsigned char *image_sse = (unsigned char *)malloc(spec->width * spec->height * 3);
    clock_t begin_sse=clock();
    mandel_sse(image_sse, spec);
    clock_t end_sse=clock();
    
    int time_sse = diffclock(end_sse, begin_sse);

    if(store_img){
        FILE *fp_sse = fopen(IMG_PATH_SSE, "wb");
        fprintf(fp_sse, "P6\n%d %d\n%d\n", spec->width, spec->height, DEPTH - 1);
        fwrite(image_sse, spec->width * spec->height, 3, fp_sse);
        fclose(fp_sse);
    }
    free(image_sse);
    
    return time_sse;
}



int run_mandel_basic(struct imgspec *spec, int store_img){
    unsigned char *image = (unsigned char *)malloc(spec->width * spec->height * 3);
    clock_t begin=clock();
    mandel_basic(image, spec);
    clock_t end=clock();
    
    int time_basic = diffclock(end,begin);
    
    if(store_img){
        FILE *fp = fopen(IMG_PATH_BASIC, "wb");
        fprintf(fp, "P6\n%d %d\n%d\n", spec->width, spec->height, DEPTH - 1);
        fwrite(image, spec->width * spec->height, 3, fp);
        fclose(fp);
    }
    free(image);
    
    return time_basic;
}



void run_test(struct imgspec *spec, int size, int storeimg){
    int avg_avx_time = 0;
    int avg_sse_time = 0;
    int avg_basic_time = 0;
    
    spec->width = size;
    spec->height = size;
    double pixels = size * size;
    
    /* AVX */
    #ifdef __x86_64__
    if(is_avx_supported()){
        int *arr_avx = (int*)malloc(TEST_LENGHT*sizeof(int));
        for(int i=0; i<TEST_LENGHT; i++){
            arr_avx[i] = run_mandel_avx(spec, ((i == TEST_LENGHT -1) && (storeimg == 1) && (size == TSIZE5)) ? 1 : 0);
        }
        avg_avx_time = average_time(arr_avx, TEST_LENGHT);
        free(arr_avx);
    }
    #endif
    
    /* SSE */
    int *arr_sse = (int*)malloc(TEST_LENGHT*sizeof(int));
    for(int i=0; i<TEST_LENGHT; i++){
        arr_sse[i] = run_mandel_sse(spec, ((i == TEST_LENGHT -1) && (storeimg == 1) && (size == TSIZE5)) ? 1 : 0);
    }
    avg_sse_time = average_time(arr_sse, TEST_LENGHT);
    free(arr_sse);
    
    /* Basic */
    int *arr_basic = (int*)malloc(TEST_LENGHT*sizeof(int));
    for(int i=0; i<TEST_LENGHT; i++){
        arr_basic[i] = run_mandel_basic(spec, ((i == TEST_LENGHT -1) && (storeimg == 1) && (size == TSIZE5)) ? 1 : 0);
    }
    avg_basic_time = average_time(arr_basic, TEST_LENGHT);
    free(arr_basic);
    
    /* Vypis statistiky */
    printf("\nPriemerny cas vykreslenia obrazku %d x %d pixelov:\n", size, size);
    printf("    Basic: %dms, Pixel/ms: %.2f\n", avg_basic_time, (pixels/avg_basic_time));
    printf("    SSE: %dms, zrychlenie %.2fx, Pixel/ms: %.2f\n", avg_sse_time, ((float)avg_basic_time/(float)avg_sse_time), (pixels/avg_sse_time));
    printf("    AVX: %dms, zrychlenie %.2fx, Pixel/ms: %.2f\n", avg_avx_time, ((float)avg_basic_time/(float)avg_avx_time), (pixels/avg_avx_time));
 
}


/* MAIN */
int main(int argc, const char * argv[]) {
    
    /* Config */
    IMGSPEC spec = {
        .width = 3000,
        .height = 3000,
        .xcoord = {-2.0, 2.0},
        .ycoord = {-2.0, 2.0},
    };
    
    int use_avx = 0;
    int use_sse = 0;
    int use_basic = 0;
    int test = 0;
    int store_img = 0;
    
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
        if(!strcmp(argv[option], "-T")){
            test = 1;
            option++;
            continue;
        }
        if(!strcmp(argv[option], "-w")){
            store_img = 1;
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
    
    
    
    if(!use_basic && !use_sse && !use_avx && !test){
        printf("ERROR: Zle zadane prepinace\n");
        return 0;
    }
    
    /* TEST */
    if(test){
        printf("Nasledovne priemerne casy su vypocitane z %d pokusov vykreslenia obrazku...\n", TEST_LENGHT);
        /* test 500 */
        run_test(&spec, TSIZE1, store_img);
        
        /* test 1000 */
        run_test(&spec, TSIZE2, store_img);
        
        /* test 2000 */
        run_test(&spec, TSIZE3, store_img);
        
        /* test 3000 */
        run_test(&spec, TSIZE4, store_img);
        
        /* test 4000 */
        run_test(&spec, TSIZE5, store_img);
        
        return 0;
    }
    
    
    
    /* AVX */
    #ifdef __x86_64__
    if(use_avx && is_avx_supported()){
        int time_avx = run_mandel_avx(&spec, store_img);
        printf("Vykresleny obrazok %d x %d pixelov:\n", spec.width, spec.height);
        printf("AVX cas: %d ms\n", time_avx);
        (store_img) ? printf("Ulozeny: %s\n",IMG_PATH_AVX) : printf("");
    }
    #endif
    
    /* SSE */
    if(use_sse){
        int time_sse = run_mandel_sse(&spec, store_img);
        printf("Vykresleny obrazok %d x %d pixelov:\n", spec.width, spec.height);
        printf("SSE cas: %d ms\n", time_sse);
        (store_img) ? printf("Ulozeny: %s\n", IMG_PATH_SSE) : printf("");
    }
    
    /* Basic */
    if(use_basic){
        int time_basic = run_mandel_basic(&spec, store_img);
        printf("Vykresleny obrazok %d x %d pixelov:\n", spec.width, spec.height);
        printf("Basic cas: %d ms\n", time_basic);
        (store_img) ? printf("Ulozeny: %s\n", IMG_PATH_BASIC) : printf("");
    }
    
    return 0;
}
