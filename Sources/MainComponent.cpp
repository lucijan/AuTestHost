#include "MainComponent.h"

MainComponent::PluginTableModel::PluginTableModel(MainComponent *parent,
                                                  juce::KnownPluginList *pluginList) :
    m_parent(parent),
    m_pluginList(pluginList)
{
}

int MainComponent::PluginTableModel::getNumRows()
{
    return m_pluginList->getNumTypes();
}

void MainComponent::PluginTableModel::paintRowBackground(Graphics &g, int rowNumber,
    int width, int height, bool rowIsSelected)
{
    ignoreUnused(rowNumber);

    if(rowIsSelected)
    {
        g.setColour(Colours::grey);
        g.fillRect(0, 0, width, height);
    }
}

void MainComponent::PluginTableModel::paintCell(Graphics &g, int rowNumber,
    int columnId, int width, int height, bool rowIsSelected)
{
    ignoreUnused(rowIsSelected);

    if(rowNumber < 0 || rowNumber > m_pluginList->getNumTypes())
    {
        return;
    }

    if(columnId != 1)
    {
        return;
    }

    const auto &types = m_pluginList->getTypes();
    auto type = types[rowNumber];

    g.setColour(Colours::white);
    g.drawText(type.name, 0, 0, width, height, Justification::centredLeft);
}

void MainComponent::PluginTableModel::cellDoubleClicked(int rowNumber, int columnId,
                                                        const MouseEvent &)
{
    ignoreUnused(columnId);

    if(rowNumber < 0 || rowNumber > m_pluginList->getNumTypes())
    {
        return;
    }

    const auto &types = m_pluginList->getTypes();
    const auto &type = types[rowNumber];

    m_parent->load(type);
}

#pragma mark MainComponent

MainComponent::MainComponent() :
    m_pluginListComponent(m_pluginFormatManager, m_pluginList, juce::File(), nullptr, true)
{
    setSize (600, 400);

    auto audioErr = m_audioDeviceManager.initialise(0, 2, nullptr, true);
    if(audioErr.isNotEmpty())
    {
        DBG("Error initializing audio device manager: " << audioErr);
    }
    else
    {
        m_audioDeviceManager.addAudioCallback(this);
    }

    auto midiDevices = MidiInput::getAvailableDevices();
    for(const auto &device : midiDevices)
    {
        DBG("[MIDI] " << device.name);
        m_audioDeviceManager.setMidiInputDeviceEnabled(device.identifier, true);
    }

    m_audioDeviceManager.addMidiInputDeviceCallback(String(), this);

    m_auFormat = new juce::AudioUnitPluginFormat();
    m_pluginFormatManager.addFormat(m_auFormat);

    m_scanner = std::make_unique<juce::PluginDirectoryScanner>(m_pluginList,
        *m_auFormat, m_auFormat->getDefaultLocationsToSearch(), true,
        juce::File(), true);

    juce::String name;
    auto i = 0;
    while(m_scanner->scanNextFile(true, name) && i < 1000)
    {
        i++;
    }

    m_pluginListComponent.setTableModel(new PluginTableModel(this, &m_pluginList));

    auto &header = m_pluginListComponent.getTableListBox().getHeader();
    header.removeColumn(2);
    header.removeColumn(3);
    header.removeColumn(4);
    header.removeColumn(5);

    m_pluginListComponent.getTableListBox().setRowHeight(35);
    addAndMakeVisible(m_pluginListComponent);
}

MainComponent::~MainComponent()
{
    delete m_editor;
}

void MainComponent::load(const juce::PluginDescription &plug)
{
    DBG("MainComponent::load " << plug.name << " " << m_sampleRate << " " << m_bufferSize);

    m_auFormat->createPluginInstanceAsync(plug, m_sampleRate, m_bufferSize,
        [this](std::unique_ptr<AudioPluginInstance> instance, const String &error)
    {
        if(!instance)
        {
            DBG("MainComponent::load [callback] Failed: " << error);
            return;
        }

        m_plugin = std::move(instance);
        m_plugin->prepareToPlay(m_sampleRate, m_bufferSize);
        m_plugin->enableAllBuses();

        auto bussesLayout = m_plugin->getBusesLayout();

        auto inCount = 0;
        auto outCount = 0;

        auto ins = bussesLayout.inputBuses;
        for(auto in : ins)
        {
            inCount += in.size();
        }

        auto outs = bussesLayout.outputBuses;
        for(auto out : outs)
        {
            outCount += out.size();
        }

        auto numChannels = jmax(inCount, outCount);
        m_processBuffer = juce::AudioBuffer<float>(numChannels, m_bufferSize);

        auto outBusses = m_plugin->getBusCount(false);

        m_meters.clear();
        m_meters.resize(static_cast<size_t>(numChannels));
        m_busComponents.clear();

        for(auto busIndex = 0; busIndex < outBusses; busIndex++)
        {
            auto bus = m_plugin->getBus(false, busIndex);

            jassert(bus->getNumberOfChannels() == 2);
            auto leftMeter = std::make_unique<LevelMeter>();
            auto rightMeter = std::make_unique<LevelMeter>();

            auto leftIndex = bus->getChannelIndexInProcessBlockBuffer(0);
            auto rightIndex = bus->getChannelIndexInProcessBlockBuffer(1);

            auto busComponent = std::make_unique<BusComponent>(bus->getName(),
                leftMeter.get(), rightMeter.get());

            addAndMakeVisible(busComponent.get());

            m_meters[(size_t)leftIndex] = std::move(leftMeter);
            m_meters[(size_t)rightIndex] = std::move(rightMeter);

            m_busComponents.push_back(std::move(busComponent));
        }

        if(m_plugin->hasEditor())
        {
            m_editor = m_plugin->createEditorIfNeeded();
            if(m_editor)
            {
                m_pluginListComponent.setVisible(false);
                addAndMakeVisible(m_editor);

                m_initialised = true;
                startTimerHz(15);
                resized();
            }
        }
    });
}


void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    m_pluginListComponent.setBounds(getLocalBounds().reduced(5));

    if(m_editor)
    {
        auto bounds = getLocalBounds();
        auto editorBounds = bounds.removeFromTop(getHeight() / 4 * 3);
        m_editor->setBounds(editorBounds);

        juce::Grid grid;
        grid.setGap(5_px);

        grid.templateRows.add(1_fr);
        grid.templateRows.add(1_fr);

        grid.templateColumns.add(1_fr);
        grid.templateColumns.add(1_fr);
        grid.templateColumns.add(1_fr);
        grid.templateColumns.add(1_fr);
        grid.templateColumns.add(1_fr);
        grid.templateColumns.add(1_fr);
        grid.templateColumns.add(1_fr);
        grid.templateColumns.add(1_fr);

        for(auto &busComp : m_busComponents)
        {
            grid.items.add(busComp.get());
        }

        grid.performLayout(bounds.reduced(5));
    }
}

void MainComponent::timerCallback()
{
    for(auto &component : m_busComponents)
    {
        component->repaint();
    }
}

void MainComponent::audioDeviceIOCallback(const float **inputChannelData,
    int numInputChannels, float **outputChannelData,
                           int numOutputChannels, int numSamples)
{
    ignoreUnused(inputChannelData);
    ignoreUnused(numInputChannels);

    AudioBuffer<float> buffer(outputChannelData, numOutputChannels, 0, numSamples);
    buffer.clear();

    auto writePtrs = buffer.getArrayOfWritePointers();

    if(m_plugin && m_initialised)
    {
        juce::MidiBuffer localBuffer;

        {
            juce::ScopedTryLock midiLock(m_midiMutex);
            if(midiLock.isLocked())
            {
                localBuffer.swapWith(m_midiBuffer);
            }
        }

        m_plugin->processBlock(m_processBuffer, localBuffer);

        auto outBusses = m_plugin->getBusCount(false);
        auto readPtrs = m_processBuffer.getArrayOfReadPointers();

        for(auto busIndex = 0; busIndex < outBusses; busIndex++)
        {
            auto bus = m_plugin->getBus(false, busIndex);

            for(auto channel = 0; channel < bus->getNumberOfChannels(); channel++)
            {
                auto sourceChannel = bus->getChannelIndexInProcessBlockBuffer(channel);

                auto sourcePtr = readPtrs[sourceChannel];
                auto addTo = writePtrs[channel];

                m_meters[(size_t)sourceChannel]->updateLevel(sourcePtr, numSamples);

                juce::FloatVectorOperations::add(addTo, sourcePtr, numSamples);
            }
        }
    }
}

void MainComponent::audioDeviceAboutToStart(AudioIODevice *device)
{
    DBG("MainComponent::audioDeviceAboutToStart " << device->getName());

    m_sampleRate = device->getCurrentSampleRate();
    m_bufferSize = device->getCurrentBufferSizeSamples();

    auto outs = device->getOutputChannelNames();
    for(const auto &out : outs)
    {
        DBG("[IF OUT] " << out);
    }
}

void MainComponent::audioDeviceStopped()
{
    DBG("MainComponent::audioDeviceStopped");
}

void MainComponent::audioDeviceError(const String &errorMessage)
{
    DBG("MainComponent::audioDeviceError " << errorMessage);
}

void MainComponent::handleIncomingMidiMessage(MidiInput *source,
                                              const MidiMessage &message)
{
    ignoreUnused(source);

    juce::ScopedLock lock(m_midiMutex);
    m_midiBuffer.addEvent(message, 0);
}
