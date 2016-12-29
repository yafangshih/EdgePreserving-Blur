#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdio>

using namespace cv;
using namespace std;

int ksize = 3;

string type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

int main( int argc, char** argv )
{
	if( argc != 2){
     cout <<"*Error* Usage: EdgePreservedBlur [imgPath]" << endl;
     return -1;
    }

    Mat image;
    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   
    if(! image.data ){
        cout <<  "*Error* Could not open or find the image" << std::endl ;
        return -1;
    }

    Mat imageyuv;
    cvtColor(image, imageyuv, CV_BGR2YCrCb); // 8UC3

/*    
	string ty =  type2str( imageyuv.type() );
	printf("Matrix: %s %dx%d \n", ty.c_str(), imageyuv.cols, imageyuv.rows );
	return 0;
*/
    Mat imageLuma(image.rows, image.cols, CV_8UC1 );
    Mat UV( image.rows, image.cols, CV_8UC2 );
    Mat out[] = { imageLuma, UV };
    int from_to[] = {0,0, 1,1, 2,2};
    mixChannels(&imageyuv, 1, out, 2, from_to, 3);

    Mat localMax(imageLuma.rows, imageLuma.cols, CV_8UC1, Scalar(0) );
    Mat localMin(imageLuma.rows, imageLuma.cols, CV_8UC1, Scalar(0) );

/*
	Vec3b intensity = img.at<Vec3b>(y, x);
	uchar blue = intensity.val[0];
	uchar green = intensity.val[1];
	uchar red = intensity.val[2];
*/

    int padwid = ksize/2;
    Mat padimageLuma(imageLuma.rows+padwid*2, imageLuma.cols+padwid*2, CV_8UC1 );
    copyMakeBorder(imageLuma, padimageLuma, padwid, padwid, padwid, padwid, BORDER_REFLECT_101);
    
    for(int i=0;i<imageLuma.rows;i++){
    	for(int j=0;j<imageLuma.cols;j++){
    		uchar myself = imageLuma.at<uchar>(i, j);
    		int mcount = 0, Mcount = 0;

    		for(int y=-padwid;y<padwid;y++){
    			for(int x=-padwid;x<padwid;x++){
    				uchar neighbor = padimageLuma.at<uchar>(padwid+i +y, padwid+j +x);
    				if(myself > neighbor){++mcount;}
    				else if(neighbor > myself){++Mcount;}
    			}
    		}

    		if(Mcount <= ksize-1){localMax.at<uchar>(i, j) = uchar(255);}
    		if(mcount <= ksize-1){localMin.at<uchar>(i, j) = uchar(255);}

    	}
    }



/*    
	threshold(localMin, localMin, 1, 255, cv::THRESH_BINARY);
    namedWindow("Output", cv::WINDOW_AUTOSIZE);
    imshow("Output", localMin);
    waitKey(0);
*/


    return 0;


}