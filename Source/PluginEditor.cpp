/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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

GloubiboulgaAudioProcessorEditor::GloubiboulgaAudioProcessorEditor (GloubiboulgaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), visualizer(p),
      scopeLeft(&audioProcessor.scopeFifoLeft, 512, 256, 64, juce::Colours::white),
      scopeRight(&audioProcessor.scopeFifoRight, 512, 256, 64, juce::Colours::white)
{
    setLookAndFeel(&fxmeLookAndFeel);

    const auto patternColour = juce::Colours::cyan;
    const auto seedColour = juce::Colours::grey.brighter(0.5f);
    const auto glitchColour = juce::Colours::grey.brighter(0.5f);    
    const auto bufferColour = juce::Colours::grey.brighter(0.5f);
    const auto timingColour = juce::Colours::pink.darker();
    
    const auto levelColour = juce::Colours::blue.brighter(1.f);
    const auto filterColour = juce::Colours::mediumpurple;
    const auto cycleColour = juce::Colours::darkturquoise;

    const auto straightColour = juce::Colours::cyan;
    const auto dottedColour = juce::Colours::magenta;
    const auto tripletColour = juce::Colours::lime;

    // Create all parameter sliders
    createKnob("RandomSeed", "Random Seed", seedColour);
    createKnob("CycleDuration", "Cycle Duration", glitchColour);
    createKnob("GlitchDurMean", "Glitch Dur Mean", glitchColour);
    createKnob("GlitchDurStdDev", "Glitch Dur StdDev", glitchColour);
    createKnob("GlitchProb", "Glitch Probability", glitchColour);
    createKnob("BufferStartPosMean", "Pos Mean", bufferColour);
    createKnob("BufferStartPosStdDev", "Pos StdDev", bufferColour);
    createKnob("BufferStartOffsetMean", "Offset Mean", bufferColour);
    createKnob("BufferStartOffsetStdDev", "Offset StdDev", bufferColour);
    createKnob("CutProb32nd", "1/32", timingColour);
    createKnob("CutProb16th", "1/16", timingColour);
    createKnob("CutProb8th", "1/8", timingColour);
    createKnob("CutProb4th", "1/4", timingColour);
    createKnob("CutProbNormal", "Straight", straightColour);
    createKnob("CutProbDotted", "Dotted", dottedColour);
    createKnob("CutProbTrippled", "Triplet", tripletColour);
    createKnob("CutLengthMean", "Cut Length Mean", timingColour);
    createKnob("CutLengthStdDev", "Cut Length StdDev", timingColour);

    createFilterTypeBox("FilterType", "Filter Type");

    createKnob("FilterFreqMean", "Freq Mean", filterColour);
    createKnob("FilterFreqStdDev", "Freq StdDev", filterColour);
    createKnob("FilterResMean", "Res Mean", filterColour);
    createKnob("FilterResStdDev", "Res StdDev", filterColour);
    createKnob("FilterEnvDepth", "Env Depth", filterColour);
    createKnob("FilterAttack", "Attack", filterColour);
    createKnob("FilterDecay", "Decay", filterColour);

    createKnob("LevelMean", "Level Mean", levelColour);
    createKnob("LevelStdDev", "Level StdDev", levelColour);
    createKnob("AttackTime", "Attack Time", levelColour);
    createKnob("DecayTime", "Decay Time", levelColour);
    createKnob("OutputMix", "Mix", levelColour);
    createKnob("OutputLevel", "Level", levelColour);

    patternBar.setBarColour(seedColour);
    patternBar.setTitle("Pattern");
    addAndMakeVisible(patternBar);

    seedBar.setBarColour(seedColour);
    seedBar.setTitle("Random");
    addAndMakeVisible(seedBar);

    glitchBar.setBarColour(glitchColour);
    glitchBar.setTitle("Glitch");
    addAndMakeVisible(glitchBar);

    bufferBar.setBarColour(bufferColour);
    bufferBar.setTitle("Buffer");
    addAndMakeVisible(bufferBar);

    timingBar.setBarColour(timingColour);
    timingBar.setTitle("Cut time probabilities");
    addAndMakeVisible(timingBar);

    metricBar.setBarColour(timingColour);
    metricBar.setTitle("Metric probabilities");
    addAndMakeVisible(metricBar);

    cutBar.setBarColour(timingColour);
    cutBar.setTitle("Grain length");
    addAndMakeVisible(cutBar);

    levelBar.setBarColour(levelColour);
    levelBar.setTitle("Level");
    addAndMakeVisible(levelBar);

    envBar.setBarColour(levelColour);
    envBar.setTitle("Enveloppe");
    addAndMakeVisible(envBar);

    filterBar.setBarColour(filterColour);
    filterBar.setTitle("Filter");
    addAndMakeVisible(filterBar);

    filterEnvBar.setBarColour(filterColour);
    filterEnvBar.setTitle("Filter Enveloppe");
    addAndMakeVisible(filterEnvBar);

    outputBar.setBarColour(levelColour);
    outputBar.setTitle("Output");
    addAndMakeVisible(outputBar);

    addAndMakeVisible(scopeLeft);
    addAndMakeVisible(scopeRight);

    addAndMakeVisible(visualizer);
    setSize (800, 600);
}

GloubiboulgaAudioProcessorEditor::~GloubiboulgaAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}
void GloubiboulgaAudioProcessorEditor::createFilterTypeBox(const juce::String& paramID, const juce::String& labelText)
{
    FilterTypeBoxData filterTypeData;
    filterTypeData.label = std::make_unique<juce::Label>();
    filterTypeData.label->setText(labelText, juce::dontSendNotification);

    filterTypeData.comboBox = std::make_unique<juce::ComboBox>();
    filterTypeData.comboBox->setComponentID(paramID);
    juce::StringArray filterChoices = {"Lowpass", "a -> o", "a -> i", "o -> i", "a -> u", "o -> u"};
    filterTypeData.comboBox->addItemList(filterChoices, 1);

    filterTypeData.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, paramID, *filterTypeData.comboBox);

    filterTypeData.label->attachToComponent(filterTypeData.comboBox.get(), false);
    addAndMakeVisible(filterTypeData.label.get());
    addAndMakeVisible(filterTypeData.comboBox.get());

    filterTypeBoxes.push_back(std::move(filterTypeData));
}


void GloubiboulgaAudioProcessorEditor::createKnob(const juce::String& paramID, const juce::String& labelText, juce::Colour c)
{
    auto slider = std::make_unique<fxme::FxmeSlider>(audioProcessor.apvts, paramID, labelText, c);
    slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider->setComponentID(paramID);
    slider->setColour (juce::Slider::trackColourId, c.darker());
    slider->setColour (juce::Slider::thumbColourId, c);
    slider->setColour (juce::Slider::rotarySliderOutlineColourId, c.darker (2.0f));
    addAndMakeVisible(slider.get());
    fxmeSliders.push_back(std::move(slider));
}

void GloubiboulgaAudioProcessorEditor::createVSlider(const juce::String& paramID, const juce::String& labelText, juce::Colour c)
{
    auto slider = std::make_unique<fxme::FxmeSlider>(audioProcessor.apvts, paramID, labelText, c);
    slider->setSliderStyle(juce::Slider::LinearBarVertical);
    slider->setComponentID(paramID);
    slider->setColour (juce::Slider::trackColourId, c.darker());
    slider->setColour (juce::Slider::thumbColourId, c);
    slider->setColour (juce::Slider::rotarySliderOutlineColourId, c.darker (2.0f));
    addAndMakeVisible(slider.get());
    fxmeSliders.push_back(std::move(slider));
}

fxme::FxmeSlider* GloubiboulgaAudioProcessorEditor::getSlider(const juce::String& paramID)
{
    for (auto& slider : fxmeSliders)
    {
        if (slider->getComponentID() == paramID)
            return slider.get();
    }
    return nullptr;
}

//==============================================================================
void GloubiboulgaAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto diagonale = (getLocalBounds().getTopLeft() - getLocalBounds().getBottomRight()).toFloat();
    auto length = diagonale.getDistanceFromOrigin();
    auto perpendicular = diagonale.rotatedAboutOrigin (juce::degreesToRadians (270.0f)) / length;
    auto height = float (getWidth() * getHeight()) / length;
    auto bluegreengrey = juce::Colour::fromFloatRGBA (0.15f, 0.15f, 0.25f, 1.0f);
    juce::ColourGradient grad (bluegreengrey.darker().darker().darker(), perpendicular * height,
                           bluegreengrey, perpendicular * -height, false);
    g.setGradientFill(grad);
    g.fillAll();
}

void GloubiboulgaAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Place visualizer at the top
    // auto visualizerArea = bounds.removeFromTop(150);
    // visualizer.setBounds(visualizerArea.reduced(10));

    // We need to keep the FlexBoxes alive during layout calculation
    std::vector<juce::FlexBox> rows;
    rows.reserve(9);

    // Helper to create a row of controls
    auto addRow = [&](const std::vector<juce::String>& paramIDs) {
        juce::FlexBox row;
        row.justifyContent = juce::FlexBox::JustifyContent::center;
        
        for (const auto& id : paramIDs) {
            if (auto* s = getSlider(id)) {
                row.items.add(juce::FlexItem(*s).withFlex(1.f));
            }
            else if (id == "FilterType") {
                // Find the filter combo box
                for (auto& box : filterTypeBoxes) {
                    if (box.comboBox->getComponentID() == id) {
                        row.items.add(juce::FlexItem(*box.comboBox).withWidth(120.0f).withHeight(25.0f).withMargin(juce::FlexItem::Margin(27.0f, 5.0f, 5.0f, 5.0f)));
                        break;
                    }
                }
            }
        }
        rows.push_back(row);
    };

    // Organize by logical groups (rows)
    addRow({"RandomSeed"});
    addRow({"CycleDuration", "GlitchDurMean", "GlitchDurStdDev", "GlitchProb"});
    addRow({"BufferStartPosMean", "BufferStartPosStdDev", "BufferStartOffsetMean", "BufferStartOffsetStdDev"});
    addRow({"CutProb32nd", "CutProb16th", "CutProb8th", "CutProb4th"});
    addRow({"CutProbNormal", "CutProbDotted", "CutProbTrippled"});
    addRow({"CutLengthMean", "CutLengthStdDev"});
    addRow({"FilterType", "FilterFreqMean", "FilterFreqStdDev", "FilterResMean", "FilterResStdDev"});
    addRow({"FilterEnvDepth", "FilterAttack", "FilterDecay"});
    addRow({"LevelMean", "LevelStdDev"});
    addRow({"AttackTime", "DecayTime"});
    addRow({"OutputMix", "OutputLevel"});
    

    juce::FlexBox mainLayout, patternBox, seedBox, glitchBox, bufferBox,
                    timingBox, metricBox, cutBox, filterBox, filterEnvBox,
                    levelBox, envBox, outputBox, lineBox1, lineBox2, lineBox3, lineBox4, scopeBox;
    mainLayout.flexDirection = juce::FlexBox::Direction::column;
    patternBox.flexDirection = juce::FlexBox::Direction::column;
    lineBox1.flexDirection = juce::FlexBox::Direction::row;
    lineBox2.flexDirection = juce::FlexBox::Direction::row;
    lineBox3.flexDirection = juce::FlexBox::Direction::row;
    lineBox4.flexDirection = juce::FlexBox::Direction::row;
    seedBox.flexDirection = juce::FlexBox::Direction::column;
    glitchBox.flexDirection = juce::FlexBox::Direction::column;
    bufferBox.flexDirection = juce::FlexBox::Direction::column;
    timingBox.flexDirection = juce::FlexBox::Direction::column;
    metricBox.flexDirection = juce::FlexBox::Direction::column;
    cutBox.flexDirection = juce::FlexBox::Direction::column;
    levelBox.flexDirection = juce::FlexBox::Direction::column;
    envBox.flexDirection = juce::FlexBox::Direction::column;
    filterBox.flexDirection = juce::FlexBox::Direction::column;
    filterEnvBox.flexDirection = juce::FlexBox::Direction::column;
    outputBox.flexDirection = juce::FlexBox::Direction::column;

    patternBox.items.add(juce::FlexItem(patternBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(.0f, 2.0f, 5.0f, 2.0f)));
    patternBox.items.add(juce::FlexItem(visualizer).withFlex(1.f));

    seedBox.items.add(juce::FlexItem(seedBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 2.0f)));
    seedBox.items.add(juce::FlexItem(rows[0]).withFlex(1.0f));

    glitchBox.items.add(juce::FlexItem(glitchBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 2.0f)));
    glitchBox.items.add(juce::FlexItem(rows[1]).withFlex(1.0f));

    bufferBox.items.add(juce::FlexItem(bufferBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 2.0f)));
    bufferBox.items.add(juce::FlexItem(rows[2]).withFlex(1.0f));

    lineBox1.items.add(juce::FlexItem(seedBox).withFlex(1.0f));
    lineBox1.items.add(juce::FlexItem(glitchBox).withFlex(4.0f));
    lineBox1.items.add(juce::FlexItem(bufferBox).withFlex(4.0f));

    timingBox.items.add(juce::FlexItem(timingBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 2.0f)));
    timingBox.items.add(juce::FlexItem(rows[3]).withFlex(1.0f));

    metricBox.items.add(juce::FlexItem(metricBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 2.0f)));
    metricBox.items.add(juce::FlexItem(rows[4]).withFlex(1.0f));

    cutBox.items.add(juce::FlexItem(cutBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 2.0f)));
    cutBox.items.add(juce::FlexItem(rows[5]).withFlex(1.0f));

    lineBox2.items.add(juce::FlexItem(timingBox).withFlex(4.0f));
    lineBox2.items.add(juce::FlexItem(metricBox).withFlex(3.0f));
    lineBox2.items.add(juce::FlexItem(cutBox).withFlex(2.0f));

    filterBox.items.add(juce::FlexItem(filterBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 2.0f)));
    filterBox.items.add(juce::FlexItem(rows[6]).withFlex(1.0f));

    filterEnvBox.items.add(juce::FlexItem(filterEnvBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 2.0f)));
    filterEnvBox.items.add(juce::FlexItem(rows[7]).withFlex(1.0f));

    lineBox3.items.add(juce::FlexItem(filterBox).withFlex(5.5f));
    lineBox3.items.add(juce::FlexItem(filterEnvBox).withFlex(3.0f));

    levelBox.items.add(juce::FlexItem(levelBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 2.0f)));
    levelBox.items.add(juce::FlexItem(rows[8]).withFlex(1.0f));

    envBox.items.add(juce::FlexItem(envBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 2.0f)));
    envBox.items.add(juce::FlexItem(rows[9]).withFlex(1.0f));

    outputBox.items.add(juce::FlexItem(outputBar).withFlex(0.15f).withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 2.0f)));
    outputBox.items.add(juce::FlexItem(rows[10]).withFlex(1.0f));

    scopeBox.flexDirection = juce::FlexBox::Direction::column;
    scopeBox.items.add(juce::FlexItem(scopeLeft).withFlex(1.0f));
    scopeBox.items.add(juce::FlexItem(scopeRight).withFlex(1.0f));

    lineBox4.items.add(juce::FlexItem(levelBox).withFlex(2.0f));
    lineBox4.items.add(juce::FlexItem(envBox).withFlex(2.0f));
    lineBox4.items.add(juce::FlexItem(outputBox).withFlex(2.0f));
    lineBox4.items.add(juce::FlexItem(scopeBox).withFlex(3.0f));           

    mainLayout.items.add(juce::FlexItem(patternBox).withFlex(1.f).withMargin(juce::FlexItem::Margin(0.f, 0.f, 5.0f, 0.f)));
    mainLayout.items.add(juce::FlexItem(lineBox1).withFlex(1.f));
    mainLayout.items.add(juce::FlexItem(lineBox2).withFlex(1.f));
    mainLayout.items.add(juce::FlexItem(lineBox3).withFlex(1.f));
    mainLayout.items.add(juce::FlexItem(lineBox4).withFlex(1.f));
    

    // for (auto& row : rows)
    //     mainLayout.items.add(juce::FlexItem(row).withFlex(1.0f));

    mainLayout.performLayout(bounds.reduced(10));
}
