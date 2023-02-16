/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_lookAndFeel.h"
//==============================================================================
/**
*/
class Sjf_verbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Sjf_verbAudioProcessorEditor (Sjf_verbAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~Sjf_verbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Sjf_verbAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;
    sjf_lookAndFeel otherLookandFeel; 
    
    juce::Slider mixSlider, preDelaySlider, sizeSlider, diffusionSlider, modulationRateSlider, modulationDepthSlider, decaySlider, lrLPFCutoffSlider, lrHPFCutoffSlider, erLPFCutoffSlider, erHPFCutoffSlider, shimLevelSlider, shimTranspositionSlider;
    juce::ComboBox interpolationTypeBox;
    juce::ToggleButton reverseButton, fbControlButton, modulationTypeButton, monoLowButton, testButton;

    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixSliderAttachment, preDelaySliderAttachment, sizeSliderAttachment, diffusionSliderAttachment, modulationRateSliderAttachment, modulationDepthSliderAttachment, decaySliderAttachment, lrLPFCutoffSliderAttachment, lrHPFCutoffSliderAttachment, erLPFCutoffSliderAttachment, erHPFCutoffSliderAttachment, shimLevelSliderAttachment, shimTranspositionSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> interpolationTypeBoxAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> reverseButtonAttachment, fbControlButtonAttachment, monoLowButtonAttachment, modulationTypeButtonAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( Sjf_verbAudioProcessorEditor )
};
