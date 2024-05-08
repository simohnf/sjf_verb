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
    m_smootherMap.find( parameterIDs::idsenum::size )->second->reset( m_SR, 0.05 );
    
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
void sjf_verb< Sample >::process( juce::AudioBuffer< Sample >& buffer )
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
        for ( auto i = 0; i < channels; i++ )
            m_inSamps[ i ] = m_samples[ i ] = buffer.getSample( i, s );
        for ( auto i = channels; i < m_samples.size(); i++ )
            m_samples[ i ] = 0;
        m_dspWrap.processER( m_samples );
        for ( auto i = 0; i < channels; i++ )
            m_outSamps[ i ] = m_samples[ i ] * m_erLevel;
        m_dspWrap.processLR( m_samples );
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
                                            [ this, smootherPtr ]( Sample v ) { smootherPtr->setTargetValue( std::pow( v*0.01, 2 ) ); });
                smootherPtr->setCurrentAndTargetValue( std::pow( val*0.01, 2 ) );
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
                    } );
            m_dspWrap.setInterpolationType( parameterIDs::interpMap.find( val )->second );
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

    juce::NormalisableRange < float > preDelayRange( 1.0f, 1000.0f, 0.001f );
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

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
template< typename Sample >
unsigned sjf_verb< Sample >::DSP_wrapper::initialise( Sample sampleRate, int numberOfChannels )
{
    m_SR = sampleRate;
    NCHANNELS = numberOfChannels;
    
    initialiseEarlyDSP( m_SR );
    initialiseLateDSP( m_SR );
    
    m_modPhasor.setFrequency( m_modRate, m_SR );
    
    
    return std::max(std::max( NCHANNELS, rdd_NCHANNELS ), fdn_NCHANNELS );
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::initialiseEarlyDSP( Sample sampleRate )
{
    if ( m_rotDelDif != nullptr )
        m_rotDelDif->initialise( sampleRate, sampleRate*0.5 );
    if ( m_multiTap.size() != 0 )
        for ( auto& m  : m_multiTap )
            m->initialise( sampleRate );
    if ( m_seriesAP.size() != 0 )
        for ( auto& s : m_seriesAP )
            s->initialise( sampleRate );
    
    
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::initialiseLateDSP( Sample sampleRate )
{
    if ( m_fdn != nullptr )
        m_fdn->initialise( sampleRate*0.5, sampleRate*0.1, sampleRate );
    if ( m_apLoop != nullptr )
        m_apLoop->initialise( sampleRate );
}





//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::er_rdd( std::vector< Sample >& samples )
{
    for ( auto i = 0; i < rdd_NSTAGES; i++ )
        for ( auto j= 0; j < rdd_NCHANNELS; j++ )
            m_rotDelDif->setDelayTime( m_rdd_modulators[i][j].process( m_rotDelDif_DT[i][j], m_modPhase, m_modDepth, m_modDamp )*m_size, i, j );
    m_rotDelDif->setDamping( m_erDamp );
    m_rotDelDif->processInPlace( samples, m_interpType );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::er_mt( std::vector< Sample >& samples )
{
    for ( auto i = 0; i < NCHANNELS; i++ )
    {
        for ( auto j = 0; j < mt_NTAPS; j++ )
            m_multiTap[ i ]->setDelayTimeSamps( m_multiTap_DT[ i ][ j ] * m_size, j );
        samples[ i ] = m_multiTapDamp[ i ].process( m_multiTap[ i ]->process( samples[ i ], m_interpType ), m_erDamp );
    }
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::er_sap( std::vector< Sample >& samples )
{
    for ( auto i = 0; i < NCHANNELS; i++ )
    {
        for ( auto j = 0; j < sap_NSTAGES; j++ )
            m_seriesAP[ i ]->setDelayTime( m_sap_modulators[i][j].process( m_seriesAP_DT[i][j], m_modPhase, m_modDepth, m_modDamp )*m_size, j );
        m_seriesAP[ i ]->setDamping( m_erDamp );
        m_seriesAP[ i ]->setCoefs( m_earlyDiff );
        samples[ i ] = m_seriesAP[ i ]->process( samples[ i ], m_interpType );
    }
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::er_mtsap( std::vector< Sample >& samples )
{
    er_mt( samples );
    er_sap( samples );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::lr_fdn( std::vector< Sample >& samples )
{
    m_fdn->setDecayInMS( m_decay );
    m_fdn->setDamping( m_lrDamp );
    m_fdn->setDiffusion( m_lateDiff );
    for ( auto i = 0; i < fdn_NCHANNELS; i++ )
        m_fdn->setDelayTime( m_fdn_modulators[ i ].process( m_fdn_DT[ i ], m_modPhase, m_modDepth, m_modDamp )*m_size, i );
    m_fdn->processInPlace( samples, sjf::rev::fdn< Sample >::mixers::hadamard, m_interpType );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::lr_apLoop( std::vector< Sample >& samples )
{
    m_apLoop->setDamping( m_lrDamp );
    m_apLoop->setDiffusion( m_lateDiff );
    m_apLoop->setDecay( m_decay );
    for ( auto i = 0; i < apl_NSTAGES; i++ )
        for ( auto j = 0; j < apl_NAP_PERSTAGE+1; j++ )
            m_apLoop->setDelayTimeSamples( m_apLoop_modulators[i][j].process( m_apLoop_DT[i][j], m_modPhase, m_modDepth, m_modDamp )*m_size, i, j );
    m_apLoop->processInPlace( samples, m_interpType );
}


//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::setEarlyType( parameterIDs::earlyTypesEnum type, Sample sampleRate )
{
    switch ( type ) {
        case parameterIDs::earlyTypesEnum::rotDelDif :
            m_rotDelDif = std::make_unique< sjf::rev::rotDelDif< Sample > >( rdd_NCHANNELS, rdd_NSTAGES );
            m_multiTap.clear(); m_multiTap_DT.clear();
            m_multiTapDamp.clear();
            m_seriesAP.clear(); m_seriesAP_DT.clear();
            erFunc = [ this ]( std::vector< Sample >& samples ) { m_rotDelDif->processInPlace( samples, m_interpType ); };
            earlyReflectionsF = &sjf_verb< Sample >::DSP_wrapper::er_rdd;
            break;
        case parameterIDs::earlyTypesEnum::multitap :
            m_rotDelDif.reset(); m_rotDelDif_DT.clear();
            for( auto c = 0; c < NCHANNELS; c ++ )
                m_multiTap.push_back( std::make_unique< sjf::rev::multiTap< Sample > >( mt_NTAPS ) );
            m_multiTapDamp.resize( NCHANNELS );
            m_seriesAP.clear(); m_seriesAP_DT.clear();
            erFunc = [ this ]( std::vector< Sample >& samples )
            {
                for ( auto c = 0; c < NCHANNELS; c++ )
                    samples[ c ] = m_multiTap[ c ]->process( samples[ c ], m_interpType );
            };
            earlyReflectionsF = &sjf_verb< Sample >::DSP_wrapper::er_mt;
            break;
        case parameterIDs::earlyTypesEnum::seriesAP  :
            m_rotDelDif.reset(); m_rotDelDif_DT.clear();
            m_multiTap.clear(); m_multiTap_DT.clear();
            m_multiTapDamp.clear();
            for( auto c = 0; c < NCHANNELS; c++ )
                m_seriesAP.push_back( std::make_unique< sjf::rev::seriesAllpass< Sample > >( sap_NSTAGES ) );
            erFunc = [ this ]( std::vector< Sample >& samples )
            {
                for ( auto c = 0; c < NCHANNELS; c++ )
                    samples[ c ] = m_seriesAP[ c ]->process( samples[ c ], m_interpType );
            };
            earlyReflectionsF = &sjf_verb< Sample >::DSP_wrapper::er_sap;
            break;
        case parameterIDs::earlyTypesEnum::mt_sAP :
            m_rotDelDif.reset(); m_rotDelDif_DT.clear();
            for( auto c = 0; c < NCHANNELS; c ++ )
            {
                m_multiTap.push_back( std::make_unique< sjf::rev::multiTap< Sample > >( mt_NTAPS ) );
                m_seriesAP.push_back( std::make_unique< sjf::rev::seriesAllpass< Sample > >( sap_NSTAGES ) );
            }
            m_multiTapDamp.resize( NCHANNELS );
            erFunc = [ this ]( std::vector< Sample >& samples )
            {
                for ( auto c = 0; c < NCHANNELS; c++ )
                {
                    samples[ c ] = m_multiTap[ c ]->process( samples[ c ], m_interpType );
                    samples[ c ] = m_seriesAP[ c ]->process( samples[ c ], m_interpType );
                }
            };
            earlyReflectionsF = &sjf_verb< Sample >::DSP_wrapper::er_mtsap;
            break;
        default:
            erFunc = [ this ]( std::vector< Sample >& samples ) { return; };
            earlyReflectionsF = &sjf_verb< Sample >::DSP_wrapper::er_default;
            break;
    }
    initialiseEarlyDSP( sampleRate );
    

//    setDSPFunction();
    initialiseDelayTimes( sampleRate );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::setLateType( parameterIDs::lateTypesEnum type, Sample sampleRate )
{
    switch ( type ){
        case parameterIDs::lateTypesEnum::fdn :
            m_fdn = std::make_unique< sjf::rev::fdn< Sample > >( fdn_NCHANNELS );
            m_apLoop.reset(); m_apLoop_DT.clear();
            lrFunc = [ this ]( std::vector< Sample >& samples ) { m_fdn->processInPlace( samples, sjf::rev::fdn< Sample >::mixers::hadamard, m_interpType ); };
            lateReflectionsF = &sjf_verb< Sample >::DSP_wrapper::lr_fdn;
            break;
        case parameterIDs::lateTypesEnum::apLoop :
            m_fdn.reset(); m_fdn_DT.clear();
            m_apLoop = std::make_unique< sjf::rev::allpassLoop< Sample > >( apl_NSTAGES, apl_NAP_PERSTAGE );
            lrFunc = [ this ]( std::vector< Sample >& samples ) { m_apLoop->processInPlace( samples, m_interpType ); };
            lateReflectionsF = &sjf_verb< Sample >::DSP_wrapper::lr_apLoop;
            break;
        default:
            lrFunc = [ this ]( std::vector< Sample >& samples ) { return; };
            break;
    }
    initialiseLateDSP( sampleRate );
//    setDSPFunction();
    initialiseDelayTimes( sampleRate );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::setInterpolationType( parameterIDs::interpTypesEnum type )
{
    switch ( type ) {
        case parameterIDs::interpTypesEnum::none :
            m_interpType = 0;
            break;
        case parameterIDs::interpTypesEnum::linear :
            m_interpType = sjf_interpolators::interpolatorTypes::linear;
            break;
        case parameterIDs::interpTypesEnum::cubic :
            m_interpType = sjf_interpolators::interpolatorTypes::cubic;
            break;
        case parameterIDs::interpTypesEnum::pureData :
            m_interpType = sjf_interpolators::interpolatorTypes::pureData;
            break;
        case parameterIDs::interpTypesEnum::fourthOrder :
            m_interpType = sjf_interpolators::interpolatorTypes::fourthOrder;
            break;
        case parameterIDs::interpTypesEnum::godot :
            m_interpType = sjf_interpolators::interpolatorTypes::godot;
            break;
        case parameterIDs::interpTypesEnum::hermite :
            m_interpType = sjf_interpolators::interpolatorTypes::hermite;
            break;
        default:
            m_interpType = 0;
            break;
    }
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::initialiseDelayTimes( Sample sampleRate )
{
    auto randCount = 0;
    if( m_fdn )
    {
//        m_fdn_modulators.resize( fdn_NCHANNELS );
        static constexpr Sample MAXDELAYTIMEMS = 100;
        const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
        const auto maxDtAPSamps = maxDtSamps * 0.25;
        m_fdn_DT.resize( fdn_NCHANNELS * 2 );
        m_fdn_modulators.resize( fdn_NCHANNELS * 2 );
        m_fdn->initialise( maxDtSamps, maxDtAPSamps, maxDtSamps );
        auto moffset = 1.0 / static_cast< Sample >( fdn_NCHANNELS * 2 );
        for ( auto i = 0; i < fdn_NCHANNELS; i ++ )
        {
            m_fdn_DT[ i ] = ((m_vn[ randCount ] * 0.5) + 0.5 ) * maxDtSamps;
            m_fdn->setDelayTime( m_fdn_DT[ i ], i );
//            DO MODULATOR
            m_fdn_modulators[ i ].initialise( moffset * i, m_fdn_DT[ i ] );
            randCount++;
            m_fdn_DT[ i + fdn_NCHANNELS ] = ((m_vn[ randCount ] * 0.5) + 0.5 ) * maxDtAPSamps;
            m_fdn->setAPTime( m_fdn_DT[ i + fdn_NCHANNELS ], i );
//            DO MODULATOR
            m_fdn_modulators[ i + fdn_NCHANNELS ].initialise( moffset * ( i + fdn_NCHANNELS ), m_fdn_DT[ i + fdn_NCHANNELS ] );
            randCount++;
        }
    }
    if( m_apLoop )
    {
        static constexpr Sample MAXDELAYTIMEMS = 100;
        const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
        m_apLoop_DT.resize( apl_NSTAGES );
        m_apLoop->initialise( sampleRate );
        auto moffset = 1.0 / static_cast< Sample >( m_apLoop_DT.size() );
        auto offsetCount = 0;
        m_apLoop_modulators.resize( apl_NSTAGES );
        for ( auto i = 0; i < apl_NSTAGES; i++ )
        {
            m_apLoop_DT[ i ].resize( apl_NAP_PERSTAGE+1 );
            m_apLoop_modulators[ i ].resize( apl_NAP_PERSTAGE+1 );
            auto targetTime = ((m_vn[ randCount ] * 0.6) + 0.4 ) * maxDtSamps;
            m_apLoop_DT[ i ][ apl_NAP_PERSTAGE ] = targetTime; // decay time needs to be a little less than sum of preceeding APs
//            DO MODULATOR
            m_apLoop_modulators[ i ][ apl_NAP_PERSTAGE ].initialise( moffset*offsetCount, targetTime );
            m_apLoop->setDelayTimeSamples( targetTime, i, apl_NAP_PERSTAGE );
            randCount++;
            offsetCount++;
            auto approxAPTime = m_apLoop_DT[ i ][ apl_NAP_PERSTAGE ] / apl_NAP_PERSTAGE;
            for ( auto j = 0; j < apl_NAP_PERSTAGE; j++ )
            {
                auto APTime = ((m_vn[ randCount ] * 0.75) + 0.5 ) * approxAPTime;
                if ( j == apl_NAP_PERSTAGE-1)
                    while ( APTime <= targetTime )
                        APTime *= 1.5;
                m_apLoop_DT[ i ][ j ] = APTime;
                m_apLoop->setDelayTimeSamples( APTime, i, j );
                m_apLoop_modulators[ i ][ j ].initialise( moffset*offsetCount, APTime );
                targetTime -= APTime;
                randCount++;
                offsetCount++;
            }
        }
        
    }
    if( m_rotDelDif )
    {
        static constexpr Sample MAXDELAYTIMEMS = 150;
        const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
        m_rotDelDif->initialise( sampleRate, maxDtSamps*2 );
        
        m_rotDelDif_DT.resize( rdd_NSTAGES );
        m_rdd_modulators.resize( rdd_NSTAGES );
        
        Sample stageLen = maxDtSamps / ( std::pow( 2, rdd_NSTAGES ) - 1 );
        auto moffset = 1.0 / static_cast< Sample >( rdd_NSTAGES * rdd_NCHANNELS );
        std::vector< size_t > channelShuffle( rdd_NCHANNELS );
        for ( auto i = 0; i < rdd_NSTAGES; i++ )
        {
            /* SHUFFLE */
            std::iota( channelShuffle.begin(), channelShuffle.end(), 0 );
            for ( auto j = 0; j < rdd_NSTAGES-1; j++ )
                std::swap( channelShuffle[ j ], channelShuffle[ (static_cast< int >( m_vn[ ++randCount ] * ( rdd_NSTAGES - j ) ) + j) ] );
            m_rotDelDif->setRotationMatrix( channelShuffle, i );
            Sample chanLen = stageLen / NCHANNELS;
//            DO MODULATORS
            m_rdd_modulators[ i ].resize( rdd_NCHANNELS );
            m_rotDelDif_DT[ i ].resize( rdd_NCHANNELS );
            for ( auto j = 0; j < rdd_NCHANNELS; j++ )
            {
                /* POLARITY FLIPS */
                m_rotDelDif->setPolarityFlip( (m_vn[ ++randCount ] >= 0.7), i, j );
                
                /* VELVET NOISE DELAY TIMES */
                m_rotDelDif_DT[ i ][ j ] = ( m_vn[ ++randCount ] * chanLen ) + ( chanLen * j );
                m_rotDelDif->setDelayTime( m_rotDelDif_DT[ i ][ j ], i, j );
                m_rdd_modulators[ i ][ j ].initialise( moffset * (j + i*rdd_NCHANNELS), m_rotDelDif_DT[ i ][ j ] );;
            }
            stageLen *= 2.0;
        }
    }
    if( m_seriesAP.size() > 0 )
    {
        static constexpr Sample MAXDELAYTIMEMS = 10;
        const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
        m_seriesAP_DT.resize( NCHANNELS );
        auto moffset = 1.0 / static_cast< Sample >( NCHANNELS * sap_NSTAGES );
        m_sap_modulators.resize( NCHANNELS );
        for ( auto i = 0; i < NCHANNELS; i++ )
        {
            m_seriesAP[ i ]->initialise( static_cast<int>(maxDtSamps*2) );
            m_seriesAP_DT[ i ].resize( sap_NSTAGES );
            m_sap_modulators[ i ].resize( sap_NSTAGES );
            for ( auto j = 0; j < sap_NSTAGES; j++ )
            {
                m_seriesAP_DT[ i ][ j ] = m_vn[ ++randCount ] * maxDtSamps;
                m_seriesAP[ i ]->setDelayTime( m_seriesAP_DT[ i ][ j ], j );
                m_sap_modulators[ i ][ j ].initialise( moffset * ( i*sap_NSTAGES + j ), m_seriesAP_DT[ i ][ j ] );
            }
        }
    }
    if( m_multiTap.size() > 0 )
    {
        static constexpr Sample MAXDELAYTIMEMS = 250;
        const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
        m_multiTap_DT.resize( NCHANNELS );
        auto vnBand = maxDtSamps / mt_NTAPS;
        for ( auto i = 0; i < NCHANNELS; i++ )
        {
            m_multiTap[ i ]->initialise( static_cast< int >( maxDtSamps ) );
            m_multiTap_DT[ i ].resize( mt_NTAPS );
            for ( auto j = 0; j < mt_NTAPS; j++ )
            {
                m_multiTap_DT[ i ][ j ] = ( m_vn[ ++randCount ]*vnBand ) + ( vnBand * j );
                m_multiTap[ i ]->setDelayTimeSamps( m_multiTap_DT[ i ][ j ], j );
                
                auto g = static_cast< Sample > ( ( mt_NTAPS + 1 ) - j ) / static_cast< Sample > ( mt_NTAPS + 1 );
                g *= m_vn[ ++randCount ] < 0.666 ? g : -g; // exponential decay
                m_multiTap[ i ]->setGain( g, j );
                
            }
        }
    }
}



template class sjf_verb<  float >;
template class sjf_verb<  double >;

