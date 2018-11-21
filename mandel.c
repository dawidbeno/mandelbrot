#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include "mandel.h"

void mandel_basic(unsigned char *image, const struct spec *s);
void mandel_avx(unsigned char *image, const struct spec *s);
void mandel_sse2(unsigned char *image, const struct spec *s);

void mandel_basic(unsigned char *image, const struct spec *s)
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

int main(int argc, char *argv[]){
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
        unsigned char *image_avx = malloc(spec.width * spec.height * 3);
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
        unsigned char *image_sse = malloc(spec.width * spec.height * 3);
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
        unsigned char *image = malloc(spec.width * spec.height * 3);
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
