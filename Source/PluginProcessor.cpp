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
    inputLPFCutoffParameter = parameters.getRawParameterValue("inputLPFCutoff"); 
    inputHPFCutoffParameter = parameters.getRawParameterValue("inputHPFCutoff");
    shimmerLevelParameter = parameters.getRawParameterValue("shimmerLevel");
    shimmerTranspositionParameter = parameters.getRawParameterValue("shimmerTransposition");
    interpolationTypeParameter = parameters.getRawParameterValue("interpolationType");
    feedbackDriveParameter = parameters.getRawParameterValue("feedbackDrive");
    monoLowParameter = parameters.getRawParameterValue("monoLow");
    earlyReflectionTypeParameter = parameters.getRawParameterValue("earlyReflectionType");
    
    setReverbParameters();
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
    
    setReverbParameters();
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
    
    setReverbParameters();
    
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
void Sjf_verbAudioProcessor::setReverbParameters()
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
    rev.setinputLPFCutoff( calculateLPFCoefficient< float >( *inputLPFCutoffParameter, sampleRate ) );
    rev.setinputHPFCutoff( calculateLPFCoefficient< float >( *inputHPFCutoffParameter, sampleRate ) ); 
    rev.setShimmerLevel( *shimmerLevelParameter );
    rev.setShimmerTransposition( *shimmerTranspositionParameter );
    rev.setInterpolationType( *interpolationTypeParameter );
    rev.setfeedbackDrive( *feedbackDriveParameter );
    rev.setMonoLow( *monoLowParameter );
    rev.setEarlyReflectionType( *earlyReflectionTypeParameter );
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
    
    static constexpr int pIDVersionNumber = 1;
    
    juce::NormalisableRange < float > preDelayRange( 1.0f, 1000.0f, 0.001f );
    preDelayRange.setSkewForCentre(50);
    juce::NormalisableRange < float > CutoffRange( 20.0f, 20000.0f, 0.001f );
    CutoffRange.setSkewForCentre( 1000.0f );
    juce::NormalisableRange < float > modRateRange( 0.0001f, 100.0f, 0.0001f );
    modRateRange.setSkewForCentre( 1.0f );
    
    juce::NormalisableRange < float > modDepthRange( 0.00f, 100.0f, 0.001f );
    modDepthRange.setSkewForCentre( 10.0f );
    
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "inputLPFCutoff", pIDVersionNumber }, "InputLPFCutoff", CutoffRange, 20000.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "inputHPFCutoff", pIDVersionNumber }, "InputHPFCutoff", CutoffRange, 20.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "preDelay", pIDVersionNumber }, "PreDelay", preDelayRange, 20.0f) );
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ "reverse", pIDVersionNumber }, "Reverse", false) );
    
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "size", pIDVersionNumber }, "Size", 0.0f, 100.0f, 80.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "diffusion", pIDVersionNumber }, "Diffusion", 0.0f, 100.0f, 80.0f) );
    params.add( std::make_unique<juce::AudioParameterInt> (juce::ParameterID{ "earlyReflectionType", pIDVersionNumber }, "EarlyReflectionType", 1, 4, 1) );
    
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "decay", pIDVersionNumber }, "Decay", 0.0f, 100.0f, 80.0f) );
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ "feedbackDrive", pIDVersionNumber }, "feedbackDrive", false) );
    
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "lrLPFCutoff", pIDVersionNumber }, "LrLPFCutoff", CutoffRange, 20000.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "lrHPFCutoff", pIDVersionNumber }, "LrHPFCutoff", CutoffRange, 10.0f) );
    
    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ "modulationRate", pIDVersionNumber }, "ModulationRate", modRateRange, 1.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "modulationDepth", pIDVersionNumber }, "ModulationDepth", modDepthRange, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ "modulationType", pIDVersionNumber }, "ModulationType", false) );
    
    
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "shimmerLevel", pIDVersionNumber }, "ShimmerLevel", 0.0f, 100.0f, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "shimmerTransposition", pIDVersionNumber }, "ShimmerTransposition", -12.0f, 12.0f, 12.0f) );
    
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ "monoLow", pIDVersionNumber }, "MonoLow", false) );
    params.add( std::make_unique<juce::AudioParameterInt> (juce::ParameterID{ "interpolationType", pIDVersionNumber }, "InterpolationType", 1, 6, 1) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "mix", pIDVersionNumber }, "Mix", 0.0f, 100.0f, 100.0f) );
    
    
    

    
    return params;
}
//==============================================================================
