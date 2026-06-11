#include <JuceHeader.h>

// ──────────────────────────────────────────────────────────────────────────────
// MainComponent
// ──────────────────────────────────────────────────────────────────────────────

class MainComponent : public juce::Component,
                      public juce::AudioIODeviceCallback,
                      public juce::Timer
{
public:
    MainComponent()
    {
        deviceSelector = std::make_unique<juce::AudioDeviceSelectorComponent>(
            deviceManager, 1, 2, 1, 2, false, false, true, false);
        addAndMakeVisible(*deviceSelector);

        for (auto* l : { &bufferLabel, &resultLabel, &statusLabel, &noteLabel })
            addAndMakeVisible(l);
        addAndMakeVisible(pingButton);

        bufferLabel.setJustificationType(juce::Justification::centred);
        bufferLabel.setFont(juce::Font(juce::FontOptions(14.0f)));

        resultLabel.setJustificationType(juce::Justification::centred);
        resultLabel.setFont(juce::Font(juce::FontOptions(32.0f).withStyle("Bold")));
        resultLabel.setColour(juce::Label::textColourId, juce::Colour(0xff89dceb));

        statusLabel.setJustificationType(juce::Justification::centred);
        statusLabel.setFont(juce::Font(juce::FontOptions(12.0f)));
        statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa6adc8));

        noteLabel.setText(
            "Loopback test: connect audio output to input (cable or virtual loopback like VB-Cable), "
            "then click Ping. The tool sends a short click and measures how long it takes to return.",
            juce::dontSendNotification);
        noteLabel.setJustificationType(juce::Justification::centredLeft);
        noteLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
        noteLabel.setColour(juce::Label::textColourId, juce::Colour(0xff6c7086));

        pingButton.setButtonText("Ping");
        pingButton.setColour(juce::TextButton::buttonColourId,  juce::Colour(0xff89b4fa));
        pingButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff1e1e2e));
        pingButton.onClick = [this] { startPing(); };

        deviceManager.initialise(2, 2, nullptr, true);
        deviceManager.addAudioCallback(this);

        startTimer(80);
        setSize(560, 700);
    }

    ~MainComponent() override
    {
        stopTimer();
        deviceManager.removeAudioCallback(this);
        deviceManager.closeAudioDevice();
    }

    // ── AudioIODeviceCallback ──────────────────────────────────────────────────

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override
    {
        currentSR.store(device->getCurrentSampleRate());
        currentBlock.store(device->getCurrentBufferSizeSamples());
        totalSamples.store(0);
    }

    void audioDeviceStopped() override {}

    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext&) override
    {
        // Silence output
        for (int ch = 0; ch < numOutputChannels; ++ch)
            if (outputChannelData[ch])
                juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);

        const int     state = testState.load();
        const int64_t pos   = totalSamples.load();

        if (state == STATE_SENDING)
        {
            // 4-sample impulse on all output channels
            const int n = juce::jmin(4, numSamples);
            for (int ch = 0; ch < numOutputChannels; ++ch)
                if (outputChannelData[ch])
                    for (int i = 0; i < n; ++i)
                        outputChannelData[ch][i] = 0.8f;

            sendPos.store(pos);
            testState.store(STATE_LISTENING);
        }
        else if (state == STATE_LISTENING)
        {
            bool found = false;
            for (int ch = 0; ch < numInputChannels && !found; ++ch)
            {
                if (!inputChannelData[ch]) continue;
                for (int i = 0; i < numSamples && !found; ++i)
                {
                    if (std::abs(inputChannelData[ch][i]) > kThreshold)
                    {
                        measuredSamples.store((int)(pos + i - sendPos.load()));
                        testState.store(STATE_DONE);
                        found = true;
                    }
                }
            }

            if (!found)
            {
                const double sr = currentSR.load();
                if (sr > 0.0 && (pos - sendPos.load()) > (int64_t)(sr * kTimeoutSec))
                    testState.store(STATE_TIMEOUT);
            }
        }

        totalSamples.fetch_add(numSamples);
    }

    // ── Timer (GUI updates at ~12 Hz) ─────────────────────────────────────────

    void timerCallback() override
    {
        const double sr    = currentSR.load();
        const int    block = currentBlock.load();

        if (sr > 0.0 && block > 0)
        {
            const double bufMs = block / sr * 1000.0;
            bufferLabel.setText(
                juce::String((int)sr) + " Hz   |   buffer " +
                juce::String(block) + " samples   |   buffer latency " +
                juce::String(bufMs, 2) + " ms",
                juce::dontSendNotification);
        }

        const int state = testState.load();

        if (state == STATE_DONE)
        {
            const int    s  = measuredSamples.load();
            const double ms = (currentSR.load() > 0.0) ? s / currentSR.load() * 1000.0 : 0.0;
            resultLabel.setText(juce::String(ms, 2) + " ms  (" + juce::String(s) + " samples)",
                                juce::dontSendNotification);
            statusLabel.setText("Round-trip complete", juce::dontSendNotification);
            pingButton.setEnabled(true);
            testState.store(STATE_IDLE);
        }
        else if (state == STATE_TIMEOUT)
        {
            resultLabel.setText("--", juce::dontSendNotification);
            statusLabel.setText("No signal returned - connect output to input for loopback test",
                                juce::dontSendNotification);
            pingButton.setEnabled(true);
            testState.store(STATE_IDLE);
        }
    }

    // ── GUI ───────────────────────────────────────────────────────────────────

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1e1e2e));
        g.setColour(juce::Colour(0xffcdd6f4));
        g.setFont(juce::Font(juce::FontOptions(20.0f).withStyle("Bold")));
        g.drawText("AlteredAudio  Latency Tester",
                   getLocalBounds().removeFromTop(48),
                   juce::Justification::centred);

        // Divider below device selector section  (48 title + 310 selector + 6 gap = 364)
        g.setColour(juce::Colour(0xff313244));
        g.drawHorizontalLine(366, 12.0f, (float)(getWidth() - 12));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(12);
        area.removeFromTop(48);

        deviceSelector->setBounds(area.removeFromTop(310));
        area.removeFromTop(6);

        bufferLabel.setBounds(area.removeFromTop(24));
        area.removeFromTop(10);

        pingButton.setBounds(area.removeFromTop(42).reduced(190, 0));
        area.removeFromTop(12);

        resultLabel.setBounds(area.removeFromTop(48));
        statusLabel.setBounds(area.removeFromTop(22));
        area.removeFromTop(8);

        noteLabel.setBounds(area.removeFromTop(52));
    }

private:
    void startPing()
    {
        pingButton.setEnabled(false);
        resultLabel.setText("...", juce::dontSendNotification);
        statusLabel.setText("Sending ping...", juce::dontSendNotification);
        testState.store(STATE_SENDING);
    }

    static constexpr float  kThreshold  = 0.05f;
    static constexpr double kTimeoutSec = 1.5;

    enum { STATE_IDLE, STATE_SENDING, STATE_LISTENING, STATE_DONE, STATE_TIMEOUT };

    juce::AudioDeviceManager deviceManager;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> deviceSelector;

    juce::TextButton pingButton;
    juce::Label      bufferLabel, resultLabel, statusLabel, noteLabel;

    std::atomic<int>     testState      { STATE_IDLE };
    std::atomic<int64_t> totalSamples   { 0 };
    std::atomic<int64_t> sendPos        { 0 };
    std::atomic<int>     measuredSamples{ 0 };
    std::atomic<double>  currentSR      { 44100.0 };
    std::atomic<int>     currentBlock   { 256 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

// ──────────────────────────────────────────────────────────────────────────────
// Window + Application
// ──────────────────────────────────────────────────────────────────────────────

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow()
        : DocumentWindow("AlteredAudio Latency Tester",
                         juce::Colour(0xff1e1e2e),
                         DocumentWindow::closeButton)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new MainComponent(), true);
        setResizable(false, false);
        centreWithSize(560, 700);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

class LatencyTesterApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName()    override { return "Latency Tester"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed()          override { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow>();
    }

    void shutdown() override { mainWindow.reset(); }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(LatencyTesterApp)
