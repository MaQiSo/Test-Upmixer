/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class MqsupmixerAudioProcessorEditor  : public AudioProcessorEditor, 
                                        private Slider::Listener
{
public:
    MqsupmixerAudioProcessorEditor (MqsupmixerAudioProcessor&);
    ~MqsupmixerAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void sliderValueChanged(Slider*) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MqsupmixerAudioProcessor& processor;

    ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> ambGApvtsSliderAttachment;
    ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> mainGApvtsSliderAttachment;
    ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> decorApvtsSliderAttachment;
    ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> frontApvtsSliderAttachment;
    ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> lfeApvtsSliderAttachment;

    Slider ambGainSlider;
    Slider mainGainSlider;
    Slider lfeSlider;
    Slider decorSlider;
    Slider frontSlider;

    Label ambGainLabel;
    Label mainGainLabel;
    Label lfeLabel;
    Label decorLabel;
    Label frontLabel;

    Colour bgColour;
    Font signature;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MqsupmixerAudioProcessorEditor)
};
