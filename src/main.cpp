#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>
#include <fstream>

using namespace cv;
using namespace std;
using namespace Eigen;

class EdgePreservingBlur{
private:
    Mat image;
    int H, W, k;
    Mat luminance, localMax, localMin, outputImg;
    float EPS = 0.000002, ALPHA  = 0.1888;

    void computeLocalExtrema();
    void computeE();
    Mat getColorExact(const Mat, const Mat);

public:
    EdgePreservingBlur(Mat, int);
    Mat getOutputImg();
};

EdgePreservingBlur::EdgePreservingBlur(Mat _image, int _k){
    image = _image;
    k = _k;
    H = image.rows;
    W = image.cols;
    luminance = Mat(H, W, CV_8UC1);
    localMax = Mat(H, W, CV_8UC1, Scalar(0));
    localMin = Mat(H, W, CV_8UC1, Scalar(0));
    outputImg = Mat(H, W, CV_8UC3);

    Mat imageyuv;
    cvtColor(image, imageyuv, CV_BGR2YCrCb); 
    extractChannel(imageyuv, luminance, 0);

    computeLocalExtrema();
    computeE();
}
void EdgePreservingBlur::computeLocalExtrema(){
    int halfk = k/2.;
    Mat padLuminance(H + halfk*2, W + halfk*2, CV_8UC1 );
    copyMakeBorder(luminance, padLuminance, halfk, halfk, halfk, halfk, BORDER_REFLECT_101);

    for(int y=0; y<H; y++){
        for(int x=0; x<W; x++){
            uchar center = luminance.at<uchar>(y, x);
            int mcount = 0, Mcount = 0;

            for(int dy=-halfk; dy<=halfk; dy++){
                for(int dx=-halfk; dx<=halfk; dx++){
                    uchar neighbor = padLuminance.at<uchar>(y+halfk+dy, x+halfk+dx);
                    if(center > neighbor){++mcount;}
                    else if(neighbor > center){++Mcount;}
                }
            }
 
            // Pixel p is reported as a maxima (resp. minima) 
            // if at most k − 1 elements in the k × k neighborhood around p are greater (resp. smaller) 
            // than the value at pixel p
            if(Mcount <= k-1){localMax.at<uchar>(y, x) = uchar(255);}
            if(mcount <= k-1){localMin.at<uchar>(y, x) = uchar(255);}
        }
    }
}
void EdgePreservingBlur::computeE(){
    luminance.convertTo(luminance, CV_32FC1); 
    image.convertTo(image, CV_32FC3);

    double minVal = 0, maxVal = 0;
    minMaxLoc(luminance, &minVal, &maxVal); 
    if(maxVal > 1.){ luminance /= 255.; }
    minMaxLoc(image, &minVal, &maxVal); 
    if(maxVal > 1.){ image /= 255.; }

    Mat bgr[3];   
    split(image, bgr);

    Mat Emaxima(H, W, CV_32FC1);
    Mat Eminima(H, W, CV_32FC1);
    Mat EMean(H, W, CV_32FC1);
    
    vector<Mat> channels;

    for(int c=0; c<3; c++){
        Emaxima = getColorExact(localMax, bgr[c]);
        Eminima = getColorExact(localMin, bgr[c]);
        EMean = (Emaxima + Eminima) / 2.;
        EMean.convertTo(bgr[c], CV_8UC1); 
        channels.push_back(bgr[c]);
    }
    merge(channels, outputImg);
}
Mat EdgePreservingBlur::getColorExact(const Mat localm, const Mat img){
    
    int halfk = k / 2.;
    int *colIndex = new int[H*W * k*k];
    int *rowIndex = new int[H*W * k*k];
    float *vals = new float[H*W * k*k];

    float *neighborhoodVals = new float[k*k]; // [neighbor0, neighbor1, ..., center]

    int rowptr = 0;
    int numPixels = 0;

    for(int y=0; y<H; y++){
        for(int x=0; x<W; x++){
            // if not a local extrema
            if(localm.at<uchar>(y, x) == uchar(0)){
                int Nr = 0;
                for(int neighborY=max(0, y-halfk); neighborY<=min(y+halfk, H-1); neighborY++){
                    for(int neighborX=max(0, x-halfk); neighborX<=min(x+halfk, W-1); neighborX++){
                        if((neighborY != y) || (neighborX != x)){
                            rowIndex[numPixels] = rowptr;
                            colIndex[numPixels] = (neighborY * W) + neighborX; 
                            neighborhoodVals[Nr] = luminance.at<float>(neighborY, neighborX);
                            numPixels++;
                            Nr++;
                        }
                    }
                }
                float center = luminance.at<float>(y, x);
                neighborhoodVals[Nr] = center;

                // Var(X) = E[(X-avg)^2]

                // local average around r
                float sum = 0.;
                for(int i=0; i<=Nr; i++){
                   sum += neighborhoodVals[i];
                }
                float average = sum / (Nr+1);

                // local variance around r
                float *dist = new float[k*k]();
                for(int i=0; i<=Nr; i++){
                   dist[i] = neighborhoodVals[i] - average;
                   dist[i] = dist[i] * dist[i];
                }
                sum = 0.;
                for(int i=0; i<=Nr; i++){
                   sum += dist[i];
                }
                float variance = sum / (Nr+1);

                // numerator of equation (2) : (I(r) - I(s)) ^ 2
                float *similarity = new float[k*k]();
                for(int i=0; i<Nr; i++){
                   similarity[i] = neighborhoodVals[i] - center; 
                   similarity[i] = similarity[i] * similarity[i];
                }
                // equation (2)
                float wrs[k*k];
                for(int i=0; i<Nr; i++){
                    wrs[i] = ALPHA * exp(-(similarity[i] / (2 * variance + EPS))); 
                }
                // wrs is a weighting function that sums to one [Levin et al., 2004]
                sum = 0.;
                for(int i=0; i<Nr; i++){
                   sum += wrs[i];
                }
                for(int i=0; i<Nr; i++){
                   wrs[i] = wrs[i] / sum;
                }
                for(int i=0; i<Nr; i++){
                    vals[ numPixels-Nr+i ] = -wrs[i];
                }
            }
            rowIndex[numPixels] = rowptr;
            colIndex[numPixels] = (y * W) + x;  
            vals[numPixels] = 1;
            
            numPixels++; 
            rowptr++;
        }
    }

    SparseMatrix<float> A(rowptr, H*W);
    VectorXf b = VectorXf::Zero(rowptr);
    VectorXf output(H*W);

    // Initialize triplets
    std::vector<Eigen::Triplet<float> > tripletList;
    tripletList.reserve(numPixels);
    for(int i=0; i<numPixels; i++){
        tripletList.push_back(Eigen::Triplet<float>(rowIndex[i], colIndex[i], vals[i]));
    }

    // Initialize A and b
    A.setFromTriplets(tripletList.begin(), tripletList.end());    
    for(int y=0; y<H; y++){
        for(int x=0; x<W; x++){
            if(localm.at<uchar>(y, x) == uchar(255)){ b(y*W + x) = img.at<float>(y, x); }
        }
    }

    // Solve Ax=b
    LeastSquaresConjugateGradient<SparseMatrix<float> > lscg;
    lscg.compute(A);
    output = lscg.solve(b);

    Mat out(H, W, CV_32FC1);
    for(int y=0; y<H; y++){
         for(int x=0; x<W; x++){
            out.at<float>(y, x) = output[y*W + x] * 255.;
        }
    }
    return out;
}
Mat EdgePreservingBlur::getOutputImg(){
    return outputImg;
}

int main( int argc, char** argv ){
	if( argc != 3){
     cout <<"Usage: EdgePreservingBlur [imgPath] [k]" << endl;
     return -1;
    }

    Mat image;
    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   
    if(! image.data ){
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    int k = atoi(argv[2]);
    EdgePreservingBlur BlurImg(image, k);
    imwrite("data/output/result-" + to_string(k) + ".png", BlurImg.getOutputImg());

    return 0;
}
