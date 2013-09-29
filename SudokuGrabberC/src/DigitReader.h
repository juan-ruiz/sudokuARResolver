#ifndef DIGITREADER_H_
#define DIGITREADER_H_
#include <ml.h>

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
#endif
