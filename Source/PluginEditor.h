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
class Sjf_verbAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
    
public:
    Sjf_verbAudioProcessorEditor (Sjf_verbAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~Sjf_verbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    void timerCallback() override;
    
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
    
    
//    juce::SharedResourcePointer<juce::TooltipWindow> tooltipWindow;
    
    juce::Label tooltipLabel;
    juce::String MAIN_TOOLTIP = "sjf_verb: \nFeedback Delay Network based reverb";
//    void mouseEnter( const juce::MouseEvent& ) override
//    {
//        const auto mouseSource = juce::Desktop::getInstance().getMainMouseSource();
//        auto* newComp = mouseSource.isTouch() ? nullptr : mouseSource.getComponentUnderMouse();
//        tooltipWindow.displayTooltip
//    }
//    
//    juce::Image m_backgroundImage;
    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory ( BinaryData::primes1_png, BinaryData::primes1_pngSize );
//    std::array< juce::Image, 4 > m_images
//    {
//        juce::ImageCache::getFromMemory ( BinaryData::primes1_png, BinaryData::primes1_pngSize ),
//        juce::ImageCache::getFromMemory ( BinaryData::primes2_png, BinaryData::primes2_pngSize ),
//        juce::ImageCache::getFromMemory ( BinaryData::primes3_png, BinaryData::primes3_pngSize ),
//        juce::ImageCache::getFromMemory ( BinaryData::primes4_png, BinaryData::primes4_pngSize )
//    };
    
//    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primes2_png, BinaryData::primes2_pngSize );
//    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primes3_png, BinaryData::primes3_pngSize );
//    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::primes4_png, BinaryData::primes4_pngSize );
//    juce::Image m_backgroundImage = juce::ImageCache::getFromMemory (BinaryData::Tree1_png, BinaryData::Tree1_pngSize );


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( Sjf_verbAudioProcessorEditor )
};
