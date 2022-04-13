#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>
#include "config.h"
using namespace std;

int numQuery = 0;
int query[10];
ifstream queryFile, indexFile;
ofstream outFile;
int numIndex = 0;
int lenIndex[maxNumIndex];
int iindex[maxNumIndex][maxLenIndex];
const int NaN = -1;

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

    int x;
    int scale[3] = {1, 100, 10000};
    for(int i = 0; i < numIndex; i++) {
        x = lenIndex[i] * 3;
        outFile.write(reinterpret_cast<char *>(&x), sizeof(int));

        for(int j = 0; j < 3; j++) {
            for(int k = 0; k < lenIndex[i]; k++) {
                x = iindex[i][k] * scale[j];
                outFile.write(reinterpret_cast<char *>(&x), sizeof(int));
            }
        }
    }

    return true;
}

int main()
{
    indexFile.open(testIndexPath, ios::in | ios::binary);
    outFile.open(binaryFile, ios::out | ios::binary);
    

    if (!ReadIndex())
    {
        cout << "Error: Fail to reading index" << endl;
        return 0;
    }

    indexFile.close();
    outFile.close();
}