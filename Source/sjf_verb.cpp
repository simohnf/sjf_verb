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
        s.reset( m_SR, 0.001 );
        s.setCurrentAndTargetValue( s.getTargetValue() );
    }
    
    auto nChans = m_dspWrap.initialise( m_SR, numberOfChannels );
    m_samples.resize( nChans, 0 );
    
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
    for ( auto s = 0; s < blockSize; s++ )
    {
        // do smoothed value stuff here
        for ( auto c = 0; c < channels; c++ )
            m_samples[ c ] = buffer.getSample( c, s );
//        m_dspWrap.erFunc( m_samples );
//        m_dspWrap.lrFunc( m_samples );
        m_dspWrap.processER( m_samples );
        m_dspWrap.processLR( m_samples );
        for ( auto c = 0; c < channels; c++ )
            buffer.setSample( c, s, m_samples[ c ] );
    }
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template < typename Sample >
void sjf_verb< Sample >::addParametersToHandler( juce::AudioProcessorValueTreeState &vts,  juce::Array<juce::AudioProcessorParameter*>& params )
{
    for ( auto& p : params )
    {
        auto id = static_cast< juce::AudioProcessorParameterWithID* >( p )->getParameterID();
        id = id.substring( parameterIDs::mainName.length() );
        auto smootherCount = 0;
        jassert( parameterIDs::id2ParamTypeEnum.find( id ) != parameterIDs::id2ParamTypeEnum.end() );
        if ( parameterIDs::id2ParamTypeEnum.find( id )->second == parameterIDs::paramType::FLOAT )
        {
            m_smoothers.push_back( juce::LinearSmoothedValue< Sample >() );
            auto smootherPtr = &m_smoothers[ smootherCount ];
            smootherCount++;
            std::function< void(float) > func = [ this, smootherPtr ]( float v )
            {
                smootherPtr->setTargetValue( v );
            };
            jassert( parameterIDs::id2enum.find( id ) != parameterIDs::id2enum.end() );
            m_smootherMap[ parameterIDs::id2enum.find(id)->second ] = smootherPtr;
            m_paramHandler.addParameter( vts, p, [ this, smootherPtr ]( Sample v )
                                        { smootherPtr->setTargetValue( v ); } );
        }
        
        if ( id == parameterIDs::interpolationType )
            m_paramHandler.addParameter( vts, p, [ this ]( Sample v )
                                        {
//                                            m_interpType = v;
                jassert ( parameterIDs::interpMap.find( v ) != parameterIDs::interpMap.end() );
                m_dspWrap.setInterpolationType( parameterIDs::interpMap.find( v )->second );
                    } );
        if ( id == parameterIDs::earlyReflectionType )
            m_paramHandler.addParameter( vts, p, [ this ]( Sample v )
                                        {
                jassert( parameterIDs::earlyTypeMap.find( v ) != parameterIDs::earlyTypeMap.end() );
                auto type = parameterIDs::earlyTypeMap.find( v )->second;
                m_dspWrap.setEarlyType( type,  m_SR );
            }
                                        );
        if ( id == parameterIDs::lateReflectionType )
            m_paramHandler.addParameter( vts, p, [ this ]( Sample v )
                                        {
                jassert( parameterIDs::lateTypeMap.find( v ) != parameterIDs::lateTypeMap.end() );
                m_dspWrap.setLateType( parameterIDs::lateTypeMap.find( v )->second, m_SR );
            } );
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



    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::inputLPFCutoff, pIDVersionNumber }, parameterIDs::inputLPFCutoff, CutoffRange, 20000.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::inputHPFCutoff, pIDVersionNumber }, parameterIDs::inputHPFCutoff, CutoffRange, 20.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::preDelay, pIDVersionNumber }, parameterIDs::preDelay, preDelayRange, 20.0f) );
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::reverse, pIDVersionNumber }, parameterIDs::reverse, false) );

    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::size, pIDVersionNumber }, parameterIDs::size, 0.0f, 100.0f, 80.0f) );

    
    params.add( std::make_unique<juce::AudioParameterChoice > (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyReflectionType, pIDVersionNumber }, parameterIDs::earlyReflectionType, parameterIDs::earlyTypes, 0) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyLPFCutoff, pIDVersionNumber }, parameterIDs::earlyLPFCutoff, CutoffRange, 20000.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::earlyHPFCutoff, pIDVersionNumber }, parameterIDs::earlyHPFCutoff, CutoffRange, 20.0f) );

    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::diffusion, pIDVersionNumber }, parameterIDs::diffusion, 0.0f, 100.0f, 80.0f) );
    
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::decay, pIDVersionNumber }, parameterIDs::decay, 0.0f, 100.0f, 80.0f) );
    params.add( std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::feedbackLimit, pIDVersionNumber }, parameterIDs::feedbackLimit, false) );

    
    params.add( std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateReflectionType, pIDVersionNumber }, parameterIDs::lateReflectionType, parameterIDs::lateTypes, 0 ) );
    
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateLPFCutoff, pIDVersionNumber }, parameterIDs::lateLPFCutoff, CutoffRange, 20000.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::lateHPFCutoff, pIDVersionNumber }, parameterIDs::lateHPFCutoff, CutoffRange, 10.0f) );

    params.add( std::make_unique<juce::AudioParameterFloat> ( juce::ParameterID{ parameterIDs::mainName + parameterIDs::modRate, pIDVersionNumber }, parameterIDs::modRate, modRateRange, 1.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::modDepth, pIDVersionNumber }, parameterIDs::modDepth, modDepthRange, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::modType, pIDVersionNumber }, parameterIDs::modType, parameterIDs::modTypes, 0 ) );

    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::shimmerLevel, pIDVersionNumber }, parameterIDs::shimmerLevel, 0.0f, 100.0f, 0.0f) );
    params.add( std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ parameterIDs::mainName + parameterIDs::shimmerTransposition, pIDVersionNumber }, parameterIDs::shimmerTransposition, -12.0f, 12.0f, 12.0f) );

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
    initialiseEarlyDSP( sampleRate );
    initialiseLateDSP( sampleRate );
    
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
    m_rotDelDif->processInPlace( samples, m_interpType );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::er_mt( std::vector< Sample >& samples )
{
    for ( auto c = 0; c < NCHANNELS; c++ )
        samples[ c ] = m_multiTap[ c ]->process( samples[ c ], m_interpType );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::er_sap( std::vector< Sample >& samples )
{
    for ( auto c = 0; c < NCHANNELS; c++ )
        samples[ c ] = m_seriesAP[ c ]->process( samples[ c ], m_interpType );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::er_mtsap( std::vector< Sample >& samples )
{
    for ( auto c = 0; c < NCHANNELS; c++ )
        samples[ c ] = m_seriesAP[ c ]->process( m_multiTap[ c ]->process( samples[ c ], m_interpType ), m_interpType );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::lr_fdn( std::vector< Sample >& samples )
{
    m_fdn->processInPlace( samples, sjf::rev::fdn< Sample >::mixers::hadamard, m_interpType );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb< Sample >::DSP_wrapper::lr_apLoop( std::vector< Sample >& samples )
{
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
            m_multiTap.clear();
            m_seriesAP.clear();
            erFunc = [ this ]( std::vector< Sample >& samples ) { m_rotDelDif->processInPlace( samples, m_interpType ); };
            earlyReflectionsF = &sjf_verb< Sample >::DSP_wrapper::er_rdd;
            break;
        case parameterIDs::earlyTypesEnum::multitap :
            m_rotDelDif.reset();
            for( auto c = 0; c < NCHANNELS; c ++ )
                m_multiTap.push_back( std::make_unique< sjf::rev::multiTap< Sample > >( mt_NTAPS ) );
            m_seriesAP.clear();
            erFunc = [ this ]( std::vector< Sample >& samples )
            {
                for ( auto c = 0; c < NCHANNELS; c++ )
                    samples[ c ] = m_multiTap[ c ]->process( samples[ c ], m_interpType );
            };
            earlyReflectionsF = &sjf_verb< Sample >::DSP_wrapper::er_mt;
            break;
        case parameterIDs::earlyTypesEnum::seriesAP  :
            m_rotDelDif.reset();
            m_multiTap.clear();
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
            m_rotDelDif.reset();
            for( auto c = 0; c < NCHANNELS; c ++ )
            {
                m_multiTap.push_back( std::make_unique< sjf::rev::multiTap< Sample > >( mt_NTAPS ) );
                m_seriesAP.push_back( std::make_unique< sjf::rev::seriesAllpass< Sample > >( sap_NSTAGES ) );
            }
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
            m_apLoop.reset();
            lrFunc = [ this ]( std::vector< Sample >& samples ) { m_fdn->processInPlace( samples, sjf::rev::fdn< Sample >::mixers::hadamard, m_interpType ); };
            lateReflectionsF = &sjf_verb< Sample >::DSP_wrapper::lr_fdn;
            break;
        case parameterIDs::lateTypesEnum::apLoop :
            m_fdn.reset();
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
        m_fdn_dt.resize( fdn_NCHANNELS * 2 );
        m_fdn->initialise( maxDtSamps, maxDtAPSamps, maxDtSamps );
        auto moffset = 1.0 / static_cast< Sample >( fdn_NCHANNELS * 2 );
        for ( auto i = 0; i < fdn_NCHANNELS; i ++ )
        {
            m_fdn_dt[ i ] = ((m_vn[ randCount ] * 0.5) + 0.5 ) * maxDtSamps;
            m_fdn->setDelayTime( m_fdn_dt[ i ], i );
//            DO MODULATOR
//            m_fdn_modulators[ i ].initialise( moffset * i, m_dtSamps[ i ] );
            randCount++;
            m_fdn_dt[ i + fdn_NCHANNELS ] = m_vn[ randCount ] * maxDtAPSamps;
            m_fdn->setAPTime( m_fdn_dt[ i + fdn_NCHANNELS ], i );
//            DO MODULATOR
//            m_fdn_modulators[ i + NCHANNELS ].initialise( moffset * (i+NCHANNELS), m_dtSampsAP[ i ] );
            randCount++;
        }
    }
    if( m_apLoop )
    {
        static constexpr Sample MAXDELAYTIMEMS = 100;
        const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
        m_apLoop_DT.resize( apl_NSTAGES * ( apl_NAP_PERSTAGE + 1 ) );
        m_apLoop->initialise( sampleRate );
        auto moffset = 1.0 / static_cast< Sample >( m_apLoop_DT.size() );
        auto offsetCount = 0;
//        m_apLoop_modulators.resize( apl_NSTAGES );
        for ( auto i = 0; i < apl_NSTAGES; i++ )
        {
//            m_apLoop_modulators[ i ].resize( apl_NAP_PERSTAGE+1 );
            auto stageStartIndex = i * ( apl_NAP_PERSTAGE + 1);
            auto delayIndex = stageStartIndex + apl_NAP_PERSTAGE;
            auto targetTime = ((m_vn[ randCount ] * 0.6) + 0.4 ) * maxDtSamps;
            m_apLoop_DT[ delayIndex ] = targetTime; // decay time needs to be a little less than sum of preceeding APs
//            DO MODULATOR
//            m_apLoop_modulators[ s ][ m ].initialise( moffset*offsetCount, targetTime );
            m_apLoop->setDelayTimeSamples( targetTime, i, apl_NAP_PERSTAGE );
            randCount++;
            offsetCount++;
            auto approxAPTime = m_apLoop_DT[ delayIndex ] / apl_NAP_PERSTAGE;
            for ( auto j = 0; j < apl_NAP_PERSTAGE; j++ )
            {
                auto APTime = ((m_vn[ randCount ] * 0.75) + 0.5 ) * approxAPTime;
                if ( j == apl_NAP_PERSTAGE-1)
                    while ( APTime <= targetTime )
                        APTime *= 1.5;
                m_apLoop_DT[ stageStartIndex + j ] = APTime;
                m_apLoop->setDelayTimeSamples( APTime, i, j );
//                m_apLoop_modulators[ s ][ m ].initialise( moffset*offsetCount, APTime );
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
//        m_rdd_modulators.resize( rdd_NSTAGES );
        
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
//            m_rdd_modulators[ i ].resize( rdd_NCHANNELS );
            m_rotDelDif_DT[ i ].resize( rdd_NCHANNELS );
            for ( auto j = 0; j < rdd_NCHANNELS; j++ )
            {
                /* POLARITY FLIPS */
                m_rotDelDif->setPolarityFlip( (m_vn[ ++randCount ] >= 0.7), i, j );
                
                /* VELVET NOISE DELAY TIMES */
                m_rotDelDif_DT[ i ][ j ] = ( m_vn[ ++randCount ] * chanLen ) + ( chanLen * j );
                m_rotDelDif->setDelayTime( m_rotDelDif_DT[ i ][ j ], i, j );
//                m_rdd_modulators[ i ][ j ].initialise( moffset * (j + i*rdd_NCHANNELS), m_dtSamps[ i ][ j ] );;
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
//        m_seriesAP_modulators.resize( sap_NSTAGES * NCHANNELS );
        for ( auto i = 0; i < NCHANNELS; i++ )
        {
            m_seriesAP[ i ]->initialise( static_cast<int>(maxDtSamps*2) );
            m_seriesAP_DT[ i ].resize( sap_NSTAGES );
//            m_seriesAP_modulators[ i ].resize( sap_NSTAGES );
            for ( auto j = 0; j < sap_NSTAGES; j++ )
            {
                m_seriesAP_DT[ i ][ j ] = m_vn[ ++randCount ] * maxDtSamps;
                m_seriesAP[ i ]->setDelayTime( m_seriesAP_DT[ i ][ j ], j );
//                m_seriesAP_modulators[ i ][ j ].initialise( moffset * ( i*sap_NSTAGES + j ), m_seriesAP_DT[ i ][ j ] );
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
                m_multiTap_DT[ i ][ j ] = m_vn[ ++randCount ] * ( vnBand * j );
                m_multiTap[ i ]->setDelayTimeSamps( m_multiTap_DT[ i ][ j ], j );
                
                auto g = static_cast< Sample > ( ( mt_NTAPS + 1 ) - j ) / static_cast< Sample > ( mt_NTAPS + 1 );
                g *= m_vn[ ++randCount ] < 0.666 ? g : -g; // exponential decay
                m_multiTap[ i ]->setGain( g, j );
                
            }
        }
    }
}


//template < typename Sample >
//void sjf_verb< Sample >::DSP_wrapper::processBlock( juce::AudioBuffer<Sample>& buffer )
//{
//    audioFunc( buffer );
//}


//template< typename Sample >
//void sjf_verb< Sample >::DSP_wrapper::setDSPFunction( )
//{
//    if( m_rotDelDif && m_fdn)
//    {
//        processBlock = [ this ]( juce::AudioBuffer<Sample>& buffer )
//        {
//            std::vector< Sample > samples( std::max( rdd_NCHANNELS, fdn_NCHANNELS), 0 );
//            auto blockSize = buffer.getNumSamples();
//            for( auto s = 0; s < blockSize; s++ )
//            {
//                for ( auto c = 0; c < NCHANNELS; c++ )
//                    samples[ c ] = buffer.getSample( c, s );
//                m_rotDelDif->processInPlace( samples );
//                m_fdn->processInPlace( samples );
//                for ( auto c = 0; c < NCHANNELS; c++ )
//                    buffer.setSample( c, s, samples[ c ] );
//            }
//        };
//        return;
//    }
//    if( m_rotDelDif && m_apLoop )
//    {
//        processBlock = [ this ]( juce::AudioBuffer<Sample>& buffer )
//        {
//            std::vector< Sample > samples(  rdd_NCHANNELS, 0 );
//            auto blockSize = buffer.getNumSamples();
//            for( auto s = 0; s < blockSize; s++ )
//            {
//                for ( auto c = 0; c < NCHANNELS; c++ )
//                    samples[ c ] = buffer.getSample( c, s );
//                m_rotDelDif->processInPlace( samples );
//                m_apLoop->processInPlace( samples );
//                for ( auto c = 0; c < NCHANNELS; c++ )
//                    buffer.setSample( c, s, samples[ c ] );
//            }
//        };
//        return;
//    }
//    if ( m_multiTap.size() != 0 && m_seriesAP.size() != 0 && m_fdn )
//    {
//        processBlock = [ this ]( juce::AudioBuffer<Sample>& buffer )
//        {
//            std::vector< Sample > samples( fdn_NCHANNELS, 0);
//            auto blockSize = buffer.getNumSamples();
//            for( auto s = 0; s < blockSize; s++ )
//            {
//                for ( auto c = 0; c < NCHANNELS; c++ )
//                {
//                    samples[ c ] = buffer.getSample( c, s );
//                    m_multiTap[ c ]->process( samples[ c ] );
//                    m_seriesAP[ c ]->process( samples[ c ] );
//                }
//                m_fdn->processInPlace( samples );
//                for ( auto c = 0; c < NCHANNELS; c++ )
//                    buffer.setSample( c, s, samples[ c ] );
//            }
//        };
//        return;
//    }
//    if( m_multiTap.size() != 0 && m_seriesAP.size() != 0 && m_apLoop )
//    {
//        processBlock = [ this ]( juce::AudioBuffer<Sample>& buffer )
//        {
//            std::vector< Sample > samples( NCHANNELS, 0);
//            auto blockSize = buffer.getNumSamples();
//            for( auto s = 0; s < blockSize; s++ )
//            {
//                for ( auto c = 0; c < NCHANNELS; c++ )
//                {
//                    samples[ c ] = buffer.getSample( c, s );
//                    m_multiTap[ c ]->process( samples[ c ] );
//                    m_seriesAP[ c ]->process( samples[ c ] );
//                }
//                m_apLoop->processInPlace( samples );
//                for ( auto c = 0; c < NCHANNELS; c++ )
//                    buffer.setSample( c, s, samples[ c ] );
//            }
//        };
//        return;
//    }
//    if( m_multiTap.size() != 0 )
//    {
//        if( m_fdn )
//        {
//            processBlock = [ this ]( juce::AudioBuffer<Sample>& buffer )
//            {
//                std::vector< Sample > samples( fdn_NCHANNELS, 0);
//                auto blockSize = buffer.getNumSamples();
//                for( auto s = 0; s < blockSize; s++ )
//                {
//                    for ( auto c = 0; c < NCHANNELS; c++ )
//                    {
//                        samples[ c ] = buffer.getSample( c, s );
//                        m_multiTap[ c ]->process( samples[ c ] );
//                    }
//                    m_fdn->processInPlace( samples );
//                    for ( auto c = 0; c < NCHANNELS; c++ )
//                        buffer.setSample( c, s, samples[ c ] );
//                }
//            };
//            return;
//        }
//        if( m_apLoop )
//        {
//            processBlock = [ this ]( juce::AudioBuffer<Sample>& buffer )
//            {
//                std::vector< Sample > samples( NCHANNELS, 0);
//                auto blockSize = buffer.getNumSamples();
//                for( auto s = 0; s < blockSize; s++ )
//                {
//                    for ( auto c = 0; c < NCHANNELS; c++ )
//                    {
//                        samples[ c ] = buffer.getSample( c, s );
//                        m_multiTap[ c ]->process( samples[ c ] );
//                    }
//                    m_apLoop->processInPlace( samples );
//                    for ( auto c = 0; c < NCHANNELS; c++ )
//                        buffer.setSample( c, s, samples[ c ] );
//                }
//            };
//            return;
//        }
//    }
//    if( m_seriesAP.size() != 0 )
//    {
//        if( m_fdn )
//        {
//            processBlock = [ this ]( juce::AudioBuffer<Sample>& buffer )
//            {
//                std::vector< Sample > samples( fdn_NCHANNELS, 0);
//                auto blockSize = buffer.getNumSamples();
//                for( auto s = 0; s < blockSize; s++ )
//                {
//                    for ( auto c = 0; c < NCHANNELS; c++ )
//                    {
//                        samples[ c ] = buffer.getSample( c, s );
//                        m_seriesAP[ c ]->process( samples[ c ] );
//                    }
//                    m_fdn->processInPlace( samples );
//                    for ( auto c = 0; c < NCHANNELS; c++ )
//                        buffer.setSample( c, s, samples[ c ] );
//                }
//            };
//            return;
//        }
//        if( m_apLoop )
//        {
//            processBlock = [ this ]( juce::AudioBuffer<Sample>& buffer )
//            {
//                std::vector< Sample > samples( NCHANNELS, 0);
//                auto blockSize = buffer.getNumSamples();
//                for( auto s = 0; s < blockSize; s++ )
//                {
//                    for ( auto c = 0; c < NCHANNELS; c++ )
//                    {
//                        samples[ c ] = buffer.getSample( c, s );
//                        m_seriesAP[ c ]->process( samples[ c ] );
//                    }
//                    m_apLoop->processInPlace( samples );
//                    for ( auto c = 0; c < NCHANNELS; c++ )
//                        buffer.setSample( c, s, samples[ c ] );
//                }
//            };
//            return;
//        }
//    }
//        
//}



template class sjf_verb<  float >;
template class sjf_verb<  double >;

