/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
//#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_reverb.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_zitaRev.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_audioUtilities.h"

//==============================================================================
/**
*/
class Sjf_verbAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    Sjf_verbAudioProcessor();
    ~Sjf_verbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void setParameters();
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    void setRevType( const bool& trueForType2 )
    {
        m_revtype = trueForType2;
    }
    
private:
    sjf_zitaRev< float > rev;
    
    juce::AudioProcessorValueTreeState parameters;
    
    std::atomic<float>* mixParameter = nullptr;
    std::atomic<float>* preDelayParameter = nullptr;
    std::atomic<float>* reverseParameter = nullptr;
    std::atomic<float>* sizeParameter = nullptr;
    std::atomic<float>* diffusionParameter = nullptr;
    std::atomic<float>* modulationRateParameter = nullptr;
    std::atomic<float>* modulationDepthParameter = nullptr;
    std::atomic<float>* modulationTypeParameter = nullptr;
    std::atomic<float>* decayParameter = nullptr;
    std::atomic<float>* lrHPFParameter = nullptr;
    std::atomic<float>* lrLPFParameter = nullptr;
    std::atomic<float>* erLPFCutoffParameter = nullptr;
    std::atomic<float>* erHPFCutoffParameter = nullptr;
    std::atomic<float>* shimmerLevelParameter = nullptr;
    std::atomic<float>* shimmerTranspositionParameter = nullptr;
    std::atomic<float>* interpolationTypeParameter = nullptr;
    std::atomic<float>* feedbackControlParameter = nullptr;
    std::atomic<float>* monoLowParameter = nullptr;
    
    bool m_revtype = false;
    
//    sjf_reverb< float, 8, 4 > rev;
//    sjf_zitaRev< float > rev;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_verbAudioProcessor)
};
