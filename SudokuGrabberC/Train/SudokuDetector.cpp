// SudokuDetector.cpp : Defines the entry point for the console application.
//

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <stdlib.h>
#include <iostream>
#include "DigitReader.h"

#define FILENAME "Train/sudoku7.jpg"
#define ROWS 28
#define COLS 28
#define HIGH_ENDIAN 0
#define LOW_ENDIAN 1


CvMat* convertFormat(IplImage* piece,int digitX, int digitY, int digitWidth, int digitHeight){

	int blockNo = 14;
	CvMat* digitMat= cvCreateMat(1,COLS*ROWS,CV_32FC1);
//	cvNamedWindow("Demo1");
//	cvNamedWindow("Demo2");
	int offsetX = 0;
	int offsetY = 0;
	IplImage* temp = cvCreateImage(
		cvGetSize(piece),
		IPL_DEPTH_8U,
		1
	);
	IplImage* resizePic = cvCreateImage(
		cvSize(ROWS,COLS),
		IPL_DEPTH_8U,
		1
	);

	cvZero(temp);

	offsetX = digitX + digitWidth/2 - ((piece->width)+1)/2;
	offsetY = digitY + digitHeight/2 - ((piece->height)+1)/2;

	// Copy the digit to the center of the new image
	for(int i = 0; i<digitHeight; i++){
		int tempY = digitY + i - offsetY;
		uchar* ptr1 = (uchar*)(piece->imageData + (i+digitY)* piece->widthStep);
		uchar* ptr2 = (uchar*)(temp->imageData + tempY* temp->widthStep);
		for(int j = 0; j< digitWidth; j++){
			int tempX = digitX + j - offsetX;
			//printf("%d\n",ptr1[digitX+j]);
			ptr2[tempX] = ptr1[digitX+j];
		}
	}
//	cvShowImage("Demo2",piece);

	cvSetImageROI(temp,cvRect((digitX-offsetX-blockNo/2),(digitY-offsetY-blockNo/2),(digitWidth+blockNo),(digitHeight+blockNo)));

	cvResize(temp,resizePic);
//	cvSaveImage("hello.jpg",resizePic);
//	cvShowImage("Demo1",resizePic);
//	cvWaitKey(0);

	for(int i = 0 ;i<ROWS;i++){
		uchar* ptr = (uchar*)(resizePic->imageData + i*resizePic->widthStep);
		for(int j = 0; j<COLS; j++){
			digitMat->data.fl[i*COLS+j] = ptr[j];
		}
	}

	// Convert IplImage into CvMat

	return digitMat;
}
void findblob(IplImage* res){

	int count = 0;
	double max = -1;
	CvPoint maxPt;
	CvConnectedComp a = CvConnectedComp();
	CvConnectedComp* comp = &a;

	for(int y = 0; y<res->height;y++){

		uchar* row = (uchar*)(res->imageData + y* res->widthStep);
		for( int x = 0; x < res->width; x++){

			if(row[x]>=128){

				cvFloodFill(res,cvPoint(x,y),CV_RGB(0,0,64),cvScalarAll(0),cvScalarAll(0),comp);
				if(comp==NULL){
				}else{
					//printf("Not NULL\n");
					if((comp->area)>max){
						max = comp->area;
						maxPt = cvPoint(x,y);
					}
				}
			}

		}
	}

	cvFloodFill(res, maxPt, CV_RGB(255,255,255));

	for(int y = 0; y<res->height;y++){

		uchar* row = (uchar*)(res->imageData + y* res->widthStep);
		for( int x = 0; x < res->width; x++){
			if(row[x] ==64 && x!=maxPt.x && y!=maxPt.y){
				cvFloodFill(res,cvPoint(x,y),CV_RGB(0,0,0));
			}
		}
	}

}

IplImage* findCorners2(IplImage* res, CvPoint* cPoints){
	using namespace cv;
	using namespace std;

	int imgHeight = res -> height;
	int imgWidth = res->width;
	Mat sudoku = Mat(res);
	vector<KeyPoint> corners;
	cv::FAST(sudoku,corners,10,false);

	// reverse the map
	for(int y = 0; y<res->height;y++){
	uchar* row = (uchar*)(res->imageData + y* res->widthStep);
	for( int x = 0; x < res->width; x++){
		if(row[x]==0){
			row[x] = 255;
		} else{
			row[x] = 128;
		}
	}
}

	CvPoint topLeft;
	CvPoint topRight;
	CvPoint bottomLeft;
	CvPoint bottomRight;

	int disTL = INT_MAX;
	int disTR = INT_MAX;
	int disBL = INT_MAX;
	int disBR = INT_MAX;

	int end = corners.size();
	int distance = 5;

	int* valueMap;
	valueMap = (int *)malloc(end*(sizeof(int)));
	int count = 0;

	int** countMap;
	int** sumMap;

	/*
	printf("Number of corners: %d\n",end);
	countMap = (int** )calloc(end/4,(sizeof(int*)));
	for(int m = 0; m<end/4; m++){
		countMap[m] = (int*)calloc(2,sizeof(int));
	}

	sumMap = (int** )calloc(end/4,(sizeof(int*)));
	for(int m = 0; m<end/4; m++){
		sumMap[m] = (int*)calloc(2,sizeof(int));
	}
	for(int i = 0;i<= end -1;i++){
		KeyPoint keyPoint = corners.at(i);
		CvPoint temp = cvPoint(keyPoint.pt.x,keyPoint.pt.y);

		//printf("Point: %d %d\n",temp.x,temp.y);


		// Read through each point
		valueMap[i] = -1;
		if(i==0){

			valueMap[i] = 0;
			countMap[0][0] = i;
			countMap[0][1] = 1;

			sumMap[0][0] = temp.x;
			sumMap[0][1] = temp.y;
		//	cout << countMap[count][0] << " " << countMap[count][1] << endl;
			count++;

		}else{
			int j;
			for(j = i-1; j>=0; j--){
				KeyPoint currentPoint = corners.at(j);
				int cX = currentPoint.pt.x;
				int cY = currentPoint.pt.y;
				int xDis = abs(cX - temp.x);
				int yDis = abs(cY - temp.y);
				if((xDis<=distance) && ( yDis<=distance)){
					valueMap[i] = valueMap[j];
					for(int k = count; k>=0; k--){
						if(countMap[k][0] == valueMap[i]){
							countMap[k][1]++;
							sumMap[k][0]+=temp.x;
							sumMap[k][1]+=temp.y;
						//	cout << countMap[k][0] << " " << countMap[k][1] << endl;
							break;
						}
					}

					break;
				}
			}
			if(valueMap[i] == -1){
				valueMap[i] = i;
				countMap[count][0] = i;
				countMap[count][1] = 1;
				sumMap[count][0] = temp.x;
				sumMap[count][1] = temp.y;
			//	cout << countMap[count][0] << " " << countMap[count][1] << endl;
				count++;
			}
		}

	}

	*/
	count = corners.size();
	printf("Count: %d\n", count);

	// find the four corners
	for( int i = 0 ; i < count; i++){
		//printf("Count: %d; NO: %d\n", countMap[i][0], countMap[i][1]);

	//	int tempX = sumMap[i][0]/countMap[i][1];
	//	int tempY = sumMap[i][1]/countMap[i][1];

		int tempX = corners.at(i).pt.x;
		int tempY = corners.at(i).pt.y;
	//	cvLine(res,cvPoint(tempX,tempY),cvPoint(tempX+5,tempY+5),CV_RGB(0,0,0),3);
		int xL = tempX;
		int xR = imgWidth - tempX;
		int yT = tempY;
		int yB = imgHeight - tempY;

		if((tempX*tempX + tempY*tempY)<disTL){
			if(tempX-2<0){
				tempX = 0;
			}else{
				tempX = tempX-2;
			}
			if(tempY - 2 < 0){
				tempY = 0;
			}else{
				tempY = tempY - 2;
			}
			topLeft = cvPoint(tempX,tempY);
			disTL = tempX*tempX + tempY*tempY;
		}

		if((xR*xR + yT*yT)<disTR){

			if(tempX+2>imgWidth){
				tempX = imgWidth;
			}else{
				tempX = tempX+2;
			}
			if(tempY - 2 < 0){
				tempY = 0;
			}else{
				tempY = tempY - 2;
			}
			topRight = cvPoint(tempX,tempY);
			disTR = xR*xR + yT*yT;
		}

		if((xL*xL + yB*yB)<disBL){
			if(tempX-2<0){
				tempX = 0;
			}else{
				tempX = tempX-2;
			}
			if(tempY +2 > imgHeight){
				tempY = imgHeight;
			}else{
				tempY = tempY + 2;
			}
			bottomLeft = cvPoint(tempX,tempY);
			disBL = xL*xL + yB*yB;
		}

		if((xR*xR + yB*yB)<disBR){
			if(tempX+2>imgWidth){
				tempX = imgWidth;
			}else{
				tempX = tempX+4;
			}
			if(tempY +2 > imgHeight){
				tempY = imgHeight;
			}else{
				tempY = tempY + 4;
			}
			bottomRight = cvPoint(tempX,tempY);
			disBR = xR*xR + yB*yB;
		}


	}
	/*
	cvNamedWindow("RES");
	cvShowImage("RES",res);
	cvWaitKey(0);
	*/

	cPoints[0] = topLeft;
	cPoints[1] = topRight;
	cPoints[2] = bottomRight;
	cPoints[3] = bottomLeft;

	cvLine( res, topLeft, bottomLeft, CV_RGB(0,0,0),1);
	cvLine( res, topLeft, topRight, CV_RGB(0,0,0),1);
	cvLine( res, bottomRight, bottomLeft, CV_RGB(0,0,0),1);
	cvLine( res, bottomRight, topRight, CV_RGB(0,0,0),1);



	// Code from AI Shack
	int maxLength = (topLeft.x - topRight.x)*(topLeft.x - topRight.x) + (topLeft.y - topRight.y)*(topLeft.y - topRight.y);
	int tempLength = (topLeft.x - bottomLeft.x)*(topLeft.x - bottomLeft.x) + (topLeft.y - bottomLeft.y)*(topLeft.y - bottomLeft.y);
	if(tempLength>maxLength){
		maxLength = tempLength;
	}
	tempLength = (bottomRight.x - bottomLeft.x)*(bottomRight.x - bottomLeft.x) + (bottomRight.y - bottomLeft.y)*(bottomRight.y - bottomLeft.y);
	if(tempLength>maxLength){
		maxLength = tempLength;
	}
	tempLength = (bottomRight.x - topRight.x)*(bottomRight.x - topRight.x) + (bottomRight.y - topRight.y)*(bottomRight.y - topRight.y);
	if(tempLength>maxLength){
		maxLength = tempLength;
	}
	maxLength = sqrt((double)maxLength);
	CvPoint2D32f src[4], dst[4];
	src[0] = cvPoint2D32f(topLeft.x,topLeft.y);
	dst[0] = cvPoint2D32f(0,0);
	src[1] = cvPoint2D32f(topRight.x,topRight.y);
	dst[1] = cvPoint2D32f(maxLength-1, 0);
	src[2] = cvPoint2D32f(bottomRight.x,bottomRight.y);
	dst[2] = cvPoint2D32f(maxLength-1, maxLength-1);
	src[3] = cvPoint2D32f(bottomLeft.x,bottomLeft.y);
	dst[3] = cvPoint2D32f(0, maxLength-1);
	IplImage* pic = cvLoadImage(FILENAME,0);
	IplImage* undistorted = cvCreateImage(
		cvSize(maxLength,maxLength),
		IPL_DEPTH_8U,
		1
	);
	CvMat* wrapMatrix = cvCreateMat(3,3,CV_32FC1);
	//Mat pic = imread("sudoku2.png");
	//Mat undistorted = Mat(Size(maxLength, maxLength), CV_8UC1);
	//cv::warpPerspective(pic, undistorted, cv::getPerspectiveTransform(src, dst), Size(maxLength, maxLength));
	//namedWindow("Undistorted");
	//imshow("Undistorted",undistorted);
	wrapMatrix = cvGetPerspectiveTransform(src,dst,wrapMatrix);
	cvWarpPerspective(pic,undistorted,wrapMatrix);

	//Draw lines in undistotred picture
	/*
	cvDrawLine(undistorted,cvPoint(0,0),cvPoint(undistorted->width,0),CV_RGB(0,0,0),2);
	cvDrawLine(undistorted,cvPoint(0,0),cvPoint(0,undistorted->height),CV_RGB(0,0,0),2);
	cvDrawLine(undistorted,cvPoint(undistorted->width,undistorted->height),cvPoint(0,undistorted->height),CV_RGB(0,0,0),2);
	cvDrawLine(undistorted,cvPoint(undistorted->width,undistorted->height),cvPoint(undistorted->width,0),CV_RGB(0,0,0),2);
	*/

	// Free Memory
	cvReleaseImage(&pic);
	free(valueMap);/*
	for(int i = 0 ; i < end/4; i++){
		free(countMap[i]);
		free(sumMap[i]);
	}
	free(countMap);
	free(sumMap);*/
	return undistorted;

}

void detectSudoku(IplImage* pic){

	int offset = 2; // offest
	int imgWidth = pic->width;
	int imgHeight = pic->height;
	int sqrWidth = imgWidth/9+1;
	int sqrHeight = imgWidth/9+1;
	int sudoku[9][9] = {{0}};

	IplImage* greyPic = cvCreateImage(
		cvGetSize(pic),
		IPL_DEPTH_8U,
		1
	);

	IplImage* temp = cvCreateImage(cvGetSize(pic),IPL_DEPTH_8U,1);
	cvCopy(pic,temp);
	//cvDilate(temp,temp);
	cvSmooth(temp,temp,CV_GAUSSIAN,5,5);
	cvAdaptiveThreshold(temp, greyPic, 255, CV_ADAPTIVE_THRESH_MEAN_C,
						CV_THRESH_BINARY_INV, 5,2);
	int coX = 0;  // The x coordinate of topleft vertex in the inner box
	int coY = 0;  // The y coordinate of topleft vertex in the inner box
	int coXL = 0; // The x coordinate of left edge
	int coXR = 0; // The x coordinate of right edge
	int coYT = 0; // The y coordinate of top edge
	int coYB = 0; // The y coordinate of bottom edge
	int midX = 0;
	int midY = 0;
	int width = 0;
	int height = 0;
	int detectThresholdX = 0;
	int detectThresholdY = 0;
	int widthThreshold = 0;
	int heightThreshold = 0;
	int areaThreshold = 50;  // areaThreshold
	int row = 0 ;
	int col = 0;
	int digitX = -1;
	int digitY = -1;
	int digitWidth = -1;
	int digitHeight = -1;
	CvPoint maxPoint;
	bool findNO = false;
	//cvNamedWindow("Piece");
	IplImage* piece;
	CvConnectedComp a = CvConnectedComp();
	CvConnectedComp* comp = &a;
	//char dataPath[] = "Train/train-images.idx3-ubyte";
	//char labelPath[] = "Train/train-labels.idx1-ubyte";
	char dataPath[] = "Train/myTrainData";
	char labelPath[] = "Train/myTrainClasses";
	DigitReader rd = DigitReader();
	rd.train(dataPath,labelPath,LOW_ENDIAN);

	for(row = 0 ; row< 9; row++){
		coY = row * sqrHeight;
		for(col = 0; col < 9 ; col++){
			coX = col * sqrWidth;
			digitX = -1;
			digitY = -1;
			digitWidth = -1;
			digitHeight = -1;
			// Calculate the outerbox
			if(coX - offset < 0){
				coXL = 0;
			}else {
				coXL = coX - offset;
			}

			if(coX + offset + sqrWidth >imgWidth){
				coXR = imgWidth;
			}else{
				coXR = coX + offset + sqrWidth;
			}

			if(coY - offset <0){
				coYT = 0;
			}else{
				coYT = coY - offset;
			}

			if(coY + offset + sqrHeight > imgHeight){
				coYB = imgHeight;
			}else{
				coYB = coY + offset + sqrHeight;
			}

			width = coXR - coXL + 1;
			height = coYB - coYT + 1;

			cvSetImageROI(greyPic,cvRect(coXL,coYT,width,height));
			piece = cvCreateImage(cvGetSize(greyPic),IPL_DEPTH_8U,1);
			cvCopy(greyPic,piece);
			//cvShowImage("Piece",piece);
			//cvWaitKey(0);

			detectThresholdY = height/4 +1;
			detectThresholdX = width/4 +1;
			widthThreshold = 2*width/3;
			heightThreshold = 3*height/4;
			midX = ( width+1 ) / 2;
			midY = ( height+1 ) / 2;
			int x = midX+5;
			int y = midY+5;

			for(;(x>detectThresholdX)&&(y>detectThresholdY); x--, y--){
				findNO = false;
				uchar* ptr = (uchar*)(piece->imageData + y*piece->widthStep);
				if(ptr[x]>128){
					cvFloodFill(piece,cvPoint(x,y),CV_RGB(0,0,64),cvScalarAll(0),cvScalarAll(0),comp);;
					if((comp->rect.height<heightThreshold)&&(comp->rect.width<widthThreshold)&& (comp->area>areaThreshold)){
						findNO = true;
						maxPoint = cvPoint(x,y);
						digitX = comp->rect.x;
						digitY = comp->rect.y;
						digitWidth = comp->rect.width;
						digitHeight = comp->rect.height;
						cvFloodFill(piece,cvPoint(x,y),CV_RGB(0,0,128));
						//cvShowImage("Piece",piece);
						//cvWaitKey(0);
						break;
					}
				}
			}
			if(findNO){
				for(int i = 0; i< height ; i++){

					uchar* ptr = (uchar*)(piece->imageData + i*piece->widthStep);
					for(int j = 0; j<width; j++){

						if(ptr[j]!=128&&ptr[j]!=0){
							if((j==width-1)||(i==height-1)){

							}else{
								cvFloodFill(piece,cvPoint(j,i),CV_RGB(0,0,0));
								//cvShowImage("Piece",piece);
								//cvWaitKey(0);
							}
						}
					}
				}
				cvFloodFill(piece,maxPoint,CV_RGB(0,0,255));
				//cvShowImage("Piece",piece);
				//cvWaitKey(0);
				CvMat* test = convertFormat(piece,digitX,digitY,digitWidth,digitHeight);
				int digit = rd.recognize(test);
				sudoku[row][col] = digit;
			}else{
				sudoku[row][col] = 0;
			}



			//cvShowImage("Piece",piece);
			//cvWaitKey(0);
			cvResetImageROI(greyPic);
			cvReleaseImage(&piece);
		}

	}

	for(int i = 0; i<9;i++){
		for(int j = 0; j<9; j++){
			printf("%d ", sudoku[i][j]);
		}
		printf("\n");
	}

//	cvNamedWindow("GreyPic");
//	cvShowImage("GreyPic",greyPic);
//	cvReleaseImage(&greyPic);

//	cvWaitKey(0);

//	cvDestroyWindow("GreyPic");
}



void findCorners(IplImage* res){


	using namespace cv;
	using namespace std;

	int imgHeight = res -> height;
	int imgWidth = res->width;
	Mat sudoku = Mat(res);
	vector<KeyPoint> corners;
	cv::FAST(sudoku,corners,10,false);

	// reverse the map
	for(int y = 0; y<res->height;y++){
	uchar* row = (uchar*)(res->imageData + y* res->widthStep);
	for( int x = 0; x < res->width; x++){
		if(row[x]==0){
			row[x] = 255;
		} else{
			row[x] = 128;
		}
	}
}

	CvPoint topLeft;
	CvPoint topRight;
	CvPoint bottomLeft;
	CvPoint bottomRight;

	int disTL = INT_MAX;
	int disTR = INT_MAX;
	int disBL = INT_MAX;
	int disBR = INT_MAX;

	int end = corners.size();


	printf("Number of corners: %d\n",end);

	for(int i = 0;i<= end -1;i++){
		KeyPoint keyPoint = corners.at(i);
		CvPoint temp = cvPoint(keyPoint.pt.x,keyPoint.pt.y);
		int tempX = temp.x;
		int tempY = temp.y;
		int xL = tempX;
		int xR = imgWidth - tempX;
		int yT = tempY;
		int yB = imgHeight - tempY;

		if((tempX*tempX + tempY*tempY)<disTL){
			topLeft = cvPoint(tempX,tempY);
			disTL = tempX*tempX + tempY*tempY;
		}

		if((xR*xR + yT*yT)<disTR){
			topRight = cvPoint(tempX,tempY);
			disTR = xR*xR + yT*yT;
		}

		if((xL*xL + yB*yB)<disBL){
			bottomLeft = cvPoint(tempX,tempY);
			disBL = xL*xL + yB*yB;
		}

		if((xR*xR + yB*yB)<disBR){
			bottomRight = cvPoint(tempX,tempY);
			disBR = xR*xR + yB*yB;
		}
	}
	cvLine( res, topLeft, bottomLeft, CV_RGB(0,0,0),1);
	cvLine( res, topLeft, topRight, CV_RGB(0,0,0),1);
	cvLine( res, bottomRight, bottomLeft, CV_RGB(0,0,0),1);
	cvLine( res, bottomRight, topRight, CV_RGB(0,0,0),1);
}

int main(int argc, char* argv[])
{
	CvPoint corners[4];
	IplImage* undistorted;
	IplImage* src = cvLoadImage(FILENAME,CV_LOAD_IMAGE_GRAYSCALE);
	IplImage* temp = cvCreateImage(
		cvGetSize(src),
		IPL_DEPTH_8U,
		1
	);
	IplImage* res = cvCreateImage(
		cvGetSize(src),
		IPL_DEPTH_8U,
		1
	);


	cvSmooth(src,temp,CV_GAUSSIAN,11,11);
	cvAdaptiveThreshold(temp, temp, 255, CV_ADAPTIVE_THRESH_MEAN_C,
						CV_THRESH_BINARY_INV, 5,2);

	int values[] = {0,1,0,1,1,1,0,1,0};
	IplConvKernel* kernal = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_CUSTOM,values);

	cvDilate(temp,res,kernal);
	//cvDilate(temp,temp);
	findblob(res);
	cvErode(res,res,kernal);
	cvReleaseStructuringElement(&kernal);

	undistorted = findCorners2(res,corners);
//	cvNamedWindow("Undistorted");
	cvNamedWindow("Original");

//	cvShowImage("Undistorted",undistorted);
	cvShowImage("Original",src);
	detectSudoku(undistorted);



	//cvNamedWindow("Outcome");
	//cvNamedWindow("Temp");




//	cvShowImage("Outcome",res);
//	cvShowImage("Temp",temp);

//	cvSaveImage("sudokuX.jpg",undistorted);

//	cvReleaseImage(&temp);
//	cvReleaseImage(&res);
//	cvReleaseImage(&undistorted);

	cvReleaseImage(&src);
	cvWaitKey(0);


//	cvDestroyWindow("Undistorted");
	cvDestroyWindow("Original");
//	cvDestroyWindow("Outcome");
//	cvDestroyWindow("Temp");
}
