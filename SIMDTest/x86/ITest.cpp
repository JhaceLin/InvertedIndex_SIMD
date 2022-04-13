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
int numIndex = 0;
int lenIndex[maxNumIndex];
int iindex[maxNumIndex][maxLenIndex];
const int NaN = -1;

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
    return true;
}

int main()
{
    indexFile.open(testIndexEnlargePath, ios::in | ios::binary);

    if (!ReadIndex())
    {
        cout << "Error: Fail to reading index" << endl;
        return 0;
    }

    for(int i = 0; i < numIndex; i++) {
        cout << lenIndex[i] << " ";
        for(int j = 0; j < lenIndex[i]; j++) {
            cout << iindex[i][j] << " ";
        }
        cout << endl;
    }

    indexFile.close();
}