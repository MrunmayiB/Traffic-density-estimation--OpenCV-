#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <fstream>
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



void processFrame(int startFrame,int endFrame,string s,cv::Mat H,vector<dd> &dens)
{
    cv::Mat background = cv::imread("empty_traffic_cropped.jpg",cv::IMREAD_GRAYSCALE);
    cv::Ptr<cv::BackgroundSubtractor> backSub;
    backSub = cv::createBackgroundSubtractorMOG2(1,60,false);
    cv::Mat backmask;
    backSub -> apply(background,backmask,1.0);
    cv::VideoCapture cap(s);
    int maxFrame;
    maxFrame = cap.get(CAP_PROP_FRAME_COUNT);
    cap.set(CAP_PROP_POS_FRAMES,startFrame);
    while(startFrame<=maxFrame and startFrame<=endFrame)
    {
        cv::Mat newFrameT;
        cap.read(newFrameT);
       // cout<<startFrame<<endl;
        if(newFrameT.empty())
            break;
        if(startFrame%5!=2)
        {
            startFrame++;
            continue;
        }
        cv::Rect cropedPart(472,52,328,778);
        Mat newFrame;
        cv::cvtColor(newFrameT, newFrame, cv::COLOR_BGR2GRAY);
        cv::Mat warpedFrame;
        cv::warpPerspective(newFrame,warpedFrame,H,cv::Size(1280,875));
        cv::Mat newFrameCropped = warpedFrame(cropedPart);
        cv::Mat backgroundremovedNewFrame;
        backSub -> apply(newFrameCropped,backgroundremovedNewFrame,0.0);
        int cnt=0;
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
        dd cn = cnt;
        dd sr = backgroundremovedNewFrame.cols*backgroundremovedNewFrame.rows;
        cn = cn/sr;
        dens[startFrame-1] = cn;
        startFrame++;
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
    //cout<<"Size of Image: "<<oldFrameCropped.rows<<" "<<oldFrameCropped.cols<<endl;
    int maxFrameNo = cap.get(CAP_PROP_FRAME_COUNT);
    vector<dd> density(maxFrameNo,-1);
    vector<thread> thr;
    cout<<maxFrameNo<<endl;
    int numIn = maxFrameNo/X;
    int currFrame=2;
    for(int i=0;i<X-1;i++)
    {
        thr.pb(thread(processFrame,currFrame,currFrame+numIn-1,s,H,ref(density)));
        currFrame += numIn;
    }
    thr.pb(thread(processFrame,currFrame,maxFrameNo,s,H,ref(density)));
    for(int i=0;i<thr.size();i++)
        if(thr[i].joinable())
            thr[i].join();
    auto endClock = chrono::high_resolution_clock::now();
    timeMeasure = clock() - timeMeasure;
    dd timeTaken;
    timeTaken = ((dd)timeMeasure)/CLOCKS_PER_SEC;
    auto timeChrono = chrono::duration_cast<std::chrono::microseconds>(endClock-clockStart);
    cout<<"Time in MicroSeconds : ,"<<timeChrono.count()<<endl;
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


