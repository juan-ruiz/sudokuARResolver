// SudokuDetector.cpp : Defines the entry point for the console application.
//
#include <jni.h>
#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/ml/ml.hpp>
#include <android/log.h>

#define ROWS 28
#define COLS 28
#define MAX_NUM 60000
#define MAX_TEST 10
#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1
#define ANDROID_LOG_VERBOSE ANDROID_LOG_DEBUG
#define LOG_TAG "SodukuDetector"
#define LOGV(...) __android_log_print(ANDROID_LOG_SILENT, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#ifdef __cplusplus
extern "C" {
#endif

class DigitReader{

public:
	DigitReader();

	bool train(char* dataPath, char* labelPath, int byteOrder);
	int recognize(CvMat* data);
	int getDataSizeX();
	int getDataSizeY();

private:
	int readInteger(FILE* fp, int byteOrder);
	bool readTrainData(char* dataPath,int byteOrder);
	bool readTrainClasses(char* labelPath,int byteOrder);
	void setDataSize(int x, int y);
	CvMat* trainData;
	CvMat* trainClasses;
	int dataSizeX;
	int dataSizeY;
	CvKNearest knn;
	int K;
};
void DigitReader::setDataSize(int x, int y){
	dataSizeX = x;
	dataSizeY = y;
}

bool DigitReader :: train(char* dataPath, char* labelPath, int byteOrder){


	if(readTrainData(dataPath,byteOrder)&&readTrainClasses(labelPath, byteOrder)){
		knn.train( trainData, trainClasses);
		return true;
	}else{
		return false;
	}

}
int DigitReader::recognize(CvMat* data){
	CvMat* nearests = cvCreateMat( 1, K, CV_32FC1);
	int response = knn.find_nearest(data,K,0,0,nearests,0);
	return response;
}
int DigitReader::readInteger(FILE* fp, int byteOrder){

	int ret;
	uchar* integer = (uchar*)(&ret);
	if(byteOrder==LITTLE_ENDIAN){
		fread(&ret,sizeof(int),1,fp);
	}else{
		fread(&integer[3],sizeof(uchar),1,fp);
		fread(&integer[2],sizeof(uchar),1,fp);
		fread(&integer[1],sizeof(uchar),1,fp);
		fread(&integer[0],sizeof(uchar),1,fp);
	}
	return ret;
}

bool DigitReader::readTrainData(char* dataPath,int byteOrder){
	FILE* pFile;
	int magicNumber = 0;
	int numOfImage = 0;
	int cols = 0;
	int rows = 0;
	int size = 0;
	uchar* temp ;
	IplImage* tempImg = cvCreateImage(
		cvSize(ROWS,COLS),
		IPL_DEPTH_8U,
		1
	);
	pFile = fopen(dataPath,"rb");

	if(pFile == NULL){
		fputs("File Error",stderr);
		return false;
	}
	magicNumber = readInteger(pFile,byteOrder);
	numOfImage = readInteger(pFile,byteOrder);
	rows = readInteger(pFile,byteOrder);
	cols = readInteger(pFile,byteOrder);
	setDataSize(rows,cols);
	size = rows * cols;
	temp = new uchar[size];

	if( numOfImage > MAX_NUM){
		numOfImage = MAX_NUM;
	}

	trainData = cvCreateMat( numOfImage, size, CV_32FC1 );

	for(int i = 0; i < numOfImage; i++ ){
		fread((void*)temp,size*sizeof(uchar),1,pFile);
		for(int j = 0; j<size; j++){
			trainData->data.fl[i*size+j] = temp[j];

		}
	}
	return true;
}

bool DigitReader::readTrainClasses(char* labelPath,int byteOrder){

	FILE* pFile;
	int magicNumber = 0;
	int numOfLabel = 0;

	pFile = fopen(labelPath,"rb");

	if(pFile == NULL){
		fputs("File Error",stderr);
		return false;
	}
	magicNumber = readInteger(pFile,byteOrder);
	numOfLabel = readInteger(pFile,byteOrder);
	if(numOfLabel > MAX_NUM){
		numOfLabel = MAX_NUM;
	}

	trainClasses = cvCreateMat( numOfLabel,1, CV_32FC1 );

	for(int i = 0; i < numOfLabel ; i++){
		uchar label;
		fread(&label, sizeof(uchar),1,pFile);
		trainClasses->data.fl[i] = label;
	}
	return true;
}

DigitReader::DigitReader(){
	knn = CvKNearest();
	K = 2;
}


/* Copied from Web */
IplImage* pImage = NULL;
IplImage* loadPixels(int* pixels, int width, int height);
IplImage* getIplImageFromIntArray(JNIEnv* env, jintArray array_data, jint width, jint height);

/* My own code */
CvMat* convertFormat(IplImage* piece,int digitX, int digitY, int digitWidth, int digitHeight);
IplImage* findCorners(IplImage* res, CvPoint* cPoints);
IplImage* smoothImage(IplImage* src);
void findblob(IplImage* res);
void recognizeDigits(IplImage* undistorted, int sudoku[9][9], int byteOrder);
double cosAngle(CvPoint p1, CvPoint p2, CvPoint p0);
bool checkSudokuSquare(CvPoint* corners);

double cosAngle(CvPoint p1, CvPoint p2, CvPoint p0){
    double dx1 = p1.x - p0.x;
    double dy1 = p1.y - p0.y;
    double dx2 = p2.x - p0.x;
    double dy2 = p2.y - p0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

bool checkSudokuSquare(CvPoint* corners){
	CvPoint topLeft = corners[0];
	CvPoint topRight = corners[1];
	CvPoint bottomRight = corners[2];
	CvPoint bottomLeft = corners[3];

	double maxCosine = 0;
	double angle = 0;

	// Check the topLeft corner
	angle = cosAngle(topRight, bottomLeft, topLeft);
	maxCosine = max(maxCosine, angle);

	// Check the topRight corner
	angle = cosAngle(topLeft, bottomRight, topRight);
	maxCosine = max(maxCosine, angle);

	// Check the bottomLeft corner
	angle = cosAngle(topLeft, bottomRight, bottomLeft);
	maxCosine = max(maxCosine, angle);

	// Check the bottomRight corner
	angle = cosAngle(topRight, bottomLeft, bottomRight);
	maxCosine = max(maxCosine, angle);


	return maxCosine<0.3;
}

JNIEXPORT jboolean JNICALL Java_com_sudoku_SudokuDetector_setSourceImage
  (JNIEnv *env, jobject obj, jintArray photo_data, jint width, jint height){
	if (pImage != NULL) {
		cvReleaseImage(&pImage);
		pImage = NULL;
	}
	pImage = getIplImageFromIntArray(env, photo_data, width, height);
	if (pImage == NULL) {
		return 0;
	}
	LOGI("Load Image Done.");
	return 1;
}


JNIEXPORT jobjectArray JNICALL Java_com_sudoku_SudokuDetector_detectSudoku
  (JNIEnv* env, jobject obj, jint byteOrder){
	jobjectArray sudoku;
	jclass intArrCls = env->FindClass("[I");
	if(intArrCls == NULL){
		return NULL; /* exception thrown */
	}
	sudoku = env->NewObjectArray(9,intArrCls,NULL);

	if(sudoku == NULL){
		return NULL; /* out of memory error thrown */
	}

	int su[9][9] = {{0}};
	CvPoint corners[4];
	IplImage* undistorted;
	IplImage* res = cvCreateImage(
		cvGetSize(pImage),
		IPL_DEPTH_8U,
		1
	);
	res = smoothImage(pImage);
	LOGI("Smooth Image Done.");

	findblob(res);
	LOGI("Finding Blob Done.");

	undistorted = findCorners(res,corners);
	LOGI("Finding Corners Done.");

	if(checkSudokuSquare(corners)){
		recognizeDigits(undistorted,su,byteOrder);
		LOGI("Recognizing Digits Done");
	}else{
		for(int i = 0; i<9; i++){
			for(int j=0; j<9; j++){
				su[i][j] = -1;
			}
		}
	}

	if(su==NULL){
		for(int i = 0; i<9; i++){
			for(int j=0; j<9; j++){
				su[i][j] = 0;
			}
		}
	}

	for(int i=0; i<9; i++){
		int temp[9] = {0};
		for(int j=0; j<9; j++){
			temp[j] = su[i][j];
		}
		jintArray iarr = env->NewIntArray(9);
		env->SetIntArrayRegion(iarr,0,9,temp);
		env->SetObjectArrayElement(sudoku,i,iarr);
		env->DeleteLocalRef(iarr);
	}
	cvReleaseImage(&res);
	return sudoku;

}


IplImage* loadPixels(int* pixels, int width, int height) {
	int x, y;
	IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	unsigned char* base = (unsigned char*) (img->imageData);
	unsigned char* ptr;
	for (y = 0; y < height; y++) {
		ptr = base + y * img->widthStep;
		for (x = 0; x < width; x++) {
			unsigned char blue = pixels[x + y * width] & 0xFF;
			unsigned char green = pixels[x + y * width] >> 8 & 0xFF;
			unsigned char red = pixels[x + y * width] >> 16 & 0xFF;
			ptr[x] = blue*0.114+green*0.587+red*0.299;
		}
	}
	//cvCvtColor(img,gray,CV_BGR2GRAY);
	return img;
}
IplImage* getIplImageFromIntArray(JNIEnv* env, jintArray array_data,
		jint width, jint height) {
	int *pixels = env->GetIntArrayElements(array_data, 0);
	if (pixels == 0) {
		return 0;
	}
	IplImage *image = loadPixels(pixels, width, height);
	env->ReleaseIntArrayElements(array_data, pixels, 0);
	if (image == 0) {
		return 0;
	}
	return image;
}

void recognizeDigits(IplImage* undistorted, int sudo[9][9], int byteOrder){
	int offset = 5; // offest
	int imgWidth = undistorted->width;
	int imgHeight = undistorted->height;
	int sqrWidth = imgWidth/9+1;
	int sqrHeight = imgWidth/9+1;

	IplImage* greyPic = cvCreateImage(
		cvGetSize(undistorted),
		IPL_DEPTH_8U,
		1
	);
	int values[] = {0,1,0,1,1,1,0,1,0};
	IplConvKernel* kernal = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_CUSTOM,values);
	IplImage* temp = cvCreateImage(cvGetSize(undistorted),IPL_DEPTH_8U,1);
	cvCopy(undistorted,temp);
	cvSmooth(temp,temp,CV_GAUSSIAN,3,3);
	cvAdaptiveThreshold(temp, greyPic, 255, CV_ADAPTIVE_THRESH_MEAN_C,
						CV_THRESH_BINARY_INV, 5,2);
	cvDilate(greyPic,greyPic,kernal);

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
	IplImage* piece;
	CvConnectedComp a = CvConnectedComp();
	CvConnectedComp* comp = &a;
	char dataPath[] = "/sdcard/Train/myTrainData";
	char labelPath[] = "/sdcard/Train/myTrainClasses";
	DigitReader rd = DigitReader();
	if(rd.train(dataPath,labelPath,byteOrder)){
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
						coXL = coX - (1.5)*offset;
					}

					if(coX + offset + sqrWidth >imgWidth){
						coXR = imgWidth;
					}else{
						coXR = coX  + sqrWidth;
					}

					if(coY - offset <0){
						coYT = 0;
					}else{
						coYT = coY - (1.5)*offset;
					}

					if(coY + offset + sqrHeight > imgHeight){
						coYB = imgHeight;
					}else{
						coYB = coY + sqrHeight;
					}

					width = coXR - coXL - offset;
					height = coYB - coYT- offset;

					cvSetImageROI(greyPic,cvRect(coXL,coYT,width,height));
					piece = cvCreateImage(cvGetSize(greyPic),IPL_DEPTH_8U,1);
					cvCopy(greyPic,piece);

					detectThresholdY = height/4 +1;
					detectThresholdX = width/4 +1;
					widthThreshold = 2*width/3;
					heightThreshold = 3*height/4;
					midX = ( width+1 ) / 2;
					midY = ( height+1 ) / 2;
					int x = midX+2*offset;
					int y = midY+2*offset;

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
									}
								}
							}
						}
						cvFloodFill(piece,maxPoint,CV_RGB(0,0,255));
						cvErode(piece,piece,kernal);

						CvMat* test = convertFormat(piece,digitX,digitY,digitWidth,digitHeight);
						int digit = rd.recognize(test);
					//	int digit = 2;
						sudo[row][col] = digit;
					}else{
						sudo[row][col] = 0;
					}

					cvResetImageROI(greyPic);
					cvReleaseImage(&piece);
				}

			}
			cvReleaseImage(&undistorted);
			cvReleaseImage(&greyPic);
	} else{
		sudo = NULL;
	}



}
IplImage* smoothImage(IplImage* src){

	IplImage* temp = cvCreateImage(
		cvGetSize(src),
		IPL_DEPTH_8U,
		1
	);
	cvSmooth(src,temp,CV_GAUSSIAN,11,11);
	cvAdaptiveThreshold(temp, temp, 255, CV_ADAPTIVE_THRESH_MEAN_C,
						CV_THRESH_BINARY_INV, 5,2);
	return temp;

}
CvMat* convertFormat(IplImage* piece,int digitX, int digitY, int digitWidth, int digitHeight){

	int blockNo = 14;
	CvMat* digitMat= cvCreateMat(1,COLS*ROWS,CV_32FC1);
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
			ptr2[tempX] = ptr1[digitX+j];
		}
	}

	cvSetImageROI(temp,cvRect((digitX-offsetX-blockNo/2),(digitY-offsetY-blockNo/2),(digitWidth+blockNo),(digitHeight+blockNo)));

	cvResize(temp,resizePic);

	for(int i = 0 ;i<ROWS;i++){
		uchar* ptr = (uchar*)(resizePic->imageData + i*resizePic->widthStep);
		for(int j = 0; j<COLS; j++){
			digitMat->data.fl[i*COLS+j] = ptr[j];
		}
	}


	return digitMat;
}

void findblob(IplImage* res){

	double max = -1;
	CvPoint maxPt;
	int values[] = {0,1,0,1,1,1,0,1,0};
	IplConvKernel* kernal = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_CUSTOM,values);
	CvConnectedComp a = CvConnectedComp();
	CvConnectedComp* comp = &a;
	cvDilate(res,res,kernal);

	for(int y = 0; y<res->height;y++){

		uchar* row = (uchar*)(res->imageData + y* res->widthStep);
		for( int x = 0; x < res->width; x++){

			if(row[x]>=128){

				cvFloodFill(res,cvPoint(x,y),CV_RGB(0,0,64),cvScalarAll(0),cvScalarAll(0),comp);
				if(comp==NULL){
				}else{
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
	cvErode(res,res,kernal);
	cvReleaseStructuringElement(&kernal);

}

IplImage* findCorners(IplImage* res, CvPoint* cPoints){
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

	int* valueMap;
	valueMap = (int *)malloc(end*(sizeof(int)));
	int count = 0;

	count = corners.size();
	printf("Count: %d\n", count);

	// find the four corners
	for( int i = 0 ; i < count; i++){

		int tempX = corners.at(i).pt.x;
		int tempY = corners.at(i).pt.y;
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

	cPoints[0] = topLeft;
	cPoints[1] = topRight;
	cPoints[2] = bottomRight;
	cPoints[3] = bottomLeft;


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
	maxLength = sqrt((double)maxLength)-10;
	CvPoint2D32f src[4], dst[4];
	src[0] = cvPoint2D32f(topLeft.x+5,topLeft.y+5);
	dst[0] = cvPoint2D32f(0,0);
	src[1] = cvPoint2D32f(topRight.x-5,topRight.y+5);
	dst[1] = cvPoint2D32f(maxLength-1, 0);
	src[2] = cvPoint2D32f(bottomRight.x-5,bottomRight.y-5);
	dst[2] = cvPoint2D32f(maxLength-1, maxLength-1);
	src[3] = cvPoint2D32f(bottomLeft.x+5,bottomLeft.y-5);
	dst[3] = cvPoint2D32f(0, maxLength-1);
	IplImage* pic = cvCreateImage(
			cvGetSize(pImage),
			IPL_DEPTH_8U,
			1
	);

	cvCopy(pImage,pic);
	IplImage* undistorted = cvCreateImage(
		cvSize(maxLength,maxLength),
		IPL_DEPTH_8U,
		1
	);
	CvMat* wrapMatrix = cvCreateMat(3,3,CV_32FC1);
	wrapMatrix = cvGetPerspectiveTransform(src,dst,wrapMatrix);
	cvWarpPerspective(pic,undistorted,wrapMatrix);

	// Free Memory
	cvReleaseImage(&pic);
	free(valueMap);
	return undistorted;

}

#ifdef __cplusplus
}
#endif
