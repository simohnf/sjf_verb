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
    
    modulationTypeButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "modulationType", modulationTypeButton ));
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
    
    
    
    setSize ( 6* potSize + indent*2, 3 * potSize );
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
}

void Sjf_verbAudioProcessorEditor::resized()
{
    float top = textHeight;
    preDelaySlider.setBounds( indent, top, potSize, potSize );
    erCutOffSlider.setBounds( preDelaySlider.getX(), preDelaySlider.getBottom(), potSize, potSize );
    
    sizeSlider.setBounds( preDelaySlider.getRight(), top, potSize, potSize );
    decaySlider.setBounds( sizeSlider.getX(), sizeSlider.getBottom(), potSize, potSize );
    fbControlButton.setBounds( decaySlider.getX(), decaySlider.getBottom(), potSize, textHeight );
    
    lrCutOffSlider.setBounds( sizeSlider.getRight(), top, potSize, potSize );
    
    modulationRateSlider.setBounds( lrCutOffSlider.getRight(), top, potSize, potSize );
    modulationDepthSlider.setBounds( modulationRateSlider.getX(), modulationRateSlider.getBottom(), potSize, potSize );
    modulationTypeButton.setBounds( modulationDepthSlider.getX(), modulationDepthSlider.getBottom(), potSize, textHeight );
    
    shimLevelSlider.setBounds( modulationRateSlider.getRight(), top, potSize, potSize );
    shimTranspositionSlider.setBounds( shimLevelSlider.getX(), shimLevelSlider.getBottom(), potSize, potSize );
    
    mixSlider.setBounds( shimLevelSlider.getRight(), top, potSize, potSize );
    
    
    
    interpolationTypeBox.setBounds( getWidth() - indent - potSize, getHeight() - textHeight, potSize, textHeight );
}
