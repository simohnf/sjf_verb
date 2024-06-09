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


void sjf_verb::initialise( Sample sampleRate, int samplesPerBlock, int numberOfChannels )
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


void sjf_verb::processBlock( juce::AudioBuffer< Sample >& buffer )
{
    m_paramHandler.triggerCallbacks();
    
    auto blockSize = buffer.getNumSamples();
    auto channels = buffer.getNumChannels();
    auto intetrnalChannels = m_revBuffer.getNumChannels();
    Sample erLev = 0, lrLev = 0, dryLev = 0, wetLev = 0;
    
    for ( auto i = 0; i < channels; i++ )
        m_revBuffer.addFrom( i, 0, buffer, i, 0, blockSize );
//        m_revBuffer.copyFrom( i, 0, buffer, i, 0, blockSize );
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


void sjf_verb::addParametersToHandler( juce::AudioProcessorValueTreeState &vts,  juce::Array<juce::AudioProcessorParameter*>& params )
{
    for ( auto& p : params )
    {
        auto val = sjf::juceStuff::getUnNormalisedParameterValue< Sample >( p );
        auto id = static_cast< juce::AudioProcessorParameterWithID* >( p )->getParameterID();
        id = id.substring( parameterIDs::mainName.length() );
        jassert( parameterIDs::id2ParamTypeEnum.find( id ) != parameterIDs::id2ParamTypeEnum.end() );
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//=======      GENERAL PARAMETERS      =========//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        if ( id == parameterIDs::mix )
        {
            m_paramHandler.addParameter(vts,
                                        p,
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
        else if ( id == parameterIDs::earlyReflectionLevel )
        {
            m_paramHandler.addParameter(vts,
                                        p,
                                        [this](Sample v) { m_erLevelSmoother.setTargetValue( (v>0?std::sqrt(v*0.01):0) ); });
            m_erLevelSmoother.setCurrentAndTargetValue( (val>0?std::sqrt(val*0.01):0) );
        }
        else if( id == parameterIDs::lateReflectionLevel )
        {
            m_paramHandler.addParameter(vts,
                                        p,
                                        [this](Sample v) { m_lrLevelSmoother.setTargetValue( (v>0?std::sqrt(v*0.01):0) ); });
            m_lrLevelSmoother.setCurrentAndTargetValue( (val>0?std::sqrt(val*0.01):0) );
        }
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//========      INPUT PARAMETERS      ==========//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        else if( id == parameterIDs::inputHPFCutoff )
        {
            m_paramHandler.addParameter( vts,
                                         p,
                                        [this](Sample v){m_inputProcessor.m_HPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
            m_inputProcessor.m_HPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));
        }
        else if( id == parameterIDs::inputLPFCutoff )
        {
            m_paramHandler.addParameter(vts,
                                        p,
                                        [this](Sample v){m_inputProcessor.m_LPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
            m_inputProcessor.m_LPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));
        }
        else if( id == parameterIDs::preDelay )
        {
            m_paramHandler.addParameter(vts, p, [this]( Sample v ) { m_inputProcessor.m_preDelaySmoother.setTargetValue( v * 0.001 * m_SR ); });
            m_inputProcessor.m_preDelaySmoother.setCurrentAndTargetValue( val * 0.001 * m_SR );
        }
        else if ( id == parameterIDs::reverse )
        {
            m_paramHandler.addParameter( vts, p, [this]( Sample v ) { m_inputProcessor.setReversed( static_cast< bool >( v ) ); } );
            m_inputProcessor.setReversed( static_cast< bool >( val ) );
        }
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//=======      EARLY&LATE PARAMETERS      ======//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        else if ( id == parameterIDs::size )
        {
            m_paramHandler.addParameter(vts, p, [this](Sample v)
                                        {
                v *= 0.01;
                v =  (v <= 0.5) ? std::pow( 2.0, -( 0.5 - v )*3.0 ) : std::pow( 2.0, v - 0.5 );
                m_earlyReflections.m_varHolder.m_sizeSmoother.setTargetValue( v );
                m_lateReflections.m_varHolder.m_sizeSmoother.setTargetValue( v );
            });
            val *= 0.01;
            val = (val <= 0.5) ? std::pow( 2.0, -( 0.5 - val )*3.0 ) : std::pow( 2.0, val - 0.5 );
            m_earlyReflections.m_varHolder.m_sizeSmoother.setCurrentAndTargetValue( val );
            m_lateReflections.m_varHolder.m_sizeSmoother.setCurrentAndTargetValue( val );
        }
        else if ( id == parameterIDs::modDepth )
        {
            m_paramHandler.addParameter(vts, p, [this](Sample v)
                                        {
                auto depth = std::pow( v * 0.008, 2 );
                m_earlyReflections.m_varHolder.m_modDSmoother.setTargetValue( depth );
                m_lateReflections.m_varHolder.m_modDSmoother.setTargetValue( depth );
            });
            auto depth = std::pow( val * 0.008, 2 );
            m_earlyReflections.m_varHolder.m_modDSmoother.setCurrentAndTargetValue( depth );
            m_lateReflections.m_varHolder.m_modDSmoother.setCurrentAndTargetValue( depth );
        }
        else if ( id == parameterIDs::modRate )
        {
            m_paramHandler.addParameter(vts, p, [this](Sample v)
                                        {
                auto coef =  1.0 - calculateLPFCoefficient( v, m_SR );
                m_earlyReflections.m_varHolder.m_modRSmoother.setTargetValue( v );
                m_earlyReflections.m_varHolder.m_modDampSmoother.setTargetValue( coef );
                m_lateReflections.m_varHolder.m_modRSmoother.setTargetValue( v );
                m_lateReflections.m_varHolder.m_modDampSmoother.setTargetValue( coef );
            });
            auto coef = 1.0 - calculateLPFCoefficient( val, m_SR );
            m_earlyReflections.m_varHolder.m_modRSmoother.setCurrentAndTargetValue( val );
            m_earlyReflections.m_varHolder.m_modDampSmoother.setCurrentAndTargetValue( coef );
            m_lateReflections.m_varHolder.m_modRSmoother.setCurrentAndTargetValue( val );
            m_lateReflections.m_varHolder.m_modDampSmoother.setCurrentAndTargetValue( coef );
        }
        
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//========      EARLY PARAMETERS      ==========//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        else if ( id == parameterIDs::earlyReflectionType )
        {
            m_paramHandler.addParameter( vts, p, [this](Sample v)
                                        {
                jassert( parameterIDs::earlyTypeMap.find( v ) != parameterIDs::earlyTypeMap.end() );
                m_earlyReflections.setEarlyType( parameterIDs::earlyTypeMap.find( v )->second );
            } );
            jassert( parameterIDs::earlyTypeMap.find( val ) != parameterIDs::earlyTypeMap.end() );
            m_earlyReflections.setEarlyType( parameterIDs::earlyTypeMap.find( static_cast<int>( val ) )->second );
        }
        
        else if ( id == parameterIDs::earlyDiffusion )
        {
            m_paramHandler.addParameter(vts, p, [this]( Sample v ){ m_earlyReflections.m_varHolder.m_diffusionSmoother.setTargetValue( ( v * 0.006 ) + 0.2 ); });
            m_earlyReflections.m_varHolder.m_diffusionSmoother.setCurrentAndTargetValue( ( val * 0.006 ) + 0.2 );
        }
        else if ( id == parameterIDs::earlyHPFCutoff )
        {
            m_paramHandler.addParameter( vts,
                                         p,
                                        [this](Sample v){m_earlyReflections.m_HPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
            m_earlyReflections.m_HPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));
        }
        else if ( id == parameterIDs::earlyLPFCutoff )
        {
            m_paramHandler.addParameter( vts,
                                         p,
                                        [this](Sample v){m_earlyReflections.m_LPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
            m_earlyReflections.m_LPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));
        }
        
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//========      LATE PARAMETERS      ===========//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        else if ( id == parameterIDs::lateReflectionType )
        {
            m_paramHandler.addParameter( vts, p, [ this ]( Sample v )
                                        {
                jassert( parameterIDs::lateTypeMap.find( v ) != parameterIDs::lateTypeMap.end() );
                m_lateReflections.setLateType( parameterIDs::lateTypeMap.find( v )->second );
            } );
            jassert( parameterIDs::earlyTypeMap.find( val ) != parameterIDs::earlyTypeMap.end() );
            m_lateReflections.setLateType( parameterIDs::lateTypeMap.find( static_cast<int>( val ) )->second );
        }
        else if ( id == parameterIDs::lateDiffusion )
        {
            m_paramHandler.addParameter(vts, p, [this]( Sample v ) {
                m_lateReflections.m_varHolder.m_diffusionSmoother.setTargetValue( ( v * 0.006 ) + 0.2 ); } );
            m_lateReflections.m_varHolder.m_diffusionSmoother.setCurrentAndTargetValue( ( val * 0.006 ) + 0.2 );
        }
        else if( id == parameterIDs::lateHPFCutoff )
        {
            m_paramHandler.addParameter( vts,
                                         p,
                                        [this](Sample v) {m_lateReflections.m_varHolder.m_hpfSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
            m_lateReflections.m_varHolder.m_hpfSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));
        }
        else if( id == parameterIDs::lateLPFCutoff )
        {
            m_paramHandler.addParameter( vts,
                                         p,
                                        [this](Sample v) {m_lateReflections.m_varHolder.m_lpfSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
            m_lateReflections.m_varHolder.m_lpfSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));
        }
        else if ( id == parameterIDs::decay )
        {
            m_paramHandler.addParameter(vts, p, [this](Sample v) { m_lateReflections.m_varHolder.m_decaySmoother.setTargetValue( v * 1000.0 ); });
            m_lateReflections.m_varHolder.m_decaySmoother.setCurrentAndTargetValue( val * 1000.0 );
        }
        else if ( id == parameterIDs::fdnMixType )
        {
            m_paramHandler.addParameter(vts,
                                        p,
                                        [this](Sample v)
                                        {
                jassert( parameterIDs::fdnMixMap.find( v ) != parameterIDs::fdnMixMap.end() );
                m_lateReflections.m_varHolder.fdnMix = parameterIDs::fdnMixMap.find(v)->second;
            } );
            m_lateReflections.m_varHolder.fdnMix = parameterIDs::fdnMixMap.find( static_cast<int>( val ) )->second;
        }
        else if ( id == parameterIDs::feedbackLimit )
        {
            m_paramHandler.addParameter( vts, p, [ this ]( Sample v ) { m_lateReflections.m_varHolder.ControlFB =  static_cast< bool >( v ); } );
            m_lateReflections.m_varHolder.ControlFB =  static_cast< bool >( val );
        }
        
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//========      OUTPUT PARAMETERS      =========//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        //==============//==============//==============//==============//==============//==============//==============//==============
        else if ( id == parameterIDs::monoLow )
        {
            m_paramHandler.addParameter( vts, p, [ this ]( Sample v ) { m_outputProcessor.setMonoLow( static_cast< bool >( v ) ); } );
            m_outputProcessor.setMonoLow( static_cast< bool >( val ) );
        }
        
        else if( id == parameterIDs::shimmerLevel )
        {
            m_paramHandler.addParameter( vts, p, [this]( Sample v ) { m_outputProcessor.m_shimLevelSmoother.setTargetValue( std::pow( v*0.003, 2 ) ); } );
            m_outputProcessor.m_shimLevelSmoother.setCurrentAndTargetValue( std::pow( val*0.003, 2 ) );
        }
        
        else if( id == parameterIDs::shimmerTransposition )
        {
            m_paramHandler.addParameter( vts, p, [this]( Sample v ) {
                m_outputProcessor.m_shimShiftSmoother[0].setTargetValue( std::pow( 2.0, v/12.0 ) );
                m_outputProcessor.m_shimShiftSmoother[1].setTargetValue( std::pow( 2.0, -v/12.0 ) );
            } );
            m_outputProcessor.m_shimShiftSmoother[0].setTargetValue( std::pow( 2.0, val/12.0 ) );
            m_outputProcessor.m_shimShiftSmoother[1].setTargetValue( std::pow( 2.0, -val/12.0 ) );
        }
    }
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================


juce::AudioProcessorValueTreeState::ParameterLayout sjf_verb::createParameterLayout( )
{
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

    /*                                      INPUT                                      */
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::inputLPFCutoff, pIDVersionNumber }, parameterIDs::inputLPFCutoff, CutoffRange, 20000.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::inputHPFCutoff, pIDVersionNumber }, parameterIDs::inputHPFCutoff, CutoffRange, 20.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::preDelay, pIDVersionNumber }, parameterIDs::preDelay, preDelayRange, 20.0f) );
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::reverse, pIDVersionNumber }, parameterIDs::reverse, false) );

    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::size, pIDVersionNumber }, parameterIDs::size, 0.0f, 100.0f, 50.0f) );

    /*                                      EARLY                                      */
    params.add( std::make_unique<juce::AudioParameterChoice > (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyReflectionType, pIDVersionNumber }, parameterIDs::earlyReflectionType, parameterIDs::earlyTypes, 0) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyLPFCutoff, pIDVersionNumber }, parameterIDs::earlyLPFCutoff, CutoffRange, 20000.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyHPFCutoff, pIDVersionNumber }, parameterIDs::earlyHPFCutoff, CutoffRange, 20.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyDiffusion, pIDVersionNumber }, parameterIDs::earlyDiffusion, 0.0f, 100.0f, 80.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyReflectionLevel, pIDVersionNumber }, parameterIDs::earlyReflectionLevel, 0.0f, 100.0f, 80.0f) );
    

    /*                                      LATE                                      */
    params.add( std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateReflectionType, pIDVersionNumber }, parameterIDs::lateReflectionType, parameterIDs::lateTypes, 0 ) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateLPFCutoff, pIDVersionNumber }, parameterIDs::lateLPFCutoff, CutoffRange, 20000.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateHPFCutoff, pIDVersionNumber }, parameterIDs::lateHPFCutoff, CutoffRange, 10.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateDiffusion, pIDVersionNumber }, parameterIDs::lateDiffusion, 0.0f, 100.0f, 80.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::decay, pIDVersionNumber }, parameterIDs::decay, decayRange, 2.0f) );
    params.add( std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::fdnMixType, pIDVersionNumber }, parameterIDs::fdnMixType, parameterIDs::fdnMixTypes, 0 ) );
    
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::feedbackLimit, pIDVersionNumber }, parameterIDs::feedbackLimit, false) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateReflectionLevel, pIDVersionNumber }, parameterIDs::lateReflectionLevel, 0.0f, 100.0f, 80.0f) );
    

    
    /*                                      MODULATION                                      */
    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ parameterIDs::mainName + parameterIDs::modRate, pIDVersionNumber }, parameterIDs::modRate, modRateRange, 1.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::modDepth, pIDVersionNumber }, parameterIDs::modDepth, modDepthRange, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::modType, pIDVersionNumber }, parameterIDs::modType, parameterIDs::modTypes, 0 ) );

    
    /*                                      SHIMMER                                      */
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::shimmerLevel, pIDVersionNumber }, parameterIDs::shimmerLevel, 0.0f, 100.0f, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::shimmerTransposition, pIDVersionNumber }, parameterIDs::shimmerTransposition, -12.0f, 12.0f, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ parameterIDs::mainName + parameterIDs::shimmerDualVoice, pIDVersionNumber }, parameterIDs::shimmerDualVoice, false) );
    
    /*                                      OUTPUT ETC                                      */
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::monoLow, pIDVersionNumber }, parameterIDs::monoLow, false) );
    params.add( std::make_unique<juce::AudioParameterChoice > (juce::ParameterID{ parameterIDs::mainName + parameterIDs::interpolationType, pIDVersionNumber }, parameterIDs::interpolationType, parameterIDs::interpTypes, 1) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::mix, pIDVersionNumber }, parameterIDs::mix, 0.0f, 100.0f, 100.0f) );

//
    
    return params;
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

