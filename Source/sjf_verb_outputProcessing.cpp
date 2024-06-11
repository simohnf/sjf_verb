//
//  sjf_verb_outputProcessing.cpp
//  sjf_verb
//
//  Created by Simon Fay on 12/05/2024.
//  Copyright © 2024 sjf. All rights reserved.


#include "sjf_verb_outputProcessing.h"

template< typename INTERPOLATION >
void sjf_verb_outputProcessor<INTERPOLATION>::initialise( Sample sampleRate, int nChannels )
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
template< typename INTERPOLATION >
void sjf_verb_outputProcessor<INTERPOLATION>::processBlock( juce::AudioBuffer<Sample> &outputBuffer, juce::AudioBuffer<Sample> &revBuffer, size_t blockSize)
{
    
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
template< typename INTERPOLATION >
void sjf_verb_outputProcessor<INTERPOLATION>::setMonoLow( bool monoLow )
{
    m_monoLow = monoLow;
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template class sjf_verb_outputProcessor<sjf::interpolation::noneInterpolate<float> >;
template class sjf_verb_outputProcessor<sjf::interpolation::linearInterpolate<float> >;
template class sjf_verb_outputProcessor<sjf::interpolation::cubicInterpolate<float> >;
template class sjf_verb_outputProcessor<sjf::interpolation::fourPointInterpolatePD<float> >;
template class sjf_verb_outputProcessor<sjf::interpolation::fourPointFourthOrderOptimal<float> >;
template class sjf_verb_outputProcessor<sjf::interpolation::cubicInterpolateGodot<float> >;
template class sjf_verb_outputProcessor<sjf::interpolation::cubicInterpolateHermite<float> >;
