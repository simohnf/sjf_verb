//
//  sjf_verb_outputProcessing.cpp
//  sjf_verb
//
//  Created by Simon Fay on 12/05/2024.
//  Copyright © 2024 sjf. All rights reserved.


#include "sjf_verb_outputProcessing.h"

template< sjf::interpolation::interpolatorTypes interpType >
void sjf_verb_outputProcessor< interpType >::initialise( Sample sampleRate, int nChannels )
{
    NCHANNELS = nChannels;
    if ( NCHANNELS != 2)
        setMonoLow( false );
    m_coef = 1.0 - calculateLPFCoefficient< Sample >( 100, sampleRate );
    
    for ( auto& s : m_shimVoice )
    {
        s.initialise( sampleRate );
        s.setWindowSize( .5 );
    }
    
    m_shimLevelSmoother.reset( sampleRate, 0.2 );
    for ( auto& s : m_shimShiftSmoother )
        s.reset( sampleRate, 0.2 );
}

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
template< sjf::interpolation::interpolatorTypes interpType >
void sjf_verb_outputProcessor<interpType>::processBlock( juce::AudioBuffer<Sample> &outputBuffer, juce::AudioBuffer<Sample> &revBuffer, size_t blockSize)
{
    m_paramHandler.triggerCallbacks();
    if( m_shimLevelSmoother.getTargetValue() == 0. && m_shimLevelSmoother.getCurrentValue() == 0. )
        revBuffer.clear();
    else
    {
        Sample samp{0}, level{0}, shift0{0}, shift1(0), scale{std::sqrt(static_cast<Sample>(1.0)/NCHANNELS)};
        if ( m_shimmerDualVoice )
        {
            for ( auto i = 0; i < blockSize; i ++ )
            {
                level = m_shimLevelSmoother.getNextValue();
                shift0 = m_shimShiftSmoother[0].getNextValue();
                shift1 = m_shimShiftSmoother[1].getNextValue();
                m_shimVoice[ 0 ].setPitchScaling( shift0 );
                m_shimVoice[ 1 ].setPitchScaling( shift1 );
                samp  = 0;
                for ( auto j = 0; j < NCHANNELS; j++ )
                    samp += outputBuffer.getSample( j, i );
                samp *= scale;
                samp = sjf::nonlinearities::tanhSimple( ( m_shimVoice[ 0 ].process( samp ) + m_shimVoice[ 1 ].process( samp ) ) * level );
                for ( auto j = 0; j < NCHANNELS; j++ )
                    revBuffer.setSample( j, i, samp );
            }
        }
        else
        {
            for ( auto i = 0; i < blockSize; i ++ )
            {
                level = m_shimLevelSmoother.getNextValue();
                shift0 = m_shimShiftSmoother[0].getNextValue();
                m_shimVoice[ 0 ].setPitchScaling( shift0 );
                samp  = 0;
                for ( auto j = 0; j < NCHANNELS; j++ )
                    samp += outputBuffer.getSample( j, i );
                samp *= scale;
                samp = sjf::nonlinearities::tanhSimple( m_shimVoice[ 0 ].process( samp ) * level );
                for ( auto j = 0; j < NCHANNELS; j++ )
                    revBuffer.setSample( j, i, samp );
            }
        }
    }
    
    if ( m_monoLow && NCHANNELS == 2 )
    {
        auto ms = sjf::utilities::MidSide< Sample >::encode( 0, 0 );
        auto lr = sjf::utilities::MidSide< Sample >::decode( ms );
        for ( auto i = 0; i < blockSize; i ++ )
        {
            ms = sjf::utilities::MidSide< Sample >::encode( outputBuffer.getSample( 0, i ), outputBuffer.getSample( 1, i ) );
            ms.side = m_monoLowFilt.processHP( ms.side, m_coef );
            
            lr = sjf::utilities::MidSide< Sample >::decode( ms );
            outputBuffer.setSample( 0, i, lr.left );
            outputBuffer.setSample( 1, i, lr.right );
        }
    }
}
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
template< sjf::interpolation::interpolatorTypes interpType >
void sjf_verb_outputProcessor<interpType>::setMonoLow( bool monoLow )
{
    m_monoLow = monoLow;
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< sjf::interpolation::interpolatorTypes interpType >
void sjf_verb_outputProcessor<interpType>::addParametersToHandler( juce::AudioProcessorValueTreeState& vts )
{
    auto p = vts.getParameter( parameterIDs::mainName + parameterIDs::monoLow );
    auto val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter(p, [ this ]( Sample v ) { setMonoLow( static_cast< bool >( v ) ); } );
    setMonoLow( static_cast< bool >( val ) );

    p = vts.getParameter( parameterIDs::mainName + parameterIDs::shimmerLevel );
    val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter( p, [this]( Sample v ) { m_shimLevelSmoother.setTargetValue( std::pow( v*0.003, 2 ) ); } );
    m_shimLevelSmoother.setCurrentAndTargetValue( std::pow( val*0.003, 2 ) );

    p = vts.getParameter( parameterIDs::mainName + parameterIDs::shimmerTransposition );
    val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter( p, [this]( Sample v )
                                {
                                    m_shimShiftSmoother[0].setTargetValue( std::pow( 2.0, v/12.0 ) );
                                    m_shimShiftSmoother[1].setTargetValue( std::pow( 2.0, -v/12.0 ) );
                                } );
    m_shimShiftSmoother[0].setTargetValue( std::pow( 2.0, val/12.0 ) );
    m_shimShiftSmoother[1].setTargetValue( std::pow( 2.0, -val/12.0 ) );

    p = vts.getParameter( parameterIDs::mainName + parameterIDs::shimmerDualVoice );
    val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter( p, [this]( Sample v ) { setShimmerDualVoice( static_cast<bool>( v ) ); } );
    setShimmerDualVoice( static_cast<bool>( val ) );
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template class sjf_verb_outputProcessor<sjf::interpolation::interpolatorTypes::none >;
template class sjf_verb_outputProcessor<sjf::interpolation::interpolatorTypes::linear >;
template class sjf_verb_outputProcessor<sjf::interpolation::interpolatorTypes::cubic >;
template class sjf_verb_outputProcessor<sjf::interpolation::interpolatorTypes::pureData >;
template class sjf_verb_outputProcessor<sjf::interpolation::interpolatorTypes::fourthOrder >;
template class sjf_verb_outputProcessor<sjf::interpolation::interpolatorTypes::godot >;
template class sjf_verb_outputProcessor<sjf::interpolation::interpolatorTypes::hermite >;
