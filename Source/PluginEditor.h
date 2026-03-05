/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
// #include <FxmeJuceTools/FxmeJuceTools.h>
// #include <FxmeJuceTools/Components/ScrollingScopeComponent.h>

//==============================================================================
class GlitchVisualizer : public juce::Component, public juce::Timer
{
public:
    GlitchVisualizer(GloubiboulgaAudioProcessor& p) : processor(p)
    {
        startTimerHz(30); // Repaint at 30fps
    }

    ~GlitchVisualizer() override
    {
        stopTimer();
    }

    void timerCallback() override
    {
        // Check if cycle data changed
        int currentVersion = processor.getCycleVersion();
        if (currentVersion != lastVersion)
        {
            cycleData = processor.getGlitchCycleData();
            lastVersion = currentVersion;
            repaint();
        }
        
        // Always repaint for playhead position
        repaint();
    }

    void paint(juce::Graphics& g) override;

private:
    GloubiboulgaAudioProcessor& processor;
    GlitchCycleData cycleData;
    int lastVersion = -1;
    
    juce::Colour getColourForInstruction(const GlitchInstruction& instr);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlitchVisualizer)
};

//==============================================================================
/**
*/
class GloubiboulgaAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    GloubiboulgaAudioProcessorEditor (GloubiboulgaAudioProcessor&);
    ~GloubiboulgaAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GloubiboulgaAudioProcessor& audioProcessor;

    GlitchVisualizer visualizer;
    fxme::ScrollingScopeComponent scopeLeft;
    fxme::ScrollingScopeComponent scopeRight;
    // UI Components
    fxme::FxmeLookAndFeel fxmeLookAndFeel;
    std::vector<std::unique_ptr<fxme::FxmeSlider>> fxmeSliders;

    fxme::TitleBar patternBar, glitchBar, seedBar, bufferBar,
                    timingBar, cutBar, metricBar, filterBar,
                    filterEnvBar, levelBar, envBar, outputBar;

    struct FilterTypeBoxData
    {
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::ComboBox> comboBox;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    std::vector<FilterTypeBoxData> filterTypeBoxes;

    void createKnob(const juce::String& paramID, const juce::String& labelText, juce::Colour c);
    void createVSlider(const juce::String &paramID, const juce::String &labelText, juce::Colour c);
    void createFilterTypeBox(const juce::String &paramID, const juce::String &labelText);
    fxme::FxmeSlider* getSlider(const juce::String& paramID);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GloubiboulgaAudioProcessorEditor)
};
