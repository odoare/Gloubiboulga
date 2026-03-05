/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RandomNormal.h"
#include "BiquadFilter.h"
#include "FormantFilter.h"
#include <FxmeJuceTools/FxmeJuceTools.h>
#include <FxmeJuceTools/Dsp/CircularFifo.h>


//==============================================================================
// Structures for glitch cycle generation
struct GlitchStep
{
    int stepDurationNumerator;  // Duration in terms of timeSigNumerator units
    bool isGlitching;           // Whether this step applies glitching
};

struct GlitchInstruction
{
    int stepIndex;                    // Which step in the cycle
    int bufferStartPos;               // Starting position in buffer (in timeSigDenominator units)
    int bufferStartOffset;            // Offset to buffer start (in 1/4 timeSigDenominator units)
    int repeatDurationDenominator;    // Denominator for repeat duration (e.g., 32 for 1/32nd)
    bool isDotted;                    // Whether the repeat duration is dotted

    bool isTripled;                   // Whether the repeat duration is triplet
    float cutLength;                  // Portion of the repeat that is actually played [0,1]
    float level;                      // Volume level of the grain
    float filterFreq;                 // Base cutoff frequency
    float filterRes;                  // Resonance (Q)
    float filterEnvDepth;             // Envelope modulation depth (Hz)
    float filterAttack;               // Filter envelope attack time (s)
    float filterDecay;                // Filter envelope decay time (s)
};

struct GlitchCycleData
{
    std::vector<GlitchStep> steps;
    std::vector<GlitchInstruction> instructions;
    int totalCycleSamples;
};

//==============================================================================
/**
*/
class GloubiboulgaAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    GloubiboulgaAudioProcessor();
    ~GloubiboulgaAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;

    // Methods for visualization
    GlitchCycleData getGlitchCycleData();
    float getNormalizedCyclePosition() const;
    int getCycleVersion() const { return cycleVersion.load(); }

    fxme::CircularFifo scopeFifoLeft { 65536 };
    fxme::CircularFifo scopeFifoRight { 65536 };

private:
    //==============================================================================
    // Parameters
    std::atomic<float>* randomSeedParam = nullptr;
    std::atomic<float>* glitchDurMeanParam = nullptr;
    std::atomic<float>* glitchDurStdDevParam = nullptr;
    std::atomic<float>* glitchProbParam = nullptr;
    std::atomic<float>* bufferStartPosMeanParam = nullptr;
    std::atomic<float>* bufferStartPosStdDevParam = nullptr;
    std::atomic<float>* bufferStartOffsetMeanParam = nullptr;
    std::atomic<float>* bufferStartOffsetStdDevParam = nullptr;
    std::atomic<float>* cutProb32ndParam = nullptr;
    std::atomic<float>* cutProb16thParam = nullptr;
    std::atomic<float>* cutProb8thParam = nullptr;
    std::atomic<float>* cutProb4thParam = nullptr;
    std::atomic<float>* cutProbNormalParam = nullptr;
    std::atomic<float>* cutProbDottedParam = nullptr;
    std::atomic<float>* cutProbTrippledParam = nullptr;
    std::atomic<float>* cutLengthMeanParam = nullptr;
    std::atomic<float>* cutLengthStdDevParam = nullptr;
    std::atomic<float>* levelMeanParam = nullptr;
    std::atomic<float>* levelStdDevParam = nullptr;
    std::atomic<float>* attackTimeParam = nullptr;
    std::atomic<float>* decayTimeParam = nullptr;
    std::atomic<float>* filterTypeParam = nullptr;
    std::atomic<float>* filterFreqMeanParam = nullptr;
    std::atomic<float>* filterFreqStdDevParam = nullptr;
    std::atomic<float>* filterResMeanParam = nullptr;
    std::atomic<float>* filterResStdDevParam = nullptr;
    std::atomic<float>* filterEnvDepthParam = nullptr;
    std::atomic<float>* filterAttackParam = nullptr;
    std::atomic<float>* filterDecayParam = nullptr;
    std::atomic<float>* cycleDurationParam = nullptr;

    // Last known parameter values for cycle regeneration
    int lastRandomSeed = -1;
    int lastGlitchDurMean = -1;
    int lastGlitchDurStdDev = -1;
    float lastGlitchProb = -1.0f;
    int lastBufferStartPosMean = -1;
    int lastBufferStartPosStdDev = -1;
    int lastBufferStartOffsetMean = -1;
    int lastBufferStartOffsetStdDev = -1;
    float lastCutProb32nd = -1.0f;
    float lastCutProb16th = -1.0f;
    float lastCutProb8th = -1.0f;
    float lastCutProb4th = -1.0f;
    float lastCutProbNormal = -1.0f;
    float lastCutProbDotted = -1.0f;
    float lastCutProbTrippled = -1.0f;
    float lastCutLengthMean = -1.0f;
    float lastCutLengthStdDev = -1.0f;
    float lastLevelMean = -1.0f;
    float lastLevelStdDev = -1.0f;
    int lastFilterType = 0;
    float lastFilterFreqMean = -1.0f;
    float lastFilterFreqStdDev = -1.0f;
    float lastFilterResMean = -1.0f;
    float lastFilterResStdDev = -1.0f;
    float lastFilterEnvDepth = -1.0f;
    float lastFilterAttack = -1.0f;
    float lastFilterDecay = -1.0f;
    int lastCycleDuration = -1;
    // Audio processing state
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    bool wasPlaying = false;
    std::atomic<float> currentNormalizedCyclePos { 0.0f };
    
    // Circular buffer for storing the last measure
    juce::AudioBuffer<float> measureBuffer;
    int measureBufferWritePos = 0;
    
    // Glitch cycle state
    std::vector<GlitchStep> glitchSteps;
    std::vector<GlitchInstruction> glitchInstructions;
    int currentStepIndex = 0;
    double currentStepSamplePos = 0.0;
    juce::CriticalSection cycleLock;
    std::atomic<int> cycleVersion { 0 };
    
    // Glitch execution state
    // These might become redundant with the new PPQ sync logic, but kept for now.
    int currentInstructionIndex = 0;
    int lastStepIndex = -1; // To detect step changes and reset filters
    std::vector<BiquadFilter> filters;
    double currentInstructionSamplePos = 0.0;
    std::vector<FormantFilter> formantFilters;
    int totalCycleSamples = 0;
    double currentCycleSamplePos = 0.0;
    
    // Synchronisation with transport using PPQ
    double cycleStartPPQ = 0.0;      // position (in quarter notes) where current cycle began
    double cycleLengthPPQ = 0.0;     // length of cycle in PPQ units (quarter notes)
    double ppqPositionOfLastCycleStart = 0.0; // Absolute PPQ position where the current cycle instance began
    // Random generator
    RandomNormalGenerator randomGen;

    // Helper functions
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void generateGlitchCycle();
    int getMeasureSizeInSamples();
    void updateGlitchCycleIfNeeded();
    float calculateGlitchSample(int channel, double samplePosInCycle, double bpm, int timeSigNumerator, int timeSigDenominator, const GlitchInstruction* activeInstruction, BiquadFilter& filter, int writePos, double currentMeasurePPQ);
    float calculateFormantFilteredSample(float input, double sampleRate, const GlitchInstruction* activeInstruction, FormantFilter& filter, float envelope);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GloubiboulgaAudioProcessor)
};
