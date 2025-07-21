#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
#include<algorithm>

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

typedef struct graph{
	unsigned char* imageData;
	int height;
	int width;
}graph;

typedef struct core{
	double* imageData;
	int height;
	int width;
}core;

unsigned char* convolution(unsigned char* imageData,graph g, core c) {
//	for(int i = 0; i < g.height * g.width; i++) cout << (int)g.imageData[i];
//	for(int i = 0; i < c.height * c.width; i++) cout << (double)c.imageData[i];
	for(int i = 0; i < g.height; i++)
		for(int j = 0; j < g.width; j++){
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
			imageData[i * g.height + j] = (int)sum;
		}
	return imageData;
}

void operating_1() {
	// operating lena.bmp

	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_3//lena.bmp","rb"))==NULL) {
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
//	cout << height << " " << lineBytes_new << endl;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage;
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}
	double c[9];
	for(int i = 0; i < 9; i++) c[i] = 1.0 /9;
	b.imgData = convolution(b.imgData,{imageData,height,lineBytes_new},core{c,3,3});

	// using neighborhood average filtering, which divides the sum of a scope of
	// nine elements with 9
	// 9 -> -1,0,1;-1,0,1
	// 25 -> -2,-1,0,1,2;-2,-1,0,1,2
	// the usage of the latter better fits the result

	
//		int big_graph[lineBytes_new + 2][height + 2] = {0};
//		for(int i = 1; i < lineBytes_new + 1; i++)
//			for(int j = 1; j < height + 1; j++)
//				big_graph[i][j] = imageData[(i - 1) * lineBytes_new + (j - 1)];
//		big_graph[0][0] = imageData[0];
//		big_graph[0][height + 1] = imageData[lineBytes_new - 1];
//		big_graph[lineBytes_new + 1][0] = imageData[lineBytes_new * (height - 1)];
//		big_graph[lineBytes_new + 1][height + 1] = imageData[lineBytes_new * height - 1];
//
//		for(int i = 1; i < lineBytes_new + 1; i++) {
//			big_graph[0][i] = imageData[i - 1];
//			big_graph[i][0] = imageData[lineBytes_new * (i - 1)];
//			big_graph[lineBytes_new + 1][i] =
//			    imageData[lineBytes_new * (height - 1) + (i - 1)];
//			big_graph[i][height + 1] = imageData[lineBytes_new * (i - 1) + height];
//		}
//
//		for(int i = 1; i < height + 1; i++)
//			for(int j = 1; j < lineBytes_new + 1; j++) {
//				int sum = 0;
//				for(int k = -1; k < 2; k++)
//					for(int l = -1; l < 2; l++) {
//						if(i + k < 0) {
//							if(j + l < 0) {
//								sum += big_graph[0][0];
//								continue;
//							} else if(j + l >= height + 1) {
//								sum += big_graph[0][height];
//								continue;
//							} else {
//								sum += big_graph[0][j + l];
//								continue;
//							}
//						}
//						if(i + k >= height + 1) {
//							if(j + l < 0) {
//								sum += big_graph[height][0];
//								continue;
//							} else if(j + l >= height + 1) {
//								sum += big_graph[height][lineBytes_new];
//								continue;
//							} else {
//								sum += big_graph[height][j + l];
//								continue;
//							}
//						} else {
//							if(j + l < 0) {
//								sum += big_graph[i + k][0];
//								continue;
//							} else if(j + l >= height + 1) {
//								sum += big_graph[i + k][j];
//								continue;
//							} else
//								sum += big_graph[i + k][j + l];
//						}
//					}
//				b.imgData[height * (i - 1) + (j - 1)] = sum / 9;
//			}
	
//	for(int i = 0; i < b.biHeader.biSizeImage; i++) cout << (int)b.imgData[i];

	char savePath1[] ="D://examples//pictures//homework_3//lena_neighborhood_average_filtering_9_cov.bmp";
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

	free(imageData);
	free(b.imgData);
}

void operating_2() {
	// operating noise2.bmp
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_3//noise2.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
	}

	bitmapFileHeader bfHeader;
	fread(&bfHeader,14,1,fp);
	bitmapInfoHeader biHeader;
	fread(&biHeader,40,1,fp);

	unsigned int imgSize = biHeader.biHeight * (biHeader.biWidth / 4 + 1) * 4;//新图片的数据大小

	int width=biHeader.biWidth;
	int height=biHeader.biHeight;
	int bitCount=biHeader.biBitCount;
//	cout << bitCount << endl;
	int lineBytes=(width*bitCount+31)/32*4;
	int imSize = lineBytes * height;
	biHeader.biSizeImage = imSize;
//	cout << "biHeader.biSizeImage:" << biHeader.biSizeImage << endl;

	fseek(fp,bfHeader.bfOffBits,SEEK_SET);
	unsigned char*imageData=(unsigned char*)malloc(imSize*sizeof(unsigned char));
	cout << imSize / 3 << endl;
	fread(imageData,imSize*sizeof(unsigned char),1,fp);
	fclose(fp);

	bmp b;
	int i,j,k;
	memcpy(&(b.bfHeader),&bfHeader,sizeof(bfHeader));
	memcpy(&(b.biHeader),&biHeader,sizeof(biHeader));
	b.bfHeader.bfOffBits=1078;
	b.biHeader.biBitCount=8;
	unsigned char* imgDataEight = new unsigned char[imgSize] {}; //存储转换成的八位图
	for (unsigned int i = 0; i < biHeader.biSizeImage; i += 3) {
		int tmp;
		tmp = (float)imageData[i] * 0.114f + (float)imageData[i + 1] * 0.587f + (float)imageData[i + 2] * 0.299f;
		imgDataEight[i / 3] = (unsigned char)tmp;
	}

	int lineBytes_new=(width*8+31)/32*4; //閲嶆柊璁＄畻姣忚鏁版嵁瀛楄妭
//	cout << "lineBytes_new:" << lineBytes_new << endl;
//	cout << "height:" << height << endl;
	b.biHeader.biSizeImage=lineBytes_new*height;
//	cout << "b.biHeader.biSizeImage:" << b.biHeader.biSizeImage << endl;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //鏀瑰彉鏂囦欢鏁版嵁澶у皬
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}

	for(int i = 0; i < height; i++)
		for(int j = 0; j < lineBytes_new; j++) {
			int a[9] = {0};
			for(int k = -1; k < 2; k++)
				for(int l = -1; l < 2; l++) {
					if(i + k < 0) {
						if(j + l < 0) {
							a[(k+1) * 3 + (l+1)] = imgDataEight[0];
							continue;
						}
						if(j + l >= lineBytes_new) {
							a[(k+1) * 3 + (l+1)] = imgDataEight[j];
							continue;
						} else {
							a[(k+1) * 3 + (l+1)] = imgDataEight[j + l];
							continue;
						}
					}
					if(i + k >= height) {
						if(j + l < 0) {
							a[(k+1) * 3 + (l+1)] = imgDataEight[lineBytes_new * i];
							continue;
						}
						if(j + l >= height) {
							a[(k+1) * 3 + (l+1)] = imgDataEight[lineBytes_new * i + j];
							continue;
						} else {
							a[(k+1) * 3 + (l+1)] = imgDataEight[lineBytes_new * i + (j + l)];
							continue;
						}
					} else {
						if(j + l < 0) {
							a[(k+1) * 3 + (l+1)] = imgDataEight[lineBytes_new * (i+k)];
							continue;
						}
						if(j + l > height) {
							a[(k+1) * 3 + (l+1)] = imgDataEight[lineBytes_new * (i+k) + j];
							continue;
						} else a[(k+1) * 3 + (l+1)] = imgDataEight[lineBytes_new * (i+k) + j + l];
					}
				}
//			for(int i = 0; i < 9; i++) cout << a[i] << " ";
//			cout << endl;
			sort(a,a+9);
			b.imgData[i * lineBytes_new + j] = a[4];
//			cout << "b.imgData[" << i * lineBytes_new + j << "]=" <<  (int)b.imgData[i * lineBytes_new + j] << endl;
		}

	char savePath1[] ="D://examples//pictures//homework_3//noise2_median_filtering.bmp";
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

	free(imageData);
	free(b.imgData);
}

int main() {
	operating_1();
	operating_2();
	return 0;
}
