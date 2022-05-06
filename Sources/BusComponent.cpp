#include "BusComponent.h"
#include "LevelMeter.h"
#include "IOMap.h"

BusComponent::BusComponent(const String &name, int leftIndex, int rightIndex,
                           LevelMeter *left, LevelMeter *right, IOMap *map) :
    m_name(name),
    m_leftIndex(leftIndex),
    m_rightIndex(rightIndex),
    m_left(left),
    m_right(right),
    m_map(map)
{
    addAndMakeVisible(m_outputCombo);

    m_outputCombo.addItemList(map->getOutputPairs(), 1);
    m_outputCombo.setSelectedItemIndex(0);

    m_outputCombo.onChange = [this]() {
        auto index = m_outputCombo.getSelectedItemIndex();
        auto outIndex = index * 2;

        m_map->map(m_leftIndex, outIndex);
        m_map->map(m_rightIndex, outIndex + 1);
    };
}

void BusComponent::paint(Graphics &g)
{
    g.setColour(Colours::white);
    auto bounds = getLocalBounds();

    auto labelBounds = bounds.removeFromTop(getHeight() / 3);
    g.drawText(m_name, labelBounds, Justification::centredLeft);

    auto levelBounds = bounds.removeFromBottom(getHeight() / 3);
    levelBounds = levelBounds.withSizeKeepingCentre(getWidth(), 25);

    auto leftBounds = levelBounds.removeFromTop(10);
    g.setColour(Colours::black);
    g.fillRect(leftBounds);
    g.setColour(Colours::greenyellow);

    auto leftFill = leftBounds.removeFromLeft(roundToInt(m_left->getCurrentLevel() * getWidth()));
    g.fillRect(leftFill);

    auto rightBounds = levelBounds.removeFromBottom(10);
    g.setColour(Colours::black);
    g.fillRect(rightBounds);
    g.setColour(Colours::greenyellow);

    auto rightFill = rightBounds.removeFromLeft(roundToInt(m_right->getCurrentLevel() * getWidth()));
    g.fillRect(rightFill);
}

void BusComponent::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(getHeight() / 3);
    bounds.removeFromBottom(getHeight() / 3);

    m_outputCombo.setBounds(bounds.reduced(0, 3));
}
