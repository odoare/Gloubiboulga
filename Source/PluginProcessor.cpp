/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GloubiboulgaAudioProcessor::GloubiboulgaAudioProcessor()
    : apvts(*this, nullptr, "Parameters", createParameterLayout())
#ifndef JucePlugin_PreferredChannelConfigurations
     , AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    randomSeedParam = apvts.getRawParameterValue("RandomSeed");
    glitchDurMeanParam = apvts.getRawParameterValue("GlitchDurMean");
    glitchDurStdDevParam = apvts.getRawParameterValue("GlitchDurStdDev");
    glitchProbParam = apvts.getRawParameterValue("GlitchProb");
    bufferStartPosMeanParam = apvts.getRawParameterValue("BufferStartPosMean");
    bufferStartPosStdDevParam = apvts.getRawParameterValue("BufferStartPosStdDev");
    bufferStartOffsetMeanParam = apvts.getRawParameterValue("BufferStartOffsetMean");
    bufferStartOffsetStdDevParam = apvts.getRawParameterValue("BufferStartOffsetStdDev");
    cutProb32ndParam = apvts.getRawParameterValue("CutProb32nd");
    cutProb16thParam = apvts.getRawParameterValue("CutProb16th");
    cutProb8thParam = apvts.getRawParameterValue("CutProb8th");
    cutProb4thParam = apvts.getRawParameterValue("CutProb4th");
    cutProbNormalParam = apvts.getRawParameterValue("CutProbNormal");
    cutProbDottedParam = apvts.getRawParameterValue("CutProbDotted");
    cutProbTrippledParam = apvts.getRawParameterValue("CutProbTrippled");
    cutLengthMeanParam = apvts.getRawParameterValue("CutLengthMean");
    cutLengthStdDevParam = apvts.getRawParameterValue("CutLengthStdDev");
    levelMeanParam = apvts.getRawParameterValue("LevelMean");
    levelStdDevParam = apvts.getRawParameterValue("LevelStdDev");
    attackTimeParam = apvts.getRawParameterValue("AttackTime");
    decayTimeParam = apvts.getRawParameterValue("DecayTime");
    filterTypeParam = apvts.getRawParameterValue("FilterType");
    filterFreqMeanParam = apvts.getRawParameterValue("FilterFreqMean");
    filterFreqStdDevParam = apvts.getRawParameterValue("FilterFreqStdDev");
    filterResMeanParam = apvts.getRawParameterValue("FilterResMean");
    filterResStdDevParam = apvts.getRawParameterValue("FilterResStdDev");
    filterEnvDepthParam = apvts.getRawParameterValue("FilterEnvDepth");
    filterAttackParam = apvts.getRawParameterValue("FilterAttack");
    filterDecayParam = apvts.getRawParameterValue("FilterDecay");
    cycleDurationParam = apvts.getRawParameterValue("CycleDuration");
}

GloubiboulgaAudioProcessor::~GloubiboulgaAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout GloubiboulgaAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "RandomSeed", "Random Seed", 0, 1000000, 0));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "GlitchDurMean", "Glitch Duration Mean", 1, 4, 1));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "GlitchDurStdDev", "Glitch Duration Std Dev", 0, 4, 1));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "GlitchProb", "Glitch Probability", 0.0f, 1.0f, 0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "BufferStartPosMean", "Buffer Start Pos Mean", 0, 15, 4));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "BufferStartPosStdDev", "Buffer Start Pos Std Dev", 0, 15, 1));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "BufferStartOffsetMean", "Buffer Start Offset Mean", 0, 3, 1));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "BufferStartOffsetStdDev", "Buffer Start Offset Std Dev", 0, 3, 0));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "CutProb32nd", "Cut Prob 1/32nd", 0.0f, 1.0f, 0.1f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "CutProb16th", "Cut Prob 1/16th", 0.0f, 1.0f, 0.25f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "CutProb8th", "Cut Prob 1/8th", 0.0f, 1.0f, 0.4f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "CutProb4th", "Cut Prob 1/4th", 0.0f, 1.0f, 0.25f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "CutProbNormal", "Cut Prob Normal", 0.0f, 1.0f, 0.6f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "CutProbDotted", "Cut Prob Dotted", 0.0f, 1.0f, 0.2f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "CutProbTrippled", "Cut Prob Triplet", 0.0f, 1.0f, 0.2f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "CutLengthMean", "Cut Length Mean", 0.0f, 1.0f, 0.8f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "CutLengthStdDev", "Cut Length Std Dev", 0.0f, 1.0f, 0.1f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "LevelMean", "Level Mean", 0.0f, 1.0f, 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "LevelStdDev", "Level Std Dev", 0.0f, 1.0f, 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "AttackTime", "Attack Time (s)", juce::NormalisableRange<float>(0.0f,0.1f,0.001f,1.f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DecayTime", "Decay Time (s)", juce::NormalisableRange<float>(0.0f,0.1f,0.001f,1.f), 0.0f));

    juce::StringArray filterChoices = {"Off", "Lowpass", "a -> o", "a -> i", "o -> i", "a -> u", "o -> u"};
    params.push_back(std::make_unique<juce::AudioParameterChoice>("FilterType", "Filter Type", filterChoices, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FilterFreqMean", "Filter Freq Mean", 20.0f, 20000.0f, 20000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FilterFreqStdDev", "Filter Freq Std Dev", 0.0f, 5000.0f, 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FilterResMean", "Filter Res Mean", 0.1f, 10.0f, 0.707f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FilterResStdDev", "Filter Res Std Dev", 0.0f, 2.0f, 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FilterEnvDepth", "Filter Env Depth", 0.0f, 10000.0f, 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FilterAttack", "Filter Attack (s)", juce::NormalisableRange<float>(0.0f,0.2f,0.001f,1.f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FilterDecay", "Filter Decay (s)", juce::NormalisableRange<float>(0.0f,0.2f,0.001f,1.f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "CycleDuration", "Cycle Duration", 1, 8, 2));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "OutputMix", "Output Mix", 0.f, 100.f, 100.f));

        params.push_back(std::make_unique<juce::AudioParameterInt>(
        "OutputLevel", "Output Level", -60.f, 6.f, 0.f));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String GloubiboulgaAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GloubiboulgaAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GloubiboulgaAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GloubiboulgaAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GloubiboulgaAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GloubiboulgaAudioProcessor::getNumPrograms()
{
    return 1;
}

int GloubiboulgaAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GloubiboulgaAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GloubiboulgaAudioProcessor::getProgramName (int index)
{
    return {};
}

void GloubiboulgaAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
int GloubiboulgaAudioProcessor::getMeasureSizeInSamples()
{
    auto* ph = getPlayHead();
    if (!ph)
        return static_cast<int>(sampleRate);

    auto posInfo = ph->getPosition();
    if (!posInfo.hasValue())
        return static_cast<int>(sampleRate);
    
    double bpm = posInfo->getBpm().orFallback(120.0);
    int timeSigNumerator = posInfo->getTimeSignature().hasValue() ? 
        posInfo->getTimeSignature()->numerator : 4;
    
    // Measure duration in seconds: (numerator / denominator) * (60 / bpm)
    // For simplicity, we assume denominator = 4 (quarter note)
    double measureDurationSeconds = (static_cast<double>(timeSigNumerator) / 4.0) * (60.0 / bpm);
    return static_cast<int>(std::ceil(measureDurationSeconds * sampleRate));
}

void GloubiboulgaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    if (sampleRate <= 0 || samplesPerBlock <= 0)
        return;

    this->sampleRate = sampleRate;
    this->samplesPerBlock = samplesPerBlock;
    
    int measureSizeInSamples = getMeasureSizeInSamples();
    measureBuffer.setSize(getTotalNumInputChannels(), measureSizeInSamples);
    measureBuffer.clear();
    measureBufferWritePos = 0;
    filters.resize(getTotalNumInputChannels());

    formantFilters.resize(getTotalNumInputChannels());
    for (auto& f : formantFilters) f.prepare(2); // Number of formants
    
    // Initialize glitch execution state
    currentInstructionIndex = 0;
    currentInstructionSamplePos = 0.0;
    currentCycleSamplePos = 0.0;
    
    // Generate initial glitch cycle
    generateGlitchCycle();
}

void GloubiboulgaAudioProcessor::releaseResources()
{
    measureBuffer.clear();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GloubiboulgaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GloubiboulgaAudioProcessor::generateGlitchCycle()
{
    std::vector<GlitchStep> newSteps;
    std::vector<GlitchInstruction> newInstructions;
    currentStepIndex = 0;
    currentStepSamplePos = 0.0;
    currentInstructionIndex = 0;
    currentInstructionSamplePos = 0.0;
    currentCycleSamplePos = 0.0;
    
    int seed = static_cast<int>(randomSeedParam->load());
    randomGen.setSeed(seed);
    lastRandomSeed = seed;
    
    auto* ph = getPlayHead();
    if (!ph)
        return;
    auto posOpt = ph->getPosition();
    if (!posOpt.hasValue())
        return;
    auto posInfo = *posOpt;
    int timeSigNumerator = posInfo.getTimeSignature().hasValue() ? 
        posInfo.getTimeSignature()->numerator : 4;
    int timeSigDenominator = posInfo.getTimeSignature().hasValue() ? 
        posInfo.getTimeSignature()->denominator : 4;
    double bpm = posInfo.getBpm().orFallback(120.0);
    double currentPPQ = posInfo.getPpqPosition().orFallback(0.0); // Current DAW playhead PPQ
    
    // Capture current parameter values
    lastRandomSeed = seed;
    lastGlitchDurMean = static_cast<int>(glitchDurMeanParam->load());
    lastGlitchDurStdDev = static_cast<int>(glitchDurStdDevParam->load());
    lastGlitchProb = glitchProbParam->load();
    lastBufferStartPosMean = static_cast<int>(bufferStartPosMeanParam->load());
    lastBufferStartPosStdDev = static_cast<int>(bufferStartPosStdDevParam->load());
    lastBufferStartOffsetMean = static_cast<int>(bufferStartOffsetMeanParam->load());
    lastBufferStartOffsetStdDev = static_cast<int>(bufferStartOffsetStdDevParam->load());
    lastCutProb32nd = cutProb32ndParam->load();
    lastCutProb16th = cutProb16thParam->load();
    lastCutProb8th = cutProb8thParam->load();
    lastCutProb4th = cutProb4thParam->load();
    lastCutProbNormal = cutProbNormalParam->load();
    lastCutProbDotted = cutProbDottedParam->load();
    lastCutProbTrippled = cutProbTrippledParam->load();
    lastCutLengthMean = cutLengthMeanParam->load();
    lastCutLengthStdDev = cutLengthStdDevParam->load();
    lastLevelMean = levelMeanParam->load();
    lastLevelStdDev = levelStdDevParam->load();
    lastFilterType = static_cast<int>(filterTypeParam->load());
    lastFilterFreqMean = filterFreqMeanParam->load();
    lastFilterFreqStdDev = filterFreqStdDevParam->load();
    lastFilterResMean = filterResMeanParam->load();
    lastFilterResStdDev = filterResStdDevParam->load();
    lastFilterEnvDepth = filterEnvDepthParam->load();
    lastFilterAttack = filterAttackParam->load();
    lastFilterDecay = filterDecayParam->load();
    lastCycleDuration = static_cast<int>(cycleDurationParam->load());

    int cycleDuration = lastCycleDuration;
    int totalStepDuration = 0;
    int stepIdx = 0;
    
    // Calculate cycleLengthPPQ (total quarter notes for the cycle duration)
    cycleLengthPPQ = cycleDuration * timeSigNumerator * (4.0 / timeSigDenominator); // This is correct for PPQ
    
    // Align ppqPositionOfLastCycleStart to the nearest cycleLengthPPQ boundary
    ppqPositionOfLastCycleStart = currentPPQ - fmod(currentPPQ, cycleLengthPPQ);
    if (ppqPositionOfLastCycleStart < 0) ppqPositionOfLastCycleStart += cycleLengthPPQ; // Ensure positive

    // Generate steps for the cycle duration
    while (totalStepDuration < cycleDuration * timeSigNumerator)
    {
        int glitchDurMean = static_cast<int>(glitchDurMeanParam->load());
        int glitchDurStdDev = static_cast<int>(glitchDurStdDevParam->load());
        
        // Generate a random duration with Gaussian distribution
        double stepDur = randomGen.normal(lastGlitchDurMean, lastGlitchDurStdDev);
        int stepDurationInt = std::max(1, std::min(timeSigNumerator, static_cast<int>(std::round(stepDur))));
        
        // Adjust if it exceeds remaining duration
        if (totalStepDuration + stepDurationInt > cycleDuration * timeSigNumerator)
            stepDurationInt = cycleDuration * timeSigNumerator - totalStepDuration;
        
        // Determine if glitching applies
        float glitchProb = lastGlitchProb;
        bool isGlitching = randomGen.uniform() < glitchProb;
        
        GlitchStep step;
        step.stepDurationNumerator = stepDurationInt;
        step.isGlitching = isGlitching;
        newSteps.push_back(step);
        
        // If glitching, generate instructions for this step
        if (isGlitching)
        {
            int bufferStartPosMean = static_cast<int>(bufferStartPosMeanParam->load());
            int bufferStartPosStdDev = static_cast<int>(lastBufferStartPosStdDev);
            double startPos = randomGen.normal(lastBufferStartPosMean, lastBufferStartPosStdDev);
            int startPosInt = std::max(0, std::min(timeSigNumerator - 1, static_cast<int>(std::round(startPos))));
            
            int bufferStartOffsetMean = static_cast<int>(bufferStartOffsetMeanParam->load());
            int bufferStartOffsetStdDev = static_cast<int>(lastBufferStartOffsetStdDev);
            double offset = randomGen.normal(lastBufferStartOffsetMean, lastBufferStartOffsetStdDev);
            int offsetInt = std::max(0, std::min(3, static_cast<int>(std::round(offset))));
            
            // Determine repeat duration using the cut probabilities
            float cutProb32 = lastCutProb32nd;
            float cutProb16 = lastCutProb16th;
            float cutProb8 = lastCutProb8th;
            float cutProb4 = lastCutProb4th;
            
            float totalCutProb = cutProb32 + cutProb16 + cutProb8 + cutProb4;
            float r = randomGen.uniform() * totalCutProb;
            
            int repeatDenominator = 4;  // Default to 1/4
            if (r < cutProb32)
                repeatDenominator = 32;
            else if (r < cutProb32 + cutProb16)
                repeatDenominator = 16;
            else if (r < cutProb32 + cutProb16 + cutProb8)
                repeatDenominator = 8;
            else
                repeatDenominator = 4;
            
            // Determine dotted or triplet
            float cutProbNormal = lastCutProbNormal;
            float cutProbDotted = lastCutProbDotted;
            float cutProbTriplet = lastCutProbTrippled;
            float totalTypeProb = cutProbNormal + cutProbDotted + cutProbTriplet;
            float typeR = randomGen.uniform() * totalTypeProb;
            
            bool isDotted = false;
            bool isTriplet = false;
            if (typeR < cutProbNormal)
            {
                // Normal
            }
            else if (typeR < cutProbNormal + cutProbDotted)
            {
                isDotted = true;
            }
            else
            {
                isTriplet = true;
            }
            
            // Cut length
            float cutLength = std::max(0.0f, std::min(1.0f, static_cast<float>(randomGen.normal(lastCutLengthMean, lastCutLengthStdDev))));

            // Level
            float level = static_cast<float>(randomGen.normal(lastLevelMean, lastLevelStdDev));
            level = std::max(0.0f, level);

            // Filter
            float filterFreq = std::max(20.0f, std::min(20000.0f, static_cast<float>(randomGen.normal(lastFilterFreqMean, lastFilterFreqStdDev))));
            float filterRes = std::max(0.1f, static_cast<float>(randomGen.normal(lastFilterResMean, lastFilterResStdDev)));
            float filterEnvDepth = lastFilterEnvDepth;
            
            
            GlitchInstruction instr;
            instr.stepIndex = stepIdx;
            instr.bufferStartPos = startPosInt;
            instr.bufferStartOffset = offsetInt;
            instr.repeatDurationDenominator = repeatDenominator;
            instr.isDotted = isDotted;
            instr.isTripled = isTriplet;
            instr.cutLength = cutLength;
            instr.level = level;
            instr.filterFreq = filterFreq;
            instr.filterRes = filterRes;
            instr.filterEnvDepth = filterEnvDepth;
            instr.filterAttack = lastFilterAttack;
            instr.filterDecay = lastFilterDecay;
            newInstructions.push_back(instr);
        }
        
        totalStepDuration += stepDurationInt;
        stepIdx++;
    }

    {
        const juce::ScopedLock sl(cycleLock);
        glitchSteps = std::move(newSteps);
        glitchInstructions = std::move(newInstructions);
        
        // Calculate totalCycleSamples based on the generated cycle length in PPQ
        // This is used for wrapping in calculateGlitchSample
        totalCycleSamples = static_cast<int>(cycleLengthPPQ * (60.0 / bpm) * sampleRate);
        cycleVersion++;
    }
}

void GloubiboulgaAudioProcessor::updateGlitchCycleIfNeeded()
{
    bool changed = false;

    // Check if any parameter has changed
    if (static_cast<int>(randomSeedParam->load()) != lastRandomSeed) changed = true;
    if (static_cast<int>(glitchDurMeanParam->load()) != lastGlitchDurMean) changed = true;
    if (static_cast<int>(glitchDurStdDevParam->load()) != lastGlitchDurStdDev) changed = true;
    if (glitchProbParam->load() != lastGlitchProb) changed = true;
    if (static_cast<int>(bufferStartPosMeanParam->load()) != lastBufferStartPosMean) changed = true;
    if (static_cast<int>(bufferStartPosStdDevParam->load()) != lastBufferStartPosStdDev) changed = true;
    if (static_cast<int>(bufferStartOffsetMeanParam->load()) != lastBufferStartOffsetMean) changed = true;
    if (static_cast<int>(bufferStartOffsetStdDevParam->load()) != lastBufferStartOffsetStdDev) changed = true;
    if (cutProb32ndParam->load() != lastCutProb32nd) changed = true;
    if (cutProb16thParam->load() != lastCutProb16th) changed = true;
    if (cutProb8thParam->load() != lastCutProb8th) changed = true;
    if (cutProb4thParam->load() != lastCutProb4th) changed = true;
    if (cutProbNormalParam->load() != lastCutProbNormal) changed = true;
    if (cutProbDottedParam->load() != lastCutProbDotted) changed = true;
    if (cutProbTrippledParam->load() != lastCutProbTrippled) changed = true;
    if (cutLengthMeanParam->load() != lastCutLengthMean) changed = true;
    if (cutLengthStdDevParam->load() != lastCutLengthStdDev) changed = true;
        if (static_cast<int>(filterTypeParam->load()) != lastFilterType) changed = true;
    if (levelMeanParam->load() != lastLevelMean) changed = true;
    if (levelStdDevParam->load() != lastLevelStdDev) changed = true;
    if (filterFreqMeanParam->load() != lastFilterFreqMean) changed = true;
    if (filterFreqStdDevParam->load() != lastFilterFreqStdDev) changed = true;
    if (filterResMeanParam->load() != lastFilterResMean) changed = true;
    if (filterResStdDevParam->load() != lastFilterResStdDev) changed = true;
    if (filterEnvDepthParam->load() != lastFilterEnvDepth) changed = true;
    if (filterAttackParam->load() != lastFilterAttack) changed = true;
    if (filterDecayParam->load() != lastFilterDecay) changed = true;
    if (static_cast<int>(cycleDurationParam->load()) != lastCycleDuration) changed = true;

    // Also check if time signature or BPM changed, as they affect cycle length and timing
    juce::Optional<juce::AudioPlayHead::PositionInfo> posInfo;
    if (auto* ph = getPlayHead())
        posInfo = ph->getPosition();
    int currentTimeSigNumerator = posInfo.hasValue() && posInfo->getTimeSignature().hasValue() ? posInfo->getTimeSignature()->numerator : 4;
    int currentTimeSigDenominator = posInfo.hasValue() && posInfo->getTimeSignature().hasValue() ? posInfo->getTimeSignature()->denominator : 4;
    double currentBpm = posInfo.hasValue() ? posInfo->getBpm().orFallback(120.0) : 120.0;

    // If any parameter or relevant transport info changed, regenerate the cycle
    if (changed || (cycleLengthPPQ != (lastCycleDuration * currentTimeSigNumerator * (4.0 / currentTimeSigDenominator))) || (std::abs(currentBpm - (posInfo.hasValue() ? posInfo->getBpm().orFallback(120.0) : 120.0)) > 0.001) )
    {
        generateGlitchCycle();
    }
}

float GloubiboulgaAudioProcessor::calculateFormantFilteredSample (float input, double sampleRate, const GlitchInstruction* activeInstruction, FormantFilter& filter, float envelope)
{
    // Vowel Formants (F1, F2) approximations
    // A: 800, 1200
    // I: 300, 2500
    // O: 500, 1000
    // U: 300, 800
    
    float startF1 = 0.0f, startF2 = 0.0f;
    float endF1 = 0.0f, endF2 = 0.0f;

    switch (lastFilterType) {
        case 2: // "a -> o"
            startF1 = 800.0f; startF2 = 1200.0f; // a
            endF1 = 500.0f;   endF2 = 1000.0f;   // o
            break;
        case 3: // "a -> i"
            startF1 = 800.0f; startF2 = 1200.0f; // a
            endF1 = 300.0f;   endF2 = 2500.0f;   // i
            break;
        case 4: // "o -> i"
            startF1 = 500.0f; startF2 = 1000.0f; // o
            endF1 = 300.0f;   endF2 = 2500.0f;   // i
            break;
        case 5: // "a -> u"
            startF1 = 800.0f; startF2 = 1200.0f; // a
            endF1 = 300.0f;   endF2 = 800.0f;    // u
            break;
        case 6: // "o -> u"
            startF1 = 500.0f; startF2 = 1000.0f; // o
            endF1 = 300.0f;   endF2 = 800.0f;    // u
            break;
        default:
            return input;  // No formant filtering
    }
    
    // Interpolate formants based on envelope (0.0 to 1.0)
    float currentF1 = startF1 + (endF1 - startF1) * envelope;
    float currentF2 = startF2 + (endF2 - startF2) * envelope;

    // Fixed bandwidths for simplicity
    float bw1 = 80.0f;
    float bw2 = 100.0f;

    filter.setFormant(0, currentF1, bw1, sampleRate);
    filter.setFormant(1, currentF2, bw2, sampleRate);

    float output = filter.process(input);
    return output;
}

float GloubiboulgaAudioProcessor::calculateGlitchSample(int channel, double samplePosInCycle, double bpm, int timeSigNumerator, int timeSigDenominator, const GlitchInstruction* activeInstruction, BiquadFilter& filter, int writePos, double currentMeasurePPQ)
{
    // Calculate samples per beat (quarter note)
    double samplesPerBeat = (sampleRate * 60.0) / bpm;

    // Calculate position within this step (passed from processBlock logic)
    // We need to reconstruct posInStep. 
    // Optimization: We could pass posInStep directly, but for now we'll recalculate stepStartPos.
    // Actually, to avoid re-looping, let's assume the caller handles step logic.
    // But wait, calculateGlitchSample needs posInStep.
    // Let's do a quick loop to find step start pos.
    double stepStartPos = 0.0;
    for (int i = 0; i < activeInstruction->stepIndex; ++i)
        stepStartPos += glitchSteps[i].stepDurationNumerator * samplesPerBeat;

    // Wrap position within cycle
    double wrappedPos = std::fmod(samplePosInCycle, totalCycleSamples);
    if (wrappedPos < 0.0) wrappedPos += totalCycleSamples;

    double posInStep = wrappedPos - stepStartPos;
    
    // Calculate repeat duration in samples
    // Problem 2 Fix: Scale repeatDurationDenominator correctly.
    // samplesPerBeat is a quarter note. repeatDurationDenominator refers to 1/Nth of a whole note.
    double repeatDurationBase = samplesPerBeat * (4.0 / activeInstruction->repeatDurationDenominator);
    // Apply dotted or triplet modification
    double repeatDuration = repeatDurationBase;
    if (activeInstruction->isDotted)
        repeatDuration *= 1.5;  // Dotted = 1.5x the duration
    else if (activeInstruction->isTripled)
        repeatDuration *= (2.0 / 3.0);  // Triplet = 2/3 the duration
    
    // Calculate position within current repeat
    double posInRepeat = std::fmod(posInStep, repeatDuration);
    if (posInRepeat < 0.0)
        posInRepeat += repeatDuration;
    
    // Determine if we're in the active part or silence part
    double activePortion = repeatDuration * activeInstruction->cutLength;
    bool isActive = posInRepeat < activePortion;
    
    if (!isActive)
        return 0.0f;  // Silence portion
    
    // Calculate envelop ( fade-in and fade-out)
    float attackSeconds = attackTimeParam->load();
    float decaySeconds = decayTimeParam->load();
    double attackSamples = attackSeconds * sampleRate;
    double decaySamples = decaySeconds * sampleRate;

    float envelope = 1.0f;
    
    float fadeIn = 1.0f;
    if (attackSamples > 0.5 && posInRepeat < attackSamples)
        fadeIn = static_cast<float>(posInRepeat / attackSamples);
    
    float fadeOut = 1.0f;
    if (decaySamples > 0.5 && posInRepeat > activePortion - decaySamples)
        fadeOut = static_cast<float>((activePortion - posInRepeat) / decaySamples);
    
    envelope = std::min(fadeIn, fadeOut);
    envelope = std::max(0.0f, envelope);
    
    // Calculate delay in PPQ from current position to target buffer position
    // Target is activeInstruction->bufferStartPos + offset
    double targetMeasurePPQ = activeInstruction->bufferStartPos + activeInstruction->bufferStartOffset * 0.25;
    
    // We want to read from the most recent occurrence of targetMeasurePPQ
    // relative to currentMeasurePPQ.
    double delayPPQ = currentMeasurePPQ - targetMeasurePPQ;
    if (delayPPQ < 0.0)
        delayPPQ += timeSigNumerator; // It was in the previous measure
        
    // Convert to samples
    double delaySamples = delayPPQ * samplesPerBeat;
    
    // Calculate read pointer relative to write pointer
    // We also add posInRepeat because we are playing forward from that anchor point
    double readPos = writePos - delaySamples + posInRepeat;
    
    int bufferSize = measureBuffer.getNumSamples();
    while (readPos < 0.0) readPos += bufferSize;
    while (readPos >= bufferSize) readPos -= bufferSize;
    
    int bufferReadPosInt = static_cast<int>(readPos);
    
    if (channel >= measureBuffer.getNumChannels())
        return 0.0f;
    
    // Read from buffer with simple linear interpolation
    float sample0 = measureBuffer.getSample(channel, bufferReadPosInt);
    float sample1 = measureBuffer.getSample(channel, (bufferReadPosInt + 1) % bufferSize);
    float frac = static_cast<float>(readPos - std::floor(readPos));
    float interpolated = sample0 * (1.0f - frac) + sample1 * frac;
    
    // Apply Filter
    // Calculate filter envelope
    float filterEnv = 0.0f;
    double filterAttackSamples = activeInstruction->filterAttack * sampleRate;
    double filterDecaySamples = activeInstruction->filterDecay * sampleRate;

    float fEnvIn = 1.0f;
    if (filterAttackSamples > 0.5 && posInRepeat < filterAttackSamples)
        fEnvIn = static_cast<float>(posInRepeat / filterAttackSamples);
    
    float fEnvOut = 1.0f;
    if (filterDecaySamples > 0.5 && posInRepeat > activePortion - filterDecaySamples)
        fEnvOut = static_cast<float>((activePortion - posInRepeat) / filterDecaySamples);
    
    filterEnv = juce::jmin(fEnvIn, fEnvOut);
    
    float filteredSignal = 0.0f;

    if (lastFilterType == 0) // Off
    {
        filteredSignal = interpolated;
    }
    else if (lastFilterType == 1) // Lowpass
    {
        float currentCutoff = activeInstruction->filterFreq + filterEnv * activeInstruction->filterEnvDepth;
        filter.setLowPass(sampleRate, currentCutoff, activeInstruction->filterRes);
        filteredSignal = filter.process(interpolated);
    }
    else // Formant
    {
        filteredSignal = calculateFormantFilteredSample(interpolated, sampleRate, activeInstruction, formantFilters[channel], filterEnv);
        filteredSignal *= juce::Decibels::decibelsToGain(12.0f);
    }

    return filteredSignal * envelope * activeInstruction->level;
}

void GloubiboulgaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output channels that don't have input
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    if (filters.size() != totalNumInputChannels)
        filters.resize(totalNumInputChannels);

    if (formantFilters.size() != totalNumInputChannels)
        formantFilters.resize(totalNumInputChannels);

    // Problem 3 Fix: Check if any parameter has changed and regenerate the glitch cycle
    // This also implicitly handles changes in time signature or BPM that affect cycle length.
    updateGlitchCycleIfNeeded();
    
    // Get DAW position info
    juce::AudioPlayHead::CurrentPositionInfo posInfo;
    posInfo.resetToDefault();

    if (auto* ph = getPlayHead())
        ph->getCurrentPosition(posInfo);

    int timeSigNumerator = posInfo.timeSigNumerator > 0 ? posInfo.timeSigNumerator : 4;
    int timeSigDenominator = posInfo.timeSigDenominator > 0 ? posInfo.timeSigDenominator : 4;
    double bpm = posInfo.bpm > 0 ? posInfo.bpm : 120.0;
    bool isPlaying = posInfo.isPlaying;
    double currentDAWPPQ = posInfo.ppqPosition >= 0 ? posInfo.ppqPosition : 0.0;

    // Problem 1 Fix: Re-align cycle start if transport starts or seeks
    if (isPlaying && !wasPlaying) // Transport just started
    {
        generateGlitchCycle(); // Re-align cycle start to current DAW position
    }
    // Update wasPlaying for the next block
    wasPlaying = isPlaying;
    
    // int measureSizeInSamples = getMeasureSizeInSamples(); // Removed to use buffer size directly
    
    // Process each sample
    double ppqPerSample = (bpm / 60.0) / sampleRate; // quarter-notes per sample

    float mix = .01f * apvts.getRawParameterValue("OutputMix")->load();
    float level = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("OutputLevel")->load());

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // Read input samples and write to circular buffer
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            float inSample = buffer.getSample(channel, sample);
            measureBuffer.setSample(channel, measureBufferWritePos, inSample);
        }
        
        // Advance circular buffer position
        measureBufferWritePos = (measureBufferWritePos + 1) % measureBuffer.getNumSamples();
        
        // Problem 1 Fix: Compute precise cycle sample position based on PPQ sync
        double currentSamplePPQ = currentDAWPPQ + sample * ppqPerSample;

        // Adjust ppqPositionOfLastCycleStart if the playhead has moved significantly
        // (e.g., user seeks or transport loops)
        if (cycleLengthPPQ > 0.0)
        {
            // If current sample is past the end of the current cycle instance
            while (currentSamplePPQ >= ppqPositionOfLastCycleStart + cycleLengthPPQ)
            {
                ppqPositionOfLastCycleStart += cycleLengthPPQ;
            }
            // If current sample is before the start of the current cycle instance
            while (currentSamplePPQ < ppqPositionOfLastCycleStart)
            {
                ppqPositionOfLastCycleStart -= cycleLengthPPQ;
            }
        }
        double elapsedPPQInCurrentCycle = currentSamplePPQ - ppqPositionOfLastCycleStart;
        double cyclePosSamples = (elapsedPPQInCurrentCycle * (60.0 / bpm)) * sampleRate;
        
        double currentMeasurePPQ = std::fmod(currentSamplePPQ, (double)timeSigNumerator);
        if (currentMeasurePPQ < 0.0) currentMeasurePPQ += timeSigNumerator;
        
        // Update normalized position for visualization
        if (totalCycleSamples > 0)
            currentNormalizedCyclePos.store(static_cast<float>(cyclePosSamples / totalCycleSamples));

        // Determine current step and instruction once per sample (shared across channels)
        int stepIdx = -1;
        bool isGlitching = false;
        const GlitchInstruction* activeInstruction = nullptr;

        if (totalCycleSamples > 0 && !glitchSteps.empty())
        {
            double wrappedPos = std::fmod(cyclePosSamples, totalCycleSamples);
            if (wrappedPos < 0.0) wrappedPos += totalCycleSamples;
            
            double currentPos = 0.0;
            double samplesPerBeat = (sampleRate * 60.0) / bpm;
            
            for (size_t i = 0; i < glitchSteps.size(); ++i)
            {
                double stepDuration = glitchSteps[i].stepDurationNumerator * samplesPerBeat;
                if (wrappedPos < currentPos + stepDuration)
                {
                    stepIdx = static_cast<int>(i);
                    isGlitching = glitchSteps[i].isGlitching;
                    break;
                }
                currentPos += stepDuration;
            }
        }

        // Reset filters if step changed
        if (stepIdx != lastStepIndex)
        {
            for (auto& f : filters) f.reset();
            for (auto& f : formantFilters) f.reset();
            lastStepIndex = stepIdx;
        }

        // Generate output per channel
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            float inputSample = buffer.getSample(channel, sample);
            float glitchSample = 0.0f;
            
            if (isGlitching)
            {
                // Find instruction for this step
                if (activeInstruction == nullptr || activeInstruction->stepIndex != stepIdx)
                {
                    for (const auto& instr : glitchInstructions)
                    {
                        if (instr.stepIndex == stepIdx)
                        {
                            activeInstruction = &instr;
                            break;
                        }
                    }
                }

                if (activeInstruction)
                {
                    glitchSample = calculateGlitchSample(channel, cyclePosSamples, bpm, timeSigNumerator, timeSigDenominator, activeInstruction, filters[channel], measureBufferWritePos, currentMeasurePPQ);
                }
                else
                {
                    // Fallback if no instruction found (shouldn't happen if isGlitching is true)
                    isGlitching = false; 
                }
            }
            
            float outputSample = level * ( inputSample * (1-mix) + glitchSample * mix );
            buffer.setSample(channel, sample, outputSample);
        }
    }

    if (buffer.getNumChannels() > 0)
    {
        // Only write to the scope FIFOs if there is enough space.
        // This prevents the FIFO from overflowing when the GUI is hidden and not consuming samples.
        juce::AudioBuffer<float> leftBuffer (buffer.getArrayOfWritePointers(), 1, buffer.getNumSamples());
        if (scopeFifoLeft.getFreeSpace() >= buffer.getNumSamples())
        {
            scopeFifoLeft.fillFifoWithBuffer(leftBuffer);
        }

        if (scopeFifoRight.getFreeSpace() >= buffer.getNumSamples())
        {
            if (buffer.getNumChannels() > 1)
            {
                juce::AudioBuffer<float> rightBuffer (buffer.getArrayOfWritePointers() + 1, 1, buffer.getNumSamples());
                scopeFifoRight.fillFifoWithBuffer(rightBuffer);
            }
            else
            {
                scopeFifoRight.fillFifoWithBuffer(leftBuffer);
            }
        }
    }
}

GlitchCycleData GloubiboulgaAudioProcessor::getGlitchCycleData()
{
    const juce::ScopedLock sl(cycleLock);
    GlitchCycleData data;
    data.steps = glitchSteps;
    data.instructions = glitchInstructions;
    data.totalCycleSamples = totalCycleSamples;
    return data;
}

float GloubiboulgaAudioProcessor::getNormalizedCyclePosition() const
{
    return currentNormalizedCyclePos.load();
}


//==============================================================================
bool GloubiboulgaAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* GloubiboulgaAudioProcessor::createEditor()
{
    return new GloubiboulgaAudioProcessorEditor (*this);
}

//==============================================================================
void GloubiboulgaAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.state.createXml();
    copyXmlToBinary(*state, destData);
}

void GloubiboulgaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GloubiboulgaAudioProcessor();
}
