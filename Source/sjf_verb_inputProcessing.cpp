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
template< typename Sample >
void sjf_verb_inputProcessor< Sample >::initialise( Sample sampleRate, int numberOfChannels )
{
    m_SR = sampleRate;
    NCHANNELS = numberOfChannels;
    
    m_preDelays.resize( NCHANNELS );
    for ( auto & pd : m_preDelays )
    {
        pd.initialise( m_SR * 0.5, m_SR*0.001 );
        pd.reverse( m_reversed );
    }
    
    
    m_inputLPF.resize( numberOfChannels );
    m_inputHPF.resize( numberOfChannels );
    
}
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
template< typename Sample >
void sjf_verb_inputProcessor< Sample >::setInterpolationType( sjf_interpolators::interpolatorTypes interpType )
{
    for ( auto & i : m_preDelays ){ i.setInterpolationType( sjf_interpolators::interpolatorTypes::none ); }
}
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================

template < typename Sample >
void sjf_verb_inputProcessor< Sample >::process( std::vector< Sample >& samples )
{
    
    for ( auto i = 0; i < NCHANNELS; i++ )
    {
        m_preDelays[ i ].setDelayTime( m_preDelayTime );
        m_preDelays[ i ].setSample( samples[ i ] );
        samples[ i ] = m_preDelayTime <= 1.0 ? samples[ i ] : m_preDelays[ i ].getSample();
        samples[ i ] = m_inputLPF[ i ].process( samples[ i ], m_inputLPFCutoff );
        samples[ i ] = m_inputHPF[ i ].processHP( samples[ i ], m_inputHPFCutoff );
    }
}

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================

template < typename Sample >
void sjf_verb_inputProcessor< Sample >::reverse( bool shouldReverse)
{
    m_reversed = shouldReverse;
    for ( auto & pd : m_preDelays )
        pd.reverse( shouldReverse );
}



template class sjf_verb_inputProcessor< float > ;
template class sjf_verb_inputProcessor< double > ;
