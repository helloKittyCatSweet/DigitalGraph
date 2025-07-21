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
	if((fp=fopen("D://examples//pictures//homework_9//region_mark.bmp","rb"))==NULL) {
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
	imSize = lineBytes * height;

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

	b.biHeader.biHeight = height;
	b.biHeader.biWidth = width;
	int lineBytes_new=(b.biHeader.biWidth*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new * b.biHeader.biHeight;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);

	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		(int)imageData[i] >= 128 ? b.imgData[i] = 255 : b.imgData[i] = 0;

	char savePath1[]="D://examples//pictures//homework_9//region_mark_to2.bmp";
	FILE* f=fopen(savePath1,"wb");
	if(f==NULL) {
		perror("can not open file!");
		return -2;
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);

	// zone marker

	int* label = (int*)malloc(sizeof(int)*b.biHeader.biSizeImage);
	memset(label,0,sizeof(int)*b.biHeader.biSizeImage);
	int cnt = 1;
	for(int i = 1; i < height; i++)
		for(int j = 1; j < lineBytes_new - 1; j++) {
			if(!b.imgData[i * lineBytes_new + j]) {
				int l1 = label[i * lineBytes_new + j - 1]; // left
				int l2 = label[(i - 1) * lineBytes_new + j - 1]; // left-up
				int l3 = label[(i - 1) * lineBytes_new + j]; // up
				int l4 = label[(i - 1) * lineBytes_new + j + 1]; // right-up
				if(l1 == 0 && l2 == 0&& l3 == 0 && l4 == 0) {
					label[i * lineBytes_new + j] = cnt++;
					continue;
				}
				int mmin = 100000000;
				if(l1 != 0 && l1 < mmin) mmin = l1;
				if(l2 != 0 && l2 < mmin) mmin = l2;
				if(l3 != 0 && l3 < mmin) mmin = l3;
				if(l4 != 0 && l4 < mmin) mmin = l4;
				label[i * lineBytes_new + j] = mmin;
			}
		}

	int col = 50;
	for(int i = 1; i <= cnt; i++) {
		if(col >= 256) col = 256;
		for(int j = 0; j < b.biHeader.biSizeImage; j++)
			if(label[j] == i)
				b.imgData[j] = col;
		col += 100;
	}
	
	char savePath2[]="D://examples//pictures//homework_9//region_mark_zone_marker.bmp";
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

	free(imageData);
	free(b.imgData);
	return 0;
}