#pragma once

#include <JuceHeader.h>

#include "IOMap.h"
#include "LevelMeter.h"
#include "BusComponent.h"

class MainComponent : public juce::Component,
                      public juce::AudioIODeviceCallback,
                      public juce::MidiInputCallback,
                      public Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void load(const juce::PluginDescription &plug);

private:
    class PluginTableModel : public TableListBoxModel
    {
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginTableModel)
    public:
        PluginTableModel(MainComponent *parent, juce::KnownPluginList *pluginList);

    private:
        int getNumRows() override;

        void paintRowBackground(Graphics &g, int rowNumber, int width, int height,
                                bool rowIsSelected) override;

        void paintCell(Graphics &g, int rowNumber, int columnId, int width,
                       int height, bool rowIsSelected) override;

        void cellDoubleClicked(int rowNumber, int columnId,
                               const juce::MouseEvent &) override;

        MainComponent *m_parent = nullptr;
        juce::KnownPluginList *m_pluginList = nullptr;
    };

    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

    void audioDeviceIOCallback(const float **inputChannelData,
        int numInputChannels, float **outputChannelData,
                               int numOutputChannels, int numSamples) override;
    void audioDeviceAboutToStart(AudioIODevice *device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const String &errorMessage) override;

    void handleIncomingMidiMessage(MidiInput *source,
                                   const MidiMessage &message) override;

    juce::AudioDeviceManager m_audioDeviceManager;
    juce::AudioBuffer<float> m_processBuffer;
    double m_sampleRate = 0.0;
    int m_bufferSize = 0;

    std::unique_ptr<IOMap> m_ioMap;

    juce::CriticalSection m_midiMutex;
    juce::MidiBuffer m_midiBuffer;

    juce::AudioPluginFormatManager m_pluginFormatManager;
    juce::KnownPluginList m_pluginList;
    juce::PluginListComponent m_pluginListComponent;
    juce::TextButton m_settingsButton;

    juce::AudioUnitPluginFormat *m_auFormat = nullptr;
    std::unique_ptr<juce::PluginDirectoryScanner> m_scanner;
    std::unique_ptr<juce::AudioPluginInstance> m_plugin;
    juce::AudioProcessorEditor *m_editor = nullptr;
    bool m_initialised = false;

    std::vector<std::unique_ptr<LevelMeter>> m_meters;
    std::vector<std::unique_ptr<BusComponent>> m_busComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
