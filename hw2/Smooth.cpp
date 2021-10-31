#include <assert.h>
#include <mpi/mpi.h>
#include <pthread.h>
#include <stdio.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <stddef.h>
#include "bmp.h"

using namespace std;

//定義平滑運算的次數
#define NSmooth 1000

/*********************************************************/
/*變數宣告：                                             */
/*  bmpHeader    ： BMP檔的標頭                          */
/*  bmpInfo      ： BMP檔的資訊                          */
/*  **BMPSaveData： 儲存要被寫入的像素資料               */
/*  **BMPData    ： 暫時儲存要被寫入的像素資料           */
/*********************************************************/
BMPHEADER bmpHeader;
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;
RGBTRIPLE **BMPData = NULL;
RGBTRIPLE **BMPTempData = NULL;

/*********************************************************/
/*函數宣告：                                             */
/*  readBMP    ： 讀取圖檔，並把像素資料儲存在BMPSaveData*/
/*  saveBMP    ： 寫入圖檔，並把像素資料BMPSaveData寫入  */
/*  swap       ： 交換二個指標                           */
/*  **alloc_memory： 動態分配一個Y * X矩陣               */
/*********************************************************/
int readBMP(char *fileName);  // read file
int saveBMP(char *fileName);  // save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory(int Y, int X);  // allocate memory

MPI_Datatype rgb_mpi_data_type;

typedef struct task_t{
    int cnt;
    int disp;
    int height;
    int width;
}task_t;

#define TEST

void initialize_rgb_mpi_data_type(){
    int block_length[3] = {1, 1, 1};
    MPI_Datatype types[3] = { MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR};
    MPI_Aint offsets[3] = { offsetof(RGBTRIPLE, rgbBlue), offsetof(RGBTRIPLE, rgbGreen), offsetof(RGBTRIPLE, rgbRed)};

    MPI_Type_create_struct(3, block_length, offsets, types, &rgb_mpi_data_type);
    MPI_Type_commit(&rgb_mpi_data_type); 
}


void print_content(RGBTRIPLE *content, int y){
    int idx = y*bmpInfo.biWidth;
    for(int i=idx, size=idx+5; i<size; ++i){
        printf("(%u %u %u) ", content[i].rgbBlue, content[i].rgbGreen, content[i].rgbRed);
    }
}

void scattering_parameters(task_t *tasks, task_t *task, int comm_sz){
    int *send_cnt = (int *)malloc(sizeof(int)*comm_sz);
    int *disp = (int *)malloc(sizeof(int)*comm_sz);
    for(int i = 0; i < comm_sz; ++i){
        send_cnt[i] = 1;
        disp[i] = i;
    }


    MPI_Datatype type;
    int block_length[4] = {1, 1, 1, 1};
    MPI_Datatype types[4] = { MPI_INT,MPI_INT,MPI_INT,MPI_INT} ;
    MPI_Aint offsets[4] = { offsetof(task_t, cnt), offsetof(task_t, disp), offsetof(task_t, height), offsetof(task_t, width) };
    
    MPI_Type_create_struct(4, block_length, offsets, types, &type);
    MPI_Type_commit(&type);

    MPI_Scatterv(tasks, send_cnt, disp, type, task, 1, type, 0, MPI_COMM_WORLD);

}


void assignment(int rank, int comm_sz, char infileName[], task_t *task, RGBTRIPLE *** triples, int **_send_cnt, int **_disp){
    RGBTRIPLE * content = NULL;
    if(rank == 0){
        // read file
        if (readBMP(infileName))
            cout << "Read file successfully!!" << endl;
        else
            cout << "Read file fails!!" << endl;
        content = BMPSaveData[0];
    }

    MPI_Barrier(MPI_COMM_WORLD); 
    
    int *send_cnt = NULL, *disp=NULL, *height;
    task_t * tasks = NULL;
    if(rank == 0){
        send_cnt = (int*)malloc(sizeof(int)*comm_sz);
        disp = (int*)malloc(sizeof(int)*comm_sz);
        height = (int*)malloc(sizeof(int)*comm_sz);

        memset(send_cnt, 0, sizeof(int)*comm_sz);
        memset(disp, 0, sizeof(int)*comm_sz);
        memset(height, 0, sizeof(int)*comm_sz);

        for(int i=0, lines=bmpInfo.biHeight/comm_sz; i < comm_sz; ++i){
            height[i] = lines;
        }

        // distribute the remainder
        int remainder = bmpInfo.biHeight % comm_sz;
        for(int i = 0; i < comm_sz&&remainder > 0; ++i){
            height[i]++;
            remainder --; 
        }

        for(int i = 0; i < comm_sz; ++i)
            send_cnt[i] = height[i]*bmpInfo.biWidth; 
    
        disp[0] = 0;
        for(int i = 1; i < comm_sz; ++i){
            disp[i] = disp[i-1] + send_cnt[i-1]; 
        }
#ifdef TEST
        printf("Test all lines are distributed.......................");
        int sum = 0;
        for(int i = 0; i < comm_sz; ++i){
            sum += height[i]; 
        }
        assert(sum == bmpInfo.biHeight);
        printf("Pass!\n");
        printf("Test all elements are distributed....................");
        sum = 0;
        for(int i =0 ; i < comm_sz; ++i){
            sum += send_cnt[i];
        }
        assert(sum == bmpInfo.biHeight*bmpInfo.biWidth);
        printf("Pass!\n");
#endif
        tasks = (task_t*)malloc(sizeof(task_t)*comm_sz);
        for(int i = 0; i < comm_sz; ++i){
            tasks[i] = task_t{.cnt = send_cnt[i], .disp = disp[i], .height = height[i], .width = bmpInfo.biWidth};
        }
    }

    // if(rank == 0){
    //     printf("cnt : ");
    //     for(int i=0;i<comm_sz;++i)
    //         printf("%d ", send_cnt[i]);
    //     printf("\ndisp : ");
    //     for(int i=0; i<comm_sz; ++i)
    //         printf("%d ", disp[i]);
    //     printf("\n");
    // } 
    // MPI_Barrier(MPI_COMM_WORLD);

    scattering_parameters(tasks, task, comm_sz); 
    // printf("Rank[%d] : \n", rank);
    // printf("\t cnt : %d", task->cnt);
    // printf("\t height : %d", task->height);
    // printf("\t width : %d",task->width);
    // printf("\t disp : %d\n", task->disp);

    RGBTRIPLE ** recv_triples =  NULL;
    recv_triples = alloc_memory(task->height+2, task->width);
    // printf("[%d] : %p\n", rank, recv_triples);
    MPI_Scatterv(content, send_cnt, disp, rgb_mpi_data_type, *(recv_triples+1), (task->height+2)*task->width, rgb_mpi_data_type, 0, MPI_COMM_WORLD);
    // printf("rank [%d] finishes\n",rank);
    // fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    // if(rank == 0){
    //     // print the answer to check if data is copied successfully
    //     printf("Ans ======\n");
    //     for(int i=0; i<4;++i){
    //         printf("Ans %d : ", i);
    //         print_content(content, i*300);
    //         printf("\n");
    //     }
    //     printf("\n==========\n");
    // }
    // MPI_Barrier(MPI_COMM_WORLD);
    // printf("Rank[%d] got ",rank);
    // print_content(recv_triples[1], 0);
    // printf("\n");
    *triples = recv_triples;

    *_send_cnt = send_cnt;
    *_disp = disp;
}

void mutual_exchange(RGBTRIPLE ** edges, RGBTRIPLE **data, int width, int comm_sz, int rank){
    MPI_Barrier(MPI_COMM_WORLD);
    int send_cnt = 2*width, recv_cnt = 2*width;
    MPI_Allgather(*data, send_cnt, rgb_mpi_data_type, *edges, recv_cnt, rgb_mpi_data_type, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    
    // for(int i = 0, size = comm_sz<<1; i < size&&rank==0; ++i){
    //     printf("rank %d\t", i >> 1);
    //     print_content(edges[i],0);
    //     printf("\n");
    // }
}

int main(int argc, char *argv[])
{
    /*********************************************************/
    /*變數宣告：                                             */
    /*  *infileName  ： 讀取檔名                             */
    /*  *outfileName ： 寫入檔名                             */
    /*  startwtime   ： 記錄開始時間                         */
    /*  endwtime     ： 記錄結束時間                         */
    /*********************************************************/
    char infileName[] = "input.bmp";
    char outfileName[] = "output.bmp";
    double startwtime = 0.0, endwtime = 0;

    int comm_sz=4, rank;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    initialize_rgb_mpi_data_type();
    //記錄開始時間
    // startwtime = MPI_Wtime();

    //動態分配記憶體給暫存空間
    BMPData = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);
    
    task_t task;
    RGBTRIPLE ** segment;
    int *send_cnt, *disp;
    assignment(rank, comm_sz, infileName, &task, &segment, &send_cnt, &disp);
    
    RGBTRIPLE ** all_edges;
    all_edges = alloc_memory(comm_sz * 2, task.width);
    
    RGBTRIPLE ** edges = alloc_memory(2, task.width);
    // printf("rank %d\n", rank);
    // for(int i = 0; i < 2; ++i){
    //     for(int j = 0; j < 5; ++j){
    //         printf("\t(%u %u %u) ", edges[i][j].rgbBlue,edges[i][j].rgbGreen, edges[i][j].rgbRed );
    //     }
    //     printf("\n");
    // }
     
    BMPData = alloc_memory(task.height+2, task.width);

    //進行多次的平滑運算
    for (int count = 0; count < NSmooth; count++) {
                
        // update edge information
        memcpy(edges[0], segment[1], sizeof(RGBTRIPLE)*task.width);
        memcpy(edges[1], segment[task.height], sizeof(RGBTRIPLE)*task.width);
        mutual_exchange(all_edges, edges, task.width, comm_sz, rank);

        int upper_idx = rank - 1 < 0 ? comm_sz - 1 : rank - 1;
        int lower_idx = rank  + 1 >= comm_sz ? 0 : rank + 1;
        upper_idx = (upper_idx << 1) + 1;
        lower_idx = (lower_idx << 1);

        memcpy(segment[0], all_edges[upper_idx], sizeof(RGBTRIPLE)*task.width);
        memcpy(segment[task.height + 1], all_edges[lower_idx], sizeof(RGBTRIPLE)*task.width);

        //把像素資料與暫存指標做交換
        swap(BMPData, segment);
        //進行平滑運算
        for (int i = 1; i <= task.height; i++)
            for (int j = 0; j < task.width; j++) {
                /*********************************************************/
                /*設定上下左右像素的位置                                 */
                /*********************************************************/
                int Top = i - 1;
                int Down = i + 1;
                int Left = j > 0 ? j - 1 : task.width - 1;
                int Right = j < task.width - 1 ? j + 1 : 0;
                /*********************************************************/
                /*與上下左右像素做平均，並四捨五入                       */
                /*********************************************************/
                segment[i][j].rgbBlue =
                    (double) (BMPData[i][j].rgbBlue + 
                              BMPData[Top][j].rgbBlue +
                              BMPData[Top][Left].rgbBlue +
                              BMPData[Top][Right].rgbBlue +
                              BMPData[Down][j].rgbBlue +
                              BMPData[Down][Left].rgbBlue +
                              BMPData[Down][Right].rgbBlue +
                              BMPData[i][Left].rgbBlue +
                              BMPData[i][Right].rgbBlue) /
                        9 +
                    0.5;
                segment[i][j].rgbGreen =
                    (double) (BMPData[i][j].rgbGreen +
                              BMPData[Top][j].rgbGreen +
                              BMPData[Top][Left].rgbGreen +
                              BMPData[Top][Right].rgbGreen +
                              BMPData[Down][j].rgbGreen +
                              BMPData[Down][Left].rgbGreen +
                              BMPData[Down][Right].rgbGreen +
                              BMPData[i][Left].rgbGreen +
                              BMPData[i][Right].rgbGreen) /
                        9 +
                    0.5;
                segment[i][j].rgbRed =
                    (double) (BMPData[i][j].rgbRed + BMPData[Top][j].rgbRed +
                              BMPData[Top][Left].rgbRed +
                              BMPData[Top][Right].rgbRed +
                              BMPData[Down][j].rgbRed +
                              BMPData[Down][Left].rgbRed +
                              BMPData[Down][Right].rgbRed +
                              BMPData[i][Left].rgbRed +
                              BMPData[i][Right].rgbRed) /
                        9 +
                    0.5;
            }
    }
    
    // if(rank == 0){
    //     for(int i = 0; i < comm_sz; ++i){
    //         printf("(%d, %d)", send_cnt[i], disp[i]);
    //     }
    //     printf("\n");
    // }
    // printf("Task.cnt : %d\n", task.cnt);
    // gether segment[1] - segment[task.height]
    MPI_Gatherv(segment[1], task.cnt, rgb_mpi_data_type, BMPSaveData == NULL ? NULL : *BMPSaveData, send_cnt, disp, rgb_mpi_data_type, 0, MPI_COMM_WORLD);

    //寫入檔案
    if(rank == 0){
        if (saveBMP(outfileName))
            cout << "Save file successfully!!" << endl;
        else
            cout << "Save file fails!!" << endl;

    }
    
    //得到結束時間，並印出執行時間
    // endwtime = MPI_Wtime();
    // cout << "The execution time = "<< endwtime-startwtime <<endl ;

    // free(BMPData[0]);
    // free(BMPSaveData[0]);
    // free(BMPData);
    // free(BMPSaveData);
    MPI_Finalize();

    return 0;
}

/*********************************************************/
/* 讀取圖檔                                              */
/*********************************************************/
int readBMP(char *fileName)
{
    //建立輸入檔案物件
    ifstream bmpFile(fileName, ios::in | ios::binary);

    //檔案無法開啟
    if (!bmpFile) {
        cout << "It can't open file!!" << endl;
        return 0;
    }

    //讀取BMP圖檔的標頭資料
    bmpFile.read((char *) &bmpHeader, sizeof(BMPHEADER));

    //判決是否為BMP圖檔
    if (bmpHeader.bfType != 0x4d42) {
        cout << "This file is not .BMP!!" << endl;
        return 0;
    }

    //讀取BMP的資訊
    bmpFile.read((char *) &bmpInfo, sizeof(BMPINFO));

    //判斷位元深度是否為24 bits
    if (bmpInfo.biBitCount != 24) {
        cout << "The file is not 24 bits!!" << endl;
        return 0;
    }

    //修正圖片的寬度為4的倍數
    while (bmpInfo.biWidth % 4 != 0)
        bmpInfo.biWidth++;

    //動態分配記憶體
    BMPSaveData = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);

    //讀取像素資料
    // for(int i = 0; i < bmpInfo.biHeight; i++)
    //	bmpFile.read( (char* )BMPSaveData[i],
    // bmpInfo.biWidth*sizeof(RGBTRIPLE));
    bmpFile.read((char *) BMPSaveData[0],
                 bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);

    //關閉檔案
    bmpFile.close();

    return 1;
}
/*********************************************************/
/* 儲存圖檔                                              */
/*********************************************************/
int saveBMP(char *fileName)
{
    //判決是否為BMP圖檔
    if (bmpHeader.bfType != 0x4d42) {
        cout << "This file is not .BMP!!" << endl;
        return 0;
    }

    //建立輸出檔案物件
    ofstream newFile(fileName, ios::out | ios::binary);

    //檔案無法建立
    if (!newFile) {
        cout << "The file can't create!!" << endl;
        return 0;
    }

    //寫入BMP圖檔的標頭資料
    newFile.write((char *) &bmpHeader, sizeof(BMPHEADER));

    //寫入BMP的資訊
    newFile.write((char *) &bmpInfo, sizeof(BMPINFO));

    //寫入像素資料
    // for( int i = 0; i < bmpInfo.biHeight; i++ )
    //        newFile.write( ( char* )BMPSaveData[i],
    //        bmpInfo.biWidth*sizeof(RGBTRIPLE) );
    newFile.write((char *) BMPSaveData[0],
                  bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);

    //寫入檔案
    newFile.close();

    return 1;
}


/*********************************************************/
/* 分配記憶體：回傳為Y*X的矩陣                           */
/*********************************************************/
RGBTRIPLE **alloc_memory(int Y, int X)
{
    //建立長度為Y的指標陣列
    RGBTRIPLE **temp = new RGBTRIPLE *[Y];
    RGBTRIPLE *temp2 = new RGBTRIPLE[Y * X];
    memset(temp, 0, sizeof(RGBTRIPLE) * Y);
    memset(temp2, 0, sizeof(RGBTRIPLE) * Y * X);

    //對每個指標陣列裡的指標宣告一個長度為X的陣列
    for (int i = 0; i < Y; i++) {
        temp[i] = &temp2[i * X];
    }

    return temp;
}
/*********************************************************/
/* 交換二個指標                                          */
/*********************************************************/
void swap(RGBTRIPLE *a, RGBTRIPLE *b)
{
    RGBTRIPLE *temp;
    temp = a;
    a = b;
    b = temp;
}
