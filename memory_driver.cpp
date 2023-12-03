#include <cerrno>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "cache.h"

using namespace std;

static vector<Trace> const readTraces(ifstream &inputFile)
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

        bool memR = stoi(s1);
        bool memW = stoi(s2);
        if (memR && memW || !memR && !memW)
            throw std::runtime_error("Invalid combination of MemR and MemW");

        Trace trace;
        trace.op = memR ? READ : WRITE;
        trace.address = stoi(s3);
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
    vector<Trace> const traces = readTraces(inputFile);

    Controller controller;
    for (Trace const &trace : traces)
    {
        trace.print();
        controller.processTrace(trace);
        controller.dumpMemory();
    }

    float L1MissRate = controller.getL1MissRate();
    float L2MissRate = controller.getL2MissRate();
    float AAT = controller.getAAT();

    cout << "(" << L1MissRate << "," << L2MissRate << "," << AAT << ")" << endl;

    return EXIT_SUCCESS;
}