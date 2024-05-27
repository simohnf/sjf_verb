//
//  sjf_verb_outputProcessing.cpp
//  sjf_verb
//
//  Created by Simon Fay on 12/05/2024.
//  Copyright © 2024 sjf. All rights reserved.


#include "sjf_verb_outputProcessing.h"

template< typename Sample >
void sjf_verb_outputProcessor< Sample >::initialise( Sample sampleRate, int nChannels )
{
    NCHANNELS = nChannels;
    if ( NCHANNELS != 2)
        setMonoLow( false );
    m_coef = 1.0 - calculateLPFCoefficient< Sample >( 100, sampleRate );
}

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
template< typename Sample >
void sjf_verb_outputProcessor<Sample>::processBlock(juce::AudioBuffer<Sample> &revBuffer, size_t blockSize)
{
    if (NCHANNELS != 2 || revBuffer.getNumChannels() < 2 )
        return;
    if ( m_monoLow )
    {
        for ( auto i = 0; i < blockSize; i ++ )
        {
            auto ms = sjf::utilities::MidSide< Sample >::encode( revBuffer.getSample( 0, i ), revBuffer.getSample( 1, i ) );
            ms.side = m_monoLowFilt.processHP( ms.side, m_coef );
            
            auto lr = sjf::utilities::MidSide< Sample >::decode( ms );
            revBuffer.setSample( 0, i, lr.left );
            revBuffer.setSample( 1, i, lr.right );
        }
    }
}
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================


template < typename Sample >
void sjf_verb_outputProcessor< Sample >::process( std::vector< Sample >& samples )
{
    if ( m_monoLow ){ applyMonoLow( samples ); }
//    monoLowProcessor( samples );
}
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================


template < typename Sample >
void sjf_verb_outputProcessor< Sample >::setMonoLow( bool monoLow )
{
    m_monoLow = monoLow;
//    monoLowProcessor = monoLow && (NCHANNELS==2) ? &sjf_verb_outputProcessor::applyMonoLow : &sjf_verb_outputProcessor::noMonoLow;
}
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================


template < typename Sample >
void sjf_verb_outputProcessor< Sample >::applyMonoLow( std::vector< Sample >& samples )
{
    auto ms = sjf::utilities::MidSide< Sample >::encode( samples[ 0 ], samples[ 1 ] );
    ms.side = m_monoLowFilt.processHP( ms.side, m_coef );
    
    auto lr = sjf::utilities::MidSide< Sample >::decode( ms );
    samples[ 0 ] = lr.left;
    samples[ 1 ] = lr.right;
}

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================

template class sjf_verb_outputProcessor< float > ;
template class sjf_verb_outputProcessor< double > ;
