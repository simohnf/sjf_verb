//
//  sjf_verb_dspWrapper.cpp
//  sjf_verb
//
//  Created by Simon Fay on 10/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#include "sjf_verb_dspWrapper.h"


//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
template< typename Sample >
unsigned sjf_verb_DSP_wrapper< Sample >::initialise( Sample sampleRate, int numberOfChannels )
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
void sjf_verb_DSP_wrapper< Sample >::initialiseEarlyDSP( Sample sampleRate )
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
void sjf_verb_DSP_wrapper< Sample >::initialiseLateDSP( Sample sampleRate )
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
void sjf_verb_DSP_wrapper< Sample >::er_rdd( std::vector< Sample >& samples )
{
    for ( auto i = 0; i < rdd_NSTAGES; i++ )
        for ( auto j= 0; j < rdd_NCHANNELS; j++ )
            m_rotDelDif->setDelayTime( m_rdd_modulators[i][j].process( m_rotDelDif_DT[i][j], m_modPhase, m_modDepth, m_modDamp )*m_size, i, j );
    m_rotDelDif->setDamping( m_erDamp );
    m_rotDelDif->processInPlace( samples );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb_DSP_wrapper< Sample >::er_mt( std::vector< Sample >& samples )
{
    for ( auto i = 0; i < NCHANNELS; i++ )
    {
        for ( auto j = 0; j < mt_NTAPS; j++ )
            m_multiTap[ i ]->setDelayTimeSamps( m_multiTap_DT[ i ][ j ] * m_size, j );
        samples[ i ] = m_multiTapDamp[ i ].process( m_multiTap[ i ]->process( samples[ i ] ), m_erDamp );
    }
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb_DSP_wrapper< Sample >::er_sap( std::vector< Sample >& samples )
{
    for ( auto i = 0; i < NCHANNELS; i++ )
    {
        for ( auto j = 0; j < sap_NSTAGES; j++ )
            m_seriesAP[ i ]->setDelayTime( m_sap_modulators[i][j].process( m_seriesAP_DT[i][j], m_modPhase, m_modDepth, m_modDamp )*m_size, j );
        m_seriesAP[ i ]->setDamping( m_erDamp );
        m_seriesAP[ i ]->setCoefs( m_earlyDiff );
        samples[ i ] = m_seriesAP[ i ]->process( samples[ i ] );
    }
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb_DSP_wrapper< Sample >::er_mtsap( std::vector< Sample >& samples )
{
    er_mt( samples );
    er_sap( samples );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb_DSP_wrapper< Sample >::lr_fdn( std::vector< Sample >& samples )
{
    m_fdn->setDecayInMS( m_decay );
    m_fdn->setDamping( m_lrDamp );
    m_fdn->setDiffusion( m_lateDiff );
    for ( auto i = 0; i < fdn_NCHANNELS; i++ )
        m_fdn->setDelayTime( m_fdn_modulators[ i ].process( m_fdn_DT[ i ], m_modPhase, m_modDepth, m_modDamp )*m_size, i );
    m_fdn->processInPlace( samples );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb_DSP_wrapper< Sample >::lr_apLoop( std::vector< Sample >& samples )
{
    m_apLoop->setDamping( m_lrDamp );
    m_apLoop->setDiffusion( m_lateDiff );
    m_apLoop->setDecay( m_decay );
    for ( auto i = 0; i < apl_NSTAGES; i++ )
        for ( auto j = 0; j < apl_NAP_PERSTAGE+1; j++ )
            m_apLoop->setDelayTimeSamples( m_apLoop_modulators[i][j].process( m_apLoop_DT[i][j], m_modPhase, m_modDepth, m_modDamp )*m_size, i, j );
    m_apLoop->processInPlace( samples );
}


//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb_DSP_wrapper< Sample >::setEarlyType( parameterIDs::earlyTypesEnum type, Sample sampleRate )
{
    switch ( type ) {
        case parameterIDs::earlyTypesEnum::rotDelDif :
            m_rotDelDif = std::make_unique< sjf::rev::rotDelDif< Sample > >( rdd_NCHANNELS, rdd_NSTAGES );
            m_multiTap.clear(); m_multiTap_DT.clear();
            m_multiTapDamp.clear();
            m_seriesAP.clear(); m_seriesAP_DT.clear();
            processEarly = &sjf_verb_DSP_wrapper::er_rdd;
            break;
        case parameterIDs::earlyTypesEnum::multitap :
            m_rotDelDif.reset(); m_rotDelDif_DT.clear();
            for( auto c = 0; c < NCHANNELS; c ++ )
                m_multiTap.push_back( std::make_unique< sjf::rev::multiTap< Sample > >( mt_NTAPS ) );
            m_multiTapDamp.resize( NCHANNELS );
            m_seriesAP.clear(); m_seriesAP_DT.clear();
            processEarly = &sjf_verb_DSP_wrapper::er_mt;
            break;
        case parameterIDs::earlyTypesEnum::seriesAP  :
            m_rotDelDif.reset(); m_rotDelDif_DT.clear();
            m_multiTap.clear(); m_multiTap_DT.clear();
            m_multiTapDamp.clear();
            for( auto c = 0; c < NCHANNELS; c++ )
                m_seriesAP.push_back( std::make_unique< sjf::rev::seriesAllpass< Sample > >( sap_NSTAGES ) );
            processEarly = &sjf_verb_DSP_wrapper::er_sap;
            break;
        case parameterIDs::earlyTypesEnum::mt_sAP :
            m_rotDelDif.reset(); m_rotDelDif_DT.clear();
            for( auto c = 0; c < NCHANNELS; c ++ )
            {
                m_multiTap.push_back( std::make_unique< sjf::rev::multiTap< Sample > >( mt_NTAPS ) );
                m_seriesAP.push_back( std::make_unique< sjf::rev::seriesAllpass< Sample > >( sap_NSTAGES ) );
            }
            m_multiTapDamp.resize( NCHANNELS );
            processEarly = &sjf_verb_DSP_wrapper::er_mtsap;
            break;
        default:
            processEarly = &sjf_verb_DSP_wrapper::er_default;
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
void sjf_verb_DSP_wrapper< Sample >::setLateType( parameterIDs::lateTypesEnum type, Sample sampleRate )
{
    switch ( type ){
        case parameterIDs::lateTypesEnum::fdn :
            m_fdn = std::make_unique< sjf::rev::fdn< Sample > >( fdn_NCHANNELS );
            m_apLoop.reset(); m_apLoop_DT.clear();
            processLate = &sjf_verb_DSP_wrapper::lr_fdn;
            break;
        case parameterIDs::lateTypesEnum::apLoop :
            m_fdn.reset(); m_fdn_DT.clear();
            m_apLoop = std::make_unique< sjf::rev::allpassLoop< Sample > >( apl_NSTAGES, apl_NAP_PERSTAGE );
            processLate = &sjf_verb_DSP_wrapper::lr_apLoop;
            break;
        default:
            processLate = &sjf_verb_DSP_wrapper::lr_default;
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

//template< typename Sample >
//void sjf_verb_DSP_wrapper< Sample >::setInterpolationType( parameterIDs::interpTypesEnum type )
//{
//    sjf_interpolators::interpolatorTypes interpType = sjf_interpolators::interpolatorTypes::none;
//    switch ( type ) {
//        case parameterIDs::interpTypesEnum::none :
//            interpType = sjf_interpolators::interpolatorTypes::none;
//            break;
//        case parameterIDs::interpTypesEnum::linear :
//            interpType = sjf_interpolators::interpolatorTypes::linear;
//            break;
//        case parameterIDs::interpTypesEnum::cubic :
//            interpType = sjf_interpolators::interpolatorTypes::cubic;
//            break;
//        case parameterIDs::interpTypesEnum::pureData :
//            interpType = sjf_interpolators::interpolatorTypes::pureData;
//            break;
//        case parameterIDs::interpTypesEnum::fourthOrder :
//            interpType = sjf_interpolators::interpolatorTypes::fourthOrder;
//            break;
//        case parameterIDs::interpTypesEnum::godot :
//            interpType = sjf_interpolators::interpolatorTypes::godot;
//            break;
//        case parameterIDs::interpTypesEnum::hermite :
//            interpType = sjf_interpolators::interpolatorTypes::hermite;
//            break;
//        default:
//            interpType = sjf_interpolators::interpolatorTypes::none;
//            break;
//    }
//    if( m_fdn ){ m_fdn->setInterpolationType( interpType ); }
//    if( m_apLoop ){ m_apLoop->setInterpolationType( interpType ); }
//    if( m_rotDelDif ){ m_rotDelDif->setInterpolationType( interpType ); }
//    if( m_multiTap.size() > 0 )
//        for( auto & i : m_multiTap ){ i->setInterpolationType( interpType ); }
//    if( m_seriesAP.size() > 0 )
//        for( auto & i : m_seriesAP ){ i->setInterpolationType( interpType ); }
////    for ( auto & i : m_preDelays ){ i.setInterpolationType( sjf_interpolators::interpolatorTypes::none ); }
//}

template< typename Sample >
void sjf_verb_DSP_wrapper< Sample >::setInterpolationType( sjf_interpolators::interpolatorTypes interpType )
{
    if( m_fdn ){ m_fdn->setInterpolationType( interpType ); }
    if( m_apLoop ){ m_apLoop->setInterpolationType( interpType ); }
    if( m_rotDelDif ){ m_rotDelDif->setInterpolationType( interpType ); }
    if( m_multiTap.size() > 0 )
        for( auto & i : m_multiTap ){ i->setInterpolationType( interpType ); }
    if( m_seriesAP.size() > 0 )
        for( auto & i : m_seriesAP ){ i->setInterpolationType( interpType ); }
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename Sample >
void sjf_verb_DSP_wrapper< Sample >::setFdnMixType( sjf::rev::mixers type )
{
    if( m_fdn )
        m_fdn->setMixType( type );
}


//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< typename Sample >
void sjf_verb_DSP_wrapper< Sample >::initialiseDelayTimes( Sample sampleRate )
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





template class sjf_verb_DSP_wrapper< float >;
template class sjf_verb_DSP_wrapper< double >;
