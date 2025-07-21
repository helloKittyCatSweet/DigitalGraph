#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<cmath>
#include<iostream>

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

typedef struct {
	int x;
	int y;
} point;

point multiple(double a[3][3], int b[3]){
	int ans[3];
	for(int i = 0; i < 3; i++){
		double sum = 0;
		for(int j = 0; j < 3; j++){
			sum += a[i][j] * b[j];
		}
		ans[i] = (int)round(sum);
//		cout << "sum:" << sum << "  " << "ans[" << i << "]:" << ans[i] << endl;
	}
	return point{ans[0],ans[1]};
}

point bigger_or_smaller(double a,double d,point p0){
	double matrix[3][3] = {0};
	matrix[0][0] = 1.0 / a;
	matrix[1][1] = 1.0 / d;
	matrix[2][2] = 1;
	int vec[3] = {p0.x, p0.y, 1};
	point ans = multiple(matrix, vec);
	return ans;
}

point rotate(double a,double b,double c,double d,point p0){
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

int main() {
	FILE *fp;
	if((fp=fopen("D://examples//pictures//lena.bmp","rb"))==NULL) {
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
		for(int j = 0; j < lineBytes_new; j++){
			point fmap = bigger_or_smaller(a,d,point{i,j});
			if(fmap.x < 0) fmap.x = 0;
			if(fmap.x >= height) fmap.x = height - 1;
			if(fmap.y < 0) fmap.y = 0;
			if(fmap.y >= width) fmap.y = width - 1;
			b.imgData[i * lineBytes_new + j] = imageData[fmap.x * lineBytes + fmap.y];
		}

	char savePath1[]="D://examples//pictures//homework_4//lena_smaller.bmp";
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
		for(int j = 0; j < lineBytes_new; j++){
			point fmap = bigger_or_smaller(a,d,point{i,j});
			if(fmap.x < 0) fmap.x = 0;
			if(fmap.x >= height) fmap.x = height - 1;
			if(fmap.y < 0) fmap.y = 0;
			if(fmap.y >= width) fmap.y = width - 1;
			b.imgData[i * lineBytes_new + j] = imageData[fmap.x * lineBytes + fmap.y];
		}

	char savePath2[]="D://examples//pictures//homework_4//lena_bigger.bmp";
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
	
	for(int i = 0; i < b.biHeader.biHeight; i++){
		for(int j = 0; j < lineBytes_new; j++){
			int x = i + dx;
			int y = j + dy;
			if(x >= height || x < 0) continue;
			if(y >= width || y < 0) continue;
			b.imgData[i * lineBytes_new + j] = imageData[x * lineBytes + j];
		}
	}

	char savePath3[]="D://examples//pictures//homework_4//lena_translation.bmp";
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
	
	// mirror the figure
	
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	
	for(int i = 0; i < b.biHeader.biHeight; i++)
		for(int j = 0; j < lineBytes_new; j++)
			b.imgData[i * lineBytes_new + j] = imageData[i * lineBytes + (lineBytes - j)];

	char savePath4[]="D://examples//pictures//homework_4//lena_mirror.bmp";
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
	
	// upside down the figure
	
	b.imgData=(unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	memset(b.imgData,0,sizeof(unsigned char)*b.biHeader.biSizeImage);
	
	for(int i = 0; i < b.biHeader.biHeight; i++)
		for(int j = 0; j < lineBytes_new; j++)
			b.imgData[i * lineBytes_new + j] = imageData[(b.biHeader.biHeight - i) * lineBytes + j];

	char savePath5[]="D://examples//pictures//homework_4//lena_upside_down.bmp";
	f=fopen(savePath5,"wb");
	if(f==NULL) {
		perror("can not open file!");
		return -2;
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
		for(int j = 0; j < lineBytes_new; j++){
			// cos(theta) -sin(theta) sin(theta) cos(theta)
			point fmap = rotate(a,d,-d,a,point{i,j});
			if(fmap.x < 0 || fmap.x >= height) continue;
			if(fmap.y < 0 || fmap.y >= width) continue;
			b.imgData[i * lineBytes_new + j] = imageData[fmap.x * lineBytes + fmap.y];
		}

	char savePath6[]="D://examples//pictures//homework_4//lena_rotate.bmp";
	f=fopen(savePath6,"wb");
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