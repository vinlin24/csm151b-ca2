#include <cerrno>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "controller.h"

#define DUMP_MEMORY false

using namespace std;

struct Trace
{
    enum Operation
    {
        READ,
        WRITE,
    } op;
    uint32_t address;
    uint8_t data;
};

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
        if ((memR && memW) || (!memR && !memW))
            throw std::runtime_error("Invalid combination of MemR and MemW");

        Trace trace;
        trace.op = memR ? Trace::READ : Trace::WRITE;
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

    // Main program loop.

    for (Trace const &trace : traces)
    {
        if (trace.op == Trace::READ)
        {
            uint8_t byte = controller.loadByte(trace.address);
            cerr << "LOADED " << trace.address << ": "
                 << static_cast<int>(byte) << endl;
        }
        else
        {
            controller.storeByte(trace.address, trace.data);
            cerr << "STORED " << trace.address << ": "
                 << static_cast<int>(trace.data) << endl;
        }
        controller.dumpCacheState();
#if DUMP_MEMORY
        controller.dumpMemory();
#endif // DUMP_MEMORY
        cerr << endl;
    }

    double L1MissRate = controller.getL1MissRate();
    double L2MissRate = controller.getL2MissRate();
    double AAT = controller.getAAT();

    cout << "("
         << setprecision(10) << L1MissRate << ","
         << setprecision(10) << L2MissRate << ","
         << setprecision(10) << AAT
         << ")" << endl;

    return EXIT_SUCCESS;
}
