//
//  sjf_verb_inputProcessing.cpp
//  sjf_verb - VST3
//
//  Created by Simon Fay on 10/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#include "sjf_verb_inputProcessing.h"

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
template< sjf::interpolation::interpolatorTypes interpType >
void sjf_verb_inputProcessor<interpType>::initialise( Sample sampleRate, int numberOfChannels )
{
    m_SR = sampleRate;
    NCHANNELS = numberOfChannels;
    
//    if( m_preDelays.size() != NCHANNELS )
        m_preDelays.resize( NCHANNELS );
    for ( auto & pd : m_preDelays )
        pd.initialise( m_SR * 0.5, m_SR*0.001 );
    
    m_preDelaySmoother.reset( m_SR, 0.2 );
    m_LPFSmoother.reset( m_SR, 0.05 );
    m_HPFSmoother.reset( m_SR, 0.05 );
    
    m_inputLPF.resize( numberOfChannels );
    m_inputHPF.resize( numberOfChannels );
    
}

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
template< sjf::interpolation::interpolatorTypes interpType >
void sjf_verb_inputProcessor<interpType>::processBlock( juce::AudioBuffer<Sample> &revBuffer, size_t blockSize )
{
    m_paramHandler.triggerCallbacks();
    Sample samp = 0.0, pdDT = 0, lpfCO = 0, hpfCO= 0;
    if ( m_reversed )
    {
        for ( auto i = 0; i < blockSize; i++ )
        {
            pdDT = m_preDelaySmoother.getNextValue();
            for ( auto c = 0; c < NCHANNELS; c++ )
            {
                m_preDelays[ c ].setDelayTime( pdDT );
                m_preDelays[ c ].setSample( revBuffer.getSample( c, i ) );
                revBuffer.setSample( c, i, m_preDelays[ c ].getSampleReversed() );
            }
        }
    }
    else
    {
        for ( auto i = 0; i < blockSize; i++ )
        {
            pdDT = m_preDelaySmoother.getNextValue();
            for ( auto c = 0; c < NCHANNELS; c++ )
            {
                m_preDelays[ c ].setDelayTime( pdDT );
                m_preDelays[ c ].setSample( revBuffer.getSample( c, i ) );
                revBuffer.setSample( c, i, m_preDelays[ c ].getSample() );
            }
        }
    }
    
    for ( auto i = 0; i < blockSize; i++ )
    {
        lpfCO = m_LPFSmoother.getNextValue();
        hpfCO = m_HPFSmoother.getNextValue();
        for ( auto c = 0; c < NCHANNELS; c++ )
        {
            samp = m_inputLPF[ c ].process( revBuffer.getSample( c, i ), lpfCO );
            samp = m_inputHPF[ c ].processHP( samp, hpfCO );
            revBuffer.setSample( c, i, samp );
        }
    }
    
}

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================

template< sjf::interpolation::interpolatorTypes interpType >
void sjf_verb_inputProcessor<interpType>::setReversed( bool shouldReverse)
{
    m_reversed = shouldReverse;
}
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
template< sjf::interpolation::interpolatorTypes interpType >
void sjf_verb_inputProcessor<interpType>::addParametersToHandler( juce::AudioProcessorValueTreeState& vts )
{
    auto p = vts.getParameter( parameterIDs::mainName + parameterIDs::inputHPFCutoff );
    auto val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter(p, [this](Sample v){ m_HPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
    m_HPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));
    
    p = vts.getParameter( parameterIDs::mainName + parameterIDs::inputLPFCutoff );
    val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter(p, [this](Sample v){ m_LPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
    m_LPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));
    
    p = vts.getParameter( parameterIDs::mainName + parameterIDs::preDelay );
    val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter(p, [this](Sample v){ m_preDelaySmoother.setTargetValue( v * 0.001 * m_SR ); });
    m_preDelaySmoother.setCurrentAndTargetValue( val * 0.001 * m_SR );
    
    p = vts.getParameter( parameterIDs::mainName + parameterIDs::reverse );
    val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter(p, [this](Sample v){ setReversed( static_cast< bool >( v ) ); } );
    setReversed( static_cast< bool >( val ) );
}
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================

template class sjf_verb_inputProcessor<sjf::interpolation::interpolatorTypes::none >;
template class sjf_verb_inputProcessor<sjf::interpolation::interpolatorTypes::linear >;
template class sjf_verb_inputProcessor<sjf::interpolation::interpolatorTypes::cubic >;
template class sjf_verb_inputProcessor<sjf::interpolation::interpolatorTypes::pureData >;
template class sjf_verb_inputProcessor<sjf::interpolation::interpolatorTypes::fourthOrder >;
template class sjf_verb_inputProcessor<sjf::interpolation::interpolatorTypes::godot >;
template class sjf_verb_inputProcessor<sjf::interpolation::interpolatorTypes::hermite >;
