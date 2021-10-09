/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using Parameter = AudioProcessorValueTreeState::Parameter;

//==============================================================================
MqsupmixerAudioProcessor::MqsupmixerAudioProcessor():apvts(*this, nullptr, "PARAMETERS", 
    { std::make_unique<Parameter>("ambientGain","Ambient Gain","Ambient Gain",NormalisableRange<float>(-48.0f,6.0f,0.1f),0.0f,nullptr,nullptr),
    std::make_unique<Parameter>("mainGain","Main Gain","Main Gain",NormalisableRange<float>(-48.0f,6.0f,0.1f),0.0f,nullptr,nullptr),
    std::make_unique<Parameter>("lfe","LFE","LFE",NormalisableRange<float>(-48.0f,12.0f,0.1f),-6.0f,nullptr,nullptr),
    std::make_unique<Parameter>("decorrelation","Decorrelation","Decorrelation",NormalisableRange<float>(0.0f,1.0f,0.01f),1.0f,nullptr,nullptr),
    std::make_unique<Parameter>("frontness","Frontness","Frontness",NormalisableRange<float>(0.0f,1.0f,0.01f),0.0f,nullptr,nullptr) }),
    lfeGain(0.5f)
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    upmixProcessor.declareHeapSize();
    upmixProcessor.setDecorrelationFilter();
}

MqsupmixerAudioProcessor::~MqsupmixerAudioProcessor()
{
}

//==============================================================================
const String MqsupmixerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MqsupmixerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MqsupmixerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MqsupmixerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MqsupmixerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MqsupmixerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MqsupmixerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MqsupmixerAudioProcessor::setCurrentProgram (int index)
{
}

const String MqsupmixerAudioProcessor::getProgramName (int index)
{
    return {};
}

void MqsupmixerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MqsupmixerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    upmixProcessor.prepare(sampleRate, samplesPerBlock, 5, 5);
    lfeFilter.setCoefficients(IIRCoefficients::makeLowPass(sampleRate, 120.0f, 0.1));
}

void MqsupmixerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MqsupmixerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MqsupmixerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto numSamples = buffer.getNumSamples();
    auto mono = new float[numSamples];
    auto left = buffer.getReadPointer(0);
    auto right = buffer.getReadPointer(1);

    // use channel no.6 takes the average from left & right channel
    for (int sample = 0; sample < numSamples; ++sample)
    {
        mono[sample] = 0.5f * (left[sample] + right[sample]);
    }

    // process for upmixProcessor
    dsp::AudioBlock<float> ab(buffer);
    dsp::ProcessContextReplacing<float> context(ab);
    upmixProcessor.process(context);

    if (totalNumOutputChannels >= 6)
    {
        // move Ls, Rs to channel no.5 and 6
        buffer.copyFrom(5, 0, buffer.getWritePointer(4), numSamples);
        buffer.copyFrom(4, 0, buffer.getWritePointer(3), numSamples);

        // lfe apply low pass filter
        lfeFilter.processSamples(mono, numSamples);
        buffer.copyFrom(3, 0, mono, numSamples);
        buffer.applyGain(3, 0, numSamples, lfeGain);
    }
}

/// <summary>
/// called when PluginEditor's sliders is changing
/// </summary>
void MqsupmixerAudioProcessor::updateParam()
{
    // update upmixProcessor's parameters
    upmixProcessor.setParameters(apvts.getRawParameterValue("ambientGain"),
        apvts.getRawParameterValue("mainGain"),
        apvts.getRawParameterValue("decorrelation"),
        apvts.getRawParameterValue("frontness"));
    float lfedB = *apvts.getRawParameterValue("lfe");
    lfeGain = Decibels::decibelsToGain(lfedB);
}

//==============================================================================
bool MqsupmixerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MqsupmixerAudioProcessor::createEditor()
{
    return new MqsupmixerAudioProcessorEditor (*this);
}

//==============================================================================
void MqsupmixerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MqsupmixerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MqsupmixerAudioProcessor();
}
