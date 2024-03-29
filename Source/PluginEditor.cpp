/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define potSize 90
#define textHeight 20
#define indent 10
#define alph 0.5f
#define WIDTH 8*potSize + indent*13
#define HEIGHT 2*potSize + textHeight*5 + indent
//==============================================================================
Sjf_verbAudioProcessorEditor::Sjf_verbAudioProcessorEditor (Sjf_verbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState (vts)
{
    
//    m_backgroundImage = m_img;
//    m_backgroundImage.multiplyAllAlphas( 0.7f );

    setLookAndFeel( &otherLookandFeel );

    mixSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "mix", mixSlider));
    addAndMakeVisible( &mixSlider );
    mixSlider.setSliderStyle (juce::Slider::Rotary);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    mixSlider.setNumDecimalPlacesToDisplay(3);
    mixSlider.setTextValueSuffix ("%");
    mixSlider.setTooltip("Dry/Wet Mix");
    mixSlider.sendLookAndFeelChange();
//    mixSlider.setColour(id1, outColour);
    
    preDelaySliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "preDelay", preDelaySlider));
    addAndMakeVisible( &preDelaySlider );
    preDelaySlider.setSliderStyle (juce::Slider::Rotary);
    preDelaySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    preDelaySlider.setNumDecimalPlacesToDisplay(3);
    preDelaySlider.setTextValueSuffix ("ms");
    preDelaySlider.setTooltip("Delay before first early reflections");
    preDelaySlider.sendLookAndFeelChange();
//    preDelaySlider.setColour( id1, inputColour);
//    preDelaySlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::black.withAlpha(0.5f));
    
    reverseButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "reverse", reverseButton ));
    addAndMakeVisible( &reverseButton );
    reverseButton.setButtonText( "reverse" );
    reverseButton.setTooltip("This will reverse the audio in the predelay buffer... if pre delay is very short this will caus very noticeable glitches. That said it may not be desirable anyway");
    
    sizeSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "size", sizeSlider));
    addAndMakeVisible( &sizeSlider );
    sizeSlider.setSliderStyle (juce::Slider::Rotary);
    sizeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    sizeSlider.setNumDecimalPlacesToDisplay(3);
    sizeSlider.setTextValueSuffix ("%");
    sizeSlider.setTooltip("This determines the size of the virtual space created");
    sizeSlider.sendLookAndFeelChange();
//    sizeSlider.setColour(id1, reverbSectionColour);
    
    diffusionSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "diffusion", diffusionSlider));
    addAndMakeVisible( &diffusionSlider );
    diffusionSlider.setSliderStyle (juce::Slider::Rotary);
    diffusionSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    diffusionSlider.setNumDecimalPlacesToDisplay(3);
    diffusionSlider.setTextValueSuffix ("%");
    diffusionSlider.setTooltip("Higher diffusion results in a denser reverb, lower diffusion results in fewer echoes - meaning that in certain circumstances individual echoes may be heard");
    diffusionSlider.sendLookAndFeelChange();
//    diffusionSlider.setColour(id1, reverbSectionColour);
    
    modulationRateSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "modulationRate", modulationRateSlider));
    addAndMakeVisible( &modulationRateSlider );
    modulationRateSlider.setSliderStyle (juce::Slider::Rotary);
    modulationRateSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    modulationRateSlider.setNumDecimalPlacesToDisplay(3);
    modulationRateSlider.setTextValueSuffix ("Hz");
    modulationRateSlider.setTooltip("This determines how fast the modulation applied to the delay times is. This can add anything from subtle chorusing to extreme frequency modulation...");
    modulationRateSlider.sendLookAndFeelChange();
//    modulationRateSlider.setColour(id1, modColour);
    
    modulationDepthSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "modulationDepth", modulationDepthSlider));
    addAndMakeVisible( &modulationDepthSlider );
    modulationDepthSlider.setSliderStyle (juce::Slider::Rotary);
    modulationDepthSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    modulationDepthSlider.setNumDecimalPlacesToDisplay(3);
    modulationDepthSlider.setTextValueSuffix ("%");
    modulationDepthSlider.setTooltip("The depth of the modulation determines how extreme the changes in delay times are. This can add anything from subtle chorusing to extreme frequency modulation... - this can  be set to 0% to save cpu");
    modulationDepthSlider.sendLookAndFeelChange();
//    modulationDepthSlider.setColour(id1, modColour);
    
    addAndMakeVisible( &modulationTypeButton );
    
    modulationTypeButton.setButtonText( "sine mod" );
    modulationTypeButton.onStateChange  = [ this ]
    {
        if ( modulationTypeButton.getToggleState() ) { modulationTypeButton.setButtonText( "rand mod" ); }
        else { modulationTypeButton.setButtonText( "sine mod" ); }
    };
    modulationTypeButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "modulationType", modulationTypeButton ));
    modulationTypeButton.setTooltip("The modulation type determines the shape of the waveform used to modulate the delaylines. Sinusoidal modulation is a little more predictable and can result in more extreme results at the same settings of modulation rate and depth");
//    modulationTypeButton.setColour( juce::ToggleButton::tickColourId, juce::Colours::white.withAlpha(0.0f));
    
    decaySliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "decay", decaySlider));
    addAndMakeVisible( &decaySlider );
    decaySlider.setSliderStyle (juce::Slider::Rotary);
    decaySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    decaySlider.setNumDecimalPlacesToDisplay(3);
    decaySlider.setTextValueSuffix ("%");
    decaySlider.setTooltip("The amount of passed back into the delay lines. Higher settings means longer reverb times. At 0% only the early reflections will be heard");
    decaySlider.sendLookAndFeelChange();
//    decaySlider.setColour(id1, reverbSectionColour);
    
    lrHPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "lrHPFCutoff", lrHPFCutoffSlider));
    addAndMakeVisible( &lrHPFCutoffSlider );
    lrHPFCutoffSlider.setSliderStyle (juce::Slider::Rotary);
    lrHPFCutoffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    lrHPFCutoffSlider.setNumDecimalPlacesToDisplay(3);
    lrHPFCutoffSlider.setTextValueSuffix("Hz");
    lrHPFCutoffSlider.setTooltip("This applies a high pass filter to the tail of the reverb");
    lrHPFCutoffSlider.sendLookAndFeelChange();
//    lrHPFCutoffSlider.setColour(id1, reverbSectionColour);
    
    lrLPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "lrLPFCutoff", lrLPFCutoffSlider));
    addAndMakeVisible( &lrLPFCutoffSlider );
    lrLPFCutoffSlider.setSliderStyle (juce::Slider::Rotary);
    lrLPFCutoffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    lrLPFCutoffSlider.setNumDecimalPlacesToDisplay(3);
    lrLPFCutoffSlider.setTextValueSuffix("Hz");
    lrLPFCutoffSlider.setTooltip("This applies a low pass filter to the tail of the reverb");
    lrLPFCutoffSlider.sendLookAndFeelChange();
//    lrLPFCutoffSlider.setColour(id1, reverbSectionColour);
    
    inputLPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "inputLPFCutoff", inputLPFCutoffSlider)); 
    addAndMakeVisible( &inputLPFCutoffSlider );
    inputLPFCutoffSlider.setSliderStyle (juce::Slider::Rotary);
    inputLPFCutoffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    inputLPFCutoffSlider.setNumDecimalPlacesToDisplay(3);
    inputLPFCutoffSlider.setTextValueSuffix("Hz");
    inputLPFCutoffSlider.setTooltip("This applies a low pass filter to the input signal before it is passed through the reverb");
    inputLPFCutoffSlider.sendLookAndFeelChange();
//    inputLPFCutoffSlider.setColour(id1, inputColour);

    
    inputHPFCutoffSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "inputHPFCutoff", inputHPFCutoffSlider));
    addAndMakeVisible( &inputHPFCutoffSlider );
    inputHPFCutoffSlider.setSliderStyle (juce::Slider::Rotary);
    inputHPFCutoffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    inputHPFCutoffSlider.setNumDecimalPlacesToDisplay(3);
    inputHPFCutoffSlider.setTextValueSuffix("Hz");
    inputHPFCutoffSlider.setTooltip("This applies a high pass filter to the input signal before it is passed through the reverb");
    inputHPFCutoffSlider.sendLookAndFeelChange();
//    inputHPFCutoffSlider.setColour(id1, inputColour);
    
    shimLevelSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "shimmerLevel", shimLevelSlider));
    addAndMakeVisible( &shimLevelSlider );
    shimLevelSlider.setSliderStyle (juce::Slider::Rotary);
    shimLevelSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    shimLevelSlider.setNumDecimalPlacesToDisplay(3);
    shimLevelSlider.setTextValueSuffix ("%");
    shimLevelSlider.setTooltip("This sets the level of the pitchshifted signal(shimmer) which is then passed back into the reverb - this can  be set to 0% to save cpu");
    shimLevelSlider.sendLookAndFeelChange();
//    shimLevelSlider.setColour(id1, shimmerColour);
    
    shimTranspositionSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "shimmerTransposition", shimTranspositionSlider));
    addAndMakeVisible( &shimTranspositionSlider );
    shimTranspositionSlider.setSliderStyle (juce::Slider::Rotary);
    shimTranspositionSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    shimTranspositionSlider.setNumDecimalPlacesToDisplay(3);
    shimTranspositionSlider.setTextValueSuffix ("st");
    shimTranspositionSlider.setTooltip("This sets the transposition of the pitchshift (shimmer) which is then passed back into the reverb");
    shimTranspositionSlider.sendLookAndFeelChange();
//    shimTranspositionSlider.setColour(id1, shimmerColour);
    
    addAndMakeVisible( &interpolationTypeBox );
    interpolationTypeBox.addItem( "linear", 1 );
    interpolationTypeBox.addItem( "cubic", 2 );
    interpolationTypeBox.addItem( "PD", 3 );
    interpolationTypeBox.addItem( "4th Order", 4 );
    interpolationTypeBox.addItem( "Godot", 5 );
    interpolationTypeBox.addItem( "Hermite", 6 );
    interpolationTypeBoxAttachment.reset (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (valueTreeState, "interpolationType", interpolationTypeBox));
    interpolationTypeBox.setTooltip("This changes between different interpolation types... linear is the least cpu intensive, cubic sounds a bit naff to me, the others are all higher quality interpolation which come at various prices with regards to cpu usage");
    
    
    fbControlButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "feedbackDrive", fbControlButton ));
    addAndMakeVisible( &fbControlButton );
    fbControlButton.setButtonText( "fb drive" );
    fbControlButton.setTooltip("This amplifies the feedback loops and then passes them through a softclipper enabling feedback of more than 100% for some extreme infinite reverb timbres ");
    
    monoLowButtonAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "monoLow", monoLowButton ));
    addAndMakeVisible( &monoLowButton );
    monoLowButton.setButtonText( "mono low" );
    monoLowButton.setTooltip("This applies MidSide processing to the output so that low frequencies are converted to mono");
    
    addAndMakeVisible( &earlyReflectionTypeBox );
//    earlyReflectionTypeBox.onChange = [ this ]
//    {
//        auto id = earlyReflectionTypeBox.getSelectedId() - 1;
//        m_backgroundImage = m_images[ id ];
//        m_backgroundImage.multiplyAllAlphas( 0.7f );
//        repaint();
//    };
    earlyReflectionTypeBox.addItem( "zitaRev", 1 );
    earlyReflectionTypeBox.addItem( "zitaRev2", 2 );
    earlyReflectionTypeBox.addItem( "GeraintLuff", 3 );
    earlyReflectionTypeBox.addItem( "Multitap", 4 );
    earlyReflectionTypeBoxAttachment.reset (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (valueTreeState, "earlyReflectionType", earlyReflectionTypeBox));
    earlyReflectionTypeBox.setTooltip("A variety of different early reflection types which can give the reverb a different character");

    
//    addAndMakeVisible( &testButton );
//    testButton.setButtonText( "test" );
//    testButton.onClick = [ this ] { audioProcessor.setRevType( testButton.getToggleState() ); };
    
    addAndMakeVisible( &tooltipsToggle );
    tooltipsToggle.setButtonText( "Hints" );
    tooltipsToggle.onStateChange = [ this ]
    {
        if (tooltipsToggle.getToggleState())
        {
            setSize ( WIDTH, HEIGHT+tooltipLabel.getHeight() );
            tooltipLabel.setVisible( true );
//            tooltipWindow.getObject().setAlpha(1.0f);
        }
        else
        {
            setSize ( WIDTH, HEIGHT );
            tooltipLabel.setVisible( false );
//            tooltipWindow.getObject().setAlpha(0.0f);
        }
    };
    tooltipsToggle.setTooltip( MAIN_TOOLTIP );
//    tooltipWindow.getObject().setAlpha(0.0f);
    
    addAndMakeVisible( &tooltipLabel );
    tooltipLabel.setVisible( false );
    tooltipLabel.setColour( juce::Label::backgroundColourId, otherLookandFeel.backGroundColour.withAlpha( 0.85f ) );
    
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
    
    static constexpr int CORNER_SIZE = 5;
    static constexpr auto SPACING = textHeight/5;
    
//    g.setColour( juce::Colours::beige.withAlpha( 0.2f ) );
    g.setColour(otherLookandFeel.panelColour.withAlpha(0.3f) );

    auto rect1 = juce::Rectangle<float>( preDelaySlider.getX() - SPACING, inputHPFCutoffSlider.getY() - textHeight, inputHPFCutoffSlider.getRight() - preDelaySlider.getX() + SPACING * 2, inputLPFCutoffSlider.getBottom() - (inputHPFCutoffSlider.getY() - textHeight) + SPACING );
    g.fillRoundedRectangle(rect1, CORNER_SIZE);
    
    auto rect2 = juce::Rectangle<float>( diffusionSlider.getX() - SPACING, sizeSlider.getY() - textHeight, lrHPFCutoffSlider.getRight() - diffusionSlider.getX() + SPACING * 2, fbControlButton.getBottom() - (sizeSlider.getY() - textHeight) + SPACING );
    g.fillRoundedRectangle(rect2, CORNER_SIZE);
    
    auto rect3 = juce::Rectangle<float>( modulationRateSlider.getX() - SPACING, modulationRateSlider.getY() - textHeight, modulationRateSlider.getWidth() + SPACING * 2, modulationTypeButton.getBottom() - (modulationRateSlider.getY() - textHeight) + SPACING );
    g.fillRoundedRectangle(rect3, CORNER_SIZE);
    
    auto rect4 = juce::Rectangle<float>( shimLevelSlider.getX() - SPACING, shimLevelSlider.getY() - textHeight, shimTranspositionSlider.getWidth() + SPACING * 2, shimTranspositionSlider.getBottom() - (shimLevelSlider.getY() - textHeight) + SPACING );
    g.fillRoundedRectangle(rect4, CORNER_SIZE);
    
    auto rect5 = juce::Rectangle<float>( mixSlider.getX() - SPACING, mixSlider.getY() - textHeight, mixSlider.getWidth() + 2*SPACING, interpolationTypeBox.getBottom() - (mixSlider.getY() - textHeight) + tooltipsToggle.getHeight() + 4*SPACING );
    g.fillRoundedRectangle(rect5, CORNER_SIZE);
    
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("sjf_verb", 0, 0, getWidth(), textHeight, juce::Justification::centred, 1);
    
    g.setColour (juce::Colours::white);
    g.drawFittedText ("Input LPF", inputLPFCutoffSlider.getX(), inputLPFCutoffSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Input HPF", inputHPFCutoffSlider.getX(), inputHPFCutoffSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Predelay", preDelaySlider.getX(), preDelaySlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Diffusion", diffusionSlider.getX(), diffusionSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("ER Type", earlyReflectionTypeBox.getX(), earlyReflectionTypeBox.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Size", sizeSlider.getX(), sizeSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Decay", decaySlider.getX(), decaySlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("HPF", lrHPFCutoffSlider.getX(), lrHPFCutoffSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("LPF", lrLPFCutoffSlider.getX(), lrLPFCutoffSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Mod Rate", modulationRateSlider.getX(), modulationRateSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Mod Depth", modulationDepthSlider.getX(), modulationDepthSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Mod Type", modulationTypeButton.getX(), modulationTypeButton.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Shimmer", shimLevelSlider.getX(), shimLevelSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Pitch", shimTranspositionSlider.getX(), shimTranspositionSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
    g.drawFittedText ("Mix", mixSlider.getX(), mixSlider.getY() - textHeight, potSize, textHeight, juce::Justification::centred, 1);
}

void Sjf_verbAudioProcessorEditor::resized()
{
    static constexpr float top = textHeight * 2;
    static constexpr float spacing = (1.0*textHeight);
    static constexpr float top2 = top + potSize*0.5;
    
    preDelaySlider.setBounds( indent, top2, potSize, potSize );
    reverseButton.setBounds( preDelaySlider.getX(), preDelaySlider.getBottom(), potSize, textHeight );
    
    inputHPFCutoffSlider.setBounds( preDelaySlider.getRight() + indent, top, potSize, potSize );
    inputLPFCutoffSlider.setBounds( inputHPFCutoffSlider.getX(), inputHPFCutoffSlider.getBottom() + spacing, potSize, potSize );
    
    
    diffusionSlider.setBounds( inputLPFCutoffSlider.getRight() + indent*2, top2, potSize, potSize );
    earlyReflectionTypeBox.setBounds( diffusionSlider.getX(), diffusionSlider.getBottom() + spacing, potSize, textHeight );
    
    
    
    sizeSlider.setBounds( diffusionSlider.getRight() + indent, top, potSize, potSize );
    decaySlider.setBounds( sizeSlider.getX(), sizeSlider.getBottom() + spacing, potSize, potSize );
    fbControlButton.setBounds( decaySlider.getX(), decaySlider.getBottom(), potSize, textHeight );
    
    lrHPFCutoffSlider.setBounds( sizeSlider.getRight()  + indent, top, potSize, potSize );
    lrLPFCutoffSlider.setBounds( lrHPFCutoffSlider.getX(), lrHPFCutoffSlider.getBottom() + spacing, potSize, potSize );
    
    modulationRateSlider.setBounds( lrLPFCutoffSlider.getRight()  + indent*2, top, potSize, potSize );
    modulationDepthSlider.setBounds( modulationRateSlider.getX(), modulationRateSlider.getBottom() + spacing, potSize, potSize );
    modulationTypeButton.setBounds( modulationDepthSlider.getX(), modulationDepthSlider.getBottom() + spacing, potSize, textHeight );
    
    shimLevelSlider.setBounds( modulationRateSlider.getRight() + indent*2, top, potSize, potSize );
    shimTranspositionSlider.setBounds( shimLevelSlider.getX(), shimLevelSlider.getBottom() + spacing, potSize, potSize );
    
    mixSlider.setBounds( shimLevelSlider.getRight() + indent*2, top*1.5, potSize, potSize );
    monoLowButton.setBounds( mixSlider.getX(), mixSlider.getBottom(), potSize, textHeight );
//    testButton.setBounds( monoLowButton.getX(), monoLowButton.getBottom(), potSize, textHeight );
    interpolationTypeBox.setBounds( monoLowButton.getX(), monoLowButton.getBottom(), potSize, textHeight );
    tooltipsToggle.setBounds( interpolationTypeBox.getX(), interpolationTypeBox.getBottom() + indent, potSize, textHeight );
    
    tooltipLabel.setBounds( 0, HEIGHT, getWidth(), textHeight*2.5 );
}


void Sjf_verbAudioProcessorEditor::timerCallback()
{
    sjf_setTooltipLabel( this, MAIN_TOOLTIP, tooltipLabel );
}
