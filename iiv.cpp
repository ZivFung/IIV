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

        // CV_INSTRUMENT_REGION();

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

        std::vector<double> xCoe(windowsize), yCoe(windowsize);
        int sx, sy, dx, dy;
        float fx, fy;
        int ksize=0, ksize2;
        double scale_x = 1./inv_scale_x, scale_y = 1./inv_scale_y;

        ksize2 = (int)(windowsize/2);
        for(dy = 0; dy < dsize.height; dy++ ){
            fy = (float)((dy+0.5)*scale_y - 0.5);
            sy = cvFloor(fy);
            fy -= sy;
            yCoe.clear();
            if(fy == .0){
                for(int i = 0; i < windowsize; i++){
                    if( i == ksize2 ){
                        yCoe.push_back(inv_scale_y);
                    }
                    else{
                        double tempCoe = std::sin(inv_scale_y * iiv_pi * (i - ksize2 + fy)) / (iiv_pi * (i - ksize2 + fy));      //sin(wc*n)/(pi*n)
                        yCoe.push_back(tempCoe);
                    }
                }
            }
            else{
                for(int i = 0; i < windowsize; i++){
                    double tempCoe = std::sin(inv_scale_y * iiv_pi * (i - ksize2 + fy)) / (iiv_pi * (i - ksize2 + fy));      //sin(wc*n)/(pi*n)
                    yCoe.push_back(tempCoe);
                }
            }

            double yCoeSum = 0;
            for(int i = 0; i < windowsize; i++){
                yCoeSum += yCoe[i];
            }

            uchar *rowStartPtr[windowsize];
            for(int j = 0; j < windowsize; j++){
               rowStartPtr[j] = src.ptr<uchar>(clip(sy + j - ksize2, 0, ssize.height));
            }

            for(dx = 0; dx < dsize.width; dx++ ){
                fx = (float)((dx+0.5)*scale_x - 0.5);
                sx = cvFloor(fx);
                fx -= sx;
                xCoe.clear();

                if(fx == .0){
                    for(int i = 0; i < windowsize; i++){
                        if( i == ksize2 ){
                            xCoe.push_back(inv_scale_x);
                        }
                        else{
                            double tempCoe = std::sin(inv_scale_x * iiv_pi * (i - ksize2 + fx)) / (iiv_pi * (i - ksize2 + fx));      //sin(wc*n)/(pi*n)
                            xCoe.push_back(tempCoe);
                        }
                    }
                }
                else{
                    for(int i = 0; i < windowsize; i++){
                        double tempCoe = std::sin(inv_scale_x * iiv_pi * (i - ksize2 + fx)) / (iiv_pi * (i - ksize2 + fx));      //sin(wc*n)/(pi*n)
                        xCoe.push_back(tempCoe);
                    }
                }
                double xCoeSum = 0;
                for(int i = 0; i < windowsize; i++){
                    xCoeSum += xCoe[i];
                }

                int channelNum = src.channels();
                double rowSum[channelNum][windowsize];
                memset(rowSum, 0, sizeof(rowSum));
                for(int j = 0; j < windowsize; j++){
                    uchar *rowPtr = rowStartPtr[j];
                    for(int i = 0; i < windowsize; i++){
                        int xMat = clip(sx + i - ksize2, 0, ssize.width) * channelNum;
                        for(int c = 0; c < channelNum; c++){
                            rowSum[c][j] += rowPtr[xMat + c] * xCoe[i];
                        }
                    }

                    for(int c = 0; c < channelNum; c++){
                        rowSum[c][j] /= xCoeSum;
                    }
                }

                double channelSum[channelNum] = {0};
                for(int c = 0; c < channelNum; c++){
                    for(int i = 0; i < windowsize; i++){
                        channelSum[c] += rowSum[c][i] * yCoe[i];
                    }
                }

                for(int c = 0; c < channelNum; c++){
                    channelSum[c] /= yCoeSum;
                }
                
                for(int c = 0; c < channelNum; c++){
                    dst.data[(dy * dsize.width + dx) * channelNum + c] = (uchar)clip((int)channelSum[c], 0, 256);
                }
            }
        }



    }


}