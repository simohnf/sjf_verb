/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "parameterIDs.h"


//==============================================================================
Sjf_verbAudioProcessorEditor::Sjf_verbAudioProcessorEditor (Sjf_verbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState (vts)
{
    
    setLookAndFeel( &otherLookandFeel );
    
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //                                  INPUT                                               //
    addAndMakeVisible( &preDelaySlider );
    preDelaySlider.initialise( POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::preDelay, "Delay before first early reflections", vts );
    preDelaySliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::preDelay, preDelaySlider));
    
    reverseButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::reverse, reverseButton ));
    addAndMakeVisible( &reverseButton );
    reverseButton.setButtonText( "reverse" );
    reverseButton.setTooltip("This will reverse the audio in the predelay buffer... if pre delay is very short this will cause very noticeable glitches. That said it may not be desirable anyway");
    
    
    
    inputLPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::inputLPFCutoff, inputLPFCutoffSlider));
    addAndMakeVisible( &inputLPFCutoffSlider );
    inputLPFCutoffSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::inputLPFCutoff, "This applies a low pass filter to the input signal before it is passed through the reverb", vts);
    
    inputHPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::inputHPFCutoff, inputHPFCutoffSlider));
    addAndMakeVisible( &inputHPFCutoffSlider );
    inputHPFCutoffSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::inputHPFCutoff, "This applies a high pass filter to the input signal before it is passed through the reverb", vts);
    
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //                                  EARLY                                               //
    
    
    addAndMakeVisible( &earlyReflectionTypeBox );
    earlyReflectionTypeBox.addItemList(parameterIDs::earlyTypes, 1);
    earlyReflectionTypeBoxAttachment.reset (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::earlyReflectionType, earlyReflectionTypeBox));
    earlyReflectionTypeBox.setTooltip("A variety of different early reflection types which can give the reverb a different character");

    
    
    earlyDiffusionSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::earlyDiffusion, earlyDiffusionSlider));
    addAndMakeVisible( &earlyDiffusionSlider );
    earlyDiffusionSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::earlyDiffusion, "Higher diffusion results in a denser reverb, lower diffusion results in fewer echoes - meaning that in certain circumstances individual echoes may be heard", vts);
//    earlyDiffusionSlider.setSliderStyle (juce::Slider::Rotary);
//    earlyDiffusionSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, POTSIZE, TEXT_HEIGHT);
//    earlyDiffusionSlider.setNumDecimalPlacesToDisplay(3);
//    earlyDiffusionSlider.setTextValueSuffix ("%");
//    earlyDiffusionSlider.setTooltip("Higher diffusion results in a denser reverb, lower diffusion results in fewer echoes - meaning that in certain circumstances individual echoes may be heard");
//    earlyDiffusionSlider.sendLookAndFeelChange();
    
    
    erHPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::earlyHPFCutoff, erHPFCutoffSlider));
    addAndMakeVisible( &erHPFCutoffSlider );
    erHPFCutoffSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::earlyHPFCutoff, "This applies a high pass filter to the output of the early reflection stage", vts);

    
    erLPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::earlyLPFCutoff, erLPFCutoffSlider));
    addAndMakeVisible( &erLPFCutoffSlider );
    erLPFCutoffSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::earlyLPFCutoff, "This applies a low pass filter to the output of the early reflection stage", vts);

//
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //                                  LATE                                               //
    
    
    addAndMakeVisible( &lateReflectionTypeBox );
    lateReflectionTypeBox.addItemList(parameterIDs::lateTypes, 1);
    lateReflectionTypeBoxAttachment.reset (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::lateReflectionType, lateReflectionTypeBox));
    lateReflectionTypeBox.setTooltip("Different late reflection types which can give the reverb a different character");

    addAndMakeVisible( &fdnMixTypeBox );
    fdnMixTypeBox.addItemList(parameterIDs::fdnMixTypes, 1);
    fdnMixTypeBoxAttachment.reset (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::fdnMixType, fdnMixTypeBox));
    fdnMixTypeBox.setTooltip("Different mixers for the feedback delay network\nNone: no mixing (more noticeable echoes)\nHouseholder: some mixing \nHadamard: most mixing");

    lateDiffusionSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::lateDiffusion, lateDiffusionSlider));
    addAndMakeVisible( &lateDiffusionSlider );
    lateDiffusionSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::lateDiffusion, "Higher diffusion results in a denser reverb, lower diffusion results in fewer echoes - meaning that in certain circumstances individual echoes may be heard", vts);
    
    
    
    lrHPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::lateHPFCutoff, lrHPFCutoffSlider));
    addAndMakeVisible( &lrHPFCutoffSlider );
    lrHPFCutoffSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::lateHPFCutoff, "This applies a high pass filter within the tail of the reverb, resulting in increasing loss of low frequency content", vts);
    
    lrLPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::lateLPFCutoff, lrLPFCutoffSlider));
    addAndMakeVisible( &lrLPFCutoffSlider );
    lrLPFCutoffSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::lateLPFCutoff, "This applies a low pass filter to the tail of the reverb, resulting in increasing loss of high frequency content", vts);

    
    
    decaySliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::decay, decaySlider));
    addAndMakeVisible( &decaySlider );
    decaySlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::decay, "This sets the desired length of time it will take for the reverb to decay by 60dB... due to the inner workings of the algorithm(s) ths will not be exact, but higher values mean longer reverb times and vice versa", vts);
    
    fbControlButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::feedbackLimit, fbControlButton ));
    addAndMakeVisible( &fbControlButton );
    fbControlButton.setButtonText( "fb limit" );
    fbControlButton.setTooltip("This passes the feedback loops through a softclipper preventing any uncontrollable feedback ");
    
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //                               EARLY&LATE                                              //
    
    sizeSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::size, sizeSlider));
    addAndMakeVisible( &sizeSlider );
    sizeSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::size, "This determines the size of the virtual space created", vts);

    
    modulationRateSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::modRate, modulationRateSlider));
    addAndMakeVisible( &modulationRateSlider );
    modulationRateSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::modRate, "This determines how fast the modulation applied to the delay times is. This can add anything from subtle chorusing to extreme frequency modulation...", vts);


    
    modulationDepthSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::modDepth, modulationDepthSlider));
    addAndMakeVisible( &modulationDepthSlider );
    modulationDepthSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::modDepth, "The depth of the modulation determines how extreme the changes in delay times are. This can add anything from subtle chorusing to extreme frequency modulation...", vts);
 

    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //                                      SHIMMER                                         //

    shimLevelSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::shimmerLevel, shimLevelSlider));
    addAndMakeVisible( &shimLevelSlider );
    shimLevelSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::shimmerLevel, "This sets the level of the pitchshifted signal(shimmer) which is then passed back into the reverb - this can  be set to 0% to save cpu", vts);

    
    shimTranspositionSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::shimmerTransposition, shimTranspositionSlider));
    addAndMakeVisible( &shimTranspositionSlider );
    shimTranspositionSlider.initialise(POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::shimmerTransposition, "This sets the transposition of the pitchshift (shimmer) which is then passed back into the reverb", vts);

    addAndMakeVisible(&dualShimButton);
    dualShimButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::shimmerDualVoice, dualShimButton ));
    dualShimButton.setButtonText( "dual voice" );
    dualShimButton.setTooltip("This creates an additional pitchshifted voice but with the transposition inverted");
    
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //                                      OUTPUT                                          //
    
 
    
    erOutputSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::earlyReflectionLevel, erOutputSlider));
    addAndMakeVisible( &erOutputSlider );
    erOutputSlider.initialise( POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::earlyReflectionLevel, "Level of the early reflections sent to the wet output", vts );
    
    lrOutputSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::lateReflectionLevel, lrOutputSlider));
    addAndMakeVisible( &lrOutputSlider );
    lrOutputSlider.initialise( POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::lateReflectionLevel, "Level of the late reflections sent to the wet output", vts );
        
    mixSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::mix, mixSlider));
    addAndMakeVisible( &mixSlider );
    mixSlider.initialise( POTSIZE, TEXT_HEIGHT, parameterIDs::mainName+parameterIDs::mix, "Dry/Wet Mix", vts );
    
    
    addAndMakeVisible( &interpolationTypeBox );
    interpolationTypeBox.addItemList(parameterIDs::interpTypes, 1);
    interpolationTypeBoxAttachment.reset (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::interpolationType, interpolationTypeBox));
    interpolationTypeBox.setTooltip("This changes between different interpolation types... linear is the least cpu intensive, cubic sounds a bit naff to me, the others are all higher quality interpolation which come at various prices with regards to cpu usage");
    
    monoLowButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, parameterIDs::mainName+parameterIDs::monoLow, monoLowButton ));
    addAndMakeVisible( &monoLowButton );
    monoLowButton.setButtonText( "mono low" );
    monoLowButton.setTooltip("This applies MidSide processing to the output so that low frequencies are converted to mono");

    
    
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //                           TOOLTIPS&SETUP                                             //

    addAndMakeVisible( &tooltipsToggle );
    tooltipsToggle.setButtonText( "Hints" );
    tooltipsToggle.onStateChange = [ this ]
    {
        if (tooltipsToggle.getToggleState())
        {
            setSize ( WIDTH, HEIGHT+tooltipLabel.getHeight() );
            tooltipLabel.setVisible( true );
        }
        else
        {
            setSize ( WIDTH, HEIGHT );
            tooltipLabel.setVisible( false );
        }
    };
    tooltipsToggle.setTooltip( MAIN_TOOLTIP );
    
    addAndMakeVisible( &tooltipLabel );
    tooltipLabel.setVisible( false );
    tooltipLabel.setColour( juce::Label::backgroundColourId, otherLookandFeel.backGroundColour.withAlpha( 0.85f ) );
    
    
    addAndMakeVisible( &setupToggle );
    setupToggle.onClick = [this](){
        auto state = setupToggle.getToggleState();
        if( state )
        {
            
        }
        else
        {
            
        }
    };
    
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    //================//================//================//================//================
    
    startTimer( 250 );
    setSize ( WIDTH, HEIGHT );
}

Sjf_verbAudioProcessorEditor::~Sjf_verbAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void Sjf_verbAudioProcessorEditor::paint (juce::Graphics& g)
{

#ifdef JUCE_DEBUG
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
#else
    juce::Rectangle<int> r = { (int)( WIDTH ), (int)(HEIGHT + tooltipLabel.getHeight()) };
    sjf_makeBackground< 40 >( g, r );
#endif
    g.setColour ( TEXT_COLOUR );
    g.setFont (15.0f);
    g.drawFittedText ("sjf_verb", 0, 0, getWidth(), TEXT_HEIGHT, textJust, 1);
    
    
    
    
    paintInputSection( g, inputRect );
    paintEarlySection( g, earlyRect );
    paintLateSection( g, lateRect );
    paintModulationSection( g, modRect );
    paintShimmerSection( g, shimmerRect );
    paintOutputSection( g, outputRect );
    
}

void Sjf_verbAudioProcessorEditor::resized()
{
    
    
    
    resizeInputSection( inputRect );
    resizeEarlySection( earlyRect );
    resizeLateSection( lateRect );
    resizeModulationSection( modRect );
    resizeShimmerSection( shimmerRect );
    resizeOutputSection( outputRect );
    
    tooltipLabel.setBounds( 0, HEIGHT, getWidth(), TEXT_HEIGHT*2.5 );
}


void Sjf_verbAudioProcessorEditor::timerCallback()
{
    sjf_setTooltipLabel( this, MAIN_TOOLTIP, tooltipLabel );
}

//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
/*                                      INPUT                                       */

void Sjf_verbAudioProcessorEditor::paintInputSection( juce::Graphics& g, juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    g.setColour( PANEL_COLOUR );
    g.fillRoundedRectangle(rect, CORNER_SIZE);
    
    
    g.setColour ( TEXT_COLOUR );
    g.drawFittedText("INPUT", x, y, w, TEXT_HEIGHT, textJust, 1 );
    g.drawFittedText ("Predelay", preDelaySlider.getX(), preDelaySlider.getY() - TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("LPF", inputLPFCutoffSlider.getX(), inputLPFCutoffSlider.getY() - TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("HPF", inputHPFCutoffSlider.getX(), inputHPFCutoffSlider.getY() - TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT, textJust, 1);
}


void Sjf_verbAudioProcessorEditor::resizeInputSection( juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    
    preDelaySlider.setBounds( x + INDENT, y + TEXT_HEIGHT*2, POTSIZE, POTSIZE );
    reverseButton.setBounds( preDelaySlider.getX(), preDelaySlider.getBottom(), POTSIZE, TEXT_HEIGHT );
    
    inputHPFCutoffSlider.setBounds( reverseButton.getX(), reverseButton.getBottom() + 3*TEXT_HEIGHT/2, POTSIZE, POTSIZE );
    inputLPFCutoffSlider.setBounds( inputHPFCutoffSlider.getX(), inputHPFCutoffSlider.getBottom() + TEXT_HEIGHT, POTSIZE, POTSIZE );
}

//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
/*                                      EARLY                                       */
void Sjf_verbAudioProcessorEditor::paintEarlySection( juce::Graphics& g, juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    
    g.setColour( PANEL_COLOUR );
    g.fillRoundedRectangle(rect, CORNER_SIZE);
    
    g.setColour ( TEXT_COLOUR );
    g.drawFittedText ("EARLY REFLECTIONS", x, y, w, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("Diffusion", earlyDiffusionSlider.getX(), earlyDiffusionSlider.getY() - TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("LPF", erLPFCutoffSlider.getX(), erLPFCutoffSlider.getY() - TEXT_HEIGHT, erLPFCutoffSlider.getWidth(), TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("HPF", erHPFCutoffSlider.getX(), erHPFCutoffSlider.getY() - TEXT_HEIGHT, erHPFCutoffSlider.getWidth(), TEXT_HEIGHT, textJust, 1);

}


void Sjf_verbAudioProcessorEditor::resizeEarlySection( juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    
    earlyReflectionTypeBox.setBounds( x + INDENT + POTSIZE/2, y + TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT );
    earlyDiffusionSlider.setBounds( earlyReflectionTypeBox.getX(), earlyReflectionTypeBox.getBottom() +TEXT_HEIGHT, POTSIZE, POTSIZE );
    
    erHPFCutoffSlider.setBounds( x+INDENT, earlyDiffusionSlider.getBottom() +3*TEXT_HEIGHT/2, POTSIZE, POTSIZE );
    erLPFCutoffSlider.setBounds( erHPFCutoffSlider.getRight(), erHPFCutoffSlider.getY(), POTSIZE, POTSIZE );
    
    
}

//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
/*                                      LATE                                       */
void Sjf_verbAudioProcessorEditor::paintLateSection( juce::Graphics& g, juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    
    g.setColour( PANEL_COLOUR );
    g.fillRoundedRectangle(rect, CORNER_SIZE);
    
    g.setColour ( TEXT_COLOUR );
    g.drawFittedText ("LATE REFLECTIONS", x, y, w, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("Diffusion", lateDiffusionSlider.getX(), lateDiffusionSlider.getY() - TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("Decay", decaySlider.getX(), decaySlider.getY() - TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("LPF", lrLPFCutoffSlider.getX(), lrLPFCutoffSlider.getY() - TEXT_HEIGHT, lrLPFCutoffSlider.getWidth(), TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("HPF", lrHPFCutoffSlider.getX(), lrHPFCutoffSlider.getY() - TEXT_HEIGHT, lrHPFCutoffSlider.getWidth(), TEXT_HEIGHT, textJust, 1);
}


void Sjf_verbAudioProcessorEditor::resizeLateSection( juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    
    lateReflectionTypeBox.setBounds( x + INDENT, y + TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT );
    fdnMixTypeBox.setBounds( lateReflectionTypeBox.getRight(), lateReflectionTypeBox.getY(), POTSIZE, TEXT_HEIGHT );
    lateDiffusionSlider.setBounds( x + INDENT, lateReflectionTypeBox.getBottom() +TEXT_HEIGHT, POTSIZE, POTSIZE );
    decaySlider.setBounds( lateDiffusionSlider.getRight(), lateDiffusionSlider.getY(), POTSIZE, POTSIZE );
    fbControlButton.setBounds( decaySlider.getRight() - TEXT_HEIGHT, decaySlider.getY(), TEXT_HEIGHT, TEXT_HEIGHT );
    
    lrHPFCutoffSlider.setBounds( x+INDENT, lateDiffusionSlider.getBottom() +3*TEXT_HEIGHT/2, POTSIZE, POTSIZE );
    lrLPFCutoffSlider.setBounds( lrHPFCutoffSlider.getRight(), lrHPFCutoffSlider.getY(), POTSIZE, POTSIZE );
}
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
/*                                   EARLY LATE                                     */
void Sjf_verbAudioProcessorEditor::paintModulationSection( juce::Graphics& g, juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    
    g.setColour( PANEL_COLOUR );
    g.fillRoundedRectangle(rect, CORNER_SIZE);
    
    g.setColour ( TEXT_COLOUR );
    g.drawFittedText("Size", sizeSlider.getX(), sizeSlider.getY() - TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("Mod Depth", modulationDepthSlider.getX(), modulationDepthSlider.getY() - TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("Mod Rate", modulationRateSlider.getX(), modulationRateSlider.getY() - TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT, textJust, 1);
}


void Sjf_verbAudioProcessorEditor::resizeModulationSection( juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    
    sizeSlider.setBounds( x + INDENT, y + TEXT_HEIGHT + INDENT, POTSIZE, POTSIZE);
    
    modulationDepthSlider.setBounds( x + INDENT, sizeSlider.getBottom() + TEXT_HEIGHT*3, POTSIZE, POTSIZE );
    modulationRateSlider.setBounds( x + INDENT, modulationDepthSlider.getBottom()+TEXT_HEIGHT, POTSIZE, POTSIZE );
    
}

//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
/*                                    Shimmer                                      */
void Sjf_verbAudioProcessorEditor::paintShimmerSection( juce::Graphics& g, juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    
    g.setColour( PANEL_COLOUR );
    g.fillRoundedRectangle(rect, CORNER_SIZE);
    
    g.setColour ( TEXT_COLOUR );
    g.drawFittedText ("SHIMMER", x, y, w, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("Level", shimLevelSlider.getX(), shimLevelSlider.getY() - TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText ("Transposition", shimTranspositionSlider.getX(), shimTranspositionSlider.getY() - TEXT_HEIGHT, POTSIZE, TEXT_HEIGHT, textJust, 1);
}



void Sjf_verbAudioProcessorEditor::resizeShimmerSection( juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    
    dualShimButton.setBounds( x+INDENT, y+ TEXT_HEIGHT + INDENT, POTSIZE, TEXT_HEIGHT );
    shimLevelSlider.setBounds( x+INDENT, dualShimButton.getBottom()+ TEXT_HEIGHT, POTSIZE, POTSIZE );
    shimTranspositionSlider.setBounds( shimLevelSlider.getX(), shimLevelSlider.getBottom()+TEXT_HEIGHT, POTSIZE, POTSIZE );
    
}
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
//===============//===============//===============//===============//===============
/*                                     OUTPUT                                      */
void Sjf_verbAudioProcessorEditor::paintOutputSection( juce::Graphics& g, juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    
    g.setColour( PANEL_COLOUR );
    g.fillRoundedRectangle(rect, CORNER_SIZE);
    
    g.setColour ( TEXT_COLOUR );
    g.drawFittedText("OUTPUT", x, y, w, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText("Early Level", erOutputSlider.getX(), erOutputSlider.getY() - TEXT_HEIGHT , POTSIZE, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText("Late Level", lrOutputSlider.getX(), lrOutputSlider.getY() - TEXT_HEIGHT , POTSIZE, TEXT_HEIGHT, textJust, 1);
    g.drawFittedText("Mix", mixSlider.getX(), mixSlider.getY() - TEXT_HEIGHT , POTSIZE, TEXT_HEIGHT, textJust, 1);
}


void Sjf_verbAudioProcessorEditor::resizeOutputSection( juce::Rectangle<float> rect )
{
    auto x = rect.getX();
    auto y = rect.getY();
    auto w = rect.getWidth();
    
    erOutputSlider.setBounds( x + INDENT, y + TEXT_HEIGHT*2, POTSIZE, POTSIZE );
    lrOutputSlider.setBounds( x + INDENT, erOutputSlider.getBottom() + TEXT_HEIGHT, POTSIZE, POTSIZE );
    mixSlider.setBounds( x + INDENT, lrOutputSlider.getBottom() + TEXT_HEIGHT, POTSIZE, POTSIZE );
    monoLowButton.setBounds( mixSlider.getX(), mixSlider.getBottom(), POTSIZE, TEXT_HEIGHT );
    interpolationTypeBox.setBounds( monoLowButton.getX(), monoLowButton.getBottom() + INDENT, POTSIZE, TEXT_HEIGHT );

    tooltipsToggle.setBounds( interpolationTypeBox.getX(), interpolationTypeBox.getBottom() + INDENT, POTSIZE, TEXT_HEIGHT );
}
