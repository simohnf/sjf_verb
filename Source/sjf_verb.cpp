//
//  sjf_verb.cpp
//  sjf_verb
//
//  Created by Simon Fay on 01/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//


#include "sjf_verb.h"
#include "../sjf_audio/sjf_audioUtilities.h"
#include "../sjf_audio/sjf_parameterHandler.h"
#include "../sjf_audio/sjf_rev.h"
#include "../sjf_audio/sjf_lpf.h"


//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< typename Sample >
void sjf_verb< Sample >::initialise( Sample sampleRate, int numberOfChannels )
{
    m_SR = sampleRate > 0 ? sampleRate : m_SR;
    for ( auto& s : m_smoothers )
    {
        s.reset( m_SR, 0.05 );
        s.setCurrentAndTargetValue( s.getTargetValue() );
    }
    
    m_inputProcessor.initialise( m_SR, numberOfChannels );
    
    auto nChans = m_dspWrap.initialise( m_SR, numberOfChannels );
    m_samples.resize( nChans, 0 );
    m_inSamps.resize( numberOfChannels, 0 );
    m_outSamps.resize( numberOfChannels, 0 );
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template < typename Sample >
inline void sjf_verb< Sample >::process( juce::AudioBuffer< Sample >& buffer )
{
    m_paramHandler.triggerCallbacks();
    
    auto blockSize = buffer.getNumSamples();
    auto channels = buffer.getNumChannels();
    Sample temp = 0.0;
    
    for ( auto s = 0; s < blockSize; s++ )
    {
        m_erLevel = getSmoothedVal( parameterIDs::idsenum::earlyReflectionLevel );
        m_dspWrap.m_earlyDiff = getSmoothedVal( parameterIDs::idsenum::earlyDiffusion );
        m_dspWrap.m_erDamp = getSmoothedVal( parameterIDs::idsenum::earlyLPFCutoff );
        
        m_lrLevel = getSmoothedVal( parameterIDs::idsenum::lateReflectionLevel );
        m_dspWrap.m_lrDamp = getSmoothedVal( parameterIDs::idsenum::lateLPFCutoff );
        m_dspWrap.m_lateDiff = getSmoothedVal( parameterIDs::idsenum::lateDiffusion );
        
        m_dspWrap.m_decay = getSmoothedVal( parameterIDs::idsenum::decay );
        m_dspWrap.m_size = getSmoothedVal( parameterIDs::idsenum::size );
        
        m_dry = getSmoothedVal( parameterIDs::idsenum::dry );
        m_wet = getSmoothedVal( parameterIDs::idsenum::wet );

        temp = m_dspWrap.m_modRate;
        m_dspWrap.m_modRate = getSmoothedVal(parameterIDs::idsenum::modRate );
        m_dspWrap.m_modDamp = temp != m_dspWrap.m_modRate ? ( 1.0 - calculateLPFCoefficient( m_dspWrap.m_modRate, m_SR ) ) : m_dspWrap.m_modDamp;
        m_dspWrap.m_modPhasor.setFrequency( m_dspWrap.m_modRate, m_SR );
        m_dspWrap.m_modPhase = m_dspWrap.m_modPhasor.process();
        m_dspWrap.m_modDepth = getSmoothedVal(parameterIDs::idsenum::modDepth );
        
        m_inputProcessor.m_preDelayTime = getSmoothedVal( parameterIDs::idsenum::preDelay );
        m_inputProcessor.m_inputLPFCutoff = getSmoothedVal( parameterIDs::idsenum::inputLPFCutoff );
        m_inputProcessor.m_inputHPFCutoff = getSmoothedVal( parameterIDs::idsenum::inputHPFCutoff );
        
        for ( auto i = 0; i < channels; i++ )
            m_inSamps[ i ] = m_samples[ i ] = buffer.getSample( i, s );
        for ( auto i = channels; i < m_samples.size(); i++ )
            m_samples[ i ] = 0;
        m_inputProcessor.process( m_samples );
        m_dspWrap.processEarly( m_samples );
        for ( auto i = 0; i < channels; i++ )
            m_outSamps[ i ] = m_samples[ i ] * m_erLevel;
        m_dspWrap.processLate( m_samples );
        for ( auto i = 0; i < channels; i++ )
            m_outSamps[ i ] += m_samples[ i ] * m_lrLevel;
        for ( auto i = 0; i < channels; i++ )
            buffer.setSample( i, s, m_outSamps[ i ]*m_wet + m_inSamps[ i ]*m_dry );
    }
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template < typename Sample >
void sjf_verb< Sample >::addParametersToHandler( juce::AudioProcessorValueTreeState &vts,  juce::Array<juce::AudioProcessorParameter*>& params )
{
    m_smoothers.reserve( params.size() ); // ensure vectore doesn't get moved... because then weird things happen!!!
    for ( auto& p : params )
    {
        // VAL CALCULATION NOW CORRECT BUT NEED TO MAKE SURE INITIAL VALUES ARE SCALES APPROPRIATELY!!!!
        auto val = sjf::juceStuff::getUnNormalisedParameterValue< Sample >( p );
        auto id = static_cast< juce::AudioProcessorParameterWithID* >( p )->getParameterID();
        id = id.substring( parameterIDs::mainName.length() );
        jassert( parameterIDs::id2ParamTypeEnum.find( id ) != parameterIDs::id2ParamTypeEnum.end() );
        if ( parameterIDs::id2ParamTypeEnum.find( id )->second == parameterIDs::paramType::FLOAT && id == parameterIDs::mix )
        {
            m_smoothers.emplace_back( juce::LinearSmoothedValue< Sample >() );
            auto* smootherPtr = &m_smoothers[ m_smoothers.size() - 1 ];
            smootherPtr->setCurrentAndTargetValue( val > 0.0 ? std::sqrt( val*0.01 ) : 0 );
            jassert( parameterIDs::id2enum.find( id ) != parameterIDs::id2enum.end() );
            m_smootherMap[ parameterIDs::idsenum::wet ] = smootherPtr;
            m_paramHandler.addParameter(vts,
                                        p,
                                        [ this, smootherPtr ]( Sample v )
                                        {
                                            v = v > 0 ? std::sqrt( v*0.01 ) : 0;
                                            smootherPtr->setTargetValue( v );
                                        });
            
            m_smoothers.emplace_back( juce::LinearSmoothedValue< Sample >() );
            auto* smootherPtr2 = &m_smoothers[ m_smoothers.size() - 1 ];
            smootherPtr2->setCurrentAndTargetValue( val < 100.0 ? std::sqrt( 1.0 - (val*0.01) ) : 0 );
            jassert( parameterIDs::id2enum.find( id ) != parameterIDs::id2enum.end() );
            m_smootherMap[ parameterIDs::idsenum::dry ] = smootherPtr2;
            m_paramHandler.addParameter(vts,
                                        p,
                                        [ this, smootherPtr2 ]( Sample v )
                                        {
                                            v = v < 100.0 ? std::sqrt( 1.0 - v*0.01 ) : 0;
                                            smootherPtr2->setTargetValue( v );
                                        });

        }
        else if ( parameterIDs::id2ParamTypeEnum.find( id )->second == parameterIDs::paramType::FLOAT )
        {
            m_smoothers.emplace_back( juce::LinearSmoothedValue< Sample >() );
            
            auto* smootherPtr = &m_smoothers[ m_smoothers.size() - 1 ];
            
            jassert( parameterIDs::id2enum.find( id ) != parameterIDs::id2enum.end() );
            m_smootherMap[ parameterIDs::id2enum.find(id)->second ] = smootherPtr;
            if ( id == parameterIDs::earlyHPFCutoff ||
                id == parameterIDs::earlyLPFCutoff ||
                id == parameterIDs::lateHPFCutoff ||
                id == parameterIDs::lateLPFCutoff ||
                id == parameterIDs::inputHPFCutoff ||
                id == parameterIDs::inputLPFCutoff )
            {
                m_paramHandler.addParameter(vts,
                                            p,
                                            [ this, smootherPtr ]( Sample v ){ smootherPtr->setTargetValue( 1.0 - calculateLPFCoefficient( v, m_SR ) );
                                                                        });
                smootherPtr->setCurrentAndTargetValue( 1.0 - calculateLPFCoefficient( val, m_SR ) );
            }
            else if ( id == parameterIDs::earlyDiffusion || id == parameterIDs::lateDiffusion )
            {
                m_paramHandler.addParameter(vts,
                                            p,
                                            [ this, smootherPtr ]( Sample v )
                                            { smootherPtr->setTargetValue( ( v * 0.007 ) + 0.2 ); });
                smootherPtr->setCurrentAndTargetValue( ( val * 0.007 ) + 0.2 );
            }
            else if ( id == parameterIDs::modDepth )
            {
                m_paramHandler.addParameter(vts,
                                            p,
                                            [ this, smootherPtr ]( Sample v ) { smootherPtr->setTargetValue( std::pow( v*0.008, 2 ) ); });
                smootherPtr->setCurrentAndTargetValue( std::pow( val*0.008, 2 ) );
            }
            else if ( id == parameterIDs::decay )
            {
                m_paramHandler.addParameter(vts,
                                            p,
                                            [ this, smootherPtr ]( Sample v ) { smootherPtr->setTargetValue( v * 1000.0 ); });
                smootherPtr->setCurrentAndTargetValue( val * 1000.0 );
            }
            else if ( id == parameterIDs::size )
            {
                m_paramHandler.addParameter(vts,
                                            p,
                                            [ this, smootherPtr ]( Sample v )
                                            {
                                                v *= 0.01;
                                                smootherPtr->setTargetValue( v <= 0.5 ? std::pow( 2.0, -( 0.5 - v )*3.0 ) : std::pow( 2.0, v - 0.5 ) ); });
                val *= 0.1;
                smootherPtr->setCurrentAndTargetValue( val <= 0.5 ? std::pow( 2.0, -( 0.5 - val )*3.0 ) : std::pow( 2.0, val - 0.5 ) );
                
            }
            else if ( id == parameterIDs::earlyReflectionLevel || id == parameterIDs::lateReflectionLevel )
            {
                m_paramHandler.addParameter(vts,
                                            p,
                                            [ this, smootherPtr ]( Sample v ) { smootherPtr->setTargetValue( std::sqrt( v*0.01 ) ); });
                smootherPtr->setCurrentAndTargetValue(  std::sqrt( val * 0.01 ) );
            }
            else if ( id == parameterIDs::preDelay )
            {
                m_paramHandler.addParameter(vts,
                                            p,
                                            [ this, smootherPtr ]( Sample v ) { smootherPtr->setTargetValue( v * 0.001 * m_SR ); });
                smootherPtr->setCurrentAndTargetValue( val * 0.001 * m_SR );
            }
            else
            {
                m_paramHandler.addParameter(vts,
                                            p,
                                            [ this, smootherPtr ]( Sample v ) { smootherPtr->setTargetValue( v ); });
                smootherPtr->setCurrentAndTargetValue( val );
            }
        }
        
        else if ( id == parameterIDs::interpolationType )
        {
            m_paramHandler.addParameter(vts,
                                        p, [ this ]( Sample v )
                                        {
                jassert ( parameterIDs::interpMap.find( v ) != parameterIDs::interpMap.end() );
                m_dspWrap.setInterpolationType( parameterIDs::interpMap.find( v )->second );
                m_inputProcessor.setInterpolationType( parameterIDs::interpMap.find( v )->second );
                    } );
            m_dspWrap.setInterpolationType( parameterIDs::interpMap.find( val )->second );
            m_inputProcessor.setInterpolationType( parameterIDs::interpMap.find( val )->second );
        }
        else if ( id == parameterIDs::earlyReflectionType )
        {
            m_paramHandler.addParameter( vts, p, [ this ]( Sample v )
                                        {
                jassert( parameterIDs::earlyTypeMap.find( v ) != parameterIDs::earlyTypeMap.end() );
                auto type = parameterIDs::earlyTypeMap.find( v )->second;
                m_dspWrap.setEarlyType( type,  m_SR );
            }
                                        );
            m_dspWrap.setEarlyType( parameterIDs::earlyTypeMap.find( static_cast<int>( val ) )->second, m_SR );
        }
        else if ( id == parameterIDs::lateReflectionType )
        {
            m_paramHandler.addParameter( vts, p, [ this ]( Sample v )
                                        {
                jassert( parameterIDs::lateTypeMap.find( v ) != parameterIDs::lateTypeMap.end() );
                m_dspWrap.setLateType( parameterIDs::lateTypeMap.find( v )->second, m_SR );
            } );
            m_dspWrap.setLateType( parameterIDs::lateTypeMap.find( static_cast<int>( val ) )->second, m_SR );
        }
        else if ( id == parameterIDs::fdnMixType )
        {
            m_paramHandler.addParameter( vts, p, [ this ]( Sample v )
                                        {
                jassert( parameterIDs::fdnMixMap.find( v ) != parameterIDs::fdnMixMap.end() );
                m_dspWrap.setFdnMixType( parameterIDs::fdnMixMap.find(v)->second );
            } );
            m_dspWrap.setLateType( parameterIDs::lateTypeMap.find( static_cast<int>( val ) )->second, m_SR );
        }
        else if ( id == parameterIDs::reverse )
        {
            m_paramHandler.addParameter( vts, p, [ this ]( Sample v )
                                        {
                m_inputProcessor.reverse( static_cast< bool >( v ) );
            } );
            m_inputProcessor.reverse( static_cast< bool >( val ) );
        }
    }
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template < typename Sample >
juce::AudioProcessorValueTreeState::ParameterLayout sjf_verb< Sample >::createParameterLayout( )
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
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::shimmerTransposition, pIDVersionNumber }, parameterIDs::shimmerTransposition, -12.0f, 12.0f, 12.0f) );

    
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



template class sjf_verb<  float >;
template class sjf_verb<  double >;

