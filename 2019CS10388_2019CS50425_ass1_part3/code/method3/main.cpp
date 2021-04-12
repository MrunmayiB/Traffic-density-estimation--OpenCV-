#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <algorithm>
#include <opencv2/opencv.hpp>
#define pb push_back
#define ff float
#define dd double

using namespace std;
using namespace cv;
const uchar thersh = 15;
string s;

inline dd getWhitePixelsDenisty(cv::Mat &img)
{
    int cnt=0;
    for(int i=0;i<img.rows;i++)
    {
        for(int j=0;j<img.cols;j++)
        {
            if(img.at<uchar>(i,j)>=thersh)
            {
                cnt++;
            }
        }
    }
    dd cn = cnt;
    dd sr = img.rows*img.cols;
    cn = cn/sr;
    return cn;
}

void setPoints(vector<cv::Point2f> &dest_points, vector<cv::Point2f> &b)
{
    cv::Point2f p(472,52);
    dest_points.pb(p);
    p.y = 830;
    dest_points.pb(p);
    p.x = 800;
    dest_points.pb(p);
    p.y = 52;
    dest_points.pb(p);
    p.x = 974;p.y = 217;
    b.pb(p);
    p.x =509; p.y = 1064;
    b.pb(p);
    p.x =1548 ;p.y=1070;
    b.pb(p);
    p.x=1266;p.y= 214;
    b.pb(p);
}
void printHelp()
{
   cout<<"Help Message: \n";
   cout<<"Pass arguments with the video file, either a relative or an absolute path\n"<<"./<yourexecutable> <image>\n"<<"Example\t"<<"./main vidoe.mp4\tor\t./main path/to/video.mp4\n";
   /*cout<<"Once Program runs, Select 4 points to get the projected image of the original image."<<endl;*/
   cout<<"To see this message, use- ./<yourexecutable> help"<<endl;
}

int X;


void processPartImage(Mat &img,Ptr<BackgroundSubtractor> backSub,int &cnt)
{
    Mat backgroundremovedNewFrame;
    backSub -> apply(img,backgroundremovedNewFrame,0.0);
    cnt=0;
    for(int i=0;i<backgroundremovedNewFrame.rows;i++)
    {
        for(int j=0;j<backgroundremovedNewFrame.cols;j++)
        {
            if(backgroundremovedNewFrame.at<uchar>(i,j)>=thersh)
            {
                cnt++;
            }
        }
    }
}


#define pow2(x) ((x)*(x))

dd getUtiliyMesasure(vector<dd> &density)
{
    ifstream fin("baseline.txt");
    vector<int> numOf;
    vector<dd> desn(density.size(),-1);
    int t;
    dd cnt=0;
    dd cc=0;
    while(fin>>t)
    {
        numOf.push_back(t);
        dd ti;
        fin>>ti;
        desn[t-1]=ti;
    }
    for(int i=0;i<density.size();i++)
    {
        if(density[i]!=-1)
        {
            cc += pow2(density[i]-desn[i]);
            cnt += 1;
        }
    }
    cc = cc/cnt;
    cc = sqrt(cc);
    return cc;
}

int main(int argc, const char * argv[])
{
    clock_t timeMeasure;
    timeMeasure = clock();
    auto clockStart = chrono::high_resolution_clock::now();
    X = 1;
    if(argc>2)
    {
        s = argv[1];
        if(s == "help")
        {
            printHelp();
            return 0;
        }
        string to;
        X = atoi(argv[2]);
        cout<<"Number of Threads: "<<X<<endl;
    }
    else
    {
        printHelp();
        return 0;
    }
    cv::VideoCapture cap(s);
    if(!cap.isOpened())
    {
        cout<<"Error in Video or Path"<<endl;
        printHelp();
        exit(-1);
    }
    dd FPS = cap.get(CAP_PROP_FPS);
    
    cv::Mat firstFrameT,firstFrame;
    cap>>firstFrameT;
    if(firstFrameT.empty())
    {
        cout<<"Error in Video"<<endl;
        printHelp();
        exit(-1);
    }
    cv::cvtColor(firstFrameT,firstFrame,cv::COLOR_BGR2GRAY);
    vector<cv::Point2f> dest_points;
    vector<cv::Point2f> src_points;
    setPoints(dest_points,src_points);
    cv::Mat oldFrameCropped;
    cv::Mat H = cv::findHomography(src_points,dest_points);
    cv::Mat warpedimg;
    cv::warpPerspective(firstFrame,warpedimg ,H,cv::Size(1280,875));
    cv::Rect cropedPart(472,52,328,778);
    oldFrameCropped = warpedimg(cropedPart);
    cout<<"Size of Image: "<<oldFrameCropped.rows<<" "<<oldFrameCropped.cols<<endl;
    // Size of Image: 778 328
    int maxFrameNo = cap.get(CAP_PROP_FRAME_COUNT);
    cout<<maxFrameNo<<endl;
    cv::Mat background = cv::imread("empty_traffic_cropped.jpg",cv::IMREAD_GRAYSCALE);
    int len_p = 778/X;
    int rem_p = 778%X;
    
    vector<Mat> backgroundParts(X);
    vector<Ptr<BackgroundSubtractor>> backSubParts(X);
    vector<Mat> backMaskParts(X);
    int startLr=0;
    vector<Rect> imgBreaker(X);
    for(int i=0;i<X;i++)
    {
        int heightOfPart=len_p;
        if(i<rem_p)
            heightOfPart++;
        Rect breakPart(0,startLr,328,heightOfPart);
        imgBreaker[i] = breakPart;
        startLr+=heightOfPart;
        backgroundParts[i] = background(breakPart);
        backSubParts[i] = createBackgroundSubtractorMOG2(1,60,false);
        backSubParts[i] -> apply(backgroundParts[i],backMaskParts[i],1.0);
    }
    vector<dd> density(maxFrameNo,-1);
    int FrameNo=2;
    while(true)
    {
        Mat newFrameT;
        cap.read(newFrameT);
        if(newFrameT.empty())
            break;
        if(FrameNo%5!=2)
        {
            FrameNo++;
            continue;
        }
        cv::Rect cropedPart(472,52,328,778);
        Mat newFrame;
        cv::cvtColor(newFrameT, newFrame, cv::COLOR_BGR2GRAY);
        cv::Mat warpedFrame;
        cv::warpPerspective(newFrame,warpedFrame,H,cv::Size(1280,875));
        cv::Mat newFrameCropped = warpedFrame(cropedPart);
        vector<Mat> croppedFrameParts(X);
        vector<thread> thr;
        vector<int> cntt(X);
        for(int i=0;i<X;i++)
        {
            croppedFrameParts[i] = newFrameCropped(imgBreaker[i]);
            thr.pb(thread(processPartImage,ref(croppedFrameParts[i]),backSubParts[i],ref(cntt[i])));
        }
        for(int i=0;i<X;i++)
            if(thr[i].joinable())
                thr[i].join();
        int cnt=0;
        for(auto x : cntt)
            cnt+=x;
        dd cn = cnt;
        dd sr = newFrameCropped.rows*newFrameCropped.cols;
        cn = cn/sr;
        density[FrameNo-1] = cn;
        FrameNo++;
    }
    auto endClock = chrono::high_resolution_clock::now();
    timeMeasure = clock() - timeMeasure;
    dd timeTaken;
    timeTaken = ((dd)timeMeasure)/CLOCKS_PER_SEC;
    auto timeChrono = chrono::duration_cast<std::chrono::microseconds>(endClock-clockStart);
    cout<<"Time in MicroSeconds : "<<timeChrono.count()<<endl;
    cout<<"Static Error : ,"<<getUtiliyMesasure(density)<<endl;
    for(int i=0;i<density.size();i++)
    {
        if(density[i]!=-1)
        {
            dd ti;
            ti = (dd)(i+1)/FPS;
            cout<<ti<<","<<density[i]<<endl;
        }
    }
    return 0;
}


