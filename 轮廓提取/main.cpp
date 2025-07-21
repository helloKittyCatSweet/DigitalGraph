#include<stdio.h>
#include<stdlib.h>
#include<string.h>
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

void circle(){
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
	cout << imSize << endl;

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
		(int)b.imgData[i] > 128 ? b.imgData[i] = 255 : b.imgData[i] = 0;

	char savePath1[]="D://examples//pictures//homework_10//circle_to_2.bmp";
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
		for(int j = 0; j < lineBytes_new; j++){
			if((int)copy[i * lineBytes_new + j] == 0){
				int m;
				bool flag = true;
				for(m = 0; m < 8; m++){
					int x = i + dx[m];
					int y = j + dy[m];
					if(x < 0 || x >= height || y < 0 || y >= lineBytes_new || 
						(int)copy[x * lineBytes_new + y] != 0){
							flag = false;
							break;
						}
				}
				if(flag){
					b.imgData[i * lineBytes_new + j] = 255;
				}else{
					b.imgData[i * lineBytes_new + j] = 0;
				}
			}
			else b.imgData[i * lineBytes_new + j] = 255;
		}

	/*
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
	*/

	char savePath2[]="D://examples//pictures//homework_10//circle_extraction.bmp";
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

void hist(){
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
	cout << imSize << endl;

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
		(int)b.imgData[i] > 128 ? b.imgData[i] = 255 : b.imgData[i] = 0;

	char savePath1[]="D://examples//pictures//homework_10//hist_to_2.bmp";
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

	char savePath2[]="D://examples//pictures//homework_10//hist_extraction.bmp";
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

int main() {
	circle();
	hist();
	return 0;
}