#pragma once

#include <JuceHeader.h>

class IOMap
{
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IOMap)

public:
    IOMap(juce::AudioIODevice *device);

    StringArray getOutputPairs() const;

    void map(int src, int dest);
    inline std::vector<int> getMap() const;

private:
    juce::AudioIODevice *m_device;

    std::vector<int> m_map = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };
};

std::vector<int> IOMap::getMap() const
{
    return m_map;
}
