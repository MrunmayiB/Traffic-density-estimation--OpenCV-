#include <iostream>
#include <vector>
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

dd getWhitePixelsDenisty(cv::Mat img)
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
int main(int argc, const char * argv[])
{
    if(argc>1)
    {
        s = argv[1];
        if(s == "help")
        {
            printHelp();
            return 0;
        }
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
    cv::Mat background = cv::imread("empty_traffic_cropped.jpg",cv::IMREAD_GRAYSCALE);
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
    cv::Mat H;
    H = cv::findHomography(src_points,dest_points);
    cv::Mat warpedimg;
    cv::warpPerspective(firstFrame,warpedimg ,H,cv::Size(1280,875));
    cv::Rect cropedPart(472,52,328,778);
    oldFrameCropped = warpedimg(cropedPart);
    cv::Ptr<cv::BackgroundSubtractor> backSub;
    backSub = cv::createBackgroundSubtractorMOG2(1,60,false);
    cv::Mat backmask;
    backSub -> apply(background,backmask,1.0);
    int frameNo=1;
    while(true)
    {
        cv::Mat newFrameT,newFrame;
        cap>>newFrameT;
        if(newFrameT.empty())
            break;
        frameNo++;
        if(frameNo%5!=2)
            continue;
        cv::cvtColor(newFrameT, newFrame, cv::COLOR_BGR2GRAY);
        cv::Mat warpedFrame;
        cv::warpPerspective(newFrame,warpedFrame,H,cv::Size(1280,875));
        cv::Mat newFrameCropped = warpedFrame(cropedPart);
        cv::Mat backgroundremovedNewFrame;
        backSub -> apply(newFrameCropped,backgroundremovedNewFrame,0.0);
        dd tim = frameNo;
        tim = tim/FPS;
        cout<<tim<<",";
        cout<<getWhitePixelsDenisty(backgroundremovedNewFrame)<<",";
        Mat opticalFlow(oldFrameCropped.size(),CV_32FC2);
        calcOpticalFlowFarneback(oldFrameCropped,newFrameCropped, opticalFlow,0.5,3,15,3,7,1.5,0);
        oldFrameCropped = newFrameCropped;
        Mat opticalFlowParts[2];
        split(opticalFlow,opticalFlowParts);
        Mat distmag,norm_mag,angle_mag;
        cartToPolar(opticalFlowParts[0], opticalFlowParts[1], distmag, angle_mag);
        normalize(distmag,norm_mag,0.0f,1.0f,NORM_MINMAX);
        angle_mag *= ((1.f / 360.f) * (180.f / 255.f));
        Mat _hsv[3], hsv, hsv8, bgr;
        _hsv[0] = angle_mag;
        _hsv[1] = Mat::ones(angle_mag.size(), CV_32F);
        _hsv[2] = norm_mag;
        merge(_hsv, 3, hsv);
        hsv.convertTo(hsv8, CV_8U, 255.0);
        cvtColor(hsv8, bgr, COLOR_HSV2BGR);
        Mat finalOpticalFlowImg;
        cvtColor(bgr,finalOpticalFlowImg,COLOR_BGR2GRAY);
        cout<<getWhitePixelsDenisty(finalOpticalFlowImg)<<endl;
    }
    return 0;
}
