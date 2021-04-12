#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <opencv2/opencv.hpp>
#define pb push_back
#define ff float
using namespace std;

int cnt;
vector<cv::Point2f> src_points;
cv::Mat cpyImg;
string s;
void getCoordinates(int event, int x,int y, int flags, void* userdata)
{
    if(event==cv::EVENT_LBUTTONDOWN)
    {
        cnt++;
        if(cnt<=4)
        {
            cv::Point2f p(x,y);
            src_points.pb(p);
            cv::circle(cpyImg,cv::Point(x,y),10,cv::Scalar(0),cv::FILLED);
            cv::imshow("Original Image",cpyImg);
        }
    }
}

void printHelp()
{
   cout<<"Help Message: \n";
   cout<<"Pass arguments with the image file, either a relative or an absolute path\n"<<"./<yourexecutable> <image>\n"<<"Example\t"<<"./main image.jpg\tor\t./main path/to/image.jpg\n";
   cout<<"Once Program runs, Select 4 points to get the projected image of the original image."<<endl;
   cout<<"To see this message, use- ./<yourexecutable> help"<<endl;
}


int main(int argc, const char * argv[]) {
    // insert code here...
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
    cv::Mat img = cv::imread(s,cv::IMREAD_GRAYSCALE);
    if(!img.data)
    {
        // File Not Found or Image not Loaded
        cout<<"Please check your image or path to image"<<endl;
        printHelp();
        return 0;
    }
    cpyImg = img.clone();
    cv::namedWindow("Original Image",cv::WINDOW_AUTOSIZE);
    cv::imshow("Original Image",cpyImg);
    cv::setMouseCallback("Original Image",getCoordinates);
    cv::waitKey(0);
    cv::destroyAllWindows();
    vector<cv::Point2f> dest_points;
    cv::Point2f p(472,52);
    dest_points.pb(p);
    p.y = 830;
    dest_points.pb(p);
    p.x = 800;
    dest_points.pb(p);
    p.y = 52;
    dest_points.pb(p);
//    for(auto x : src_points)
//    {
//        cout<<x.x<<" "<<x.y<<endl;
//    }
    cv::Mat H;
    if(src_points.size()==4)
    {
        H = cv::findHomography(src_points,dest_points);
    }
    else
    {
        cout<<"Please Click Exactly 4 points in counterclockwise direction"<<endl;
        return 0;
    }
    cv::Mat warpedimg;
    cv::warpPerspective(img,warpedimg ,H,cv::Size(1280,875));
    cv::namedWindow("Corrected Image",cv::WINDOW_AUTOSIZE);
    cv::imshow("Corrected Image",warpedimg);
    cv::waitKey(0);
    cv::destroyAllWindows();
    cv::Rect cropedPart(472,52,328,778);
    cv::Mat finalImg = warpedimg(cropedPart);
    cv::namedWindow("Cropped Image",cv::WINDOW_AUTOSIZE);
    cv::imshow("Cropped Image",finalImg);
    cv::waitKey(0);
    cv::destroyAllWindows();
    int pos=-1;
    for(auto i=s.size()-1;i>=0;i--)
    {
        if(s[i]=='.')
        {
            pos=(int)i;
            break;
        }
        if(s[i]=='/')
            break;
    }
    string f1,f2;
    if(pos!=-1)
    {
        string gh = s.substr(0,pos);
        f1 = gh + "_projected.jpg";
        f2 = gh +"_cropped.jpg";
    }
    else
    {
        f1 = s + "_projected.jpg";
        f2 = s + "_cropped.jpg";
    }
    cv::imwrite(f1,warpedimg);
    cv::imwrite(f2,finalImg);
//    cv::perspectiveTransform(img,warpedimg,H);
//    cv::imshow("Corrected Image",warpedimg);
//    cv::waitKey(0);
    return 0;
}



