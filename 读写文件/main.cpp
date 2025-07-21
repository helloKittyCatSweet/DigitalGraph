#include <iostream>
#include <fstream>

using namespace std;

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned long LONG;
typedef unsigned char BYTE;

// bitmap file header
typedef struct tagBITMAPFILEHEADER {
	WORD   	bfType; //type of the file, must be “BM”
	DWORD	bfSize; // the size of the bitmap file header
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits; //从文件头到实际位图数据的偏移字节数
} BITMAPFILEHEADER;

// bitmap info header
typedef struct tagBITMAPINFOHEADER {
	DWORD   biSize; //the length of this structure, 为40
	LONG	    biWidth; //图像宽度
	LONG	    biHeight; //图像高度
	WORD	    biPlanes; //must be 1
	WORD	    biBitCount; //颜色位数，
	DWORD   biCompression; //是否压缩
	DWORD   biSizeImage; //实际位图数据占用的字节数
	LONG	    biXPelsPerMeter;//目标设备水平分辨率
	LONG	    biYPelsPerMeter;//目标设备垂直分辨率
	DWORD   biClrUsed;//实际使用的颜色数
	DWORD   biClrImportant;//图像中重要的颜色数
} BITMAPINFOHEADER;

// color board
typedef struct tagRGBQUAD {
	BYTE  rgbBlue;   	 //该颜色的蓝色分量
	BYTE  rgbGreen;	 //该颜色的绿色分量
	BYTE  rgbRed;		 //该颜色的红色分量
	BYTE  rgbReserved;	 //保留值 透明度
} RGBQUAD;

FILE *read_fp;
FILE *write_fp_1;
FILE *write_fp_2;
FILE *write_fp_3;

// read file header
void readFileHeader(tagBITMAPFILEHEADER* fileHeader) {
	fread(fileHeader,sizeof(tagBITMAPFILEHEADER),1,read_fp);
}

// read info header
void readInfoHeader(tagBITMAPINFOHEADER* infoHeader) {
	fread(infoHeader,sizeof(tagBITMAPINFOHEADER),1,read_fp);
}

// read RgbQuad
void readRgbQuad(tagRGBQUAD* rgbQuad, const int &rgbQuadSize) {
	fread(rgbQuad,rgbQuadSize,1,read_fp);
}

// read image data
void readImgData(BYTE* imgData,const DWORD& imgSize) {
	fread(imgData,imgSize,1,read_fp);
}

// write file1 header
void writeFileHeader(tagBITMAPFILEHEADER* fileHeader,FILE* write_fp) {
	fwrite(fileHeader,sizeof(tagBITMAPFILEHEADER),1,write_fp);
}

// write file1 info header
void writeInfoHeader(tagBITMAPINFOHEADER* infoHeader,FILE* write_fp) {
	fwrite(infoHeader,sizeof(tagBITMAPINFOHEADER),1,write_fp);
}

// write file1 rgb quad
void writeRgbQuad(RGBQUAD* rgbQuad,const int& rgbQuadSize,FILE* write_fp) {
	fwrite(rgbQuad,rgbQuadSize,1,write_fp);
}

// write image data
void writeImgData(BYTE* imgData,const DWORD& imgSize,FILE* write_fp) {
	fwrite(imgData,imgSize,1,write_fp);
}
tagBITMAPFILEHEADER* bitmapFileHeader;
tagBITMAPINFOHEADER *bitmapInfoHeader;
BYTE* imgData;
DWORD imgSize;
RGBQUAD* rgbQuad;

void init(const char* readFilePath,const char* writeFilePath) {
	read_fp = fopen(readFilePath,"rb"); // read only binary
	FILE* write_fp = fopen(writeFilePath,"wb"); // write only binary

	bitmapFileHeader = new tagBITMAPFILEHEADER;
	readFileHeader(bitmapFileHeader);

	// check if bitmap
	if(bitmapFileHeader->bfType == 0x4d42) {
		bitmapInfoHeader = new tagBITMAPINFOHEADER;
		readInfoHeader(bitmapInfoHeader);

		// 24bit
		if(bitmapInfoHeader->biBitCount == 24) {
			imgSize = bitmapInfoHeader->biSizeImage;
			imgData = new BYTE[imgSize];
			readImgData(imgData,imgSize);

			fclose(read_fp);
		}

		// 8bit
		if(bitmapInfoHeader->biBitCount == 8) {
			rgbQuad = new RGBQUAD[256];
			readRgbQuad(rgbQuad,sizeof(RGBQUAD) * 256);

			imgSize = bitmapInfoHeader->biSizeImage;
			BYTE* imgData = new BYTE[imgSize];
			readImgData(imgData,imgSize);

			fclose(read_fp);
		}
	}
}

void createGrey_8() {
	BYTE rgb = 0;
	BYTE rgbReserved = 0;

	RGBQUAD* rgbQuad = new RGBQUAD[256];
	for(int i = 0; i < 256; i++) {
		rgbQuad[i].rgbBlue = rgb;
		rgbQuad[i].rgbGreen = rgb;
		rgbQuad[i].rgbRed = rgb;
		rgbQuad[i].rgbReserved = rgbReserved;
		rgb++;
	}
}

void rgb24_to_grey8(FILE *write_fp) {
	DWORD imgSize = bitmapInfoHeader->biHeight * (bitmapInfoHeader->biWidth / 4 + 1) * 4;
	BYTE* imgData = new BYTE[imgSize] {};

	bitmapFileHeader->bfOffBits = 54 + 1024; // color table 256*4
	bitmapFileHeader->bfSize = bitmapFileHeader->bfOffBits + imgSize;

	bitmapInfoHeader->biBitCount = 8;
	bitmapInfoHeader->biSizeImage = imgSize;
	bitmapInfoHeader->biClrUsed = 256;

	createGrey_8();

	int tmp;
	for(unsigned int i = 0; i < imgSize; i+=3) {
		tmp = (float) imgData[i] * 0.114f + (float)imgData[i + 1] * 0.587f +
		      (float)imgData[i + 2] * 0.299f;
		imgData[ i / 3 ] = BYTE(tmp);
	}

	writeFileHeader(bitmapFileHeader,write_fp);
	writeInfoHeader(bitmapInfoHeader,write_fp);
	writeRgbQuad(rgbQuad,256 * sizeof(RGBQUAD),write_fp);
	writeImgData(imgData,imgSize,write_fp);
}

void grey_to_antiGrey8(FILE* write_fp) {
	for(unsigned int i = 0; i < imgSize; i++) {
		imgData[i] = abs(imgData[i] - 255);
	}
	writeFileHeader(bitmapFileHeader,write_fp);
	writeInfoHeader(bitmapInfoHeader,write_fp);
	writeRgbQuad(rgbQuad,256 * sizeof(RGBQUAD),write_fp);
	writeImgData(imgData,imgSize,write_fp);
}

void rgb24_to_3grey8(FILE* write_fp_1,FILE* write_fp_2,FILE* write_fp_3) {
	DWORD imgSize = bitmapInfoHeader->biHeight * (bitmapInfoHeader->biWidth / 4 + 1) * 4;
	BYTE* imgData0 = new BYTE[imgSize] {};
	BYTE* imgData1 = new BYTE[imgSize] {};
	BYTE* imgData2 = new BYTE[imgSize] {};

	// the first picture
	bitmapFileHeader->bfOffBits = 54 + 1024;
	bitmapFileHeader->bfSize = bitmapFileHeader->bfOffBits + imgSize;

	bitmapInfoHeader->biBitCount = 8;
	bitmapInfoHeader->biSizeImage = imgSize;
	bitmapInfoHeader->biClrUsed = 256;

	createGrey_8();

	for(unsigned int i = 0; i < imgSize; i+=3)
		imgData0[i / 3] = imgData[i];

	writeFileHeader(bitmapFileHeader,write_fp_1);
	writeInfoHeader(bitmapInfoHeader,write_fp_1);
	writeRgbQuad(rgbQuad,256 * sizeof(RGBQUAD),write_fp_1);
	writeImgData(imgData,imgSize,write_fp_1);

	// second
	for(unsigned int i = 0; i < imgSize; i+=3)
		imgData0[i / 3] = imgData[i + 1];

	writeFileHeader(bitmapFileHeader,write_fp_2);
	writeInfoHeader(bitmapInfoHeader,write_fp_2);
	writeRgbQuad(rgbQuad,256 * sizeof(RGBQUAD),write_fp_2);
	writeImgData(imgData,imgSize,write_fp_2);
	
	// third
	for(unsigned int i = 0; i < imgSize; i+=3)
		imgData0[i / 3] = imgData[i + 2];

	writeFileHeader(bitmapFileHeader,write_fp_3);
	writeInfoHeader(bitmapInfoHeader,write_fp_3);
	writeRgbQuad(rgbQuad,256 * sizeof(RGBQUAD),write_fp_3);
	writeImgData(imgData,imgSize,write_fp_3);
}

void saveToBMP(){
	char* readFilename = "D://examples//pictures//rgb.bmp";
	read_fp = fopen(readFilename,"rb");
	if(read_fp == NULL) {
		cout << "something error occurs when reading." << endl;
		exit(0);
	}
	BITMAPFILEHEADER src_head;
	BITMAPINFOHEADER src_info;
	RGBQUAD src_rgbQuad;
	fread(&src_head, 1,sizeof(BITMAPFILEHEADER), read_fp);
	fread(&src_info, 1,sizeof(BITMAPINFOHEADER), read_fp);
	int src_image_size = src_info.biSize;
	int width = src_info.biWidth;
	int height = src_info.biHeight;
	int bitCount = src_info.biBitCount;
	int lineBytes = (width * bitCount + 31) / 32 * 4;
	
	fseek(read_fp,src_head.bfOffBits,SEEK_SET);
	
	fread(&src_rgbQuad,1,sizeof(RGBQUAD),read_fp);

//	cout << src_image_size << endl;
	unsigned char* src_image_buff = new unsigned char[src_image_size];
	fread(src_image_buff,1,src_image_size,read_fp);
	fclose(read_fp);
	
	FILE* write_fp_1 = fopen("D://examples//pictures//rgb_cut1.bmp","wb+");
	if(write_fp_1 == NULL){
		cout << "something error occurs when changing 24-color colorful image into 8-color grey image" << endl;
	}
	
	
	int colorTablesize = 0;
	auto biBitCount = src.depth();
	
}

int main() {
	saveToBMP();
	return 0;
}