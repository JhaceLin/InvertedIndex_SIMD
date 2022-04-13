#include <iostream>
#include <fstream>
#include "config.h"
using namespace std;

void work(bool makeFile, bool forIndex) {
    ifstream inFile(binaryFile, ios::in | ios::binary);
    if (!inFile)
    {
        cout << "Error opening file.";
        return;
    }

    if(!makeFile) { // Display on Terminal
        if(forIndex) {
            int len;
            int buffer[1000];
            while(inFile.read(reinterpret_cast<char *>(&len), sizeof(int))) {
                inFile.read(reinterpret_cast<char *>(buffer), sizeof(int) * len);

                cout << len << " ";
                for(int i = 0; i < len; i++)
                    cout << buffer[i] << " ";
                cout << endl;
            }
        }
    }
}

int main() {
    work(false, true);

    return 0;
}