/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Sjf_verbAudioProcessor::Sjf_verbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
, parameters( *this, nullptr, juce::Identifier("sjf_verb"), createParameterLayout() )
{
    rev.initialise( getSampleRate(), getTotalNumInputChannels(), getTotalNumOutputChannels(), getBlockSize() ); 
    
    mixParameter = parameters.getRawParameterValue("mix");
    preDelayParameter = parameters.getRawParameterValue("preDelay");
    reverseParameter = parameters.getRawParameterValue("reverse");
    sizeParameter = parameters.getRawParameterValue("size");
    diffusionParameter = parameters.getRawParameterValue("diffusion");
    modulationDepthParameter = parameters.getRawParameterValue("modulationDepth");
    modulationRateParameter = parameters.getRawParameterValue("modulationRate");
    modulationTypeParameter = parameters.getRawParameterValue("modulationType");
    decayParameter = parameters.getRawParameterValue("decay");
    lrLPFParameter = parameters.getRawParameterValue("lrLPFCutoff");
    lrHPFParameter =  parameters.getRawParameterValue("lrHPFCutoff");
    erLPFCutoffParameter = parameters.getRawParameterValue("erLPFCutoff");
    erHPFCutoffParameter = parameters.getRawParameterValue("erHPFCutoff");
    shimmerLevelParameter = parameters.getRawParameterValue("shimmerLevel");
    shimmerTranspositionParameter = parameters.getRawParameterValue("shimmerTransposition");
    interpolationTypeParameter = parameters.getRawParameterValue("interpolationType");
    feedbackControlParameter = parameters.getRawParameterValue("feedbackControl");
    monoLowParameter = parameters.getRawParameterValue("monoLow");
    
    setParameters();
}

Sjf_verbAudioProcessor::~Sjf_verbAudioProcessor()
{
}

//==============================================================================
const juce::String Sjf_verbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Sjf_verbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Sjf_verbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Sjf_verbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Sjf_verbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Sjf_verbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Sjf_verbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Sjf_verbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Sjf_verbAudioProcessor::getProgramName (int index)
{
    return {};
}

void Sjf_verbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Sjf_verbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    rev.initialise( sampleRate, getTotalNumInputChannels(), getTotalNumOutputChannels(), samplesPerBlock);
    
    setParameters();
}

void Sjf_verbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Sjf_verbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Sjf_verbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
    
    setParameters();
    rev.processAudio( buffer );
}

//==============================================================================
bool Sjf_verbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Sjf_verbAudioProcessor::createEditor()
{
    return new Sjf_verbAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void Sjf_verbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void Sjf_verbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
}
//==============================================================================
void Sjf_verbAudioProcessor::setParameters()
{
    auto sampleRate = getSampleRate();
    rev.setSize( *sizeParameter );
    rev.setDiffusion( *diffusionParameter );
    rev.setPreDelay( *preDelayParameter * 0.001 * sampleRate );
    rev.reversePredelay( *reverseParameter );
    rev.setModulationRate( *modulationRateParameter ); 
    rev.setModulationDepth( *modulationDepthParameter );
    rev.setModulationType( *modulationTypeParameter );
    rev.setDecay( *decayParameter );
    rev.setMix( *mixParameter );
    rev.setLrLPFCutoff( calculateLPFCoefficient< float >( *lrLPFParameter, sampleRate ) );
    rev.setLrHPFCutoff( calculateLPFCoefficient< float >( *lrHPFParameter, sampleRate ) );
    rev.setErLPFCutoff( calculateLPFCoefficient< float >( *erLPFCutoffParameter, sampleRate ) );
    rev.setErHPFCutoff( calculateLPFCoefficient< float >( *erHPFCutoffParameter, sampleRate ) );
    rev.setShimmerLevel( *shimmerLevelParameter );
    rev.setShimmerTransposition( *shimmerTranspositionParameter );
    rev.setInterpolationType( *interpolationTypeParameter );
    rev.setFeedbackControl( *feedbackControlParameter );
    rev.setMonoLow( *monoLowParameter );
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sjf_verbAudioProcessor();
}


//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout Sjf_verbAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;
    
    juce::NormalisableRange < float > CutoffRange( 20.0f, 20000.0f, 0.001f );
    CutoffRange.setSkewForCentre( 1000.0f );
    
    params.add( std::make_unique<juce::AudioParameterFloat> ("erLPFCutoff", "ErLPFCutoff", CutoffRange, 20000.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> ("erHPFCutoff", "ErHPFCutoff", CutoffRange, 20.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> ("preDelay", "PreDelay", 1.0f, 100.0f, 20.0f) );
    params.add( std::make_unique<juce::AudioParameterBool> ("reverse", "Reverse", false) );
    
    juce::NormalisableRange < float > modRateRange( 0.0001f, 100.0f, 0.0001f );
    modRateRange.setSkewForCentre( 1.0f );
    params.add( std::make_unique<juce::AudioParameterFloat> ("modulationRate", "ModulationRate", modRateRange, 1.0f) );
    juce::NormalisableRange < float > modDepthRange( 0.00f, 100.0f, 0.001f );
    modDepthRange.setSkewForCentre( 10.0f );
    params.add( std::make_unique<juce::AudioParameterFloat> ("modulationDepth", "ModulationDepth", modDepthRange, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterBool> ("modulationType", "ModulationType", false) );
    
    params.add( std::make_unique<juce::AudioParameterFloat> ("size", "Size", 0.0f, 100.0f, 80.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> ("diffusion", "Diffusion", 0.0f, 100.0f, 80.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> ("decay", "Decay", 0.0f, 100.0f, 80.0f) );
    params.add( std::make_unique<juce::AudioParameterBool> ("feedbackControl", "FeedbackControl", false) );
    
    params.add( std::make_unique<juce::AudioParameterBool> ("monoLow", "MonoLow", false) );
    params.add( std::make_unique<juce::AudioParameterFloat> ("lrLPFCutoff", "LrLPFCutoff", CutoffRange, 20000.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> ("lrHPFCutoff", "lrHPFCutoff", CutoffRange, 10.0f) );
    
    params.add( std::make_unique<juce::AudioParameterFloat> ("shimmerLevel", "ShimmerLevel", 0.0f, 100.0f, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> ("shimmerTransposition", "ShimmerTransposition", -12.0f, 12.0f, 12.0f) );
    
    params.add( std::make_unique<juce::AudioParameterFloat> ("mix", "Mix", 0.0f, 100.0f, 100.0f) );
    params.add( std::make_unique<juce::AudioParameterInt> ("interpolationType", "InterpolationType", 1, 6, 1) );

    
    
    return params;
}
//==============================================================================
