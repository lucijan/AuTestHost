#include "BusComponent.h"
#include "LevelMeter.h"

BusComponent::BusComponent(const String &name, LevelMeter *left, LevelMeter *right) :
    m_name(name),
    m_left(left),
    m_right(right)
{
}

void BusComponent::paint(Graphics &g)
{
    g.setColour(Colours::white);
    auto bounds = getLocalBounds();

    auto labelBounds = bounds.removeFromTop(getHeight() / 2);
    g.drawText(m_name, labelBounds, Justification::centredLeft);

    auto levelBounds = bounds.withSizeKeepingCentre(getWidth(), 25);

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
