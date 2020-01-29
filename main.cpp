#include "main.h"
#include "iiv.h"

int main(int argc, char **argv){
    std::cout << "test1" << std::endl;
    cv::Mat img = cv::imread("test1.png", cv::IMREAD_COLOR);
    cv::Mat test;
    cv::Size s(0,0);
    ImgInterp4Video::IIVresize(img, test, s, 0.5, 0.5, 6);
    cv::namedWindow("test");
    cv::imshow("test", test);
    cv::waitKey(0);
    return 0;
}

