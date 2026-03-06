/*
  ==============================================================================

    GlitchVisualizerComponent.h
    Created: 6 Mar 2026 11:01:02pm
    Author:  doare

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

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
