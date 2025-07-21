#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<cmath>
#include<iostream>
#include<algorithm>

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

int main() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_5//lena.bmp","rb"))==NULL) {
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

	bmp b;
	int i,j,k;
	memcpy(&(b.bfHeader),&bfHeader,sizeof(bfHeader));
	memcpy(&(b.biHeader),&biHeader,sizeof(biHeader));
	b.bfHeader.bfOffBits=1078;
	b.biHeader.biBitCount=8;

	for(i=0; i<256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}

	// calculate hist

	int colors[256] = {0};
	for(int i = 0; i < imSize; i++)
		colors[imageData[i]]++;

	int maxHeight = 0;
	for(int i = 0; i < 256; i++)
		maxHeight = max(colors[i], maxHeight);
	cout << "the highest level of grey color in the origin lena.bmp is: " << maxHeight << endl;

	int cut = maxHeight / 256;
	b.biHeader.biHeight = maxHeight / cut;
	b.biHeader.biWidth = 256;
	int lineBytes_new=(b.biHeader.biWidth*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new * b.biHeader.biHeight;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,255,sizeof(unsigned char)*b.biHeader.biSizeImage);

	// pick 70 as the threshold
	int threshold = 110;
	for(int i = 0; i < b.biHeader.biWidth; i++)
		if(i == threshold)
			for(int j = 0; j < b.biHeader.biHeight; j++)
				b.imgData[j * b.biHeader.biWidth + i] = 100;
		else
			for(int j = 0, h = colors[i] / cut; j < b.biHeader.biHeight; j++)
				if(j <= h) b.imgData[j * b.biHeader.biWidth + i] = 0;
				else break;

	char savePath1[]="D://examples//pictures//homework_5//lena_hist110.bmp";
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


	// manually choose the threshold on the origin picture

	b.biHeader.biHeight = height;
	b.biHeader.biWidth = width;
	lineBytes_new=(b.biHeader.biWidth*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new * b.biHeader.biHeight;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);

	for(int i = 0; i < imSize; i++)
		b.imgData[i] = imageData[i] >= threshold ? 255 : 0;

	cout << "manually drawing chooses the : " << threshold << endl;

	char savePath2[]="D://examples//pictures//homework_5//lena_manually_threshold110.bmp";
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

	// iteration for theshold and put it on the figure

	int tmp[imSize];
	for(int i = 0; i < imSize; i++) tmp[i] = imageData[i];
	sort(tmp, tmp + imSize);
	int t = tmp[imSize / 2];
	int t1 = 0;

	while(abs(t1 - t) >= 10) {
		int cnt1[t] = {0};
		int cnt2[256 - t] = {0};
		for(int i = 0; i < imSize; i++)
			imageData[i] >= t ? cnt2[imageData[i] - t]++ : cnt1[imageData[i]]++;
		int up1 = 0;
		int down1 = 0;
		for(int i = 0; i < t; i++) up1 += i * cnt1[i], down1 += cnt1[i];
		int u1 = up1 / down1;
		int up2 = 0;
		for(int i = 0; i < 255 - t; i++) up2 += (i + t) * cnt2[i];
		int u2 = up2 / (imSize - down1);
		t1 = t;
		t = (u1 + u2) / 2;
	}

	threshold = t1;
	cout << "iteration chooses the : " << threshold << endl;

	for(int i = 0; i < imSize; i++)
		b.imgData[i] = imageData[i] >= threshold ? 255 : 0;

	char savePath3[]="D://examples//pictures//homework_5//lena_iteration.bmp";
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

	// otsu for theshold and put it on the figure

	double sigma = 200.0;
	int grey_level = -1;
	for(int i = 0; i < 256; i++) {
		int cnt1[i] = {0};
		int cnt2[256 - i] = {0};
		for(int j = 0; j < imSize; j++)
			imageData[j] >= i ? cnt2[imageData[j] - i]++ : cnt1[imageData[j]]++;
		int points1 = 0;
		for(int j = 0; j < i; j++) points1 += cnt1[imageData[i]];
		int points2 = imSize - points1;
		double w1 = 1.0 * points1 / imSize;
		double w2 = 1.0 * points2 / imSize;
		double u1 = 0;
		for(int j = 0; j < i; j++) u1 += 1.0 * cnt1[j] / imSize * j;
		double u2 = 0;
		for(int j = 0; j < 256 - i; j++) u2 += 1.0 * cnt2[j] / imSize * (j + i);
		double tmp = w1 * w2 * (u1 - u2) * (u1 - u2);
		if(tmp > sigma) {
			sigma = tmp;
			grey_level = i;
		}
	}

	threshold = grey_level;
	cout << "otsu chooses the : " << threshold << endl;

	b.imgData[i] = imageData[i] >= threshold ? 255 : 0;

	char savePath4[]="D://examples//pictures//homework_5//lena_otsu.bmp";
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