#pragma once

#include <JuceHeader.h>

class LevelMeter
{
public:
    LevelMeter();

    void updateLevel(const float *data, int numSamples);
    float getCurrentLevel() const;

private:
    std::atomic<float> m_level = 0.0;

};
