#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <pthread.h>
#include <assert.h>
#include "bmp.h"

using namespace std;

#define NSmooth 1000

BMPHEADER bmpHeader;
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;
RGBTRIPLE **BMPData = NULL;

int readBMP(char *fileName);  // read file
int saveBMP(char *fileName);  // save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory(int Y, int X);  // allocate memory

typedef struct thread_data_t{
    int id;
    int upper, lower;
    int number_of_lines;
    RGBTRIPLE ** bmp_data;
    RGBTRIPLE ** bmp_save_data;
}thread_data_t;

void *runner(void * _data){
   thread_data_t * tdata = (thread_data_t*)_data;
   RGBTRIPLE **_BMPData, **_BMPSaveData;
   _BMPData = tdata->bmp_data;
   _BMPSaveData = tdata->bmp_save_data;
   for(int i = tdata->lower; i < tdata->upper; ++i){
        for(int j = 0; j < bmpInfo.biWidth; ++j){
             int Top = i > 0 ? i - 1 : bmpInfo.biHeight - 1;
             int Down = i < bmpInfo.biHeight - 1 ? i + 1 : 0;
             int Left = j > 0 ? j - 1 : bmpInfo.biWidth - 1;
             int Right = j < bmpInfo.biWidth - 1 ? j + 1 : 0;
             _BMPSaveData[i][j].rgbBlue =
                    (double) (_BMPData[i][j].rgbBlue + 
                              _BMPData[Top][j].rgbBlue +
                              _BMPData[Top][Left].rgbBlue +
                              _BMPData[Top][Right].rgbBlue +
                              _BMPData[Down][j].rgbBlue +
                              _BMPData[Down][Left].rgbBlue +
                              _BMPData[Down][Right].rgbBlue +
                              _BMPData[i][Left].rgbBlue +
                              _BMPData[i][Right].rgbBlue) /
                        9 +
                    0.5;
             _BMPSaveData[i][j].rgbGreen =
                    (double) (_BMPData[i][j].rgbGreen +
                              _BMPData[Top][j].rgbGreen +
                              _BMPData[Top][Left].rgbGreen +
                              _BMPData[Top][Right].rgbGreen +
                              _BMPData[Down][j].rgbGreen +
                              _BMPData[Down][Left].rgbGreen +
                              _BMPData[Down][Right].rgbGreen +
                              _BMPData[i][Left].rgbGreen +
                              _BMPData[i][Right].rgbGreen) /
                        9 +
                    0.5;
             _BMPSaveData[i][j].rgbRed =
                    (double) (_BMPData[i][j].rgbRed + 
                              _BMPData[Top][j].rgbRed +
                              _BMPData[Top][Left].rgbRed +
                              _BMPData[Top][Right].rgbRed +
                              _BMPData[Down][j].rgbRed +
                              _BMPData[Down][Left].rgbRed +
                              _BMPData[Down][Right].rgbRed +
                              _BMPData[i][Left].rgbRed +
                              _BMPData[i][Right].rgbRed) /
                        9 +
                    0.5;
        }
   }
   // pthread_exit(NULL);
   return NULL;
}

#define NUMBER_OF_THREADS 16
#define TEST

int main(int argc, char *argv[])
{
    char *infileName = "input.bmp";
    char *outfileName = "output.bmp";
    double startwtime = 0.0, endwtime = 0;

    if (readBMP(infileName)) // read bmp data and store it at BMPSaveData
        cout << "Read file successfully!!" << endl;
    else
        cout << "Read file fails!!" << endl;

    BMPData = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);

    pthread_t threads[NUMBER_OF_THREADS];
    thread_data_t thread_data[NUMBER_OF_THREADS];
    
    int line_idx = 0; 
    for (int i = 0, lines = bmpInfo.biHeight/NUMBER_OF_THREADS, remainder = bmpInfo.biHeight % NUMBER_OF_THREADS; i < NUMBER_OF_THREADS; ++i) {
        thread_data[i].id = i;
        thread_data[i].lower = line_idx;
        thread_data[i].number_of_lines = lines + (int)(remainder > 0);
        line_idx = thread_data[i].upper = thread_data[i].lower + thread_data[i].number_of_lines;
        --remainder;
    }

#ifdef TEST // test thread data have all lines
    int sum = 0;
    for(int i = 0; i < NUMBER_OF_THREADS; ++i){
        printf("Thread[%d] takes (%d, %d)=>%d\n", thread_data[i].id, thread_data[i].lower, thread_data[i].upper, thread_data[i].number_of_lines);
        sum += thread_data[i].number_of_lines; 
    }
    assert(sum == bmpInfo.biHeight);
    if(NUMBER_OF_THREADS > 0){
        assert(thread_data[NUMBER_OF_THREADS - 1].upper == bmpInfo.biHeight);
    }
#endif

    for(int count = 0; count < NSmooth; ++count){
        swap(BMPSaveData, BMPData);
        // use BMPData to compute 
        for(int i = 0; i < NUMBER_OF_THREADS; ++i){
            thread_data[i].bmp_data = BMPData;
            thread_data[i].bmp_save_data = BMPSaveData;
            runner(&thread_data[i]);
            // pthread_create(&threads[i], NULL, runner, (void*)&thread_data[i]);
        }
        // for(int i = 0; i < NUMBER_OF_THREADS; ++i){
        //     pthread_join(threads[i], NULL);
        // }
        
    }
    
    if (saveBMP(outfileName))
        cout << "Save file successfully!!" << endl;
    else
        cout << "Save file fails!!" << endl;


    free(BMPData[0]);
    free(BMPSaveData[0]);
    free(BMPData);
    free(BMPSaveData);

    return 0;
}

int readBMP(char *fileName)
{
    ifstream bmpFile(fileName, ios::in | ios::binary);

    if (!bmpFile) {
        cout << "It can't open file!!" << endl;
        return 0;
    }

    bmpFile.read((char *) &bmpHeader, sizeof(BMPHEADER));

    if (bmpHeader.bfType != 0x4d42) {
        cout << "This file is not .BMP!!" << endl;
        return 0;
    }

    bmpFile.read((char *) &bmpInfo, sizeof(BMPINFO));

    if (bmpInfo.biBitCount != 24) {
        cout << "The file is not 24 bits!!" << endl;
        return 0;
    }

    while (bmpInfo.biWidth % 4 != 0)
        bmpInfo.biWidth++;

    BMPSaveData = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);

    bmpFile.read((char *) BMPSaveData[0],
                 bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);

    bmpFile.close();

    return 1;
}
int saveBMP(char *fileName)
{
    if (bmpHeader.bfType != 0x4d42) {
        cout << "This file is not .BMP!!" << endl;
        return 0;
    }

    ofstream newFile(fileName, ios::out | ios::binary);

    if (!newFile) {
        cout << "The File can't create!!" << endl;
        return 0;
    }

    newFile.write((char *) &bmpHeader, sizeof(BMPHEADER));

    newFile.write((char *) &bmpInfo, sizeof(BMPINFO));

    newFile.write((char *) BMPSaveData[0],
                  bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);

    newFile.close();

    return 1;
}


RGBTRIPLE **alloc_memory(int Y, int X)
{
    RGBTRIPLE **temp = new RGBTRIPLE *[Y];
    RGBTRIPLE *temp2 = new RGBTRIPLE[Y * X];
    memset(temp, 0, sizeof(RGBTRIPLE) * Y);
    memset(temp2, 0, sizeof(RGBTRIPLE) * Y * X);
    for (int i = 0; i < Y; i++) {
        temp[i] = &temp2[i * X];
    }

    return temp;
}
void swap(RGBTRIPLE *a, RGBTRIPLE *b)
{
    RGBTRIPLE *temp;
    temp = a;
    a = b;
    b = temp;
}
