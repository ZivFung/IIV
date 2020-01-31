/*
 * @Author: ZivFung 
 * @Date: 2020-01-30 22:32:31 
 * @Last Modified by: ZivFung
 * @Last Modified time: 2020-02-01 01:07:58 
 */

#include "main.h"
#include "iiv.h"

#define EXP_ALGORITHM_NUM 2
#define VID_WRITE_NUM 2
struct Job{
    cv::VideoCapture cap;
    int resizeMethod = 0;
    cv::Size dstSize;
    double resizeXScale, resizeYScale;
    int IIVWinSize = 6;
    std::string opencvWinName;
};

void vidResizePlayThread(Job *job){
    Job *j = (Job *)job;
    cv::Mat frame;
    cv::Mat show;
    cv::Size s = j->dstSize;
    double vid_FPS = j->cap.get(cv::CAP_PROP_FPS);
    double pauseTime = 1000.0 / vid_FPS;
    while(j->cap.read(frame)){
        switch(j->resizeMethod){
            case 0:         //opencv lanczos
                if(s.empty())
                    cv::resize(frame, show, s, j->resizeXScale, j->resizeYScale, cv::INTER_LANCZOS4);
                else
                    cv::resize(frame, show, s, 0, 0, cv::INTER_LANCZOS4);
                cv::imshow(j->opencvWinName, show);
                cv::waitKey(pauseTime);
                break;
            case 1:         //IIV
                if(s.empty())
                    ImgInterp4Video::IIVresize(frame, show, s, j->resizeXScale, j->resizeYScale, j->IIVWinSize);
                else 
                    ImgInterp4Video::IIVresize(frame, show, s, 0, 0, j->IIVWinSize);
                cv::imshow(j->opencvWinName, show);
                cv::waitKey(1);
                break;
            case 2:         //fast IIV
                if(s.empty())
                    ImgInterp4Video::fastIIVresize(frame, show, s, j->resizeXScale, j->resizeYScale, j->IIVWinSize);
                else 
                    ImgInterp4Video::fastIIVresize(frame, show, s, 0, 0, j->IIVWinSize);
                cv::imshow(j->opencvWinName, show);
                cv::waitKey(1);
                break;
        }
    }
}

void vidResizeWriteThread(Job *job){
     Job *j = (Job *)job;
    cv::Mat frame;
    cv::Mat show;
    cv::Size s = j->dstSize;
    cv::VideoWriter vidWrite;
    if(s.empty()){
        cv::Size size;
        size = cv::Size(cv::saturate_cast<int>(j->cap.get(cv::CAP_PROP_FRAME_WIDTH)*j->resizeXScale),
                    cv::saturate_cast<int>(j->cap.get(cv::CAP_PROP_FRAME_HEIGHT)*j->resizeYScale));
        vidWrite = cv::VideoWriter(std::to_string(j->resizeMethod) + '_' + j->opencvWinName + ".mp4", 
                                    cv::VideoWriter::fourcc('M', 'P', '4', 'V'), j->cap.get(cv::CAP_PROP_FPS), size);
    }
    else {
        vidWrite = cv::VideoWriter(std::to_string(j->resizeMethod) + '_' + j->opencvWinName + ".mp4", 
                                    cv::VideoWriter::fourcc('M', 'P', '4', 'V'), j->cap.get(cv::CAP_PROP_FPS), s);
    }
    while(j->cap.read(frame)){
        switch(j->resizeMethod){
            case 0:         //opencv lanczos
                if(s.empty())
                    cv::resize(frame, show, s, j->resizeXScale, j->resizeYScale, cv::INTER_LANCZOS4);
                else
                    cv::resize(frame, show, s, 0, 0, cv::INTER_LANCZOS4);
                vidWrite.write(show);
                
                break;
            case 1:         //IIV
                if(s.empty())
                    ImgInterp4Video::IIVresize(frame, show, s, j->resizeXScale, j->resizeYScale, j->IIVWinSize);
                else 
                    ImgInterp4Video::IIVresize(frame, show, s, 0, 0, j->IIVWinSize);
                vidWrite.write(show);
                break;
            // case 2:         //fast IIV
            //     if(s.empty())
            //         ImgInterp4Video::fastIIVresize(frame, show, s, j->resizeXScale, j->resizeYScale, j->IIVWinSize);
            //     else 
            //         ImgInterp4Video::fastIIVresize(frame, show, s, 0, 0, j->IIVWinSize);
            //     vidWrite.write(show);
            //     break;
        }
    }
    vidWrite.release();
}

std::mutex mtx;
int reorderFlag = 0;
void vidResizeThread(cv::Mat frame, std::queue<cv::Mat> *resultFIFO, int threadNum, cv::Size dsize, double xScale, double yscale, int windowSize){
    cv::Mat show;
    ImgInterp4Video::IIVresize(frame, show, dsize, xScale, yscale, windowSize);
    // cv::resize(frame, show, dstSize, 0.2, 0.2, cv::INTER_LANCZOS4);
    while(reorderFlag != (threadNum - 1));
    mtx.lock();
    resultFIFO->push(show);
    reorderFlag++;
    mtx.unlock();
}

int playFinishFlag = 0;
void vidSuperPlayThread(std::queue<cv::Mat> *resultFIFO){
    cv::Mat show;
    while((!playFinishFlag) || (!resultFIFO->empty())){
        for(;;){
            if(!resultFIFO->empty()){
                show = resultFIFO->front();
                mtx.lock();
                resultFIFO->pop();
                mtx.unlock();
                break;
            }
        }
        cv::imshow("vid_thread_test", show);
        cv::waitKey(15);
    }
}

int sync_flag = 0;

void vidPlayThread(cv::VideoCapture vid, std::string windowName){
    cv::Mat frame;
    double vid_FPS = vid.get(cv::CAP_PROP_FPS);
    double pauseTime = 1000.0 / vid_FPS;
    while(!sync_flag);
    while(vid.read(frame)){
        cv::imshow(windowName, frame);
        cv::waitKey(pauseTime);
    }

}

void compareAndWriteVideo(cv::VideoCapture *vid, cv::Size dsize, double xScale, double yscale, int windowSize){
    Job expJob[VID_WRITE_NUM];
    
    for(int i = 0; i < VID_WRITE_NUM; i++){
        expJob[i].cap = vid[i];
        expJob[i].dstSize = dsize;
        expJob[i].IIVWinSize = windowSize;
        expJob[i].resizeXScale = xScale;
        expJob[i].resizeYScale = yscale;
        switch(i){
            case 0:
                expJob[i].opencvWinName = "opencv_lanczos";
                expJob[i].resizeMethod = 0;
                break;
            case 1:
                expJob[i].opencvWinName = "IIV";
                expJob[i].resizeMethod = 1;
                break;
            // case 2:
            //     expJob[i].opencvWinName = "Fast IIV";
            //     expJob[i].resizeMethod = 2;
            //     break;
        }
    }
    std::thread vidExpPlay[VID_WRITE_NUM];
    for(int i = 0; i < VID_WRITE_NUM; i++){
        vidExpPlay[i] =  std::thread(vidResizeWriteThread, &expJob[i]);
    }
    
    for(int i = 0; i < VID_WRITE_NUM; i++){
        vidExpPlay[i].join();
    }

    cv::VideoCapture exp[VID_WRITE_NUM];
    for(int i = 0; i < VID_WRITE_NUM; i++){
        exp[i].open(std::to_string(i) + '_' + expJob[i].opencvWinName + ".mp4");
    }
    
    for(int i = 0; i < VID_WRITE_NUM; i++){
        vidExpPlay[i] =  std::thread(vidPlayThread, exp[i], expJob[i].opencvWinName);
    }

    for(int i = 0; i < VID_WRITE_NUM; i++){
        vidExpPlay[i].join();
    }
}

void normalFunctionCopmpare(cv::VideoCapture *vid, cv::Size dsize, double xScale, double yscale, int windowSize){
    Job expJob[EXP_ALGORITHM_NUM];
    for(int i = 0; i < EXP_ALGORITHM_NUM; i++){
        expJob[i].cap = vid[i];
        expJob[i].dstSize = dsize;
        expJob[i].IIVWinSize = windowSize;
        expJob[i].resizeXScale = xScale;
        expJob[i].resizeYScale = yscale;
        switch(i){
            case 0:
                expJob[i].opencvWinName = "opencv_lanczos";
                expJob[i].resizeMethod = 0;
                break;
            case 1:
                expJob[i].opencvWinName = "IIV";
                expJob[i].resizeMethod = 1;
                break;
            case 2:
                expJob[i].opencvWinName = "Fast IIV";
                expJob[i].resizeMethod = 2;
                break;
        }
    }
    std::thread vidExpPlay[EXP_ALGORITHM_NUM];
    for(int i = 0; i < EXP_ALGORITHM_NUM; i++){
        vidExpPlay[i] =  std::thread(vidResizePlayThread, &expJob[i]);  
    }
    
    for(int i = 0; i < EXP_ALGORITHM_NUM; i++){
        vidExpPlay[i].join();
    }
}

void multiThreadPlay(cv::VideoCapture vid, cv::Size dsize, double xScale, double yscale, int windowSize){
        int playThreadNum = 8;
        cv::Mat frame[playThreadNum];
        std::queue<cv::Mat> resizeResult;
        cv::Mat show;
        std::thread vidResize[playThreadNum];

        int readFinishFlag = 0;
        std::thread viPlay(vidSuperPlayThread, &resizeResult);
        while(1){
            if(!readFinishFlag){
                for(int i = 0; i < playThreadNum; i++){
                    while(resizeResult.size() > 10);            //In case the queue is too large
                    if(vid.read(frame[i]))
                        vidResize[i] = std::thread(vidResizeThread, frame[i], &resizeResult, i + 1, dsize, xScale, yscale, windowSize);
                    else{
                        readFinishFlag = 1;
                        break;
                    }
                }

                for(int i = 0; i < playThreadNum; i++){
                    vidResize[i].join();
                }
                mtx.lock();
                reorderFlag = 0;
                mtx.unlock();
                if(readFinishFlag)
                    playFinishFlag = 1;
            }
            else break;
        }
        viPlay.join();
}

void testResultPlay(){
    cv::VideoCapture vid[VID_WRITE_NUM];
    vid[0].open("0_opencv_Lanczos.mp4");
    vid[1].open("1_IIV.mp4");
    std::thread test0(vidPlayThread, vid[0], "opencv_Lanczos");
    std::thread test1(vidPlayThread, vid[1], "IIV");
    sync_flag = 1;
    test1.join();
    test0.join();
}

static struct option const long_opts[] = {
        {"function",            required_argument,  NULL,   'f'},
        {"video_path",   		required_argument,  NULL,   'v'},
        {"IIV_window_size",   	required_argument,  NULL,   's'},
        {"dst_x_scale",   	optional_argument,  NULL,   'm'},
        {"dst_Y_scale",   	optional_argument,  NULL,   'n'},
        {"dst_Y_width",   	optional_argument,  NULL,   'x'},
        {"dst_Y_height",   	optional_argument,  NULL,   'y'},
        {"help",    no_argument,        NULL,   'h'},
        {0, 0, 0, 0}
};

static void usage(const char *name)
{
    int i = 0;
    fprintf(stdout, "%s\n\n", name);
    fprintf(stdout, "usage: %s [OPTIONS]\n\n", name);
    fprintf(stdout, "Program for testing IIV algorithm.\n\n");

    fprintf(stdout, "  -%c (--%s) specify experiment method.(0: directly compare the result of IIV and opencv Lanczos4; 1: write video then compare; 2: multi thread play IIV result.)\n",
            long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout, "  -%c (--%s) specify test video path.\n",
            long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout, "  -%c (--%s) specify test IIV window size.\n",
            long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout, "  -%c (--%s) specify test resize x scale.\n",
            long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout, "  -%c (--%s) specify test resize y scale.\n",
            long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout, "  -%c (--%s) specify test target width.\n",
            long_opts[i].val, long_opts[i].name);
    i++;    
    fprintf(stdout, "  -%c (--%s) specify test target height.\n",
            long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout, "  -%c (--%s) print this help.\n",
            long_opts[i].val, long_opts[i].name);
    i++;
}

int main(int argc, char **argv){
	int cmd_opt;
    int function = 0;
    std::string vid_path;
    char *videoPath = NULL;
    int windowsize = 8;
    double xScale, yScale;
    cv::Size dsize(0,0);
    while((cmd_opt = getopt_long(argc, argv, "hf:v:s:m:n:x:y:", long_opts, NULL)) != -1)
    {
        switch (cmd_opt) {
        case 'f':
        	function = atoll(optarg);
            break;
        case 'v':
        	videoPath = strdup(optarg);
            break;
        case 's':
        	windowsize = atoll(optarg);
            break;
        case 'm':
        	xScale = atof(optarg);
            break;
        case 'n':
        	yScale = atof(optarg);
            break;
        case 'x':
        	dsize.width = atoll(optarg);
            break;
        case 'y':
        	dsize.height = atoll(optarg);
            break;
        case 'h':
        default:
            usage(argv[0]);
            exit(0);
            break;
        }
    }

    vid_path = videoPath;
    switch(function){
        case 0:{             //normal compare
            cv::VideoCapture vid[EXP_ALGORITHM_NUM];
            for(int i = 0; i< EXP_ALGORITHM_NUM; i++)
                vid[i].open(videoPath);
            normalFunctionCopmpare(vid, dsize, xScale, yScale, windowsize);
            break;
        }
        case 1:{             //Write than Compare
            cv::VideoCapture vid[EXP_ALGORITHM_NUM];
            for(int i = 0; i< EXP_ALGORITHM_NUM; i++)
                vid[i].open(vid_path);
            compareAndWriteVideo(vid, dsize, xScale, yScale, windowsize);
            break;
        }
        case 2:{             //Multi thread play
            cv::VideoCapture vid;
            vid.open(vid_path);
            multiThreadPlay(vid, dsize, xScale, yScale, windowsize);
            break;
        }

    }
    // cv::VideoCapture vid[EXP_ALGORITHM_NUM];
    // for(int i = 0; i< EXP_ALGORITHM_NUM; i++)
        // vid[i].open("F:/Photograph/Games/romeoooo_highlight_19-10-31_21-57-51.mp4");
        // vid[i].open("F:/Photograph/micromouse/IMG_6792.mov");
        // vid[i].open("1_IIV.mp4");

    // compareAndWriteVideo(vid, 0.2, 0.2, 8);
    // normalFunctionCopmpare(vid, 0.2, 0.2, 8);
    // normalFunctionCopmpare(vid, 1.2, 1.2, 8);
    // testResultPlay();
    return 0;
}

