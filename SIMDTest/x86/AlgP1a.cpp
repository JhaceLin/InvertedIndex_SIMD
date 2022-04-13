#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <pmmintrin.h>
#include <ctime>
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


class IList
{
public:
    int *list;
    int length;
    int remainLen;
    int start;

    IList()
    {
        list = NULL;
        length = 0;
        remainLen = 0;
        start = 0;
    }

    IList(int list[], int length, int remainLen, int start)
    {
        this->length = length;
        this->remainLen = remainLen;
        this->start = start;
        this->list = new int[length];
        memcpy(this->list, list, sizeof(int) * length);
    }

    void modify(int list[], int length, int remainLen, int start)
    {
        this->length = length;
        this->remainLen = remainLen;
        this->start = start;
        this->list = new int[length];
        memcpy(this->list, list, sizeof(int) * length);
    }

    void deepCopy(const IList &t) {
        length = t.length;
        remainLen = t.remainLen;
        start = t.start;
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

void ShowIList(IList &ilist, bool showNaN = true) {
    if(showNaN) {
        cout << ilist.length << ": ";
        for(int j = 0; j < ilist.length; j++) {
            cout << ilist.list[j] << " ";
        }
    }
    else {
        cout << ilist.remainLen << ": ";
        for(int j = 0; j < ilist.length; j++) {
            if(ilist.list[j] != NaN)
                cout << ilist.list[j] << " ";
        }
    }
    cout << endl;
}


void print_m128i(__m128i var) {
    int val[4];
    memcpy(val, &var, sizeof(var));
    printf("Numerical: %d %d %d %d\n", val[0], val[1], val[2], val[3]);
}

int GetOffset(void const *address, long long modOp) {
    long long t = reinterpret_cast<long long>(address);
    return t % modOp;
}

void GetVisitOrder(IList *ilist, int n, int *visit) {
    int rank[n];
    for(int i = 0; i < n; i++) {
        rank[i] = 0;
        for(int j = 0; j < i; j++) {
            if(ilist[i] < ilist[j]) {
                rank[j]++;
            } else {
                rank[i]++;
            }
        }
    }

    for(int i = 0; i < n; i++) {
        visit[rank[i]] = i;
    }
}

int GetX_Search_P(IList &ilist, int e)
{
    // cout << "Find e: " << e << " from: "<< endl;
    // ShowIList(ilist, true);

    int n = ilist.length - ilist.start;
    int *arr = ilist.list + ilist.start;

    // cout << ilist.list << " + " << ilist.start << " = " << arr << endl;

    int offset = GetOffset(arr, 16LL);

    // cout << "Offset = " << offset << endl;

    int i;
    int start;
    if(offset) {
        start = (16 - offset) / 4;
    }
    else {
        start = 0;
    }

    // cout << "Start = " << start << endl;

    for(i = 0; i < start; i++) {
        if(arr[i] >= e) {
            return i + ilist.start;
        }
    }

    __m128i x1, x2, r;
    x1 = _mm_set1_epi32(e);
    // cout << "x1 "; print_m128i(x1);
    
    for(i = start; i < n; i += 4) {
        x2 = _mm_load_si128(reinterpret_cast<const __m128i*>(arr + i));
        // cout << "x2 "; print_m128i(x2);
        r = _mm_cmpgt_epi32(x1, x2);
        // cout << "r "; print_m128i(r);
        int preI = _mm_movemask_ps(reinterpret_cast<__m128>(r));
        // printf("preI(16): %x\n", preI);
        int cmp = preI ^ 0xF;
        // printf("cmp(16): %x\n", cmp);
        if(cmp) {
            int andOp = 1;
            int zeroPos = 0;
            while(!(cmp & andOp)) {
                andOp <<= 1;
                zeroPos++;
            }
            // printf("zeroPos: %d\n", zeroPos);
            return i + zeroPos + ilist.start; 
        }
    }
    if(i == n) return i + ilist.start - 1;

    for(i = i - 3; i < n; i++) {
        if(arr[i] >= e) {
            return i + ilist.start;
        }
    }

    return ilist.length - 1;
}

int GetEliminatorOrder(IList *ilist, int n, int *visit)
{
    return ilist[visit[0]].start;
}

void RemoveE(IList &ilist, int order) {
    ilist.list[order] = NaN;
    ilist.start = order + 1;
    ilist.remainLen = ilist.length - ilist.start;
}

void RemoveX(IList &ilist, int order) {
    ilist.start = order;
    ilist.remainLen = ilist.length - ilist.start;
}

void GetIntersection(int *result, int &resultCount)
{
    if(numQuery < 2) return;

    //制造拷贝
    IList *ilist = new IList[numQuery];

    for(int i = 0; i < numQuery; i++) {
        ilist[i].modify(iindex[query[i]], lenIndex[query[i]], lenIndex[query[i]], 0);
    }

    int visit[numQuery];
    GetVisitOrder(ilist, numQuery, visit);
    
    resultCount = 0;
    while(ilist[visit[0]].remainLen > 0) {
        // Get e
        int eOrder = GetEliminatorOrder(ilist, numQuery, visit);
        int e = ilist[visit[0]].list[eOrder];
        RemoveE(ilist[visit[0]], eOrder);

        int xOrder, x;
        int i; // i : Currently visit order
        for(i = 1; i < numQuery; i++) {

            xOrder = GetX_Search_P(ilist[visit[i]], e);

            x = ilist[visit[i]].list[xOrder];

            // ShowIList(ilist[visit[i]], true);
            // cout << "Find e: " << e << " get x: " << x << endl;
            
            if(x == e) {
                RemoveE(ilist[visit[i]], xOrder);
            }
            else {
                RemoveX(ilist[visit[i]], xOrder);
                break;
            }
        }
        if(i == numQuery) {
            result[resultCount++] = e;
        }

        GetVisitOrder(ilist, numQuery, visit); // Let the list with the shortest remaining length be visit[0]
    }

    // ShowIList(ilist, numQuery, false);

    delete[] ilist;
}

void ShowResult(int *result, int resultCount) {
    cout << resultCount << ": ";
    for(int i = 0; i < resultCount; i++) {
        cout << result[i] << " ";
    }
    cout << endl;
}

double getTime(long long start, long long finish, long long freq) {
    // ms
    double duration = (finish - start) * 1000.0 / freq;
    return duration;
}

int main()
{
    // queryFile.open(testQueryPath, ios ::in);
    // indexFile.open(testIndexPath, ios::in | ios::binary);
    queryFile.open(queryPath, ios ::in);
    indexFile.open(indexPath, ios::in | ios::binary);

    if (!ReadIndex())
    {
        cout << "Error: Fail to reading index" << endl;
        return 0;
    }

    long long start, finish, freq;
    double timeMS = 0;
    QueryPerformanceFrequency((LARGE_INTEGER*) &freq);

    QueryPerformanceCounter((LARGE_INTEGER*) &start);

    int *result = new int[maxLenIndex];
    int resultCount = 0;

    int readCount = 0;
    while(ReadQuery()) {
        GetIntersection(result, resultCount);

        // cout << "Result: " << endl;
        // ShowResult(result, resultCount);
        printf("Finish: (%d/1000)\n", ++readCount);
    }

    QueryPerformanceCounter((LARGE_INTEGER*) &finish);
    timeMS = getTime(start, finish, freq);
    printf("Time Used: %8.4fms\n", timeMS);

    delete[] result;
    queryFile.close();
    indexFile.close();
}