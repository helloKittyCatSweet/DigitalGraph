#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<cmath>
#include<iostream>
#include<algorithm>
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

void pic(){
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
			//将每一个像素都按公式y=B*0.299+G*0.587+R*0.114进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.299
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.114;
		}
	}

	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		(int)b.imgData[i] >= 128 ? b.imgData[i] = 255 : b.imgData[i] = 0;

	char savePath1[]="D://examples//pictures//homework_9//pic_to2.bmp";
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
//			cout << i1 << " == " << i2 << endl;
		}
	}
	cout << cnt << endl;
	cout << "set size is: " << s.size() << endl;

	int col = 50;
	for(int i = 1; i <= cnt; i++) {
//		if(col >= 255) col = 255;
		for(int j = 0; j < b.biHeader.biSizeImage; j++)
			if(label[j] == i)
				b.imgData[j] = col;
		col += 10;
	}

	char savePath2[]="D://examples//pictures//homework_9//pic_zone_marker.bmp";
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

void col()
{
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
			//将每一个像素都按公式y=B*0.299+G*0.587+R*0.114进行转化
			b.imgData[lineBytes_new*i+j]=imageData[lineBytes*i+j*3]*0.299
			                             +imageData[lineBytes*i+j*3+1]*0.587+imageData[lineBytes*i+j*3+2]*0.114;
		}
	}

	for(int i = 0; i < b.biHeader.biSizeImage; i++)
		(int)b.imgData[i] >= 128 ? b.imgData[i] = 255 : b.imgData[i] = 0;

	char savePath1[]="D://examples//pictures//homework_9//col_to2.bmp";
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
//			cout << i1 << " == " << i2 << endl;
		}
	}
	cout << cnt << endl;
	cout << "set size is: " << s.size() << endl;

	int col = 50;
	for(int i = 1; i <= cnt; i++) {
//		if(col >= 255) col = 255;
		for(int j = 0; j < b.biHeader.biSizeImage; j++)
			if(label[j] == i)
				b.imgData[j] = col;
		col += 10;
	}

	char savePath2[]="D://examples//pictures//homework_9//col_marker.bmp";
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
	pic();
	col();
	return 0;
}