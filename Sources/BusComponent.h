#pragma once

#include <JuceHeader.h>

class LevelMeter;

class BusComponent : public juce::Component
{
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BusComponent)

public:
    BusComponent(const String &name, LevelMeter *left, LevelMeter *right);

private:
    void paint(Graphics &g) override;

    String m_name;
    LevelMeter *m_left = nullptr;
    LevelMeter *m_right = nullptr;
};
