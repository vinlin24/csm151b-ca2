#include "controller.h"

#include <iostream>

using namespace std;

Controller::Controller()
    : m_L1(), m_L2(), m_VC(), m_MM{0}, m_stats{0, 0, 0, 0, 0, 0} {}

void Controller::processTrace(Trace const &trace)
{
    if (trace.op == READ)
    {
        uint8_t byte = loadByte(trace.address);
        cerr << "Loaded " << static_cast<int>(byte) << "." << endl;
    }
    else
    {
        storeByte(trace.address, trace.data);
        cerr << "Stored " << static_cast<int>(trace.data) << " to "
             << trace.address << "." << endl;
    }
}

uint8_t Controller::loadByte(uint32_t address)
{
    // TODO.
}

void Controller::storeByte(uint32_t address, uint8_t byte)
{
    bool written;

    // Case A: L1 Hit.

    written = m_L1.writeByte(address, byte);
    if (written)
    {
        m_MM[address] = byte; // Write-through.
        return;
    }

    // Case B: L1 Miss, VC Hit.

    written = m_VC.writeByte(address, byte);
    if (written)
    {
        m_MM[address] = byte; // Write-through.
        return;
    }

    // Case C: L1 Miss, VC Miss L2 Hit.

    written = m_L2.writeByte(address, byte);
    if (written)
    {
        m_MM[address] = byte; // Write-through.
        return;
    }

    // Case D: L1 Miss, VC Miss, L2 Miss.

    m_MM[address] = byte; // Write-no-allocate.
}

double Controller::getL1MissRate() const
{
    return static_cast<double>(m_stats.missL1) / m_stats.accessL1;
}

double Controller::getL2MissRate() const
{
    return static_cast<double>(m_stats.missL2) / m_stats.accessL2;
}

double Controller::getAAT() const
{
    return 0; // TODO.
}

void Controller::dumpMemory() const
{
    for (size_t address = 0; address < MEM_SIZE; address++)
    {
        uint8_t byte = m_MM[address];
        if (byte == 0)
            continue;
        cerr << address << ": " << static_cast<int>(byte) << endl;
    }
}
