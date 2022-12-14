/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_reverb.h" 

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
private:
    juce::AudioProcessorValueTreeState parameters;
    
    std::atomic<float>* mixParameter = nullptr;
    std::atomic<float>* sizeParameter = nullptr;
    std::atomic<float>* modulationParameter = nullptr;
    std::atomic<float>* decayParameter = nullptr;
    std::atomic<float>* lrCutoffParameter = nullptr;
    std::atomic<float>* erCutoffParameter = nullptr;
    std::atomic<float>* shimmerLevelParameter = nullptr;
    std::atomic<float>* shimmerTranspositionParameter = nullptr;
    std::atomic<float>* interpolationTypeParameter = nullptr;
    std::atomic<float>* feedbackControlParameter = nullptr;
    
    
    sjf_reverb rev;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_verbAudioProcessor)
};
