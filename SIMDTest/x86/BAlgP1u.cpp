#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>
#include <bitset>
#include "config.h"
#include <windows.h>
#include <pmmintrin.h>
using namespace std;

const int INF = 1 << 30;

int numQuery = 0;
int query[10];
ifstream queryFile, indexFile;
int numIndex = 0;
int lenIndex[maxNumIndex];
int iindex[maxNumIndex][maxLenIndex];

bitset <32> **bindex;
int *len_bindex;

bitset <32> *rst;

int *result;
int resultCount;

int max_len_bindex = -1;

class MyTimer {
    private:
        long long start_time, finish_time, freq;
        double timeMS;

    public:
        MyTimer() {
            timeMS = 0;
            QueryPerformanceFrequency((LARGE_INTEGER*) &freq);
        }

        void start() {
            QueryPerformanceCounter((LARGE_INTEGER*) &start_time);
        }

        void finish() {
            QueryPerformanceCounter((LARGE_INTEGER*) &finish_time);
        }
        
        double getTime() {
            // ms
            double duration = (finish_time - start_time) * 1000.0 / freq;
            return duration;
        }
};

bool ReadIndex()
{
    if (!indexFile)
    {
        cout << "Error opening file.";
        return false;
    }

    numIndex = 0;
    int num = 0;
    while (indexFile.read(reinterpret_cast<char *>(&num), sizeof(int)))
    {
        lenIndex[numIndex] = num;
        indexFile.read(reinterpret_cast<char *>(iindex[numIndex]), sizeof(int) * num);
        numIndex++;
    }

    len_bindex = new int[numIndex];
    bindex = new bitset<32> *[numIndex];
    for(int i = 0; i < numIndex; i++) {
        int bstNum = iindex[i][lenIndex[i] - 1];
        bstNum /= 32;
        bstNum++;
        len_bindex[i] = bstNum;

        if(bstNum > max_len_bindex) {
            max_len_bindex = bstNum;
        }

        bindex[i] = new bitset<32>[bstNum];
        for(int j = 0; j < lenIndex[i]; j++) {
            int t = iindex[i][j];
            bindex[i][t / 32].set(t % 32);
        }
    }

    return true;
}

bool ReadQuery()
{ // Read A Row At a Time
    if (!queryFile)
    {
        cout << "Error opening file.";
        return false;
    }

    numQuery = 0;
    int x;
    string list;
    if (getline(queryFile, list))
    {
        istringstream islist(list);
        while (islist >> x)
        {
            query[numQuery++] = x;
        }
        return true;
    }
    else
    {
        return false;
    }
}

void ReleaseSpace() {
    // Delete bindex
    for(int i = 0; i < numIndex; i++) {
        delete[] bindex[i];
    }
    delete[] bindex;

    // Delete len_bindex
    delete[] len_bindex;

    // Delete result
    delete[] result;

    // Delete rst
    delete[] rst;
}

__m128i zero_m128i = _mm_setzero_si128();
char andOp[4] = {1, 2, 4, 8};

void GetAnswer(int rstLen) {
    // 将判断none()部分SIMD并行化
    int sect = 0;

    __m128i x1;
    __m128 cmp;
    char r;
    for(sect = 0; sect + 4 < rstLen; sect += 4) {
        x1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(rst + sect));
        
        cmp = reinterpret_cast<__m128>( _mm_cmpeq_epi32(x1, zero_m128i) );
        r = _mm_movemask_ps(cmp);
        if(r == 0xF) continue;
        
        for(int k = 0; k < 4; k++) {
            if( !(r & andOp[k]) ) { // [sect + k]
                for(int i = 0; i < 32; i++) {
                    if(rst[sect + k].test(i)) {
                        result[resultCount++] = (sect + k) * 32 + i;
                    }
                }
            }
        }
    }

    // Remaining section 
    for(; sect < rstLen; sect++) {
        if(rst[sect].none()) continue;

        for(int i = 0; i < 32; i++) {
            if(rst[sect].test(i)) {
                result[resultCount++] = sect * 32 + i;
            }
        }
    }
}

void GetIntersection() {
    if(numQuery < 2) return;

    int rstLen = INF;
    for(int i = 0; i < numQuery; i++) {
        int t = query[i];
        if(len_bindex[t] < rstLen) {
            rstLen = len_bindex[t];
        }
    }

    int sect = 0;
    __m128i x1, x2;
    for(sect = 0; sect + 4 < rstLen; sect += 4) {
        x1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(bindex[query[0]] + sect));
        for(int i = 1; i < numQuery; i++) {
            x2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(bindex[query[i]] + sect));
            x1 = _mm_and_si128(x1, x2);
        }

        _mm_storeu_si128(reinterpret_cast<__m128i*>(rst + sect), x1);
    }
    // Remaining section
    for(; sect < rstLen; sect++) {
        rst[sect] = bindex[query[0]][sect];
        for(int i = 1; i < numQuery; i++) {
            rst[sect] &= bindex[query[i]][sect];
        }
    }

    // Get Answer
    GetAnswer(rstLen);
}

void ShowResult() {
    cout << resultCount << ": ";
    for(int i = 0; i < resultCount; i++) {
        cout << result[i] << " ";
    }
    cout << endl;
}

void test_code() {
    for(int i = 0; i < numIndex; i++) {
        for(int j = 0; j < len_bindex[i]; j++) {
            cout << bindex[i][j] << " ";
        }
        cout << endl;
    }
}

int main() {
    bool testMode = false;
    bool perfMode = true;

    MyTimer fileInTimer, computeTimer;

    fileInTimer.start();
    if(testMode) {
        indexFile.open(testIndexEnlargePath, ios::in | ios::binary);
        queryFile.open(testQueryPath, ios ::in);
    }
    else {
        queryFile.open(queryPath, ios ::in);
        indexFile.open(indexPath, ios::in | ios::binary);
    }

    if (!ReadIndex())
    {
        cout << "Error: Fail to reading index" << endl;
        return 0;
    }

    fileInTimer.finish();
    printf("Time Used in File Reading: %8.4fms\n", fileInTimer.getTime());

    computeTimer.start();

    rst = new bitset <32> [max_len_bindex];
    result = new int[maxLenIndex];
    resultCount = 0;
    int readCount = 0;
    if(testMode) {
        while(ReadQuery()) {
            GetIntersection();

            cout << "Result: " << endl;
            ShowResult();
            resultCount = 0;
        }
    }
    else {
        if(perfMode) {
            while(ReadQuery()) {
                GetIntersection();
                resultCount = 0;
            }
        }
        else {
            cout << "Start Computing" << endl;

            while(ReadQuery() && readCount++ < 10) {
                GetIntersection();
                printf("Finish: (%d/1000)\n", readCount);
                resultCount = 0;
            }
        }
    }
    computeTimer.finish();
    printf("Time Used in Computing Intersection: %8.4fms\n", computeTimer.getTime());

    ReleaseSpace();
    queryFile.close();
    indexFile.close();

    cout << "End" << endl;
}