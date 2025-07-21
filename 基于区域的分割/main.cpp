#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
#include<queue>
#include<utility>
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

	int point1 = lineBytes_new * height / 2 + lineBytes_new / 3 - 20;
	int point2 = lineBytes_new + 100;
	int origin1 = b.imgData[point1];
	int origin2 = b.imgData[point2];
	b.imgData[point1] = 255;
	b.imgData[point2] = 255;

	char savePath1[]="D://examples//pictures//homework_6//bubble_to_grey.bmp";
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
//				b.imgData[tmp] = 0;
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
//				b.imgData[tmp] = 0;
				q2.push(tmp);
				visit[tmp] = true;
			}
		}
	}
	for(int i = 0; i < height; i++)
		for(int j = 0; j < lineBytes_new; j++)
			if(visit[i * lineBytes_new + j])
				b.imgData[i * lineBytes_new + j] = 255;

	char savePath2[]="D://examples//pictures//homework_6//bubble_seed_growth.bmp";
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
//	cout << dsum << endl;
	return round(dsum);
}

//	up down left right
void dfs(int up, int down, int left, int right,unsigned char* imageData,unsigned char* modify,int lineBytes_new) {
	int threshold = 8;
	int length = up - down;
	int width = right - left;
//	cout << "length:" << length << " width:" << width << endl;
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

	cout << "height: " << height << ",width:" << width << endl;

	// prepare for grey figure

	for(i=0; i<height; i++) {
		for(j=0; j<width; j++) {
			//将每一个像素都按公式y=B*0.299+G*0.587+R*0.114进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.299
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.114;
		}
	}

	unsigned char* save = (unsigned char*)malloc(sizeof(unsigned char)*b.biHeader.biSizeImage);
	for(int i = 0; i < b.biHeader.biSizeImage; i++) save[i] = b.imgData[i];

	// mark the seed point

	int point1 = lineBytes_new * height * 2 / 3 + 10;
	int origin1 = b.imgData[point1];
	b.imgData[point1] = 0;

	char savePath1[]="D://examples//pictures//homework_6//gap_to_grey.bmp";
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
//				b.imgData[tmp] = 0;
				q1.push(tmp);
				visit[tmp] = true;
			}
		}
	}
	for(int i = 0; i < height; i++)
		for(int j = 0; j < lineBytes_new; j++)
			if(visit[i * lineBytes_new + j])
				b.imgData[i * lineBytes_new + j] = 0;

	char savePath2[]="D://examples//pictures//homework_6//gap_seed_growth.bmp";
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

	char savePath3[]="D://examples//pictures//homework_6//gap_aera_division.bmp";
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

int main() {
	bubble();
	gap();
	return 0;
}