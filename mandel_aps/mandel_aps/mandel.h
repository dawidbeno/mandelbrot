//
//  mandel.h
//  mandel_aps
//
//  Created by Dávid Beňo on 21/11/2018.
//  Copyright © 2018 Dávid Beňo. All rights reserved.
//

#ifndef mandel_h
#define mandel_h


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



#endif /* mandel_h */
