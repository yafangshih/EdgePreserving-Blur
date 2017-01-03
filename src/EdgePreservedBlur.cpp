#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdio>
#include <math.h>
#include <fstream>
#include <omp.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

int ksize = 5;
int N, M;

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

void getColorExact(Mat localm, Mat Y, Mat Img, int channel, char Mm){
    // 0-255, 1-255, 0-255
    // 0-255, 0-1, 0-1

    int** indsM = new int*[N];
    for(int i = 0; i < N; ++i) indsM[i] = new int[M];

    int c = 0;
    for(int i=0;i<N;i++){  
        for(int j=0;j<M;j++){
            indsM[i][j] = c;
            ++c;
        }
    } 

    int *lblInds = new int[N*M];
    for(int i=0;i<N*M; i++){lblInds[i] = -1;}

    c=0;
    
    int wd = 1;
    int *col_inds = new int[N*M * 3*3];
    int *row_inds = new int[N*M * 3*3];
    float *vals = new float[N*M * 3*3];

    float gvals[3*3];


    int consts_len = 0;
    int len = 0;

    for(int i=0;i<N;i++){
        for(int j=0;j<M;j++){
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

    for(int i=0;i<N;i++){
        for(int j=0;j<M;j++){
            if(localm.at<uchar>(i, j) == uchar(255)){ fileb << Img.at<float>(i, j) << endl; }
            else{ fileb << "0" << endl; }
        }
    }
    fileb.close();

}

void localMm(Mat& localMax, Mat& localMin, Mat Y){
    
    int padwid = ksize/2;
    Mat padimageLuma(N+padwid*2, M+padwid*2, CV_8UC1 );
    copyMakeBorder(Y, padimageLuma, padwid, padwid, padwid, padwid, BORDER_REFLECT_101);
    
    for(int i=0;i<N;i++){
        for(int j=0;j<M;j++){
            uchar myself = Y.at<uchar>(i, j);
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
}

struct MatchPathSeparator{
    bool operator()( char ch ) const {
        return ch == '/';
    }
};
string basename( string const& pathname ){
    return string( 
        find_if( pathname.rbegin(), pathname.rend(), MatchPathSeparator() ).base(), pathname.end() );
}


int main( int argc, char** argv ){

	if( argc != 3){
     cout <<"*Error* Usage: EdgePreservedBlur [imgPath] ksize" << endl;
     return -1;
    }

    Mat image;
    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   
    if(! image.data ){
        cout <<  "*Error* Could not open or find the image" << std::endl ;
        return -1;
    }

    ksize = (int)(strtol(argv[2], NULL, 10));

    N = image.rows;
    M = image.cols;

    Mat imageyuv;
    cvtColor(image, imageyuv, CV_BGR2YCrCb); // 8UC3

    Mat YUV[3];   
    split(imageyuv, YUV);

    Mat localMax(N, M, CV_8UC1, Scalar(0) );
    Mat localMin(N, M, CV_8UC1, Scalar(0) );
    localMm(localMax, localMin, YUV[0]);

    YUV[0].convertTo(YUV[0], CV_32FC1);  
    YUV[0] = YUV[0]/255.0;

    Mat bgr[3];   
    split(image, bgr);

    ofstream filedim;
    filedim.open ("dim.txt");
    filedim << basename(argv[1]) << " " << N*M << " " << N << " " << M << " " << ksize << endl;
    filedim.close();
    
    #pragma omp parallel for
    for(int i=0;i<3;i++){
        bgr[i].convertTo(bgr[i], CV_32FC1);  
        bgr[i] = bgr[i]/255.0; 
        getColorExact(localMax, YUV[0], bgr[i], i, 'M');
        getColorExact(localMin, YUV[0], bgr[i], i, 'm');
    }

    return 0;


}
