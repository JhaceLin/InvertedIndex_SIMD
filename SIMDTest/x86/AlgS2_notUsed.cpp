#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
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

long long section_Esti[10], section_Real[10];
const int divisor[] = {100000, 10000, 1000, 100, 10, 1};

int GetX_Search(IList &ilist, int e)
{
    int n = ilist.length;
    int *arr = ilist.list;
    int l = ilist.start, r = n - 1;

    int i;

    int mid, cmp = 0;
    while(r - l > 10000) {
        mid = (l + r) >> 1;
        cmp = arr[mid] - e;
        if(!cmp) {
            return mid;
        }
        if(cmp < 0) { // arr[mid] < e
            l++;
        }
        else {
            r--;
        }
    }

    for(i = l; i <= r; i++) {
        if(arr[i] >= e) {
            break;
        }
    }
    return i;
}

int GetX_Search_Old(IList &ilist, int e)
{
    int n = ilist.length;
    int *arr = ilist.list;

    int i;
    for(i = ilist.start; i < n; i++) {
        if(arr[i] >= e) {
            break;
        }
    }

    for(int k = 0; k < 6; k++) {
        if((n - ilist.start) / divisor[k]) {
            section_Esti[k]++;
            break;
        }
    }
    for(int k = 0; k < 6; k++) {
        if((i - ilist.start) / divisor[k]) {
            section_Real[k]++;
            break;
        }
    }

    return i;
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
            xOrder = GetX_Search_Old(ilist[visit[i]], e);

            /*
            int xOrder_Old = GetX_Search_Old(ilist[visit[i]], e);
            if(xOrder != xOrder_Old) {
                cout << "Error!" << endl;
                cout << xOrder << " " << xOrder_Old << endl;
            }*/

            x = ilist[visit[i]].list[xOrder];
            
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

void testCode()
{
    // query[0] = 4; query[1] = 1; query[2] = 5;
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

    int *result = new int[maxLenIndex];
    int resultCount = 0;

    int readCount = 0;
    while(ReadQuery() && readCount < 50) {
        GetIntersection(result, resultCount);

        // cout << "Result: " << endl;
        // ShowResult(result, resultCount);
        printf("Finish: (%d/1000)\n", ++readCount);
    }

    cout << "Esti: " <<  endl;
    for(int i = 0; i < 6; i++) {
        printf("%d: %d, ", divisor[i], section_Esti[i]);
    }
    cout << endl;
    cout << "Real: " <<  endl;
    for(int i = 0; i < 6; i++) {
        printf("%d: %d, ", divisor[i], section_Real[i]);
    }

    delete[] result;
    queryFile.close();
    indexFile.close();
}