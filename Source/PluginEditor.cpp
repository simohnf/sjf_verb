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
    
    mixSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "mix", mixSlider));
    addAndMakeVisible( &mixSlider );
    mixSlider.setSliderStyle (juce::Slider::Rotary);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    mixSlider.setNumDecimalPlacesToDisplay(3);
    mixSlider.setTextValueSuffix ("%");
    
    preDelaySliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "preDelay", preDelaySlider));
    addAndMakeVisible( &preDelaySlider );
    preDelaySlider.setSliderStyle (juce::Slider::Rotary);
    preDelaySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    preDelaySlider.setNumDecimalPlacesToDisplay(3);
    preDelaySlider.setTextValueSuffix ("ms");
    
    reverseButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "reverse", reverseButton ));
    addAndMakeVisible( &reverseButton );
    reverseButton.setButtonText( "reverse" );
    
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
    
    
    addAndMakeVisible( &modulationTypeButton );
    modulationTypeButton.setButtonText( "sine mod" );
    modulationTypeButton.onStateChange  = [ this ]
    {
        if ( modulationTypeButton.getToggleState() )
        {
            modulationTypeButton.setButtonText( "rand mod" );
        }
        else
        {
            modulationTypeButton.setButtonText( "sine mod" );
        }
    };
    modulationTypeButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "modulationType", modulationTypeButton ));
    
    decaySliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "decay", decaySlider));
    addAndMakeVisible( &decaySlider );
    decaySlider.setSliderStyle (juce::Slider::Rotary);
    decaySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    decaySlider.setNumDecimalPlacesToDisplay(3);
    decaySlider.setTextValueSuffix ("%");
    
    lrHPFCutOffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "lrHPFCutOff", lrHPFCutOffSlider));
    addAndMakeVisible( &lrHPFCutOffSlider );
    lrHPFCutOffSlider.setSliderStyle (juce::Slider::Rotary);
    lrHPFCutOffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    lrHPFCutOffSlider.setNumDecimalPlacesToDisplay(3);
    lrHPFCutOffSlider.setTextValueSuffix("Hz");
    
    lrLPFCutOffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "lrLPFCutOff", lrLPFCutOffSlider));
    addAndMakeVisible( &lrLPFCutOffSlider );
    lrLPFCutOffSlider.setSliderStyle (juce::Slider::Rotary);
    lrLPFCutOffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    lrLPFCutOffSlider.setNumDecimalPlacesToDisplay(3);
    lrLPFCutOffSlider.setTextValueSuffix("Hz");
    
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
    
    
    
    addAndMakeVisible( &interpolationTypeBox );
    interpolationTypeBox.addItem( "linear", 1 );
    interpolationTypeBox.addItem( "cubic", 2 );
    interpolationTypeBox.addItem( "PD", 3 );
    interpolationTypeBox.addItem( "4th Order", 4 );
    interpolationTypeBox.addItem( "Godot", 5 );
    interpolationTypeBox.addItem( "Hermite", 6 );
    interpolationTypeBoxAttachment.reset (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (valueTreeState, "interpolationType", interpolationTypeBox));
//    interpolationTypeBox.setSelectedId( 1 );
    
    
    fbControlButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "feedbackControl", fbControlButton ));
    addAndMakeVisible( &fbControlButton );
    fbControlButton.setButtonText( "fb control" );
    
    
    
    setSize ( 6*potSize + indent*7, 4*potSize );
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
    g.drawFittedText ("sjf_verb", 0, 0, getWidth(), textHeight, juce::Justification::centred, 1);
    
    g.drawFittedText ("Input LPF", erCutOffSlider.getX(), erCutOffSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Predelay", preDelaySlider.getX(), preDelaySlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Size", sizeSlider.getX(), sizeSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Decay", decaySlider.getX(), decaySlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("HPF", lrHPFCutOffSlider.getX(), lrHPFCutOffSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("LPF", lrLPFCutOffSlider.getX(), lrLPFCutOffSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Mod Rate", modulationRateSlider.getX(), modulationRateSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Mod Depth", modulationDepthSlider.getX(), modulationDepthSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Shimmer", shimLevelSlider.getX(), shimLevelSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Pitch", shimTranspositionSlider.getX(), shimTranspositionSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Mix", mixSlider.getX(), mixSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
}

void Sjf_verbAudioProcessorEditor::resized()
{
    float top = textHeight * 2;
    float spacing = (1.0*textHeight);
    
    erCutOffSlider.setBounds( indent, top, potSize, potSize );
    
    preDelaySlider.setBounds( erCutOffSlider.getX(), erCutOffSlider.getBottom() + spacing, potSize, potSize );
    reverseButton.setBounds( preDelaySlider.getX(), preDelaySlider.getBottom(), potSize, textHeight );
    
    
    sizeSlider.setBounds( preDelaySlider.getRight() + indent, top, potSize, potSize );
    decaySlider.setBounds( sizeSlider.getX(), sizeSlider.getBottom() + spacing, potSize, potSize );
    fbControlButton.setBounds( decaySlider.getX(), decaySlider.getBottom(), potSize, textHeight );
    
    lrHPFCutOffSlider.setBounds( sizeSlider.getRight()  + indent, top, potSize, potSize );
    lrLPFCutOffSlider.setBounds( lrHPFCutOffSlider.getX(), lrHPFCutOffSlider.getBottom() + spacing, potSize, potSize );
    
    modulationRateSlider.setBounds( lrLPFCutOffSlider.getRight()  + indent, top, potSize, potSize );
    modulationDepthSlider.setBounds( modulationRateSlider.getX(), modulationRateSlider.getBottom() + spacing, potSize, potSize );
    modulationTypeButton.setBounds( modulationDepthSlider.getX(), modulationDepthSlider.getBottom(), potSize, textHeight );
    
    shimLevelSlider.setBounds( modulationRateSlider.getRight() + indent, top, potSize, potSize );
    shimTranspositionSlider.setBounds( shimLevelSlider.getX(), shimLevelSlider.getBottom() + spacing, potSize, potSize );
    
    mixSlider.setBounds( shimLevelSlider.getRight() + indent, top, potSize, potSize );
    
    
    
    interpolationTypeBox.setBounds( getWidth() - indent - potSize, getHeight() - textHeight, potSize, textHeight );
}
