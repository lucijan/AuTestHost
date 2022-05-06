#pragma once

#include <JuceHeader.h>

class LevelMeter;
class IOMap;

class BusComponent : public juce::Component
{
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BusComponent)

public:
    BusComponent(const String &name, int leftIndex, int rightIndex,
                 LevelMeter *left, LevelMeter *right,
                 IOMap *map);

private:
    void paint(Graphics &g) override;
    void resized() override;

    String m_name;
    int m_leftIndex = -1;
    int m_rightIndex = -1;
    LevelMeter *m_left = nullptr;
    LevelMeter *m_right = nullptr;
    IOMap *m_map = nullptr;

    ComboBox m_outputCombo;
};
