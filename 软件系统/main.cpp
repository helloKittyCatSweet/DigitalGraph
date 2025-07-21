#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
#include<algorithm>
#include<cmath>
#include<queue>
#include<utility>
#include<set>
#include<stack>

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

// prepare for test3

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

// prepare for test4

typedef struct {
	int x;
	int y;
} point;

point multiple(double a[3][3], int b[3]) {
	int ans[3];
	for(int i = 0; i < 3; i++) {
		double sum = 0;
		for(int j = 0; j < 3; j++) {
			sum += a[i][j] * b[j];
		}
		ans[i] = (int)round(sum);
	}
	return point{ans[0],ans[1]};
}

point bigger_or_smaller(double a,double d,point p0) {
	double matrix[3][3] = {0};
	matrix[0][0] = 1.0 / a;
	matrix[1][1] = 1.0 / d;
	matrix[2][2] = 1;
	int vec[3] = {p0.x, p0.y, 1};
	point ans = multiple(matrix, vec);
	return ans;
}

point rotate(double a,double b,double c,double d,point p0) {
	double matrix[3][3] = {0};
	matrix[0][0] = a;
	matrix[0][1] = b;
	matrix[1][0] = c;
	matrix[1][1] = d;
	matrix[2][2] = 1;
	int vec[3] = {p0.x, p0.y, 1};
	point ans = multiple(matrix, vec);
	return ans;
}

void test1() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_1//rgb.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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
	b.biHeader.biBitCount=8;  //改变图像位数
	int lineBytes_new=(width*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new*height;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}

	// prepare for grey figure

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			//将每一个像素都按公式y=B*0.114+G*0.587+R*0.299进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.114
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.299;
		}
	}

	char savePath1[]="D://examples//pictures//homework_11//rgb_to_grey.bmp";
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

	// prepare for anti-grey figure

	for(i=0; i<height; i++)
		for(j=0; j<width; j++)
			b.imgData[lineBytes_new*i+j]= 255 - b.imgData[lineBytes_new*i+j];

	char savePath2[] ="D://examples//pictures//homework_11//rgb_to_antigrey.bmp";
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

	// prepare for separate R/G/B figure
	// the figure is constucted with the sequence of B and G and R

	// blue

	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<imSize; i+=3) b.imgData[i / 3]= imageData[i];

	char savePath3[] ="D://examples//pictures//homework_11//rgb_extract_blue.bmp";
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

	// green

	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<imSize; i+=3) b.imgData[i / 3]= imageData[i + 1];

	char savePath4[] ="D://examples//pictures//homework_11//rgb_extract_green.bmp";
	f=fopen(savePath4,"wb");
	if(f==NULL) {
		perror("can not open file!");
		exit(-2);
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);

	// red

	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<imSize; i+=3) b.imgData[i / 3]= imageData[i + 2];

	char savePath5[] ="D://examples//pictures//homework_11//rgb_extract_red.bmp";
	f=fopen(savePath5,"wb");
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

void test2() {
	int colors[256];
	memset(colors,0,sizeof(colors));

	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_2//dim.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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

	for(int i = 0; i < imSize; i++)	colors[imageData[i]]++;

	int maxHeight = 0;
	int sum = 0;
	for(int i = 0; i < 256; i++) {
		maxHeight = max(maxHeight,colors[i]);
		sum += colors[i];
	}

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

	char savePath2[]="D://examples//pictures//homework_11//dim_hist_normalization.bmp";
	FILE *f=fopen(savePath2,"wb");
	if(f==NULL) {
		perror("can not open file!");
		exit(-2);
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

	char savePath3[]="D://examples//pictures//homework_11//dim_equalization.bmp";
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

	// equalization hist

	b.biHeader.biHeight = 256;
	b.biHeader.biWidth = 256;
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

	char savePath4[]="D://examples//pictures//homework_11//dim_hist_equalization.bmp";
	f=fopen(savePath4,"wb");
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

	double c[9];
	for(int i = 0; i < 9; i++) c[i] = 1.0 /9;
	b.imgData = convolution(b.imgData, {imageData,height,lineBytes_new},core{c,3,3});

	char savePath1[] ="D://examples//pictures//homework_11//lena_neighborhood_average_filtering_9_cov.bmp";
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
	int lineBytes=(width*bitCount+31)/32*4;
	int imSize = lineBytes * height;
	biHeader.biSizeImage = imSize;

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
	unsigned char* imgDataEight = new unsigned char[imgSize] {}; //存储转换成的八位图
	for (unsigned int i = 0; i < biHeader.biSizeImage; i += 3) {
		int tmp;
		tmp = (float)imageData[i] * 0.114f + (float)imageData[i + 1] * 0.587f + (float)imageData[i + 2] * 0.299f;
		imgDataEight[i / 3] = (unsigned char)tmp;
	}

	int lineBytes_new=(width*8+31)/32*4; //閲嶆柊璁＄畻姣忚鏁版嵁瀛楄妭
	b.biHeader.biSizeImage=lineBytes_new*height;
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
			sort(a,a+9);
			b.imgData[i * lineBytes_new + j] = a[4];
		}

	char savePath1[] ="D://examples//pictures//homework_11//noise2_median_filtering.bmp";
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

void test3() {
	operating_1();
	operating_2();
}

void test4() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//lena.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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

	b.biHeader.biHeight = height / 2;
	b.biHeader.biWidth = width / 2;
	int lineBytes_new=(b.biHeader.biWidth*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new * b.biHeader.biHeight;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}

	// a smaller lena.bmp

	double a = 0.5;
	double d = 0.5;

	for(int i = 0; i < b.biHeader.biHeight; i++)
		for(int j = 0; j < lineBytes_new; j++) {
			point fmap = bigger_or_smaller(a,d,point{i,j});
			if(fmap.x < 0) fmap.x = 0;
			if(fmap.x >= height) fmap.x = height - 1;
			if(fmap.y < 0) fmap.y = 0;
			if(fmap.y >= width) fmap.y = width - 1;
			b.imgData[i * lineBytes_new + j] = imageData[fmap.x * lineBytes + fmap.y];
		}

	char savePath1[]="D://examples//pictures//homework_11//lena_smaller.bmp";
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

	// a bigger lena.bmp

	b.biHeader.biHeight = height * 2;
	b.biHeader.biWidth = width * 2;
	lineBytes_new=(b.biHeader.biWidth*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new * b.biHeader.biHeight;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);

	a = 2.0;
	d = 2.0;

	for(int i = 0; i < b.biHeader.biHeight; i++)
		for(int j = 0; j < lineBytes_new; j++) {
			point fmap = bigger_or_smaller(a,d,point{i,j});
			if(fmap.x < 0) fmap.x = 0;
			if(fmap.x >= height) fmap.x = height - 1;
			if(fmap.y < 0) fmap.y = 0;
			if(fmap.y >= width) fmap.y = width - 1;
			b.imgData[i * lineBytes_new + j] = imageData[fmap.x * lineBytes + fmap.y];
		}

	char savePath2[]="D://examples//pictures//homework_11//lena_bigger.bmp";
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

	// translation of the figure

	b.biHeader.biHeight = height;
	b.biHeader.biWidth = width;
	lineBytes_new=(b.biHeader.biWidth*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new * b.biHeader.biHeight;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,255,sizeof(unsigned char)*b.biHeader.biSizeImage);

	int dx = -50;
	int dy = -50;

	for(int i = 0; i < b.biHeader.biHeight; i++) {
		for(int j = 0; j < lineBytes_new; j++) {
			int x = i + dx;
			int y = j + dy;
			if(x >= height || x < 0) continue;
			if(y >= width || y < 0) continue;
			b.imgData[i * lineBytes_new + j] = imageData[x * lineBytes + j];
		}
	}

	char savePath3[]="D://examples//pictures//homework_11//lena_translation.bmp";
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

	// mirror the figure

	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);

	for(int i = 0; i < b.biHeader.biHeight; i++)
		for(int j = 0; j < lineBytes_new; j++)
			b.imgData[i * lineBytes_new + j] = imageData[i * lineBytes + (lineBytes - j)];

	char savePath4[]="D://examples//pictures//homework_11//lena_mirror.bmp";
	f=fopen(savePath4,"wb");
	if(f==NULL) {
		perror("can not open file!");
		exit(-2);
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);

	// upside down the figure

	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);

	for(int i = 0; i < b.biHeader.biHeight; i++)
		for(int j = 0; j < lineBytes_new; j++)
			b.imgData[i * lineBytes_new + j] = imageData[(b.biHeader.biHeight - i) * lineBytes + j];

	char savePath5[]="D://examples//pictures//homework_11//lena_upside_down.bmp";
	f=fopen(savePath5,"wb");
	if(f==NULL) {
		perror("can not open file!");
		exit(-2);
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);

	// rotate the figure

	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,255,sizeof(unsigned char)*b.biHeader.biSizeImage);

	double theta = 30 * 3.1415926 / 180.0;
	a = cos(theta);
	d = -sin(theta);

	for(int i = 0; i < b.biHeader.biHeight; i++)
		for(int j = 0; j < lineBytes_new; j++) {
			// cos(theta) -sin(theta) sin(theta) cos(theta)
			point fmap = rotate(a,d,-d,a,point{i,j});
			if(fmap.x < 0 || fmap.x >= height) continue;
			if(fmap.y < 0 || fmap.y >= width) continue;
			b.imgData[i * lineBytes_new + j] = imageData[fmap.x * lineBytes + fmap.y];
		}

	char savePath6[]="D://examples//pictures//homework_11//lena_rotate.bmp";
	f=fopen(savePath6,"wb");
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

void test5() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_5//lena.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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

	int cut = maxHeight / 256;
	b.biHeader.biHeight = maxHeight / cut;
	b.biHeader.biWidth = 256;
	int lineBytes_new=(b.biHeader.biWidth*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new * b.biHeader.biHeight;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,255,sizeof(unsigned char)*b.biHeader.biSizeImage);

	// pick 110 as the threshold
	int threshold = 110;
	for(int i = 0; i < b.biHeader.biWidth; i++)
		if(i == threshold)
			for(int j = 0; j < b.biHeader.biHeight; j++)
				b.imgData[j * b.biHeader.biWidth + i] = 100;
		else
			for(int j = 0, h = colors[i] / cut; j < b.biHeader.biHeight; j++)
				if(j <= h) b.imgData[j * b.biHeader.biWidth + i] = 0;
				else break;

	char savePath1[]="D://examples//pictures//homework_11//lena_hist110.bmp";
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

	char savePath2[]="D://examples//pictures//homework_11//lena_manually_threshold110.bmp";
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
	for(int i = 0; i < imSize; i++)
		b.imgData[i] = imageData[i] >= threshold ? 255 : 0;

	char savePath3[]="D://examples//pictures//homework_11//lena_iteration.bmp";
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
	b.imgData[i] = imageData[i] >= threshold ? 255 : 0;

	char savePath4[]="D://examples//pictures//homework_11//lena_otsu.bmp";
	f=fopen(savePath4,"wb");
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

void bubble() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_6//bubble.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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
	b.biHeader.biBitCount=8;  //改变图像位数
	int lineBytes_new=(width*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new*height;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}

	// prepare for grey figure

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			//将每一个像素都按公式y=B*0.114+G*0.587+R*0.299进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.114
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.299;
		}
	}

	int point1 = lineBytes_new * height / 2 + lineBytes_new / 3 - 20;
	int point2 = lineBytes_new + 100;
	int origin1 = b.imgData[point1];
	int origin2 = b.imgData[point2];
	b.imgData[point1] = 255;
	b.imgData[point2] = 255;

	char savePath1[]="D://examples//pictures//homework_11//bubble_to_grey.bmp";
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

	// seed growth

	b.imgData[point1] = origin1;
	b.imgData[point2] = origin2;

	// point1

	int threshold1 = 2;
	int dx[9] = {-1,0,1,-1,0,1,-1,0,1};
	int dy[9] = {1,1,1,0,0,0,-1,-1,-1};
	bool visit[lineBytes_new * height];
	memset(visit,false,sizeof(bool)*lineBytes_new * height);

	queue<int> q1;
	q1.push(point1);
	while(!q1.empty()) {
		int t = q1.front();
		q1.pop();
		for(int i = 0; i < 9; i++) {
			int x = t / lineBytes_new + dx[i];
			int y = t % lineBytes_new + dy[i];
			int tmp = x * lineBytes_new + y;
			if(x >= 0 && x < height && y >= 0 && y < lineBytes_new
			        && abs(b.imgData[tmp] - b.imgData[t]) <= threshold1 && !visit[tmp]) {
				q1.push(tmp);
				visit[tmp] = true;
			}
		}
	}
	for(int i = 0; i < height; i++)
		for(int j = 0; j < lineBytes_new; j++)
			if(visit[i * lineBytes_new + j])
				b.imgData[i * lineBytes_new + j] = 255;

	// point2

	int threshold2 = 2;
	memset(visit,false,sizeof(bool)*lineBytes_new * height);

	queue<int> q2;
	q2.push(point2);
	while(!q2.empty()) {
		int t = q2.front();
		q2.pop();
		for(int i = 0; i < 9; i++) {
			int x = t / lineBytes_new + dx[i];
			int y = t % lineBytes_new + dy[i];
			int tmp = x * lineBytes_new + y;
			if(x >= 0 && x < height && y >= 0 && y < lineBytes_new
			        && abs(b.imgData[tmp] - b.imgData[t]) <= threshold2 && !visit[tmp]) {
				q2.push(tmp);
				visit[tmp] = true;
			}
		}
	}
	for(int i = 0; i < height; i++)
		for(int j = 0; j < lineBytes_new; j++)
			if(visit[i * lineBytes_new + j])
				b.imgData[i * lineBytes_new + j] = 0;

	char savePath2[]="D://examples//pictures//homework_11//bubble_seed_growth.bmp";
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

	free(imageData);
	free(b.imgData);
}

int check(int up,int down,int left,int right, unsigned char* imageData,int lineBytes_new) {
	int sum = 0;
	for(int i = down ; i <= up; i++)
		for(int j = left ; j <= right; j++)
			sum += (int)imageData[i * lineBytes_new + j];
	double avg = 1.0 * sum / ((up - down) * (right - left));
	double dsum = 0;
	for(int i = down ; i <= up; i++)
		for(int j = left ; j <= right; j++)
			dsum += pow(((int)imageData[i * lineBytes_new + j] - avg),2) ;
	dsum /= (up - down) * (right - left);
	return round(dsum);
}

//	up down left right
void dfs(int up, int down, int left, int right,unsigned char* imageData,unsigned char* modify,int lineBytes_new) {
	int threshold = 8;
	int length = up - down;
	int width = right - left;
	if(length / 2 <= threshold && width / 2 <= threshold) return;

	if(check(up,down,left,right,imageData,lineBytes_new) > 400) {
		if(length / 2 >= threshold && width / 2 >= threshold) {
			for(int i = down; i <= up; i++) modify[i * lineBytes_new + width / 2 + left] = 255;
			for(int i = left; i <= right; i++) modify[(down + length / 2) * lineBytes_new + i] = 255;
			dfs(up,down + length / 2,left,left + width / 2,imageData,modify,lineBytes_new);
			dfs(down + length / 2, down, left, left + width / 2, imageData,modify,lineBytes_new);
			dfs(up,down + length / 2,left + width / 2, right, imageData, modify, lineBytes_new);
			dfs(down + length / 2, down,left + width / 2,right, imageData, modify, lineBytes_new);
		}
	}
	return;
}

void gap() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_6//gap.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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
	b.biHeader.biBitCount=8;  //改变图像位数
	int lineBytes_new=(width*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new*height;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}

	// prepare for grey figure

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			//将每一个像素都按公式y=B*0.114+G*0.587+R*0.299进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.114
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.299;
		}
	}

	unsigned char* save = (unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(int i = 0; i < b.biHeader.biSizeImage; i++) save[i] = b.imgData[i];

	// mark the seed point

	int point1 = lineBytes_new * height * 2 / 3 + 10;
	int origin1 = b.imgData[point1];
	b.imgData[point1] = 0;

	char savePath1[]="D://examples//pictures//homework_11//gap_to_grey.bmp";
	FILE* f=fopen(savePath1,"wb");
	if(f==NULL) {
		perror("can not open file!");
		exit(-2);
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);

	// seed growth

	b.imgData[point1] = origin1;

	// point1

	int threshold1 = 12;
	int dx[9] = {-1,0,1,-1,0,1,-1,0,1};
	int dy[9] = {1,1,1,0,0,0,-1,-1,-1};
	bool visit[lineBytes_new * height];
	memset(visit,false,sizeof(bool)*lineBytes_new * height);

	queue<int> q1;
	q1.push(point1);
	while(!q1.empty()) {
		int t = q1.front();
		q1.pop();
		for(int i = 0; i < 9; i++) {
			int x = t / lineBytes_new + dx[i];
			int y = t % lineBytes_new + dy[i];
			int tmp = x * lineBytes_new + y;
			if(x >= 0 && x < height && y >= 0 && y < lineBytes_new
			        && abs(b.imgData[tmp] - b.imgData[t]) <= threshold1 && !visit[tmp]) {
				q1.push(tmp);
				visit[tmp] = true;
			}
		}
	}
	for(int i = 0; i < height; i++)
		for(int j = 0; j < lineBytes_new; j++)
			if(visit[i * lineBytes_new + j])
				b.imgData[i * lineBytes_new + j] = 0;

	char savePath2[]="D://examples//pictures//homework_11//gap_seed_growth.bmp";
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

	// aera division

	for(int i = 0; i < b.biHeader.biSizeImage; i++) b.imgData[i] = save[i];
	dfs(height - 1,0,0,lineBytes_new - 1,save,b.imgData,lineBytes_new);

	char savePath3[]="D://examples//pictures//homework_11//gap_aera_division.bmp";
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
}


void test6() {
	bubble();
	gap();
}

void test7() {
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
	unsigned char* gx = (unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(gx,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	unsigned char* gy = (unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(gy,0,sizeof(unsigned char)*b.biHeader.biSizeImage);

	double c1[9] = {-1,0,1,-1,0,1,-1,0,1};
	gx = convolution(b.imgData, {imageData,height,lineBytes_new},core{c1,3,3});
	double c2[9] = {-1,-1,-1,0,0,0,1,1,1};
	gy = convolution(b.imgData, {imageData,height,lineBytes_new},core{c2,3,3});
	for(int i = 0; i < b.biHeader.biSizeImage; i++) {
		if(sqrt(gx[i] * gx[i] + gy[i] * gy[i]) > 75) b.imgData[i] = 255;
		else b.imgData[i] = 0;
	}

	char savePath1[] ="D://examples//pictures//homework_11//lena_prewitt.bmp";
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
	memset(gx,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(gy,0,sizeof(unsigned char)*b.biHeader.biSizeImage);

	double c3[9] = {-1,0,1,-2,0,2,-1,0,1};
	gx = convolution(b.imgData, {imageData,height,lineBytes_new},core{c3,3,3});
	double c4[9] = {-1,-2,-1,0,0,0,1,2,1};
	gy = convolution(b.imgData, {imageData,height,lineBytes_new},core{c4,3,3});
	for(int i = 0; i < b.biHeader.biSizeImage; i++) {
		if(sqrt(gx[i] * gx[i] + gy[i] * gy[i]) > 75) b.imgData[i] = 255;
		else b.imgData[i] = 0;
	}

	char savePath2[] ="D://examples//pictures//homework_11//lena_sobel.bmp";
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
	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		b.imgData[i] >= threshold ? 255 : 0;

	char savePath3[] ="D://examples//pictures//homework_11//lena_log.bmp";
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
}

void test8() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_8//draw.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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
	b.biHeader.biBitCount=8;  //改变图像位数
	int lineBytes_new=(width*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new*height;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}

	// prepare for grey figure

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			//将每一个像素都按公式y=B*0.114+G*0.587+R*0.299进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.114
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.299;
		}
	}

	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		b.imgData[i] = b.imgData[i] > 128 ?  255 : 0;

	char savePath1[]="D://examples//pictures//homework_11//draw_to_2.bmp";
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

	// hough

	int r = (int)round(sqrt(height * height + lineBytes_new * lineBytes_new));
	int delta = 1;
	int sigma = 180 / delta;
	int* sum = (int*) malloc (sizeof(int) * 2 * r * sigma);
	memset(sum,0,sizeof(int) * 2 * r * sigma);

	for(int i = 0; i < height; i++)
		for(int j = 0; j < lineBytes_new; j++) {
			if(b.imgData[i * lineBytes_new + j]) {
				int s = 0;
				while(true) {
					const int p = j * cos(s * M_PI / 180) + i * sin(s * M_PI / 180) + r;
					sum[p * sigma + s / delta]++;
					s += delta;
					if(s > 180) break;
				}
			}
		}

	int max_result = 0;
	for(int i = 0; i < 2 * r * sigma; i++)
		if(sum[i] > max_result)
			max_result = sum[i];

	for(int i = 0; i < height; i++)
		for(int j = 0; j < lineBytes_new ; j++) {
			int s = 0;
			while(true) {
				const int p = j * cos(s * M_PI / 180) + i * sin(s * M_PI / 180) + r;
				if(sum[p * sigma + s / delta] > max_result * 0.2)
					b.imgData[i * lineBytes_new + j] = 200;
				s += delta;
				if(s > 180) break;
			}
		}

	char savePath2[]="D://examples//pictures//homework_11//draw_hough.bmp";
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

	free(imageData);
	free(b.imgData);
}

void pic() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_9//pic.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			//将每一个像素都按公式y=B*0.114+G*0.587+R*0.299进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.114
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.299;
		}
	}

	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		(int)b.imgData[i] >= 128 ? b.imgData[i] = 255 : b.imgData[i] = 0;

	char savePath1[]="D://examples//pictures//homework_11//pic_to2.bmp";
	FILE* f=fopen(savePath1,"wb");
	if(f==NULL) {
		perror("can not open file!");
		exit(-2);
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);

	// zone marker

	stack<int> st;
	int* label = (int*)malloc(sizeof(int)*b.biHeader.biSizeImage);
	memset(label,0,sizeof(int)*b.biHeader.biSizeImage);
	int cnt = 1;
	for(int i = 1; i < height; i++)
		for(int j = 1; j < lineBytes_new - 1; j++) {
			if(b.imgData[i * lineBytes_new + j] == 255) {
				int l1 = label[i * lineBytes_new + j - 1]; // left
				int l2 = label[(i - 1) * lineBytes_new + j - 1]; // left-up
				int l3 = label[(i - 1) * lineBytes_new + j]; // up
				int l4 = label[(i - 1) * lineBytes_new + j + 1]; // right-up
				if(l1 == 0 && l2 == 0&& l3 == 0 && l4 == 0) {
					label[i * lineBytes_new + j] = cnt++;
					continue;
				}
				label[i * lineBytes_new + j]=0;

				if(l1 != 0) {
					label[i * lineBytes_new + j] = l1;
				}
				if(l2 != 0) {
					if(label[i * lineBytes_new + j] != 0) {
						if(label[i * lineBytes_new + j] != l2) {
							st.push(label[i * lineBytes_new + j]);
							st.push(l2);
						}
					} else {
						label[i * lineBytes_new + j] = l2;
					}
				}
				if(l3 != 0) {
					if(label[i * lineBytes_new + j] != 0) {
						if(label[i * lineBytes_new + j] != l3) {
							st.push(label[i * lineBytes_new + j]);
							st.push(l3);
						}
					} else {
						label[i * lineBytes_new + j] = l3;
					}
				}
				if(l4 != 0) {
					if(label[i * lineBytes_new + j] != 0) {
						if(label[i * lineBytes_new + j] != l4) {
							st.push(label[i * lineBytes_new + j]);
							st.push(l4);
						}
					} else {
						label[i * lineBytes_new + j] = l4;
					}
				}
			}
		}

	set<int> s;
	while(!st.empty()) {
		int i1 = st.top();
		st.pop();
		int i2 = st.top();
		st.pop();
		int small = min(i1,i2);
		int big = max(i1,i2);

		s.insert(small);
		if(s.count(big)) {
			s.erase(big);
			for(int i = 0; i < b.biHeader.biSizeImage; i++)
				if(label[i] == big)
					label[i] = small;
		}
	}

	int col = 50;
	for(int i = 1; i <= cnt; i++) {
		for(int j = 0; j < b.biHeader.biSizeImage; j++)
			if(label[j] == i)
				b.imgData[j] = col;
		col += 10;
	}

	char savePath2[]="D://examples//pictures//homework_11//pic_zone_marker.bmp";
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

	free(imageData);
	free(b.imgData);
}

void coll() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_9//col.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			//将每一个像素都按公式y=B*0.114+G*0.587+R*0.299进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.114
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.299;
		}
	}

	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		(int)b.imgData[i] >= 128 ? b.imgData[i] = 255 : b.imgData[i] = 0;

	char savePath1[]="D://examples//pictures//homework_11//col_to2.bmp";
	FILE* f=fopen(savePath1,"wb");
	if(f==NULL) {
		perror("can not open file!");
		exit(-2);
	}
	fwrite(&b.bfHeader,sizeof(bitmapFileHeader),1,f);
	fwrite(&b.biHeader,sizeof(bitmapInfoHeader),1,f);
	fwrite(&b.palette,1024,1,f);
	fwrite(b.imgData,sizeof(unsigned char)*b.biHeader.biSizeImage,1,f);
	fclose(f);

	// zone marker

	stack<int> st;
	int* label = (int*)malloc(sizeof(int)*b.biHeader.biSizeImage);
	memset(label,0,sizeof(int)*b.biHeader.biSizeImage);
	int cnt = 1;
	for(int i = 1; i < height; i++)
		for(int j = 1; j < lineBytes_new - 1; j++) {
			if(b.imgData[i * lineBytes_new + j] == 0) {
				int l1 = label[i * lineBytes_new + j - 1]; // left
				int l2 = label[(i - 1) * lineBytes_new + j - 1]; // left-up
				int l3 = label[(i - 1) * lineBytes_new + j]; // up
				int l4 = label[(i - 1) * lineBytes_new + j + 1]; // right-up
				if(l1 == 0 && l2 == 0&& l3 == 0 && l4 == 0) {
					label[i * lineBytes_new + j] = cnt++;
					continue;
				}
				label[i * lineBytes_new + j]=0;

				if(l1 != 0) {
					label[i * lineBytes_new + j] = l1;
				}
				if(l2 != 0) {
					if(label[i * lineBytes_new + j] != 0) {
						if(label[i * lineBytes_new + j] != l2) {
							st.push(label[i * lineBytes_new + j]);
							st.push(l2);
						}
					} else {
						label[i * lineBytes_new + j] = l2;
					}
				}
				if(l3 != 0) {
					if(label[i * lineBytes_new + j] != 0) {
						if(label[i * lineBytes_new + j] != l3) {
							st.push(label[i * lineBytes_new + j]);
							st.push(l3);
						}
					} else {
						label[i * lineBytes_new + j] = l3;
					}
				}
				if(l4 != 0) {
					if(label[i * lineBytes_new + j] != 0) {
						if(label[i * lineBytes_new + j] != l4) {
							st.push(label[i * lineBytes_new + j]);
							st.push(l4);
						}
					} else {
						label[i * lineBytes_new + j] = l4;
					}
				}
			}
		}

	set<int> s;
	while(!st.empty()) {
		int i1 = st.top();
		st.pop();
		int i2 = st.top();
		st.pop();
		int small = min(i1,i2);
		int big = max(i1,i2);

		s.insert(small);
		if(s.count(big)) {
			s.erase(big);
			for(int i = 0; i < b.biHeader.biSizeImage; i++)
				if(label[i] == big)
					label[i] = small;
		}
	}

	int col = 50;
	for(int i = 1; i <= cnt; i++) {
		for(int j = 0; j < b.biHeader.biSizeImage; j++)
			if(label[j] == i)
				b.imgData[j] = col;
		col += 10;
	}

	char savePath2[]="D://examples//pictures//homework_11//col_marker.bmp";
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

	free(imageData);
	free(b.imgData);
}

void test9() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_9//region_mark.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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

	char savePath1[]="D://examples//pictures//homework_11//region_mark_to2.bmp";
	FILE* f=fopen(savePath1,"wb");
	if(f==NULL) {
		perror("can not open file!");
		exit(-2);
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

	char savePath2[]="D://examples//pictures//homework_11//region_mark_zone_marker.bmp";
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

	free(imageData);
	free(b.imgData);

	pic();
	coll();
}

void circle() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_10//circle.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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
	imSize = height * lineBytes;

	fseek(fp,bfHeader.bfOffBits,SEEK_SET);
	unsigned char*imageData=(unsigned char*)malloc(imSize*sizeof(unsigned char));
	fread(imageData,imSize*sizeof(unsigned char),1,fp);
	fclose(fp);

	bmp b;
	int i,j,k;
	memcpy(&(b.bfHeader),&bfHeader,sizeof(bfHeader));
	memcpy(&(b.biHeader),&biHeader,sizeof(biHeader));
	b.bfHeader.bfOffBits=1078;
	b.biHeader.biBitCount=8;  //改变图像位数
	int lineBytes_new=(width*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new*height;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}

	// prepare for grey figure

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			//将每一个像素都按公式y=B*0.114+G*0.587+R*0.299进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.114
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.299;
		}
	}

	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		(int)b.imgData[i] > 128 ? b.imgData[i] = 255 : b.imgData[i] = 0;

	char savePath1[]="D://examples//pictures//homework_11//circle_to_2.bmp";
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

	// circle extraction

	unsigned char* copy = (unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		copy[i] = b.imgData[i];

	int dx[8] = {-1,0,1,-1,1,-1,0,1};
	int dy[8] = {1,1,1,0,0,-1,-1,-1};
	for(int i = 0; i < height; i++)
		for(int j = 0; j < lineBytes_new; j++) {
			if((int)copy[i * lineBytes_new + j] == 0) {
				int m;
				bool flag = true;
				for(m = 0; m < 8; m++) {
					int x = i + dx[m];
					int y = j + dy[m];
					if(x < 0 || x >= height || y < 0 || y >= lineBytes_new ||
					        (int)copy[x * lineBytes_new + y] != 0) {
						flag = false;
						break;
					}
				}
				if(flag) {
					b.imgData[i * lineBytes_new + j] = 255;
				} else {
					b.imgData[i * lineBytes_new + j] = 0;
				}
			} else b.imgData[i * lineBytes_new + j] = 255;
		}

	char savePath2[]="D://examples//pictures//homework_11//circle_extraction.bmp";
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

	free(imageData);
	free(b.imgData);
}

void hist() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//homework_10//hist.bmp","rb"))==NULL) {
		perror("can not open file!");
		exit(-1);
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
	imSize = height * lineBytes;

	fseek(fp,bfHeader.bfOffBits,SEEK_SET);
	unsigned char*imageData=(unsigned char*)malloc(imSize*sizeof(unsigned char));
	fread(imageData,imSize*sizeof(unsigned char),1,fp);
	fclose(fp);

	bmp b;
	int i,j,k;
	memcpy(&(b.bfHeader),&bfHeader,sizeof(bfHeader));
	memcpy(&(b.biHeader),&biHeader,sizeof(biHeader));
	b.bfHeader.bfOffBits=1078;
	b.biHeader.biBitCount=8;  //改变图像位数
	int lineBytes_new=(width*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new*height;
	b.bfHeader.bfSize=1078+b.biHeader.biSizeImage; //改变文件数据大小
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(i=0; i<256; i++) {
		b.palette[i].rgbBlue=i;
		b.palette[i].rgbGreen=i;
		b.palette[i].rgbRed=i;
	}

	// prepare for grey figure

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			//将每一个像素都按公式y=B*0.114+G*0.587+R*0.299进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.114
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.299;
		}
	}

	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		(int)b.imgData[i] > 128 ? b.imgData[i] = 255 : b.imgData[i] = 0;

	char savePath1[]="D://examples//pictures//homework_11//hist_to_2.bmp";
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

	// hist extraction

	unsigned char* copy = (unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		copy[i] = b.imgData[i];

	for(int i = 1; i < height - 1; i++)
		for(int j = 1; j < lineBytes_new - 1; j++) {
			// 255 white is the background, and 0 black is the target
			// all points should be white, so this point should be inner one
			if((int)copy[i * lineBytes_new + j] == 0 && (int)copy[(i - 1) * lineBytes_new + j + 1] == 0 &&
			        (int)copy[i * lineBytes_new + j + 1] == 0 && (int)copy[(i + 1) * lineBytes_new + j + 1] == 0 &&
			        (int)copy[(i - 1) * lineBytes_new + j] == 0 && (int)copy[(i + 1) * lineBytes_new + j] == 0 &&
			        (int)copy[(i - 1) * lineBytes_new + (j - 1)] == 0 && (int)copy[i * lineBytes_new + j - 1] == 0 &&
			        (int)copy[(i + 1) * lineBytes_new + j - 1] == 0) {
				b.imgData[i * lineBytes_new + j] = 255;
			}
		}

	char savePath2[]="D://examples//pictures//homework_11//hist_extraction.bmp";
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

	free(imageData);
	free(b.imgData);
}

void test10() {
	circle();
	hist();
}

void test11() {
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();
	test8();
	test9();
	test10();
}

int main() {
	bool flag = true;
	cout << "********************************************************************" << endl;
	cout << "***********************digital image processing*********************" << endl;
	cout << "***********1 convert true color image to gray image,  **************" << endl;
	cout << "***********  invert the grey value and separate rgb value***********" << endl;
	cout << "***********  and make anti-color gray image ************************" << endl;
	cout << "***********2 draw histogram of an image and do histogram ***********" << endl;
	cout << "***********  equalization*******************************************" << endl;
	cout << "***********3 use template convolution to smooth an image ***********" << endl;
	cout << "***********  and use medium filter to smooth an image **************" << endl;
	cout << "***********4 scale the image, translate the image, *****************" << endl;
	cout << "***********  mirror the image, rotate the image ********************" << endl;
	cout << "***********5 do threshold given T, do threshold by iterating, ******" << endl;
	cout << "***********  do otsu thresholding **********************************" << endl;
	cout << "***********6 do region growing and do region splitting *************" << endl;
	cout << "***********7 do Prewitt edge detection, do Sobel edge detection*****" << endl;
	cout << "***********  and do LOG edge detection *****************************" << endl;
	cout << "***********8 do hough transform*************************************" << endl;
	cout << "***********9 mark the regions***************************************" << endl;
	cout << "***********10 extract the edges*************************************" << endl;
	cout << "***********11 do batch processing***********************************" << endl;
	cout << "********************************************************************" << endl;
	cout << endl;
	cout << "******please input 1-11 to run the function, or input 0 to exit*****" << endl;
	while(flag) {
		int n;
		cin >> n;
		switch(n) {
			case 0:
				flag = false;
				break;
			case 1:
				test1();
				cout << "function 1 has been finished, please check in the folder homework11" << endl;
				break;
			case 2:
				test2();
				cout << "function 2 has been finished, please check in the folder homework11" << endl;
				break;
			case 3:
				test3();
				cout << "function 3 has been finished, please check in the folder homework11" << endl;
				break;
			case 4:
				test4();
				cout << "function 4 has been finished, please check in the folder homework11" << endl;
				break;
			case 5:
				test5();
				cout << "function 5 has been finished, please check in the folder homework11" << endl;
				break;
			case 6:
				test6();
				cout << "function 6 has been finished, please check in the folder homework11" << endl;
				break;
			case 7:
				test7();
				cout << "function 7 has been finished, please check in the folder homework11" << endl;
				break;
			case 8:
				test8();
				cout << "function 8 has been finished, please check in the folder homework11" << endl;
				break;
			case 9:
				test9();
				cout << "function 9 has been finished, please check in the folder homework11" << endl;
				break;
			case 10:
				test10();
				cout << "function 10 has been finished, please check in the folder homework11" << endl;
				break;
			case 11:
				test11();
				cout << "function 11 has been finished, please check in the folder homework11" << endl;
				break;
			default:
				continue;
		}
		cout << "continue..." << endl;
	}
	return 0;
}