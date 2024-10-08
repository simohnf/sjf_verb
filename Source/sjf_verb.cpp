//
//  sjf_verb.cpp
//  sjf_verb
//
//  Created by Simon Fay on 01/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//


#include "sjf_verb.h"
#include "../sjf_audio/sjf_audioUtilitiesC++.h"
#include "../sjf_audio/sjf_parameterHandler.h"
#include "../sjf_audio/sjf_rev.h"


//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< sjf::interpolation::interpolatorTypes interpType >
void sjf_verb<interpType>::initialise( Sample sampleRate, int samplesPerBlock, int numberOfChannels )
{
    m_SR = sampleRate > 0 ? sampleRate : m_SR;
    m_erLevelSmoother.reset( m_SR, 0.05 );
    m_lrLevelSmoother.reset( m_SR, 0.05 );
    m_drySmoother.reset( m_SR, 0.05 );
    m_wetSmoother.reset( m_SR, 0.05 );
    
    m_inputProcessor.initialise( m_SR, numberOfChannels );
    auto nInternalChannels = m_earlyReflections.initialise( m_SR, numberOfChannels);
    nInternalChannels = std::max( nInternalChannels, m_lateReflections.initialise( m_SR, numberOfChannels ) );
    m_outputProcessor.initialise( m_SR, numberOfChannels );
    m_revBuffer.setSize( static_cast<int>(nInternalChannels), samplesPerBlock );
    m_revBuffer.clear();
    m_outputBuffer.setSize( static_cast<int>(nInternalChannels), samplesPerBlock );
    m_outputBuffer.clear();
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< sjf::interpolation::interpolatorTypes interpType >
void sjf_verb<interpType>::processBlock( juce::AudioBuffer< Sample >& buffer )
{
    m_paramHandler.triggerCallbacks();
    
    auto blockSize = buffer.getNumSamples();
    auto channels = buffer.getNumChannels();
    auto intetrnalChannels = m_revBuffer.getNumChannels();
    Sample erLev = 0, lrLev = 0, dryLev = 0, wetLev = 0;
    
    for ( auto i = 0; i < channels; i++ )
        m_revBuffer.addFrom( i, 0, buffer, i, 0, blockSize );
    for ( auto i = channels; i < intetrnalChannels; i++ )
        m_revBuffer.clear( i, 0, m_revBuffer.getNumSamples() );
    m_inputProcessor.processBlock( m_revBuffer, blockSize );
    m_earlyReflections.processBlock( m_revBuffer, blockSize );
    m_outputBuffer.makeCopyOf( m_revBuffer );
        
    m_lateReflections.processBlock( m_revBuffer, blockSize );
    
    for ( auto i = 0; i < blockSize; i++ )
    {
        erLev = m_erLevelSmoother.getNextValue();
        lrLev = m_lrLevelSmoother.getNextValue();
        for ( auto c = 0; c < channels; c++ )
            m_outputBuffer.setSample( c, i, m_outputBuffer.getSample( c, i )*erLev + m_revBuffer.getSample( c, i )*lrLev );
    }

    m_outputProcessor.processBlock( m_outputBuffer, m_revBuffer, blockSize );
    
    for ( auto i = 0; i < blockSize; i++ )
    {
        dryLev = m_drySmoother.getNextValue();
        wetLev = m_wetSmoother.getNextValue();
        for ( auto c = 0; c < channels; c++ )
            buffer.setSample( c, i, m_outputBuffer.getSample( c, i )*wetLev + buffer.getSample( c, i )*dryLev );
    }
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< sjf::interpolation::interpolatorTypes interpType >
void sjf_verb<interpType>::addParametersToHandler( )
{
    
    auto p = m_vts.getParameter( parameterIDs::mainName + parameterIDs::earlyReflectionLevel );
    auto val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter(p, [this](Sample v) { m_erLevelSmoother.setTargetValue( (v>0?std::sqrt(v*0.01):0) ); });
    m_erLevelSmoother.setCurrentAndTargetValue( (val>0?std::sqrt(val*0.01):0) );


    p = m_vts.getParameter( parameterIDs::mainName + parameterIDs::lateReflectionLevel );
    val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter(p, [this](Sample v) { m_lrLevelSmoother.setTargetValue( (v>0?std::sqrt(v*0.01):0) ); });
    m_lrLevelSmoother.setCurrentAndTargetValue( (val>0?std::sqrt(val*0.01):0) );

    
    p = m_vts.getParameter( parameterIDs::mainName + parameterIDs::mix );
    val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter( p,
                                [this](Sample v)
                                {
                                    v *= 0.01;
                                    auto wet = v > 0 ? std::sqrt( v ) : 0;
                                    m_wetSmoother.setTargetValue( wet );
                                    auto dry = 1.0 - v;
                                    dry = dry > 0 ? std::sqrt( dry ) : 0;
                                    m_drySmoother.setTargetValue( dry );
                                });
    val *= 0.01;
    auto wet = val > 0 ? std::sqrt( val ) : 0;
    m_wetSmoother.setTargetValue( wet );
    auto dry = 1.0 - val;
    dry = dry > 0 ? std::sqrt( dry ) : 0;
    m_drySmoother.setTargetValue( dry );
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< sjf::interpolation::interpolatorTypes interpType >
juce::AudioProcessorValueTreeState::ParameterLayout sjf_verb<interpType>::createParameterLayout( )
{
    using att = juce::AudioParameterFloatAttributes;
    juce::AudioProcessorValueTreeState::ParameterLayout params;
    
    static constexpr int pIDVersionNumber = 1;

    juce::NormalisableRange < float > preDelayRange( 0.0f, 1000.0f, 0.001f );
    preDelayRange.setSkewForCentre(50);
    juce::NormalisableRange < float > CutoffRange( 20.0f, 20000.0f, 0.001f );
    CutoffRange.setSkewForCentre( 1000.0f );
    juce::NormalisableRange < float > modRateRange( 0.0001f, 100.0f, 0.0001f );
    modRateRange.setSkewForCentre( 1.0f );
    juce::NormalisableRange < float > modDepthRange( 0.00f, 100.0f, 0.001f );
    modDepthRange.setSkewForCentre( 10.0f );
    juce::NormalisableRange < float > decayRange( 0.01f, 100.0f, 0.001f );
    decayRange.setSkewForCentre(5);
    juce::NormalisableRange < float > percentRange( 0.0f, 100.0f, 0.001f );
    juce::NormalisableRange < float > semiToneRange( -12.0f, 12.0f, 0.001f );
    
    auto msAttributes = att().withStringFromValueFunction ([] (auto x, auto) { return juce::String (x); }).withLabel("ms");
    auto sAttributes = att().withStringFromValueFunction ([] (auto x, auto) { return juce::String (x); }).withLabel("s");
    auto percentAttributes = att().withStringFromValueFunction ([] (auto x, auto) { return juce::String (x); }).withLabel("%");
    auto hzAttributes = att().withStringFromValueFunction ([] (auto x, auto) { return juce::String (x); }).withLabel("Hz");
    auto stAttributes = att().withStringFromValueFunction ([] (auto x, auto) { return juce::String (x); }).withLabel("st");
    
    /*                                      INPUT                                      */
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::inputLPFCutoff, pIDVersionNumber }, parameterIDs::inputLPFCutoff, CutoffRange, 20000.0f, hzAttributes) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::inputHPFCutoff, pIDVersionNumber }, parameterIDs::inputHPFCutoff, CutoffRange, 20.0f, hzAttributes) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::preDelay, pIDVersionNumber }, parameterIDs::preDelay, preDelayRange, 20.0f, msAttributes) );
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::reverse, pIDVersionNumber }, parameterIDs::reverse, false) );

    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::size, pIDVersionNumber }, parameterIDs::size, percentRange, 50.0f, percentAttributes) );

    /*                                      EARLY                                      */
    params.add( std::make_unique<juce::AudioParameterChoice > (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyReflectionType, pIDVersionNumber }, parameterIDs::earlyReflectionType, parameterIDs::earlyTypes, 0) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyLPFCutoff, pIDVersionNumber }, parameterIDs::earlyLPFCutoff, CutoffRange, 20000.0f, hzAttributes) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyHPFCutoff, pIDVersionNumber }, parameterIDs::earlyHPFCutoff, CutoffRange, 20.0f, hzAttributes) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyDiffusion, pIDVersionNumber }, parameterIDs::earlyDiffusion, percentRange, 80.0f, percentAttributes) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyReflectionLevel, pIDVersionNumber }, parameterIDs::earlyReflectionLevel, percentRange, 80.0f, percentAttributes) );
    

    /*                                      LATE                                      */
    params.add( std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateReflectionType, pIDVersionNumber }, parameterIDs::lateReflectionType, parameterIDs::lateTypes, 0 ) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateLPFCutoff, pIDVersionNumber }, parameterIDs::lateLPFCutoff, CutoffRange, 20000.0f, hzAttributes) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateHPFCutoff, pIDVersionNumber }, parameterIDs::lateHPFCutoff, CutoffRange, 20.0f, hzAttributes) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateDiffusion, pIDVersionNumber }, parameterIDs::lateDiffusion, percentRange, 80.0f, percentAttributes) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::decay, pIDVersionNumber }, parameterIDs::decay, decayRange, 2.0f, sAttributes) );
    params.add( std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::fdnMixType, pIDVersionNumber }, parameterIDs::fdnMixType, parameterIDs::fdnMixTypes, 0 ) );
    
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::feedbackLimit, pIDVersionNumber }, parameterIDs::feedbackLimit, false) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateReflectionLevel, pIDVersionNumber }, parameterIDs::lateReflectionLevel, percentRange, 80.0f, percentAttributes) );
    

    
    /*                                      MODULATION                                      */
    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ parameterIDs::mainName + parameterIDs::modRate, pIDVersionNumber }, parameterIDs::modRate, modRateRange, 1.0f, hzAttributes) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::modDepth, pIDVersionNumber }, parameterIDs::modDepth, modDepthRange, 0.0f, percentAttributes ) );
//    params.add( std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::modType, pIDVersionNumber }, parameterIDs::modType, parameterIDs::modTypes, 0 ) );

    
    /*                                      SHIMMER                                      */
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::shimmerLevel, pIDVersionNumber }, parameterIDs::shimmerLevel, percentRange, 0.0f, percentAttributes) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::shimmerTransposition, pIDVersionNumber }, parameterIDs::shimmerTransposition, semiToneRange, 0.0f, stAttributes) );
    params.add( std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ parameterIDs::mainName + parameterIDs::shimmerDualVoice, pIDVersionNumber }, parameterIDs::shimmerDualVoice, false) );
    
    /*                                      OUTPUT ETC                                      */
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::monoLow, pIDVersionNumber }, parameterIDs::monoLow, false) );
    params.add( std::make_unique<juce::AudioParameterChoice > (juce::ParameterID{ parameterIDs::mainName + parameterIDs::interpolationType, pIDVersionNumber }, parameterIDs::interpolationType, parameterIDs::interpTypes, 1) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::mix, pIDVersionNumber }, parameterIDs::mix, percentRange, 100.0f, percentAttributes) );
  
    return params;
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template class sjf_verb<sjf::interpolation::interpolatorTypes::none >;
template class sjf_verb<sjf::interpolation::interpolatorTypes::linear >;
template class sjf_verb<sjf::interpolation::interpolatorTypes::cubic >;
template class sjf_verb<sjf::interpolation::interpolatorTypes::pureData >;
template class sjf_verb<sjf::interpolation::interpolatorTypes::fourthOrder >;
template class sjf_verb<sjf::interpolation::interpolatorTypes::godot >;
template class sjf_verb<sjf::interpolation::interpolatorTypes::hermite >;
