#include "LevelMeter.h"

LevelMeter::LevelMeter()
{
}

void LevelMeter::updateLevel(const float *data, int numSamples)
{
    auto localLevel = m_level.load();

    for(int j = 0; j < numSamples; ++j)
    {
        const float decayFactor = 0.99992f;

        auto s = std::abs(data[j]);
        if(s > localLevel)
        {
            localLevel = s;
        }
        else if(localLevel > 0.001f)
        {
            localLevel *= decayFactor;
        }
        else
        {
            localLevel = 0;
        }
    }

    m_level = localLevel;

/*
 auto localLevel = level.get();

     if (numChannels > 0)
     {
         for (int j = 0; j < numSamples; ++j)
         {
             float s = 0;

             for (int i = 0; i < numChannels; ++i)
                 s += std::abs (channelData[i][j]);

             s /= (float) numChannels;

             const float decayFactor = 0.99992f;

             if (s > localLevel)
                 localLevel = s;
             else if (localLevel > 0.001f)
                 localLevel *= decayFactor;
             else
                 localLevel = 0;

 */
}

float LevelMeter::getCurrentLevel() const
{
    return m_level.load();
}
