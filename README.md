# Image Interpolation for Video
The most popular interpolation algorithm for image is "sinc interpolation" or its improved version "lanczos". However, this algorithm may work fine for image, but when sinc interpolation is used for video, its effect sometimes can be disastrous. Because this interpolation has no anti-aliasing effect. When it comes to image, invariant aliasing result works fine. But when it comes to video, different aliasing effect of every frame of video could makes the video dazzle.  
In order to get a suitable interpolation algorithm that can be `easily implemented on FPGA` through hardware design language(applicable resources utilization and easy to be paralleled), I designed IIV algorithm based on basic interpolation theory.  

  
# IIV's Formula
The formula of original sinc interpolation is `sin(Π*x)/Π*x`  
As for zoomout function, the formula of IIV is `sin(scale*Π*x)/Π*x`  
>scale is zoomout scale.(xScale = dst.width/src.width, yScale = dst.height/src.height)  
  
As for zoomin function, the algorithm is still under development.

# IIV's code
The function of IIV defined in iiv.cpp file.
The code is realized in the aid of openCV.
