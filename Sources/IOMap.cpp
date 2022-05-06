#include "IOMap.h"

IOMap::IOMap(juce::AudioIODevice *device) :
    m_device(device)
{
}

StringArray IOMap::getOutputPairs() const
{
    StringArray results;

    auto chanMask = m_device->getActiveOutputChannels();
    auto count = chanMask.countNumberOfSetBits();

    for(auto i = 0; i < count; i += 2)
    {
        results.add(String(i + 1) + "/" + String(i + 2));
    }

    return results;
}

void IOMap::map(int src, int dest)
{
    DBG("Map: " << src << " => " << dest);

    m_map[(size_t)src] = dest;
}
