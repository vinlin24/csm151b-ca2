#include "cache.h"

using namespace std;

Controller::Controller() : m_MM{0}, m_stats{0}
{
    for (size_t index = 0; index < L1_CACHE_SETS; index++)
        m_L1[index].valid = false;

    for (size_t way = 0; way < VICTIM_SIZE; way++)
        m_VC[way].valid = false;

    for (size_t index = 0; index < L2_CACHE_SETS; index++)
        for (int way = 0; way < L2_CACHE_WAYS; way++)
            m_L2[index][way].valid = false;
}

void Controller::processTrace(Trace const &trace)
{
    // TODO: Add your code here.
}

float Controller::getL1MissRate() const
{
    return static_cast<float>(m_stats.missL1) / m_stats.accessL1;
}

float Controller::getL2MissRate() const
{
    return static_cast<float>(m_stats.missL2) / m_stats.accessL2;
}

float Controller::getAAT() const
{
    return 0; // TODO.
}
