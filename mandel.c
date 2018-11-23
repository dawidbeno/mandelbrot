#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include "mandel.h"

void mandel_basic(unsigned char *image,  struct imgspec *s);
void mandel_avx(unsigned char *image,  struct imgspec *s);
void mandel_sse2(unsigned char *image,  struct imgspec *s);

void mandel_basic(unsigned char *image,  struct imgspec *s)
{
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

void printImageMeta(IMGSPEC *s){
    printf("Image metadata:\n");
    printf("Width: %d\n", s->width);
    printf("Height: %d\n", s->height);
}




int run_mandel_avx(IMGSPEC *spec, int store_img){
    unsigned char *image_avx = (unsigned char *)malloc(spec->width * spec->height * 3);
    clock_t begin_avx=clock();
    mandel_avx(image_avx, spec);
    clock_t end_avx=clock();
    
    int time_avx = diffclock(end_avx, begin_avx);
    
    if(store_img){
        FILE *fp_avx = fopen(IMG_PATH_AVX, "wb");
        fprintf(fp_avx, "P6\n%d %d\n%d\n", spec->width, spec->height, spec->depth - 1);
        fwrite(image_avx, spec->width * spec->height, 3, fp_avx);
        fclose(fp_avx);
    }
    free(image_avx);
    return time_avx;
}



int run_mandel_sse(IMGSPEC *spec, int store_img){
    unsigned char *image_sse = (unsigned char *)malloc(spec->width * spec->height * 3);
    clock_t begin_sse=clock();
    mandel_sse2(image_sse, spec);
    clock_t end_sse=clock();
    
    int time_sse = diffclock(end_sse, begin_sse);

    if(store_img){
        FILE *fp_sse = fopen(IMG_PATH_SSE, "wb");
        fprintf(fp_sse, "P6\n%d %d\n%d\n", spec->width, spec->height, spec->depth - 1);
        fwrite(image_sse, spec->width * spec->height, 3, fp_sse);
        fclose(fp_sse);
    }
    free(image_sse);
    
    return time_sse;
}



int run_mandel_basic(IMGSPEC *spec, int store_img){
    unsigned char *image = (unsigned char *)malloc(spec->width * spec->height * 3);
    clock_t begin=clock();
    mandel_basic(image, spec);
    clock_t end=clock();
    
    int time_basic = diffclock(end,begin);
    
    if(store_img){
        FILE *fp = fopen(IMG_PATH_BASIC, "wb");
        fprintf(fp, "P6\n%d %d\n%d\n", spec->width, spec->height, spec->depth - 1);
        fwrite(image, spec->width * spec->height, 3, fp);
        fclose(fp);
    }
    free(image);
    
    return time_basic;
}


void run_test(IMGSPEC *spec, int size){
    int avg_avx_time = 0;
    int avg_sse_time = 0;
    int avg_basic_time = 0;
    
    spec->width = size;
    spec->height = size;
    
    /* AVX */
    int *arr_avx = (int*)malloc(TEST_LENGHT*sizeof(int));
    for(int i=0; i<TEST_LENGHT; i++){
        arr_avx[i] = run_mandel_avx(spec, 0);
    }
    avg_avx_time = average_time(arr_avx, TEST_LENGHT);
    free(arr_avx);
    
    /* SSE */
    int *arr_sse = (int*)malloc(TEST_LENGHT*sizeof(int));
    for(int i=0; i<TEST_LENGHT; i++){
        arr_sse[i] = run_mandel_sse(spec, 0);
    }
    avg_sse_time = average_time(arr_sse, TEST_LENGHT);
    free(arr_sse);
    
    /* Basic */
    int *arr_basic = (int*)malloc(TEST_LENGHT*sizeof(int));
    for(int i=0; i<TEST_LENGHT; i++){
        arr_basic[i] = run_mandel_basic(spec, 0);
    }
    avg_basic_time = average_time(arr_basic, TEST_LENGHT);
    free(arr_basic);
    
    /* Vypis statistiky */
    printf("Priemerny cas vykreslenia obrazku %dx%d pixelov:\n", size, size);
    printf("    Basic: %dms\n", avg_basic_time);
    printf("    SSE: %d\n", avg_sse_time);
    printf("    AVX: %d\n", avg_avx_time);
    printf("Zrychlenie pri pouziti specialnych instrukcii:");
    printf("    SSE oproti Basic: %dx zrychlenie\n", (avg_basic_time/avg_sse_time));
    printf("    AVX oproti Basic: %dx zrychlenie\n", (avg_basic_time/avg_avx_time));
    printf("    AVX oproti SSE: %dx zrychlenie\n\n", (avg_sse_time/avg_avx_time));
}



int main(int argc, const char * argv[]) {
    
    /* Config */
    IMGSPEC spec = {
        .width = 3000,
        .height = 3000,
        .depth = 256,
        .xlim = {-2.5, 1.5},
        .ylim = {-1.5, 1.5},
        .iterations = 256
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
        /* test 256 */
        run_test(&spec, 256);
        
        /* test 512 */
        run_test(&spec, 512);
        
        /* test 1024 */
        run_test(&spec, 1024);
        
        /* test 2048 */
        run_test(&spec, 2048);
        
        /* test 4000 */
        run_test(&spec, 4000);
        
        /* test 8000 */
        run_test(&spec, 8000);
        
        return 0;
    }
    
    
    
    /* AVX */
    if(use_avx){
        int time_avx = run_mandel_avx(&spec, store_img);
        printf("C OUTPUT: AVX time: %d ms\n", time_avx);
    }
    
    /* SSE */
    if(use_sse){
        int time_sse = run_mandel_sse(&spec, store_img);
        printf("C OUTPUT: SSE time: %d ms\n", time_sse);
    }
    
    /* Basic */
    if(use_basic){
        int time_basic = run_mandel_basic(&spec, store_img);
        printf("C OUTPUT: Basic time: %d ms\n", time_basic);
    }
    
    return 0;
}
