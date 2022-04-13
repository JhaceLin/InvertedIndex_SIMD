#include <iostream>
#include <fstream>
#include "config.h"
using namespace std;

int main() {
    ifstream inFile(textFile, ios::in);
    if (!inFile)
    {
        cout << "Error opening file.";
        return 0;
    }

    ofstream outFile(binaryFile, ios::out | ios::binary);

    int x;
    while(inFile >> x) {
        outFile.write(reinterpret_cast<char *>(&x), sizeof(int));
    }

    inFile.close();
    outFile.close();
}