/*
  ==============================================================================

    GlitchVisualizerComponent.cpp
    Created: 6 Mar 2026 11:01:02pm
    Author:  doare

  ==============================================================================
*/

#include "GlitchVisualizerComponent.h"

//==============================================================================
void GlitchVisualizer::paint(juce::Graphics& g)
{

    const auto straightColour = juce::Colours::cyan;
    const auto dottedColour = juce::Colours::magenta;
    const auto tripletColour = juce::Colours::lime;

    auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRect(bounds);

    if (cycleData.steps.empty())
        return;

    // Calculate total duration in numerator units
    int totalDuration = 0;
    for (const auto& step : cycleData.steps)
        totalDuration += step.stepDurationNumerator;

    if (totalDuration == 0) return;

    // Get Time Signature Denominator for correct repeat duration calculation
    int timeSigDenominator = 4;
    if (auto* ph = processor.getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo posInfo;
        posInfo.resetToDefault();
        ph->getCurrentPosition(posInfo);
        if (posInfo.timeSigDenominator > 0)
            timeSigDenominator = posInfo.timeSigDenominator;
    }

    float xPos = 0.0f;
    int stepIdx = 0;

    for (const auto& step : cycleData.steps)
    {
        float width = (float)step.stepDurationNumerator / totalDuration * bounds.getWidth();
        juce::Rectangle<float> stepRect(bounds.getX() + xPos, bounds.getY(), width, bounds.getHeight());

        // Draw step background
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.drawRect(stepRect);

        if (step.isGlitching)
        {
            // Find instruction
            const GlitchInstruction* instr = nullptr;
            for (const auto& i : cycleData.instructions) {
                if (i.stepIndex == stepIdx) {
                    instr = &i;
                    break;
                }
            }

            if (instr)
            {
                // Determine color based on type
                juce::Colour grainColor = juce::Colours::cyan; // Normal
                if (instr->isDotted)
                    grainColor = juce::Colours::magenta;
                else if (instr->isTripled)
                    grainColor = juce::Colours::lime;

                // Calculate repeat duration in step units
                // Formula: (timeSigDenominator / repeatDurationDenominator) * modifier
                double repeatDuration = (double)timeSigDenominator / (double)instr->repeatDurationDenominator;

                if (instr->isDotted)
                    repeatDuration *= 1.5;
                else if (instr->isTripled)
                    repeatDuration *= (2.0 / 3.0);

                // Draw grains
                double currentPos = 0.0;
                if (repeatDuration > 0.001)
                {
                    while (currentPos < step.stepDurationNumerator)
                    {
                        float grainRelX = (float)(currentPos / step.stepDurationNumerator) * width;
                        float grainX = stepRect.getX() + grainRelX;

                        float repeatWidthPx = (float)(repeatDuration / step.stepDurationNumerator) * width;
                        float activeWidthPx = repeatWidthPx * instr->cutLength;

                        if (grainX + activeWidthPx > stepRect.getRight())
                            activeWidthPx = stepRect.getRight() - grainX;

                        if (activeWidthPx > 0.5f)
                        {
                            g.setColour(grainColor.withAlpha(0.7f));
                            juce::Rectangle<float> grainRect(grainX, stepRect.getY(), activeWidthPx, stepRect.getHeight());
                            g.fillRect(grainRect.reduced(0.5f, 0.0f));
                        }

                        currentPos += repeatDuration;
                    }
                }
            }
        }
        else
        {
            g.setColour(juce::Colours::green.withAlpha(0.2f));
            g.fillRect(stepRect.reduced(1.0f));
        }

        xPos += width;
        stepIdx++;
    }

    // Draw Playhead
    float playheadPos = processor.getNormalizedCyclePosition();
    if (playheadPos >= 0.0f && playheadPos <= 1.0f)
    {
        float phX = bounds.getX() + playheadPos * bounds.getWidth();
        g.setColour(juce::Colours::white);
        g.drawLine(phX, bounds.getY(), phX, bounds.getBottom(), 2.0f);
    }
}
