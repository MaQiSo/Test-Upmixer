/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MqsupmixerAudioProcessorEditor::MqsupmixerAudioProcessorEditor (MqsupmixerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), bgColour(0xFF6DABE4), signature(20.0f, Font::italic)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (420, 360);

	ambGApvtsSliderAttachment = new AudioProcessorValueTreeState::SliderAttachment(processor.apvts, "ambientGain", ambGainSlider);
	mainGApvtsSliderAttachment = new AudioProcessorValueTreeState::SliderAttachment(processor.apvts, "mainGain", mainGainSlider);
	decorApvtsSliderAttachment = new AudioProcessorValueTreeState::SliderAttachment(processor.apvts, "decorrelation", decorSlider);
	frontApvtsSliderAttachment = new AudioProcessorValueTreeState::SliderAttachment(processor.apvts, "frontness", frontSlider);
	lfeApvtsSliderAttachment = new AudioProcessorValueTreeState::SliderAttachment(processor.apvts, "lfe", lfeSlider);

	ambGainSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
	ambGainSlider.setRange(-48.0f, 6.0f, 0.1f);
	ambGainSlider.setTextValueSuffix("dB");
	ambGainSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
	ambGainSlider.setSkewFactorFromMidPoint(-6.0f);
	ambGainSlider.addListener(this);
	ambGainSlider.setColour(Slider::thumbColourId,juce::Colours::white);
	addAndMakeVisible(ambGainSlider);

	ambGainLabel.setText("AMBIENT", juce::dontSendNotification);
	ambGainLabel.setJustificationType(Justification::centred);
	ambGainLabel.attachToComponent(&ambGainSlider,false);
	addAndMakeVisible(ambGainLabel);

	mainGainSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
	mainGainSlider.setRange(-48.0f, 6.0f, 0.1f);
	mainGainSlider.setTextValueSuffix("dB");
	mainGainSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
	mainGainSlider.setSkewFactorFromMidPoint(-6.0f);
	mainGainSlider.addListener(this);
	mainGainSlider.setColour(Slider::thumbColourId, juce::Colours::white);
	addAndMakeVisible(mainGainSlider);

	mainGainLabel.setText("MAIN", juce::dontSendNotification);
	mainGainLabel.setJustificationType(Justification::centred);
	mainGainLabel.attachToComponent(&mainGainSlider, false);
	addAndMakeVisible(mainGainLabel);

	lfeSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
	lfeSlider.setRange(-48.0f, 12.0f, 0.1f);
	lfeSlider.setTextValueSuffix("dB");
	lfeSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
	lfeSlider.setSkewFactorFromMidPoint(-6.0f);
	lfeSlider.addListener(this);
	lfeSlider.setColour(Slider::thumbColourId, juce::Colours::white);
	addAndMakeVisible(lfeSlider);

	lfeLabel.setText("LFE", juce::dontSendNotification);
	lfeLabel.setJustificationType(Justification::centred);
	lfeLabel.attachToComponent(&lfeSlider, false);
	addAndMakeVisible(lfeLabel);

	decorSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
	decorSlider.setRange(0.0f, 1.0f, 0.01f);
	decorSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
	decorSlider.addListener(this);
	decorSlider.setColour(Slider::thumbColourId, juce::Colours::white);
	addAndMakeVisible(decorSlider);

	decorLabel.setText("DECORRELATION", juce::dontSendNotification);
	decorLabel.attachToComponent(&decorSlider, false);
	decorLabel.setJustificationType(Justification::centred);
	decorLabel.setBounds(decorLabel.getX() - 20, decorLabel.getY(), 100, decorLabel.getHeight());
	addAndMakeVisible(decorLabel);

	frontSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
	frontSlider.setRange(0.0f, 1.0f, 0.01f);
	frontSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
	frontSlider.addListener(this);
	frontSlider.setColour(Slider::thumbColourId, juce::Colours::white);
	addAndMakeVisible(frontSlider);

	frontLabel.setText("FRONT", juce::dontSendNotification);
	frontLabel.setJustificationType(Justification::centred);
	frontLabel.attachToComponent(&frontSlider, false);
	addAndMakeVisible(frontLabel);
}

MqsupmixerAudioProcessorEditor::~MqsupmixerAudioProcessorEditor()
{
}

//==============================================================================
void MqsupmixerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(bgColour);

    g.setColour(Colours::white);
    g.setFont(signature);

    g.drawFittedText("MQS Stereo to 5.1 Upmixer", getLocalBounds(), Justification::bottomRight, 1);
}

void MqsupmixerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	int sliderNum = 5;
	int windowW = getBounds().getWidth();
	int windowH = getBounds().getHeight();
	int sideW = 30;
	int sideH = 60;
	int sliderW = 60;
	int sliderH = 242;
	int intrvlW = (int)(windowW - 2 * sideW - sliderNum * sliderW) / (sliderNum - 1);
	ambGainSlider.setBounds(sideW, sideH, sliderW, sliderH);
	mainGainSlider.setBounds(sideW + (sliderW + intrvlW), sideH, sliderW, sliderH);
	lfeSlider.setBounds(sideW + 2 * (sliderW + intrvlW), sideH, sliderW, sliderH);
	decorSlider.setBounds(sideW + 3 * (sliderW + intrvlW), sideH, sliderW, sliderH);
	frontSlider.setBounds(sideW + 4 * (sliderW + intrvlW), sideH, sliderW, sliderH);
}

void MqsupmixerAudioProcessorEditor::sliderValueChanged(Slider*)
{
	processor.updateParam();
}