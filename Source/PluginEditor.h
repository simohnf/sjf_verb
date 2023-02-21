/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_lookAndFeel.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_compileTimeRandom.h"

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
    
    juce::Slider mixSlider, preDelaySlider, sizeSlider, diffusionSlider, modulationRateSlider, modulationDepthSlider, decaySlider, lrLPFCutoffSlider, lrHPFCutoffSlider, inputLPFCutoffSlider, inputHPFCutoffSlider, shimLevelSlider, shimTranspositionSlider;
    juce::ComboBox interpolationTypeBox, earlyReflectionTypeBox;
    juce::ToggleButton reverseButton, fbControlButton, modulationTypeButton, monoLowButton, tooltipsToggle, testButton;

    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixSliderAttachment, preDelaySliderAttachment, sizeSliderAttachment, diffusionSliderAttachment, modulationRateSliderAttachment, modulationDepthSliderAttachment, decaySliderAttachment, lrLPFCutoffSliderAttachment, lrHPFCutoffSliderAttachment, inputLPFCutoffSliderAttachment, inputHPFCutoffSliderAttachment, shimLevelSliderAttachment, shimTranspositionSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> interpolationTypeBoxAttachment, earlyReflectionTypeBoxAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> reverseButtonAttachment, fbControlButtonAttachment, monoLowButtonAttachment, modulationTypeButtonAttachment;
    
    
    juce::SharedResourcePointer<juce::TooltipWindow> tooltipWindow;
    
    
//    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primeBackGround_png, BinaryData::primeBackGround_pngSize );
//    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primeBackGround2_png, BinaryData::primeBackGround2_pngSize );
    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primeBackGround3_png, BinaryData::primeBackGround3_pngSize );
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( Sjf_verbAudioProcessorEditor )
};
