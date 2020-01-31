/*
 * @Author: ZivFung 
 * @Date: 2020-01-30 22:33:00 
 * @Last Modified by: ZivFung
 * @Last Modified time: 2020-02-01 01:07:49
 */
#ifndef IIV_H_ 
#define IIV_H_

#include "main.h"
#include <vector>
#include <cmath>
#include <thread>



namespace ImgInterp4Video{
    #define iiv_pi 3.14159265f
    #define FAST_IIV_THREAD_NUM 8
    void IIVresize(cv::InputArray _src, cv::OutputArray _dst, cv::Size dsize,
                    double inv_scale_x, double inv_scale_y, int windowsize );
    void fastIIVresize(cv::InputArray _src, cv::OutputArray _dst, cv::Size dsize,                   //Multi thread version
                double inv_scale_x, double inv_scale_y, int windowsize );

}

#endif