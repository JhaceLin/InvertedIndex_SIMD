#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>
#include <windows.h>
#include "config.h"
using namespace std;

int numQuery = 0;
int query[10];
ifstream queryFile, indexFile;
int numIndex = 0;
int lenIndex[maxNumIndex];
int iindex[maxNumIndex][maxLenIndex];
const int NaN = -1;

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

class IList
{
public:
    int *list;
    int length;
    int remainLen;

    IList()
    {
        list = NULL;
        length = 0;
        remainLen = 0;
    }

    IList(int list[], int length, int remainLen)
    {
        this->length = length;
        this->remainLen = remainLen;
        this->list = new int[length];
        memcpy(this->list, list, sizeof(int) * length);
    }

    void modify(int list[], int length, int remainLen)
    {
        this->length = length;
        this->remainLen = remainLen;
        this->list = new int[length];
        memcpy(this->list, list, sizeof(int) * length);
    }

    void deepCopy(const IList &t) {
        length = t.length;
        remainLen = t.remainLen;
        list = new int[length];
        memcpy(list, t.list, sizeof(int) * length);
    }

    ~IList()
    {
        delete[] list;
    }

    bool operator<(const IList &t) const
    {
        return remainLen < t.remainLen;
    }
};

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

void ShowIList(IList *ilist, int n, bool showNaN = true) {
    for(int i = 0; i < n; i++) {
        if(showNaN) {
            cout << ilist[i].length << ": ";
            for(int j = 0; j < ilist[i].length; j++) {
                cout << ilist[i].list[j] << " ";
            }
        }
        else {
            cout << ilist[i].remainLen << ": ";
            for(int j = 0; j < ilist[i].length; j++) {
                if(ilist[i].list[j] != NaN)
                    cout << ilist[i].list[j] << " ";
            }
        }
        
        cout << endl;
    }
}

void SortIList(IList *ilist, int n) {
    for(int i = n - 1; i > 0; i--) {
        int target = 0;
        for(int j = 1; j <= i; j++) {
            if(ilist[target] < ilist[j]) {
                target = j;
            }
        }
        IList temp;
        temp.deepCopy(ilist[i]);
        ilist[i].deepCopy(ilist[target]);
        ilist[target].deepCopy(temp);
    }
}

int GetX_Search(IList &ilist, int e)
{
    int n = ilist.length;
    int *arr = ilist.list;

    int i;
    for(i = 0; i < n; i++) {
        if(arr[i] >= e) {
            break;
        }
    }
    return i;
}

int GetEliminatorOrder(IList *ilist, int n)
{
    int order = 0;
    while(ilist[0].list[order] == NaN) {
        order++;
    }
    return order;
}

void Remove(IList &ilist, int order) {
    ilist.remainLen--;
    ilist.list[order] = NaN;
}

void GetIntersection(int *result, int &resultCount)
{
    if(numQuery < 2) return;

    //制造拷贝
    IList *ilist = new IList[numQuery];
    for(int i = 0; i < numQuery; i++) {
        ilist[i].modify(iindex[query[i]], lenIndex[query[i]], lenIndex[query[i]]);
    }
    SortIList(ilist, numQuery);

    resultCount = 0;
    while(ilist[0].remainLen > 0) {
        // Get e
        int eOrder = GetEliminatorOrder(ilist, numQuery);
        int e = ilist[0].list[eOrder];
        Remove(ilist[0], eOrder);

        int xOrder, x;
        int i; // i : Currently Searching list
        for(i = 1; i < numQuery; i++) {
            xOrder = GetX_Search(ilist[i], e);
            x = ilist[i].list[xOrder];
            
            if(x == e) {
                Remove(ilist[i], xOrder);
            }
            else {
                break;
            }
        }
        if(i == numQuery) {
            result[resultCount++] = e;
        }
    }

    delete []ilist;
}

void ShowResult(int *result, int resultCount) {
    cout << resultCount << ": ";
    for(int i = 0; i < resultCount; i++) {
        cout << result[i] << " ";
    }
    cout << endl;
}

void testCode()
{
    // query[0] = 4; query[1] = 1; query[2] = 5;
}

int main()
{
    // queryFile.open(testQueryPath, ios ::in);
    // indexFile.open(testIndexEnlargePath, ios::in | ios::binary);
    queryFile.open(queryPath, ios ::in);
    indexFile.open(indexPath, ios::in | ios::binary);

    MyTimer fileInTimer, computeTimer;
    fileInTimer.start();
    if (!ReadIndex())
    {
        cout << "Error: Fail to reading index" << endl;
        return 0;
    }

    fileInTimer.finish();
    printf("Time Used in File Reading: %8.4fms\n", fileInTimer.getTime());

    computeTimer.start();

    int *result = new int[maxLenIndex];
    int resultCount = 0;

    int readCount = 0;
    while(ReadQuery() && readCount < 5) {
        GetIntersection(result, resultCount);

        // cout << "Result: " << endl;
        // ShowResult(result, resultCount);
        printf("Finish: (%d/1000)\n", ++readCount);
        // cout << resultCount << endl;
    }

    computeTimer.finish();
    printf("Time Used in Computing Intersection: %8.4fms\n", computeTimer.getTime());

    delete[] result;
    queryFile.close();
    indexFile.close();
}