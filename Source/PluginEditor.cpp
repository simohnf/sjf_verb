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
    
    
//    mixSliderAttachment, wetSliderAttachment, sizeSliderAttachment, modulationSliderAttachment, decaySliderAttachment;
    
    mixSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "mix", mixSlider));
    addAndMakeVisible( &mixSlider );
    mixSlider.setSliderStyle (juce::Slider::Rotary);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    mixSlider.setNumDecimalPlacesToDisplay(3);
    mixSlider.setTextValueSuffix ("%");
    
    
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
    
    lrCutOffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "lrCutOff", lrCutOffSlider));
    addAndMakeVisible( &lrCutOffSlider );
    lrCutOffSlider.setSliderStyle (juce::Slider::Rotary);
    lrCutOffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    lrCutOffSlider.setNumDecimalPlacesToDisplay(3);
    
    erCutOffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "erCutOff", erCutOffSlider));
    addAndMakeVisible( &erCutOffSlider );
    erCutOffSlider.setSliderStyle (juce::Slider::Rotary);
    erCutOffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    erCutOffSlider.setNumDecimalPlacesToDisplay(3);
    
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
    mixSlider.setBounds( sizeSlider.getRight(), sizeSlider.getY(), potSize, potSize );
    lrCutOffSlider.setBounds( mixSlider.getX(), mixSlider.getBottom(), potSize, potSize );
    erCutOffSlider.setBounds( lrCutOffSlider.getX(), lrCutOffSlider.getBottom(), potSize, potSize );
}
