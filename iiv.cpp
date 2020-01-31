/*
 * @Author: ZivFung 
 * @Date: 2020-01-30 22:32:46 
 * @Last Modified by: ZivFung
 * @Last Modified time: 2020-02-01 01:10:13
 */
#include "iiv.h"
namespace ImgInterp4Video{
    static inline int clip(int x, int a, int b)
    {
        return x >= a ? (x < b ? x : b-1) : a;
    }

    static inline double clip(double x, double a, double b)
    {
        return x >= a ? (x < b ? x : b-1) : a;
    }

    static inline uchar clip(uchar x, uchar a, uchar b)
    {
        return x >= a ? (x < b ? x : b-1) : a;
    }

    void IIVresize(cv::InputArray _src, cv::OutputArray _dst, cv::Size dsize,
                 double inv_scale_x, double inv_scale_y, int windowsize = 6 ){

        cv::Size ssize = _src.size();
        CV_Assert( !ssize.empty() );
        CV_Assert( !(windowsize%2) );
        if( dsize.empty() )
        {
            CV_Assert(inv_scale_x > 0); CV_Assert(inv_scale_y > 0);
            dsize = cv::Size(cv::saturate_cast<int>(ssize.width*inv_scale_x),
                        cv::saturate_cast<int>(ssize.height*inv_scale_y));
            CV_Assert( !dsize.empty() );
        }
        else
        {
            inv_scale_x = (double)dsize.width/ssize.width;
            inv_scale_y = (double)dsize.height/ssize.height;
            CV_Assert(inv_scale_x > 0); CV_Assert(inv_scale_y > 0);
        }

        cv::Mat src = _src.getMat();
        _dst.create(dsize, src.type());
        cv::Mat dst = _dst.getMat();

        if (dsize == ssize)
        {
            // Source and destination are of same size. Use simple copy.
            src.copyTo(dst);
            return;
        }

        CV_Assert(windowsize >= 4);

        std::vector<float> xCoe(windowsize), yCoe(windowsize);
        int sx, sy, dx, dy;
        float fx, fy;
        int ksize=0, ksize2;
        float scale_x = 1./inv_scale_x, scale_y = 1./inv_scale_y;

        ksize2 = (int)(windowsize/2);
        int channelNum = src.channels();

        float tempCoe;
        float yCoeSum = 0, xCoeSum = 0;
        float rowSum[channelNum][windowsize];
        float channelSum[channelNum] = {0};
        
        for(dy = 0; dy < dsize.height; dy++ ){
            fy = (float)((dy+0.5)*scale_y - 0.5);
            sy = cvFloor(fy);
            fy -= sy;
            yCoeSum = 0;
            if(fy == .0){
                for(int i = 0; i < windowsize; i++){
                    if(inv_scale_y <= 1){
                        if( i == (ksize2 - 1) ){
                            yCoe[i] = inv_scale_y;
                        }
                        else{
                            float pos = (i - ksize2 + 1);
                            tempCoe = std::sin(inv_scale_y * iiv_pi * pos) / (iiv_pi * pos);      //sin(wc*n)/(pi*n)
                            yCoe[i] = tempCoe;
                        }
                    }
                    else if(inv_scale_y > 1){
                        if( i == (ksize2 - 1) ){
                            yCoe[i] = 1;
                        }
                        else{
                            float pos = (i - ksize2 + 1);
                            tempCoe = std::sin(iiv_pi * pos) / (iiv_pi * pos);      //sin(pi*n)/(pi*n)
                            yCoe[i] = tempCoe;
                        }
                    }
                    yCoeSum += yCoe[i];
                }
            }
            else{
                for(int i = 0; i < windowsize; i++){
                    if(inv_scale_y <= 1){
                        float pos = (i - ksize2 - fy + 1);
                        tempCoe = std::sin(inv_scale_y * iiv_pi * pos) / (iiv_pi * pos);      //sin(wc*n)/(pi*n)
                        yCoe[i] = tempCoe;
                    }
                    else if(inv_scale_y > 1){
                        float pos = (i - ksize2 - fy + 1);
                        tempCoe = std::sin(iiv_pi * pos) / (iiv_pi * pos);      //sin(n)/(pi*n)
                        yCoe[i] = tempCoe;
                    }
                    yCoeSum += yCoe[i];
                }
            }

            for(int i = 0; i < windowsize; i++){
                yCoe[i] /= yCoeSum;
            }

            uchar *rowStartPtr[windowsize];
            for(int j = 0; j < windowsize; j++){
               rowStartPtr[j] = src.ptr<uchar>(clip(sy + j - ksize2 + 1, 0, ssize.height));
            }

            for(dx = 0; dx < dsize.width; dx++ ){
                fx = (float)((dx+0.5)*scale_x - 0.5);
                sx = cvFloor(fx);
                fx -= sx;
                xCoeSum = 0;
                if(fx == .0){
                    for(int i = 0; i < windowsize; i++){
                        if(inv_scale_x <= 1){
                            if( i == (ksize2 - 1) ){
                                xCoe[i] = inv_scale_x;
                            }
                            else{
                                float pos = (i - ksize2 + 1);
                                tempCoe = std::sin(inv_scale_x * iiv_pi * pos) / (iiv_pi * pos);      //sin(wc*pi*n)/(pi*n)
                                xCoe[i] = tempCoe;
                            }
                        }
                        else if(inv_scale_x > 1){
                            if( i == (ksize2 - 1) ){
                                xCoe[i] = 1;
                            }
                            else{
                                float pos = (i - ksize2 + 1);
                                tempCoe = std::sin(iiv_pi * pos) / (iiv_pi * pos);     //sin(pi*n)/(pi*n)
                                xCoe[i] = tempCoe;
                            }
                        }
                        xCoeSum += xCoe[i];
                    }
                }
                else{
                    for(int i = 0; i < windowsize; i++){
                        if(inv_scale_x <= 1){
                            float pos = (i - ksize2 - fx + 1);
                            tempCoe = std::sin(inv_scale_x * iiv_pi * pos) / (iiv_pi * pos);      //sin(wc*pi*n)/(pi*n)
                            xCoe[i] = tempCoe;
                        }
                        else if(inv_scale_x > 1){
                            float pos = (i - ksize2 - fx + 1);
                            tempCoe = std::sin(iiv_pi * pos) / (iiv_pi * pos);       //sin(pi*n)/(pi*n)
                            xCoe[i] = tempCoe;
                        }
                        xCoeSum += xCoe[i];
                    }
                }

                for(int i = 0; i < windowsize; i++){
                    xCoe[i] /= xCoeSum;
                }

                memset(rowSum, 0, sizeof(rowSum));
                for(int j = 0; j < windowsize; j++){
                    uchar *rowPtr = rowStartPtr[j];
                    for(int i = 0; i < windowsize; i++){
                        int xMat = clip(sx + i - ksize2 + 1, 0, ssize.width) * channelNum;
                        for(int c = 0; c < channelNum; c++){
                            rowSum[c][j] += rowPtr[xMat + c] * xCoe[i];
                        }
                    }
                }

                memset(channelSum, 0, sizeof(channelSum));
                for(int c = 0; c < channelNum; c++){
                    for(int i = 0; i < windowsize; i++){
                        channelSum[c] += rowSum[c][i] * yCoe[i];
                    }
                }
                
                for(int c = 0; c < channelNum; c++){
                    dst.data[(dy * dsize.width + dx) * channelNum + c] = (uchar)clip((int)channelSum[c], 0, 256);
                }
            }
        }
    }

    void IIVThread(cv::Mat *src, cv::Mat *dst, double inv_scale_x, double inv_scale_y, cv::Size dsize, cv::Size ssize,
            int dy, int windowsize = 6){

        std::vector<double> xCoe(windowsize), yCoe(windowsize);
        int sx, sy, dx;
        float fx, fy;
        int ksize=0, ksize2;    

        ksize2 = (int)(windowsize/2);
        int channelNum = src->channels();

        double tempCoe;
        double yCoeSum = 0, xCoeSum = 0;
        double rowSum[channelNum][windowsize];
        double channelSum[channelNum] = {0};

        fy = (float)((dy+0.5)/inv_scale_y - 0.5);
        sy = cvFloor(fy);
        fy -= sy;
        if(fy == .0){
            for(int i = 0; i < windowsize; i++){
                if(inv_scale_y < 1){
                    if( i == ksize2 ){
                        yCoe[i] = inv_scale_y;
                    }
                    else{
                        tempCoe = std::sin(inv_scale_y * iiv_pi * (i - ksize2 + fy)) / (iiv_pi * (i - ksize2 + fy));      //sin(wc*n)/(pi*n)
                        yCoe[i] = tempCoe;
                    }
                }
                else if(inv_scale_y > 1){
                    if( i == ksize2 ){
                        yCoe[i] = 1;
                    }
                    else{
                        tempCoe = std::sin(iiv_pi * (i - ksize2 + fy)) / (iiv_pi * (i - ksize2 + fy));      //sin(wc*n)/(pi*n)
                        yCoe[i] = tempCoe;
                    }
                }
            }
        }
        else{
            for(int i = 0; i < windowsize; i++){
                if(inv_scale_y < 1){
                    tempCoe = std::sin(inv_scale_y * iiv_pi * (i - ksize2 + fy)) / (iiv_pi * (i - ksize2 + fy));      //sin(wc*n)/(pi*n)
                    yCoe[i] = tempCoe;
                }
                else if(inv_scale_y > 1){
                    tempCoe = std::sin(iiv_pi * (i - ksize2 + fy)) / (iiv_pi * (i - ksize2 + fy));      //sin(wc*n)/(pi*n)
                    yCoe[i] = tempCoe;
                }
            }
        }

        yCoeSum = 0;
        for(int i = 0; i < windowsize; i++){
            yCoeSum += yCoe[i];
        }

        uchar *rowStartPtr[windowsize];
        for(int j = 1; j <= windowsize; j++){
           rowStartPtr[j] = src->ptr<uchar>(clip(sy + j - ksize2, 0, ssize.height));
        }

        for(dx = 0; dx < dsize.width; dx++ ){
            fx = (float)((dx+0.5)/inv_scale_x - 0.5);
            sx = cvFloor(fx);
            fx -= sx;

            if(fx == .0){
                for(int i = 0; i < windowsize; i++){
                    if(inv_scale_x < 1){
                        if( i == ksize2 ){
                            xCoe[i] = inv_scale_x;
                        }
                        else{
                            tempCoe = std::sin(inv_scale_x * iiv_pi * (i - ksize2 + fx)) / (iiv_pi * (i - ksize2 + fx));      //sin(wc*n)/(pi*n)
                            xCoe[i] = tempCoe;
                        }
                    }
                    else if(inv_scale_x > 1){
                        if( i == ksize2 ){
                            xCoe[i] = 1;
                        }
                        else{
                            tempCoe = std::sin(iiv_pi * (i - ksize2 + fx)) / (iiv_pi * (i - ksize2 + fx));      //sin(wc*n)/(pi*n)
                            xCoe[i] = tempCoe;
                        }
                    }
                }
            }
            else{
                for(int i = 0; i < windowsize; i++){
                    if(inv_scale_x < 1){
                        tempCoe = std::sin(inv_scale_x * iiv_pi * (i - ksize2 + fx)) / (iiv_pi * (i - ksize2 + fx));      //sin(wc*n)/(pi*n)
                        xCoe[i] = tempCoe;
                    }
                    else if(inv_scale_x > 1){
                        tempCoe = std::sin(iiv_pi * (i - ksize2 + fx)) / (iiv_pi * (i - ksize2 + fx));      //sin(wc*n)/(pi*n)
                        xCoe[i] = tempCoe;
                    }
                }
            }
            xCoeSum = 0;
            for(int i = 0; i < windowsize; i++){
                xCoeSum += xCoe[i];
            }
            
            memset(rowSum, 0, sizeof(rowSum));
            for(int j = 0; j < windowsize; j++){
                uchar *rowPtr = rowStartPtr[j];
                for(int i = 0; i < windowsize; i++){
                    int xMat = clip(sx + i - ksize2 + 1, 0, ssize.width) * channelNum;
                    for(int c = 0; c < channelNum; c++){
                        rowSum[c][j] += rowPtr[xMat + c] * xCoe[i];
                    }
                }

                for(int c = 0; c < channelNum; c++){
                    rowSum[c][j] /= xCoeSum;
                }
            }

            memset(channelSum, 0, sizeof(channelSum));
            for(int c = 0; c < channelNum; c++){
                for(int i = 0; i < windowsize; i++){
                    channelSum[c] += rowSum[c][i] * yCoe[i];
                }
            }

            for(int c = 0; c < channelNum; c++){
                channelSum[c] /= yCoeSum;
            }
            
            for(int c = 0; c < channelNum; c++){
                dst->data[(dy * dsize.width + dx) * channelNum + c] = (uchar)clip((int)channelSum[c], 0, 256);
            }
        }

    }
    void fastIIVresize(cv::InputArray _src, cv::OutputArray _dst, cv::Size dsize,                   //Multi thread version
                double inv_scale_x, double inv_scale_y, int windowsize = 6 ){

        cv::Size ssize = _src.size();

        CV_Assert( !ssize.empty() );
        CV_Assert( !(windowsize%2) );
        if( dsize.empty() )
        {
            CV_Assert(inv_scale_x > 0); CV_Assert(inv_scale_y > 0);
            dsize = cv::Size(cv::saturate_cast<int>(ssize.width*inv_scale_x),
                        cv::saturate_cast<int>(ssize.height*inv_scale_y));
            CV_Assert( !dsize.empty() );
        }
        else
        {
            inv_scale_x = (double)dsize.width/ssize.width;
            inv_scale_y = (double)dsize.height/ssize.height;
            CV_Assert(inv_scale_x > 0); CV_Assert(inv_scale_y > 0);
        }
        cv::Mat src[FAST_IIV_THREAD_NUM];
        for(int i = 0; i < FAST_IIV_THREAD_NUM; i++)
            src[i] = _src.getMat();
        _dst.create(dsize, src[0].type());
        cv::Mat dst = _dst.getMat();

        if (dsize == ssize)
        {
            // Source and destination are of same size. Use simple copy.
            src[0].copyTo(dst);
            return;
        }
        CV_Assert(windowsize >= 4);

        int threadGroupNum = (int)(dsize.height/FAST_IIV_THREAD_NUM);
        int remaindThreadNum = dsize.height%FAST_IIV_THREAD_NUM;
        int dy = 0;
        std::thread rowThread[FAST_IIV_THREAD_NUM];
        for(int groupNum = 0; groupNum < threadGroupNum; groupNum++){
            
            for(int i = 0; i < FAST_IIV_THREAD_NUM; i++){
                rowThread[i] = std::thread(IIVThread, &src[i], &dst, inv_scale_x, inv_scale_y, dsize, ssize, dy+i, windowsize);
            }
            for(int i = 0; i < FAST_IIV_THREAD_NUM; i++){
                rowThread[i].join();
            }
            dy += FAST_IIV_THREAD_NUM;
        }

        std::thread rowRemainThread[remaindThreadNum];
        for(int i = 0; i < remaindThreadNum; i++){
            rowRemainThread[i] = std::thread(IIVThread, &src[i], &dst, inv_scale_x, inv_scale_y, dsize, ssize, dy+i, windowsize);
        }
        for(int i = 0; i < remaindThreadNum; i++){
            rowRemainThread[i].join();
        }
    
    }

}