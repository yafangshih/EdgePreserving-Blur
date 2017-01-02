#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdio>
#include <math.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>
#include <fstream>

#define N 256
#define M 382

using namespace cv;
using namespace std;
using namespace Eigen;

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

//Mat getColorExact(Mat localm, Mat Y, Mat Img){
void getColorExact(Mat localm, Mat Y, Mat Img, int channel, char Mm){
    // 0-255, 1-255, 0-255
    // 0-255, 0-1, 0-1

//    int N = localm.rows, M = localm.cols; // 256, 382
/*
    double min, max;
    minMaxLoc(Img, &min, &max); 
    cout << "min " << min << ", max " << max << endl;
*/



//    int indsM[N][M];

    int** indsM = new int*[N];
    for(int i = 0; i < N; ++i) indsM[i] = new int[M];


    int c = 0;
    
    for(int j=0;j<M;j++){
        for(int i=0;i<N;i++){
            indsM[i][j] = c;
            ++c;
        }
    } 
/*    
    for(int i=0;i<6;i++){
        for(int j=0;j<5;j++){
            printf("%d ", indsM[i][j]); 
        }
        printf("\n");
    } 
*/
    int *lblInds = new int[N*M];
    for(int i=0;i<N*M; i++){lblInds[i] = -1;}

    c=0;
/*
    int d = 0;
    for(int j=0;j<M;j++){
        for(int i=0;i<N;i++){
            if(localm.at<uchar>(i, j) == uchar(255)){
                lblInds[c] = d; 
                
            }
            c++; 
            d++;
        }
    }
*/
//    printf("%d %d %d", c, lblInds[0], lblInds[c-1]);
//    return Y;
    
    int wd = 1;
    int *col_inds = new int[N*M * 3*3];
    int *row_inds = new int[N*M * 3*3];
    float *vals = new float[N*M * 3*3];

/*    
    int col_inds[N*M * 3*3];
    int row_inds[N*M * 3*3];
    int vals[N*M * 3*3];
*/
    float gvals[3*3];


    int consts_len = 0;
    int len = 0;

    for(int j=0;j<M;j++){
        for(int i=0;i<N;i++){
            
            if(localm.at<uchar>(i, j) == uchar(0)){
                
                int tlen = 0;
                for(int ii=max(0, i-wd); ii<=min(i+wd, N-1); ii++){
                    for(int jj=max(0, j-wd); jj<=min(j+wd, M-1); jj++){

                        if((ii!=i) || (jj!=j)){
                            row_inds[len] = consts_len;
                            col_inds[len] = indsM[ii][jj];
                            gvals[tlen] = Y.at<float>(ii, jj);
                            len++;
                            tlen++;
                            
                        }
                    }
                }
                
                float t_val = Y.at<float>(i, j);
                gvals[tlen] = t_val;

                float sum = 0;
                for(int k=0; k<=tlen; k++){
                   sum += gvals[k];
                }
                float avg = sum / (tlen+1);

                float tmp[3*3];
                for(int k=0; k<=tlen; k++){
                   tmp[k] = gvals[k] - avg;
                   tmp[k] = tmp[k]*tmp[k];
                }
                sum = 0;
                for(int k=0; k<=tlen; k++){
                   sum += tmp[k];
                }
                float c_var = sum / (tlen+1);

                float csig = c_var * 0.6;

                for(int k=0; k<tlen; k++){
                   tmp[k] = gvals[k] - t_val;
                   tmp[k] = tmp[k]*tmp[k];
                }
                float mgv = tmp[0];
                for(int k=0; k<tlen; k++){
                   if(tmp[k] < mgv){ mgv = tmp[k]; }
                }

                if( csig<(-mgv/log(0.01)) ){
                    csig = -mgv/log(0.01);
                }
                if( csig<0.000002 ){
                    csig = 0.000002;
                }
                
                for(int k=0; k<tlen; k++){
                    tmp[k] = -(tmp[k] / csig);
                    gvals[k] = exp(tmp[k]);
                }
                sum = 0;
                for(int k=0; k<tlen; k++){
                   sum += gvals[k];
                }
                for(int k=0; k<tlen; k++){
                   gvals[k] = gvals[k]/sum;
                }

                for(int k=0; k<tlen; k++){
                    vals[ len-tlen+k ] = -gvals[k];
                }
                
                
            }
            
            row_inds[len] = consts_len;
            col_inds[len] = indsM[i][j];
            vals[len] = 1;
            
            len++; 
            consts_len++;
            
            
        }
    }

    // ------------------------------------------------------------------------------------------------------------
    
    if(channel == 0 && (Mm == 'M')){
        ofstream filedim;
        filedim.open ("dim.txt");
        filedim << consts_len << " " << N << " " << M << endl;
        filedim.close();
    }

    char buffer [50];
    sprintf(buffer, "A%d%c.txt", channel, Mm);
    
    ofstream fileA;
    fileA.open (buffer);
    for(int k=0;k<len;k++){
        fileA << row_inds[k] << " " << col_inds[k] << " " << vals[k] << endl;
    }
    fileA.close();

    char buffer2 [50];
    sprintf(buffer2, "b%d%c.txt", channel, Mm);
    ofstream fileb;
    fileb.open (buffer2);
    c=0;
    for(int j=0;j<M;j++){
        for(int i=0;i<N;i++){
            if(localm.at<uchar>(i, j) == uchar(255)){ fileb << Img.at<float>(i, j) << endl; }
            else{ fileb << "0" << endl; }
        }
    }
    fileb.close();




    // ------------------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------------------------
    //return output;
//    */

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

    		for(int y=-padwid;y<=padwid;y++){
    			for(int x=-padwid;x<=padwid;x++){
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
    imwrite( "Max.png", localMax);
    imwrite("min.png", localMin);     

    namedWindow("Output", cv::WINDOW_AUTOSIZE);
    imshow("Output", localMax);
    waitKey(0);   
*/
    double min, max;
    minMaxLoc(imageLuma, &min, &max); 
    if(max > 1){ imageLuma.convertTo(imageLuma, CV_32FC1);  imageLuma = imageLuma/255.0; }

    Mat bgr[3];   
    split(image, bgr);

    Mat Emaxima(image.rows, image.cols, CV_32FC1 );
//    Mat Eminima(image.rows, image.cols, CV_32FC1 );
//    Mat EM(image.rows, image.cols, CV_32FC1 );
    
    for(int i=0;i<3;i++){
        minMaxLoc(bgr[i], &min, &max); 
        if(max > 1){ bgr[i].convertTo(bgr[i], CV_32FC1);  bgr[i] = bgr[i]/255.0; }
    //    Emaxima = getColorExact(localMax, imageLuma, bgr[0]);
        getColorExact(localMax, imageLuma, bgr[i], i, 'M');
        getColorExact(localMin, imageLuma, bgr[i], i, 'm');
    }


/*    Eminima = getColorExact(localMin, imageLuma, bgr[0]);
    EM = (Emaxima + Eminima) / 2.0;
    EM.convertTo(bgr[0], CV_8UC1); 

    namedWindow("Output", cv::WINDOW_AUTOSIZE);
    imshow("Output", bgr[0]);
    waitKey(0);
*/


    return 0;


}