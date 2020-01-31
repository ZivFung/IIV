# Image Interpolation for Video
The most popular interpolation algorithm for image is "sinc interpolation" or its improved version "lanczos". However, this algorithm may work fine for image, but when sinc interpolation is used for video and the zoomout scale is too small, its effect sometimes can be disastrous. Because this interpolation `has no anti-aliasing effect`. When it comes to image, invariant aliasing result works fine. But when it comes to video, different aliasing effect of every frame of video could makes the video dazzle.  
  
In order to get a suitable interpolation algorithm that can be `easily implemented on FPGA` through hardware design language(applicable resources utilization and easy to be paralleled), I designed IIV algorithm based on basic interpolation theory.  

  
# IIV's Formula
The formula of original sinc interpolation is `sin(Π*x)/Π*x`  
As for zoomout function, the formula of IIV is `sin(scale*Π*x)/Π*x`  
>scale is zoomout scale.(xScale = dst.width/src.width, yScale = dst.height/src.height)  
  
As for zoomin function, the formula of IIV is still `sin(Π*x)/Π*x`  
>Interpolation algorithm's zoomin effect is much worse than super-resolution algorithm.

# Compare the IIV with openCV resize function in zoomout condition
I choose a video with 1920*1080 size, 30FPS to do the experiment. The zoomout sacle for both the vertical and horizontal is 0.2. The window size of opencv lanzcos and IIV is both 8 * 8. This is the result of opencv lanczos4 and IIV.  

## openCV lanzcos4  
>![image](https://github.com/ZivFung/IIV/blob/master/opencv_lanczos.gif)  
>Please focus on the every edge in the video. We can observe plenty of aliasing effect.  
  
## IIV  
>![image](https://github.com/ZivFung/IIV/blob/master/IIV.gif)  
>By contrast, the IIV eliminates most of aliasing effect in the video zoomout result.

# IIV's code
This Program can test the performance of IIV algorithm.  
The function of IIV defined in iiv.cpp file.  
The code is realized in the aid of openCV.  
## Usage
This repo can directly open with vscode. Then you can run or debug IIV.  
Also, you can run exe file in windows with arguments:  
>./IIV.exe -f function_num -v video_path -s IIV_windowsize -m horizontal_scale -n vertical_scale -x target_width -y target_height  
>function number detail:  
>>0: directly open opencv window and compare opencv lanczos and IIV result.(Since the code of IIV does not do optimization for speed, it will running to slow)  
>>1: first do resize for opencv lanczos and IIV, write the result to mp4 format file. Then open opencv window to read two mp4 file and show its contend. (recommend)  
>>2: only do IIV, and play the result with multi thread.  
>video_path: the path of target test video.  
>IIV_windowsize: specify the window size of IIV.  
>horizontal_scale and vertical_scale: specify the resize scale.  
>target_width and target_height: specify the target size.  
>Note: you can only specify whether the scale or target size.  
