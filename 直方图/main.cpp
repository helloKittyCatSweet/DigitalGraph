#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
#include<algorithm>
#include<cmath>

#pragma pack(1)   //全紧凑模式

using namespace std;

// bit map consists of bitmapFileHeader ,bitmapInfoHeader, rgbQUAD and exact imageData

typedef struct {
	unsigned char bfType[2]; // BM -> the type of the file
	unsigned int bfSize;// the size of the file
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int bfOffBits;// the offset from the bitmapFileHeader to exact imageData
} bitmapFileHeader;

typedef struct {
	unsigned int biSize;
	unsigned int biWidth;
	unsigned int biHeight;
	unsigned short biPlanes;// must be 1
	unsigned short biBitCount;// the bit of the color representation
	// 1 stands for two-color figure, 4 stands for sixteen-color figure,
	// 8 stands for 256-color figure,24 stands for true color figure.
	unsigned int biCompression; // whether is compressed or not
	unsigned int biSizeImage;// biSizeImage=LineBytes× biHeight
	// for grey figure,just the nearest digit over the biWidth
	// for true color figure,should be the nearest digit over three times of the biWidth
	unsigned int biXPixPerMeter;
	unsigned int biYPixPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
} bitmapInfoHeader;


typedef struct {
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
} rgbQUAD;

typedef struct {
	bitmapFileHeader bfHeader;
	bitmapInfoHeader biHeader;
	rgbQUAD palette[256];
	unsigned char *imgData;
} bmp;

int colors[256];

int main() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_2//dim.bmp","rb"))==NULL) {
		perror("can not open file!");
		return -1;
	}
	//读入彩色bmp图像文件头，信息头和图像数据
	bitmapFileHeader bfHeader;
	fread(&bfHeader,14,1,fp);
	bitmapInfoHeader biHeader;
	fread(&biHeader,40,1,fp);
	int imSize=biHeader.biSizeImage;
	int width=biHeader.biWidth;
	int height=biHeader.biHeight;
	int bitCount=biHeader.biBitCount;
	int lineBytes=(width*bitCount+31)/32*4;

	fseek(fp,bfHeader.bfOffBits,SEEK_SET);
	unsigned char*imageData=(unsigned char*)malloc(imSize*sizeof(unsigned char));
	fread(imageData,imSize*sizeof(unsigned char),1,fp);
	fclose(fp);

	for(int i = 0; i < imSize; i++)
		colors[imageData[i]]++;

	int maxHeight = 0;
	int sum = 0;
	for(int i = 0; i < 256; i++) {
		maxHeight = max(maxHeight,colors[i]);
		sum += colors[i];
	}
//	cout << "total points:" << sum << endl;
//	cout << "maxHeight:" << maxHeight << endl;

	bmp b;
	int i,j,k;
	memcpy(&(b.bfHeader),&bfHeader,sizeof(bfHeader));
	memcpy(&(b.biHeader),&biHeader,sizeof(biHeader));
	b.bfHeader.bfOffBits=1078;
	b.biHeader.biBitCount=8;  //改变图像位数
	b.biHeader.biWidth = 256;
	b.biHeader.biHeight = maxHeight;
	int lineBytes_new=(b.biHeader.biWidth*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new*b.biHeader.biHeight;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,255,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(int i = 0; i < 256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}

	// origin hist

	for(int i = 0; i < b.biHeader.biWidth; i++)
		for(int j = 0; j < b.biHeader.biHeight; j++)
			if(j <= colors[i]) b.imgData[j * b.biHeader.biWidth + i] = 0;
			else break;

	char savePath1[]="D://examples//pictures//homework_2//dim_hist.bmp";
	FILE *f=fopen(savePath1,"wb");
	if(f==NULL) {
		perror("can not open file!");
		return -2;
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);

	fclose(f);

	// normalization hist
	// because of the big gap of the height(2916) and the width(256),
	// devide their quotient of division, which is 12

	int to_normalization[256];
	for(int i = 0; i < 256; i++)
		to_normalization[i] = colors[i] / 12;

	b.biHeader.biHeight = b.biHeader.biWidth;
	lineBytes_new=(b.biHeader.biWidth*8+31)/32*4;
	b.biHeader.biSizeImage=lineBytes_new*b.biHeader.biHeight;
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,255,sizeof(unsigned char)*b.biHeader.biSizeImage);
	
	for(int i = 0; i < b.biHeader.biWidth; i++)
		for(int j = 0; j < b.biHeader.biHeight; j++)
			if(j <= to_normalization[i])
				b.imgData[j * b.biHeader.biWidth + i] = 0;
			else break;
	cout << colors[0] << endl;

	char savePath2[]="D://examples//pictures//homework_2//dim_hist_normalization.bmp";
	f=fopen(savePath2,"wb");
	if(f==NULL) {
		perror("can not open file!");
		return -2;
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);

	// equalization

	double color_p[256];
	for(int i = 0; i < 256; i++) color_p[i] = 1.0 * colors[i] / sum;
	double color_s[256];
	color_s[0] = color_p[0]; // the s of the first one is the exactly p
	for(int i = 1; i < 256; i++) color_s[i] = color_s[i-1] + color_p[i];
	int new_colors[256];
	for(int i = 0; i < 256; i++) new_colors[i] = 255 * color_s[i] + 0.5;

	b.biHeader.biWidth = width;
	b.biHeader.biHeight = height;
	lineBytes_new=(b.biHeader.biWidth*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new*b.biHeader.biHeight;
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,255,sizeof(unsigned char)*b.biHeader.biSizeImage);

	for(int i = 0; i < b.biHeader.biSizeImage; i++) {
		int cur = imageData[i];
		for(int j = 0; j < 256; j++)
			if(cur == j) b.imgData[i] = new_colors[j];
	}

	char savePath3[]="D://examples//pictures//homework_2//dim_equalization.bmp";
	f=fopen(savePath3,"wb");
	if(f==NULL) {
		perror("can not open file!");
		return -2;
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);

	// equalization hist

	b.biHeader.biHeight = 256;
	b.biHeader.biWidth = 256;
//	cout << b.biHeader.biHeight << endl;
	lineBytes_new=(b.biHeader.biWidth*8+31)/32*4;

	for(int i = 0; i < 256; i++) colors[i] = 0;
	for(int i = 0; i < b.biHeader.biSizeImage; i++) colors[b.imgData[i]]++;
	maxHeight = 0;
	for(int i = 0; i < 256; i++) maxHeight = max(maxHeight,colors[i]);
	int divide = maxHeight / 256 + 1;
	for(int i = 0; i < 256; i++) to_normalization[i] = colors[i] / divide;
	
	b.biHeader.biSizeImage=lineBytes_new*b.biHeader.biHeight;
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,255,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(int i = 0; i < b.biHeader.biWidth; i++)
		for(int j = 0; j < b.biHeader.biHeight; j++)
			if(j <= to_normalization[i])
				b.imgData[j * b.biHeader.biWidth + i] = 0;
			else break;

	char savePath4[]="D://examples//pictures//homework_2//dim_hist_equalization.bmp";
	f=fopen(savePath4,"wb");
	if(f==NULL) {
		perror("can not open file!");
		return -2;
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);

	free(imageData);
	free(b.imgData);
	return 0;
}