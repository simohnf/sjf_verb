/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../sjf_audio/sjf_LookAndFeel.h"
#include "../sjf_audio/sjf_compileTimeRandom.h"

#define POTSIZE 90
#define TEXT_HEIGHT 20
#define INDENT 5

//#define WIDTH 9*POTSIZE + INDENT*15
//#define HEIGHT 4*POTSIZE + TEXT_HEIGHT*5 + INDENT

//==============================================================================
/**
*/

namespace sjf
{

    class simpleSlider : public juce::Slider
    {
    public:
        void initialise( const int textWidth, const int textHeight, juce::String paramID, juce::String tooltip, juce::AudioProcessorValueTreeState& vts )
        {
            setSliderStyle (juce::Slider::Rotary);
            setTextBoxStyle (juce::Slider::TextBoxBelow, false, textWidth, textHeight);
            setTextValueSuffix( vts.getParameter(paramID)->label);
            setTooltip( tooltip );
        }
    };
}

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
    
    void paintInputSection( juce::Graphics& g, juce::Rectangle<float> rect );
    void resizeInputSection( juce::Rectangle<float> rect );
    void paintEarlySection( juce::Graphics& g, juce::Rectangle<float> rect );
    void resizeEarlySection( juce::Rectangle<float> rect );
    void paintLateSection( juce::Graphics& g, juce::Rectangle<float> rect );
    void resizeLateSection( juce::Rectangle<float> rect );
    void paintModulationSection( juce::Graphics& g, juce::Rectangle<float> rect );
    void resizeModulationSection( juce::Rectangle<float> rect );
    void paintShimmerSection( juce::Graphics& g, juce::Rectangle<float> rect );
    void resizeShimmerSection( juce::Rectangle<float> rect );
    
    void paintOutputSection( juce::Graphics& g, juce::Rectangle<float> rect );
    void resizeOutputSection( juce::Rectangle<float> rect );
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Sjf_verbAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;
    sjf_lookAndFeel otherLookandFeel; 
    
    /*              INPUT                   */
    sjf::simpleSlider preDelaySlider, inputLPFCutoffSlider, inputHPFCutoffSlider;
    juce::ToggleButton reverseButton;
    
    /*              EARLY&LATE                   */
    sjf::simpleSlider sizeSlider, modulationRateSlider, modulationDepthSlider;
    
    /*              EARLY                   */
    sjf::simpleSlider earlyDiffusionSlider, erLPFCutoffSlider, erHPFCutoffSlider;
    juce::ComboBox earlyReflectionTypeBox;
    
    /*              LATE                   */
    sjf::simpleSlider decaySlider, lateDiffusionSlider, lrLPFCutoffSlider, lrHPFCutoffSlider;
    juce::ComboBox lateReflectionTypeBox, fdnMixTypeBox;
    juce::ToggleButton fbControlButton;
    
    /*              SHIMMER                   */
    sjf::simpleSlider shimLevelSlider, shimTranspositionSlider;
    juce::ToggleButton dualShimButton;
    /*              OUTPUT                   */
    sjf::simpleSlider erOutputSlider, lrOutputSlider, mixSlider;
    juce::ToggleButton monoLowButton;

    /*              OTHER                   */
    juce::ComboBox interpolationTypeBox;
    juce::ToggleButton tooltipsToggle, setupToggle;
    
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixSliderAttachment, preDelaySliderAttachment, sizeSliderAttachment, earlyDiffusionSliderAttachment, lateDiffusionSliderAttachment, modulationRateSliderAttachment, modulationDepthSliderAttachment, decaySliderAttachment, lrLPFCutoffSliderAttachment, lrHPFCutoffSliderAttachment, erLPFCutoffSliderAttachment, erHPFCutoffSliderAttachment, inputLPFCutoffSliderAttachment, inputHPFCutoffSliderAttachment, shimLevelSliderAttachment, shimTranspositionSliderAttachment, erOutputSliderAttachment, lrOutputSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> interpolationTypeBoxAttachment, earlyReflectionTypeBoxAttachment, lateReflectionTypeBoxAttachment, fdnMixTypeBoxAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> reverseButtonAttachment, fbControlButtonAttachment, monoLowButtonAttachment, modulationTypeButtonAttachment, dualShimButtonAttachment;
        
    juce::Label tooltipLabel;
    juce::String MAIN_TOOLTIP = "sjf_verb: \nAlgorithmic reverb";

    juce::Rectangle<float> inputRect{ INDENT, TEXT_HEIGHT, POTSIZE+INDENT*2, POTSIZE*3+6*TEXT_HEIGHT};
    juce::Rectangle<float> modRect{ inputRect.getRight() + INDENT, TEXT_HEIGHT, inputRect.getWidth(), inputRect.getHeight() };
    juce::Rectangle<float> earlyRect{ modRect.getRight() + INDENT, TEXT_HEIGHT, POTSIZE*2+INDENT*2, POTSIZE*2+5*TEXT_HEIGHT};
    juce::Rectangle<float> lateRect{ earlyRect.getRight() + INDENT, TEXT_HEIGHT, POTSIZE*2+INDENT*2, POTSIZE*2+5*TEXT_HEIGHT};
    juce::Rectangle<float> shimmerRect{ lateRect.getRight() +INDENT, TEXT_HEIGHT, POTSIZE+INDENT*2, TEXT_HEIGHT*2 + INDENT + 2*POTSIZE };
    juce::Rectangle<float> outputRect{ shimmerRect.getRight() + INDENT, TEXT_HEIGHT, POTSIZE+INDENT*2, POTSIZE*2+5*TEXT_HEIGHT};

    
    const float WIDTH{ inputRect.getWidth() + modRect.getWidth() + earlyRect.getWidth() + shimmerRect.getWidth() + lateRect.getWidth() + outputRect.getWidth() + 8*INDENT};
    const float HEIGHT{ inputRect.getHeight() + TEXT_HEIGHT + INDENT*2 };
    
    static constexpr int CORNER_SIZE = 5;
    const juce::Colour TEXT_COLOUR = juce::Colours::white;
    const juce::Colour PANEL_COLOUR = otherLookandFeel.panelColour.withAlpha(0.3f);
    
    int textJust = juce::Justification::centred;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( Sjf_verbAudioProcessorEditor )
};
