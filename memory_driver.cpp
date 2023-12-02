#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "cache.h"

using namespace std;

struct Trace
{
    bool MemR;
    bool MemW;
    int adr;
    int data;
};

static vector<Trace> readTraces(ifstream &inputFile)
{
    string line;
    vector<Trace> traces;
    string s1, s2, s3, s4;

    while (getline(inputFile, line))
    {
        stringstream ss(line);
        getline(ss, s1, ',');
        getline(ss, s2, ',');
        getline(ss, s3, ',');
        getline(ss, s4, ',');

        Trace trace;
        trace.MemR = stoi(s1);
        trace.MemW = stoi(s2);
        trace.adr = stoi(s3);
        trace.data = stoi(s4);

        traces.push_back(trace);
    }

    return traces;
}

/**
 * USAGE: ./program FILENAME
 */
int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        cerr << "USAGE: " << argv[0] << " FILENAME [MODE]" << endl;
        return EINVAL;
    }

    string inputFileName(argv[1]);
    ifstream inputFile;
    inputFile.open(inputFileName);
    if (!inputFile.is_open())
    {
        cerr << "Error opening " << inputFileName << "." << endl;
        return EXIT_FAILURE;
    }

    vector<Trace> traces = readTraces(inputFile);

    Cache myCache;
    int myMem[MEM_SIZE];

    int traceCounter = 0;
    bool cur_MemR;
    bool cur_MemW;
    int cur_adr;
    int cur_data;

    // Main program loop.
    while (traceCounter < traces.size())
    {
        cur_MemR = traces[traceCounter].MemR;
        cur_MemW = traces[traceCounter].MemW;
        cur_data = traces[traceCounter].data;
        cur_adr = traces[traceCounter].adr;
        traceCounter += 1;
        // TODO: In your memory controller you need to implement your FSM, LW,
        // SW, and MM.
        myCache.controller(cur_MemR, cur_MemW, &cur_data, cur_adr, myMem);
    }

    float L1_miss_rate, L2_miss_rate, AAT;

    // TODO: Compute the stats here.

    cout << "(" << L1_miss_rate << "," << L2_miss_rate << "," << AAT << ")"
         << endl;

    return EXIT_SUCCESS;
}
