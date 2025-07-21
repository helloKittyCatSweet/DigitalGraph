#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>

#pragma pack(1)

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
	unsigned int biSizeImage;// biSizeImage=LineBytes  biHeight
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

typedef struct graph {
	unsigned char* imageData;
	int height;
	int width;
} graph;

typedef struct core {
	double* imageData;
	int height;
	int width;
} core;

unsigned char* convolution(unsigned char* imageData,graph g, core c) {
	for(int i = 0; i < g.height; i++)
		for(int j = 0; j < g.width; j++) {
			double sum = 0;
			for(int m = -1 * (c.height - 1) / 2; m < ((c.height - 1) / 2 + 1); m++)
				for(int n = (-1 * (c.width - 1) / 2); n < ((c.width - 1) / 2 + 1); n++) {
					int x = i;
					int y = j;
					if(x + m < 0) x = 0;
					else if(x + m >= g.height) x = g.height - 1;
					else x += m;
					if(y + n < 0) y = 0;
					else if(y + n >= g.width) y = g.width - 1;
					else y += n;
					sum += 1.0 * (int)g.imageData[x * g.width + y] *
					       c.imageData[(m + (c.height - 1) / 2) * c.width + ((c.width - 1) / 2) + n];
				}
			imageData[i * g.height + j] = abs((int)sum);
		}
	return imageData;
}

int main() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_7//lena.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
	}
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
//	for(int i = 0; i < imSize; i++) cout << (int)imageData[i];
	fclose(fp);

	bmp b;
	int i,j,k;
	memcpy(&(b.bfHeader),&bfHeader,sizeof(bfHeader));
	memcpy(&(b.biHeader),&biHeader,sizeof(biHeader));
	b.bfHeader.bfOffBits=1078;
	int lineBytes_new=(width*8+31)/32*4;
	b.biHeader.biSizeImage=lineBytes_new*height;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage;
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}

	// prewitt

	int threshold = 140;	

	double c[9] = {-1,-1,-1,0,0,0,1,1,1};
	b.imgData = convolution(b.imgData, {imageData,height,lineBytes_new},core{c,3,3});

	for(int i = 0; i < imSize; i++)
		b.imgData[i] = b.imgData[i] >= threshold ? 255 : 0;

	char savePath1[] ="D://examples//pictures//homework_7//lena_prewitt.bmp";
	FILE *f=fopen(savePath1,"wb");
	if(f==NULL) {
		perror("can not open file!");
		exit(-2);
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);

	// sobel

	c[0] = -1;
	c[1] = 0;
	c[2] = 1;
	c[3] = -2;
	c[4] = 0;
	c[5] = 2;
	c[6] = -1;
	c[7] = 0;
	c[8] = 1;
	b.imgData = convolution(b.imgData, {imageData,height,lineBytes_new},core{c,3,3});
	for(int i = 0; i < imSize; i++)
		b.imgData[i] = b.imgData[i] >= threshold ? 255 : 0;

	char savePath2[] ="D://examples//pictures//homework_7//lena_sobel.bmp";
	f=fopen(savePath2,"wb");
	if(f==NULL) {
		perror("can not open file!");
		exit(-2);
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);
	
	// log
	
	double d[25] = {0,0,-1,0,0,0,-1,-2,-1,0,-1,-2,16,-2,-1,0,-1,-2,-1,0,0,0,-1,0,0};
	b.imgData = convolution(b.imgData, {imageData,height,lineBytes_new},core{d,5,5});
	for(int i = 0; i < imSize; i++)
		b.imgData[i] = b.imgData[i] >= threshold ? 255 : 0;

	char savePath3[] ="D://examples//pictures//homework_7//lena_log.bmp";
	f=fopen(savePath3,"wb");
	if(f==NULL) {
		perror("can not open file!");
		exit(-2);
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