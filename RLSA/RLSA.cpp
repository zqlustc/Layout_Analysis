#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp> 
#include <vector>
#include <string>

using namespace std;
using namespace cv;	

void printFunc(Mat input){
	for(int row = 0;row < input.rows;row++){
		for(int col = 0;col < input.cols;col++){
			int value = input.at<uchar>(row,col);
			cout<<value<<" ";
		}
		cout<<endl;
	}
}

Mat loadImage(string filepath){
	Mat image = imread(filepath);
	
	if (image.empty()) {
        cout<<"read image failed"<<endl;
        return Mat();
    }

    cvtColor(image, image, CV_BGR2RGB);
    return image;
}

Mat convert2gray(Mat input){
	Mat grayImage( input.size(), CV_8UC1 );
    cvtColor ( input, grayImage, CV_RGB2GRAY );
    return grayImage;
}

Mat binaryzation(Mat grayImage){
	if(grayImage.channels() != 1){
		cout<<"binarylize must be the gray image"<<endl;
		return Mat();
	}
	Mat binaryImage;

	threshold(grayImage,binaryImage,125,255,THRESH_BINARY);
	return binaryImage;
}

Mat digStorage(Mat binaryImage){
	Mat temp = ~binaryImage;
	Mat digitImage = (1/255.0) * temp;
	return digitImage;
}

Mat lengthSmoothHor(Mat digitImage , int thresholdC){
	Mat digitImageHor = digitImage.clone();
	for(int row = 0;row < digitImageHor.rows;row++){
		int cursor1 = 0;
		int cursor2 = 0;
		for(int col = 0;col < digitImageHor.cols;){
			if(digitImageHor.at<uchar>(row,cursor1) == 0){
				cursor2 = cursor1 + 1;
				while(((int)digitImageHor.at<uchar>(row,cursor2) != 1) && (cursor2 < digitImageHor.cols)){
					cursor2 ++;
				}
				if((cursor2 - cursor1) <= thresholdC){
					while(cursor1 < cursor2){
						digitImageHor.at<uchar>(row,cursor1) = 1;
						cursor1 ++;
					}
				}
				cursor1 = cursor2;
			}else{
				cursor1++;
				cursor2++;
			}
			col = cursor2;
		}
	}
	return digitImageHor;
}

Mat lengthSmoothVer(Mat digitImage,int thresholdC){
	Mat digitImageVer = digitImage.clone();
	for(int col = 0;col < digitImageVer.cols;col++){
		// cout<<"Now process Col-"<<col<<endl;
		int cursor1 = 0;
		int cursor2 = 0;
		for(int row = 0;row < digitImageVer.rows;){
			if((int)digitImageVer.at<uchar>(cursor1,col) == 0){
				cursor2 = cursor1 + 1;
				while(((int)digitImageVer.at<uchar>(cursor2,col) != 1) && (cursor2 < digitImageVer.rows)){
					cursor2 ++;
				}
				if((cursor2 - cursor1) <= thresholdC){
					while(cursor1 < cursor2){
						digitImageVer.at<uchar>(cursor1,col) = 1;
						cursor1 ++;
					}
				}
				cursor1 = cursor2;
			}else{
				cursor2++;
				cursor1++;
			}
			row = cursor2;
		}
	}
	return digitImageVer;
}

Mat doDilation(Mat smoothImage,int times){
	Mat dilateImage;
	Mat element = getStructuringElement(MORPH_RECT,Size(3,3));
	int index = 0;
	Mat temp = smoothImage.clone();
	while(index < times){
		dilate(temp,temp,element);
		index++;
	}
	// dilate(smoothImage,dilateImage,element);
	dilateImage = temp.clone();
	return dilateImage;
}

// this function need too much memory storage 
Mat plotRect(Mat& input,std::vector<Rect> inputRects){
	Mat rectImage = Mat::zeros(input.size(),CV_8UC1);
	for(std::vector<Rect>::const_iterator it1 = inputRects.begin();it1 != inputRects.end();it1++){
		for(int row = it1->tl().y;row <= it1->br().y;row++){
			for(int col = it1->tl().x;col <= it1->br().x;col++){
				rectImage.at<uchar>(row,col) = 255;
			}
		}
	}
	return rectImage;
}

// so we can change it with this one 
Mat getBlock(Mat& input,std::vector<Rect> inputRects){
	Mat blockImage = input.clone();
	for(std::vector<Rect>::const_iterator it = inputRects.begin();it != inputRects.end();it++){
		rectangle(blockImage,it->tl(),it->br(),cv::Scalar(0,0,255),1,1,0);
	}
	return blockImage;
}

std::vector<Rect> largestComp(Mat dilateImage){
	std::vector<std::vector<Point>> contours;
	findContours( dilateImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE );
	// cout<<"the contours' size is: "<<contours.size()<<endl;
	std::vector<Rect> boundingRects;
	boundingRects.reserve(contours.size());

	for(std::vector<std::vector<Point>>::const_iterator it = contours.begin();it != contours.end();it++){
		Rect bRect = boundingRect(*it);
		boundingRects.push_back(bRect);
	}
	return boundingRects;
}

std::vector<Mat> cropImage(Mat& input,std::vector<Rect>& cropRects){
	std::vector<Mat> cropImages;
	cropImages.reserve(cropRects.size());

	for(std::vector<Rect>::const_iterator itr = cropRects.begin();itr != cropRects.end();itr++){
		Mat cutImage = Mat(input,*itr);
		cropImages.push_back(cutImage);
	}

	return cropImages;
}

int main(int argc, char ** argv){

	if(argc != 2){
		cout<<"usage: ./RLSA XXX.jpg"<<endl;
		return -1;
	}

	// load the input image
	string filepath = argv[1];
	Mat input = loadImage("sample/" + filepath);
	Mat inputCopy = input.clone();
	// convert to gray-scale
	Mat grayImage = convert2gray(input);
	// binaryzation
	Mat binaryImage = binaryzation(grayImage);
	// change the binary-image to 0&1 pixel
	Mat digitImage = digStorage(binaryImage);
	// RSLA via horizon and vertical
	Mat afterSmoothHor = lengthSmoothHor(digitImage,200);
	Mat afterSmoothVer = lengthSmoothVer(digitImage,200);
	// AND operation
	Mat afterSmooth = afterSmoothVer & afterSmoothHor;
	// dilation operation 4 times with the kernal(3,3)
	// paper suggest the times is 2
	Mat dilateImage = doDilation(afterSmooth,5);

	Mat afterSmooth2show = 255 * afterSmooth;
	Mat dilateImage2show = 255 * dilateImage;
	std::vector<Rect> componentRects = largestComp(dilateImage);
	std::vector<Mat> cutImages = cropImage(inputCopy,componentRects);

	
	int index = 0;
	for(std::vector<Mat>::const_iterator itm = cutImages.begin();itm != cutImages.end();itm++){
		imwrite("result/" + to_string(index) + ".jpg",*itm);
		index++;
	}

	cout<<"generate the result..."<<endl;
	Mat result = getBlock(input,componentRects);
	Mat stepResult(2 * input.rows + 10,2 * input.cols + 10,CV_8UC1);
	grayImage.copyTo(stepResult(Rect(0,0,grayImage.cols,grayImage.rows)));
	afterSmooth2show.copyTo(stepResult(Rect(input.cols + 10,0,afterSmooth.cols,afterSmooth.rows)));
	dilateImage2show.copyTo(stepResult(Rect(0,input.rows + 10,dilateImage.cols,dilateImage.rows)));
	cvtColor(stepResult,stepResult,CV_GRAY2RGB);
	result.copyTo(stepResult(Rect(input.cols + 10,input.rows + 10,result.cols,result.rows)));
	imwrite("result/temp.jpg",stepResult);
	cout<<"done!"<<endl;
}
