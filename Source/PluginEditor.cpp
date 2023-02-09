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
    
    
//    mixSliderAttachment, wetSliderAttachment, sizeSliderAttachment, modulationDepthSliderAttachment, decaySliderAttachment;
    
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
    
    modulationRateSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "modulationRate", modulationRateSlider));
    addAndMakeVisible( &modulationRateSlider );
    modulationRateSlider.setSliderStyle (juce::Slider::Rotary);
    modulationRateSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    modulationRateSlider.setNumDecimalPlacesToDisplay(3);
    modulationRateSlider.setTextValueSuffix ("Hz");
    
    modulationDepthSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "modulationDepth", modulationDepthSlider));
    addAndMakeVisible( &modulationDepthSlider );
    modulationDepthSlider.setSliderStyle (juce::Slider::Rotary);
    modulationDepthSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    modulationDepthSlider.setNumDecimalPlacesToDisplay(3);
    modulationDepthSlider.setTextValueSuffix ("%");
    
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
    lrCutOffSlider.setTextValueSuffix("Hz");
    
    erCutOffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "erCutOff", erCutOffSlider));
    addAndMakeVisible( &erCutOffSlider );
    erCutOffSlider.setSliderStyle (juce::Slider::Rotary);
    erCutOffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    erCutOffSlider.setNumDecimalPlacesToDisplay(3);
    erCutOffSlider.setTextValueSuffix("Hz");
    
    shimLevelSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "shimmerLevel", shimLevelSlider));
    addAndMakeVisible( &shimLevelSlider );
    shimLevelSlider.setSliderStyle (juce::Slider::Rotary);
    shimLevelSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    shimLevelSlider.setNumDecimalPlacesToDisplay(3);
    shimLevelSlider.setTextValueSuffix ("%");
    
    shimTranspositionSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "shimmerTransposition", shimTranspositionSlider));
    addAndMakeVisible( &shimTranspositionSlider );
    shimTranspositionSlider.setSliderStyle (juce::Slider::Rotary);
    shimTranspositionSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    shimTranspositionSlider.setNumDecimalPlacesToDisplay(3);
    shimTranspositionSlider.setTextValueSuffix ("st");
    
    
    interpolationTypeBoxAttachment.reset (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (valueTreeState, "interpolationType", interpolationTypeBox));
    addAndMakeVisible( &interpolationTypeBox );
    interpolationTypeBox.addItem( "linear", 1 );
    interpolationTypeBox.addItem( "cubic", 2 );
    interpolationTypeBox.addItem( "PD", 3 );
    interpolationTypeBox.addItem( "4th Order", 4 );
    interpolationTypeBox.addItem( "Godot", 5 );
    interpolationTypeBox.addItem( "Hermite", 6 );
    interpolationTypeBox.setSelectedId( 1 );
    
    
    fbControlButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "feedbackControl", fbControlButton ));
    addAndMakeVisible( &fbControlButton );
    fbControlButton.setButtonText( "fb control" );
    
    
    
    setSize (400, 400);
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
    g.drawFittedText ("sjf_verb", getLocalBounds(), juce::Justification::centred, 1);
}

void Sjf_verbAudioProcessorEditor::resized()
{
    sizeSlider.setBounds( indent, textHeight, potSize, potSize );
    modulationRateSlider.setBounds( indent, sizeSlider.getBottom(), potSize, potSize );
    modulationDepthSlider.setBounds( indent, modulationRateSlider.getBottom(), potSize, potSize );
    decaySlider.setBounds( indent, modulationDepthSlider.getBottom(), potSize, potSize );
    fbControlButton.setBounds( indent, decaySlider.getBottom(), potSize, textHeight );
    
    mixSlider.setBounds( sizeSlider.getRight(), textHeight, potSize, potSize );
    lrCutOffSlider.setBounds( mixSlider.getX(), mixSlider.getBottom(), potSize, potSize );
    erCutOffSlider.setBounds( lrCutOffSlider.getX(), lrCutOffSlider.getBottom(), potSize, potSize );
    
    shimLevelSlider.setBounds( lrCutOffSlider.getRight(), textHeight, potSize, potSize );
    shimTranspositionSlider.setBounds( shimLevelSlider.getX(), shimLevelSlider.getBottom(), potSize, potSize );
    
    interpolationTypeBox.setBounds( getWidth() - indent - potSize, getHeight() - textHeight, potSize, textHeight );
}
