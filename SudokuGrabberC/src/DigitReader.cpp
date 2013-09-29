#include <fstream>
#include <string.h>
#include "DigitReader.h"

#define MAX_NUM 60000
#define MAX_TEST 10
#define HIGH_ENDIAN 0
#define LOW_ENDIAN 1
#define ROWS 28
#define COLS 28


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
	if(byteOrder==LOW_ENDIAN){
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
