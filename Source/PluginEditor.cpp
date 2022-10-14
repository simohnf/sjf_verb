/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define potSize 80
#define textHeight 20
#define indent 10
#define alph 0.5f

//==============================================================================
Sjf_verbAudioProcessorEditor::Sjf_verbAudioProcessorEditor (Sjf_verbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState (vts)
{
    
    setLookAndFeel( &otherLookandFeel );
    
    
//    drySliderAttachment, wetSliderAttachment, sizeSliderAttachment, modulationSliderAttachment, decaySliderAttachment;
    
    drySliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dry", drySlider));
    addAndMakeVisible( &drySlider );
    drySlider.setSliderStyle (juce::Slider::Rotary);
    drySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    drySlider.setNumDecimalPlacesToDisplay(3);
    drySlider.setTextValueSuffix ("%");
    
    wetSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "wet", wetSlider));
    addAndMakeVisible( &wetSlider );
    wetSlider.setSliderStyle (juce::Slider::Rotary);
    wetSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    wetSlider.setNumDecimalPlacesToDisplay(3);
    wetSlider.setTextValueSuffix ("%");
    
    sizeSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "size", sizeSlider));
    addAndMakeVisible( &sizeSlider );
    sizeSlider.setSliderStyle (juce::Slider::Rotary);
    sizeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    sizeSlider.setNumDecimalPlacesToDisplay(3);
    sizeSlider.setTextValueSuffix ("%");
    
    modulationSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "modulation", modulationSlider));
    addAndMakeVisible( &modulationSlider );
    modulationSlider.setSliderStyle (juce::Slider::Rotary);
    modulationSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    modulationSlider.setNumDecimalPlacesToDisplay(3);
    modulationSlider.setTextValueSuffix ("%");
    
    decaySliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "decay", decaySlider));
    addAndMakeVisible( &decaySlider );
    decaySlider.setSliderStyle (juce::Slider::Rotary);
    decaySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    decaySlider.setNumDecimalPlacesToDisplay(3);
    decaySlider.setTextValueSuffix ("%");
    
    setSize (400, 300);
}

Sjf_verbAudioProcessorEditor::~Sjf_verbAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void Sjf_verbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Sjf_verbAudioProcessorEditor::resized()
{
    sizeSlider.setBounds( indent, textHeight, potSize, potSize );
    modulationSlider.setBounds( indent, sizeSlider.getBottom(), potSize, potSize );
    decaySlider.setBounds( indent, modulationSlider.getBottom(), potSize, potSize );
}
