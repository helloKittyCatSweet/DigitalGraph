#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
#include<queue>
#include<cmath>
#include<algorithm>
#include<vector>

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
//	cout << imSize;

	fseek(fp,bfHeader.bfOffBits,SEEK_SET);
	unsigned char*imageData=(unsigned char*)malloc(imSize*sizeof(unsigned char));
	fread(imageData,imSize*sizeof(unsigned char),1,fp);
	fclose(fp);

	bmp b;
	int i,j,k;
	memcpy(&(b.bfHeader),&bfHeader,sizeof(bfHeader));
	memcpy(&(b.biHeader),&biHeader,sizeof(biHeader));
	b.bfHeader.bfOffBits=1078; // ??? 54 + 1024
	//因新增了调色板，需调整图像数据偏移位置
	b.biHeader.biBitCount=8;  //改变图像位数
	int lineBytes_new=(width*8+31)/32*4; //重新计算每行数据字节
	b.biHeader.biSizeImage=lineBytes_new*height; // why doesn't the height change
	//改变图像数据大小
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
			//将每一个像素都按公式y=B*0.299+G*0.587+R*0.114进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.299
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.114;
		}
	}

	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		b.imgData[i] = b.imgData[i] > 128 ?  255 : 0;

	char savePath1[]="D://examples//pictures//homework_8//draw_to_2.bmp";
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
	double delta = 0.1;
	int sigma = 180 / delta;
	int* sum = (int*) malloc (sizeof(int) * 2 * r * sigma);
	memset(sum,0,sizeof(int) * 2 * r * sigma);

	cout << "***" << endl;
	for(int i = 0; i < height; i++)
		for(int j = 0; j < lineBytes_new; j++) {
			if(b.imgData[i * lineBytes_new + j]) {
				double s = 0;
				while(true) {
					const int p = j * cos(s * M_PI / 180) + i * sin(s * M_PI / 180) + r;
					sum[p * sigma + static_cast<int>(s / delta)]++;
					s += delta;
					if(s > 180) break;
				}
			}
		}
	cout << "***" << endl;

	int max_result = 0;
	for(int i = 0; i < 2 * r * sigma; i++)
		if(sum[i] > max_result)
			max_result = sum[i];

	for(int i = 0; i < height; i++)
		for(int j = 0; j < lineBytes_new ; j++) {
			double s = 0;
			while(true) {
				const int p = j * cos(s * M_PI / 180) + i * sin(s * M_PI / 180) + r;
				if(sum[p * sigma + static_cast<int>(s / delta)] > max_result * 0.2)
					b.imgData[i * lineBytes_new + j] = 200;
				s += delta;
				if(s > 180) break;
			}
		}

	char savePath2[]="D://examples//pictures//homework_8//draw_hough.bmp";
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

	return 0;
}