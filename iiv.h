#ifndef IIV_H_ 
#define IIV_H_

#include "main.h"
#include <vector>
#include <cmath>


namespace ImgInterp4Video{
    #define iiv_pi 3.14159265f

    void IIVresize(cv::InputArray _src, cv::OutputArray _dst, cv::Size dsize,
                    double inv_scale_x, double inv_scale_y, int interpolation );


}

#endif