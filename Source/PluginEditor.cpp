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
    
    diffusionSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "diffusion", diffusionSlider));
    addAndMakeVisible( &diffusionSlider );
    diffusionSlider.setSliderStyle (juce::Slider::Rotary);
    diffusionSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    diffusionSlider.setNumDecimalPlacesToDisplay(3);
    diffusionSlider.setTextValueSuffix ("%");
    
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
    
    lrHPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "lrHPFCutoff", lrHPFCutoffSlider));
    addAndMakeVisible( &lrHPFCutoffSlider );
    lrHPFCutoffSlider.setSliderStyle (juce::Slider::Rotary);
    lrHPFCutoffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    lrHPFCutoffSlider.setNumDecimalPlacesToDisplay(3);
    lrHPFCutoffSlider.setTextValueSuffix("Hz");
    
    lrLPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "lrLPFCutoff", lrLPFCutoffSlider));
    addAndMakeVisible( &lrLPFCutoffSlider );
    lrLPFCutoffSlider.setSliderStyle (juce::Slider::Rotary);
    lrLPFCutoffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    lrLPFCutoffSlider.setNumDecimalPlacesToDisplay(3);
    lrLPFCutoffSlider.setTextValueSuffix("Hz");
    
    erLPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "erLPFCutoff", erLPFCutoffSlider));
    addAndMakeVisible( &erLPFCutoffSlider );
    erLPFCutoffSlider.setSliderStyle (juce::Slider::Rotary);
    erLPFCutoffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    erLPFCutoffSlider.setNumDecimalPlacesToDisplay(3);
    erLPFCutoffSlider.setTextValueSuffix("Hz");

    erHPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "erHPFCutoff", erHPFCutoffSlider));
    addAndMakeVisible( &erHPFCutoffSlider );
    erHPFCutoffSlider.setSliderStyle (juce::Slider::Rotary);
    erHPFCutoffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    erHPFCutoffSlider.setNumDecimalPlacesToDisplay(3);
    erHPFCutoffSlider.setTextValueSuffix("Hz");
    
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
    
    monoLowButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "monoLow", monoLowButton ));
    addAndMakeVisible( &monoLowButton );
    monoLowButton.setButtonText( "mono low" );
    
    addAndMakeVisible( &testButton );
    testButton.setButtonText( "test" );
    testButton.onClick = [ this ]
    {
        audioProcessor.rev.filterBeforeShimmer( testButton.getToggleState() );
    };
    
    setSize ( 7*potSize + indent*9, 4*potSize );
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
    
    g.drawFittedText ("Input LPF", erLPFCutoffSlider.getX(), erLPFCutoffSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Input HPF", erHPFCutoffSlider.getX(), erHPFCutoffSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Predelay", preDelaySlider.getX(), preDelaySlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Diffusion", diffusionSlider.getX(), diffusionSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Size", sizeSlider.getX(), sizeSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Decay", decaySlider.getX(), decaySlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("HPF", lrHPFCutoffSlider.getX(), lrHPFCutoffSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("LPF", lrLPFCutoffSlider.getX(), lrLPFCutoffSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
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
    
    erHPFCutoffSlider.setBounds( indent, top, potSize, potSize );
    erLPFCutoffSlider.setBounds( erHPFCutoffSlider.getX(), erHPFCutoffSlider.getBottom() + spacing, potSize, potSize );
    
    preDelaySlider.setBounds( erLPFCutoffSlider.getRight()+ indent, top, potSize, potSize );
    diffusionSlider.setBounds( preDelaySlider.getX(), preDelaySlider.getBottom() + spacing, potSize, potSize );
    reverseButton.setBounds( diffusionSlider.getX(), diffusionSlider.getBottom(), potSize, textHeight );
    
    
    sizeSlider.setBounds( preDelaySlider.getRight() + indent, top, potSize, potSize );
    decaySlider.setBounds( sizeSlider.getX(), sizeSlider.getBottom() + spacing, potSize, potSize );
    fbControlButton.setBounds( decaySlider.getX(), decaySlider.getBottom(), potSize, textHeight );
    
    lrHPFCutoffSlider.setBounds( sizeSlider.getRight()  + indent, top, potSize, potSize );
    lrLPFCutoffSlider.setBounds( lrHPFCutoffSlider.getX(), lrHPFCutoffSlider.getBottom() + spacing, potSize, potSize );
    
    modulationRateSlider.setBounds( lrLPFCutoffSlider.getRight()  + indent, top, potSize, potSize );
    modulationDepthSlider.setBounds( modulationRateSlider.getX(), modulationRateSlider.getBottom() + spacing, potSize, potSize );
    modulationTypeButton.setBounds( modulationDepthSlider.getX(), modulationDepthSlider.getBottom(), potSize, textHeight );
    
    shimLevelSlider.setBounds( modulationRateSlider.getRight() + indent, top, potSize, potSize );
    shimTranspositionSlider.setBounds( shimLevelSlider.getX(), shimLevelSlider.getBottom() + spacing, potSize, potSize );
    
    mixSlider.setBounds( shimLevelSlider.getRight() + indent, top, potSize, potSize );
    monoLowButton.setBounds( mixSlider.getX(), mixSlider.getBottom(), potSize, textHeight );
    testButton.setBounds( monoLowButton.getX(), monoLowButton.getBottom(), potSize, textHeight );
    
    
    
    interpolationTypeBox.setBounds( getWidth() - indent - potSize, getHeight() - textHeight, potSize, textHeight );
}
