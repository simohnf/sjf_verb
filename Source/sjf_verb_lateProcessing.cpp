//
//  sjf_verb_lateProcessing.cpp
//  sjf_verb
//
//  Created by Simon Fay on 26/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#include "sjf_verb_lateProcessing.h"
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< typename INTERPOLATION >
size_t sjf_verb_lateProcessor<INTERPOLATION>::initialise( Sample sampleRate, int numberOfChannels )
{
    m_SR = sampleRate;
    NCHANNELS = numberOfChannels;
    
    m_varHolder.sampleRate = m_SR;
    m_varHolder.m_sizeSmoother.reset( m_SR, 0.2 );
    m_varHolder.m_decaySmoother.reset( m_SR, 0.05 );
    m_varHolder.m_lpfSmoother.reset( m_SR, 0.05 );
    m_varHolder.m_hpfSmoother.reset( m_SR, 0.05 );
    m_varHolder.m_diffusionSmoother.reset( m_SR, 0.05 );
    m_varHolder.m_modDSmoother.reset( m_SR, 0.05 );
    m_varHolder.m_modRSmoother.reset( m_SR, 0.05 );
    m_varHolder.m_modDampSmoother.reset( m_SR, 0.05 );
    m_varHolder.m_modPhasor.setFrequency( 0.001, m_SR );
    
    return fdn_NCHANNELS > numberOfChannels ? fdn_NCHANNELS : numberOfChannels;
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< typename INTERPOLATION >
void sjf_verb_lateProcessor<INTERPOLATION>::processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize )
{
    switch (m_lateType) {
        case parameterIDs::lateTypesEnum::fdn:
            if( m_varHolder.fdnMix != m_fdnMixer || m_varHolder.ControlFB != m_fbLimit )
            {
                m_fdnMixer = m_varHolder.fdnMix;
                m_fbLimit = m_varHolder.ControlFB;
                setLateType(m_lateType);
            }
            std::visit( fdnVisitor{ buffer,blockSize,m_varHolder}, m_fdn );
            break;
        case parameterIDs::lateTypesEnum::apLoop:
            if( m_varHolder.ControlFB != m_fbLimit )
            {
                m_fbLimit = m_varHolder.ControlFB;
                setLateType(m_lateType);
            }
            std::visit( apLoopVisitor{ buffer,blockSize,m_varHolder}, m_apLoop );
            break;
        default:
            break;
    }
}



//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< typename INTERPOLATION >
void sjf_verb_lateProcessor<INTERPOLATION>::setLateType( parameterIDs::lateTypesEnum type )
{
    m_lateType = type;
    
    switch (m_lateType) {
        case parameterIDs::lateTypesEnum::fdn:
//            m_apLoop.reset();
            std::visit( apLoopReseter(), m_apLoop );
            if( m_fbLimit )
                switch (m_fdnMixer) {
                    case sjf::rev::mixers::none:
                        m_fdn = std::make_unique< fdnWrapNoMixFB >(fdn_NCHANNELS, m_randArray, m_SR );
                        break;
                    case sjf::rev::mixers::householder:
                        m_fdn = std::make_unique< fdnWrapHouseFB >(fdn_NCHANNELS, m_randArray, m_SR );
                        break;
                    case sjf::rev::mixers::hadamard:
                        m_fdn = std::make_unique< fdnWrapHadFB >(fdn_NCHANNELS, m_randArray, m_SR );
                        break;
                    default:
                        break;
                }
            else
                switch (m_fdnMixer) {
                    case sjf::rev::mixers::none:
                        m_fdn = std::make_unique< fdnWrapNoMix >(fdn_NCHANNELS, m_randArray, m_SR );
                        break;
                    case sjf::rev::mixers::householder:
                        m_fdn = std::make_unique< fdnWrapHouse >(fdn_NCHANNELS, m_randArray, m_SR );
                        break;
                    case sjf::rev::mixers::hadamard:
                        m_fdn = std::make_unique< fdnWrapHad >(fdn_NCHANNELS, m_randArray, m_SR );
                        break;
                    default:
                        break;
                }
            break;
        case parameterIDs::lateTypesEnum::apLoop:
//            m_apLoop = std::make_unique<lateDSP::apLoopWrapper<Sample> >(NCHANNELS, apl_NSTAGES, apl_NAP_PERSTAGE, m_randArray, m_SR);
            if( m_fbLimit )
                m_apLoop = std::make_unique< apLoopWrapFB >(NCHANNELS, apl_NSTAGES, apl_NAP_PERSTAGE, m_randArray, m_SR);
            else
                m_apLoop = std::make_unique< apLoopWrap >(NCHANNELS, apl_NSTAGES, apl_NAP_PERSTAGE, m_randArray, m_SR);
            std::visit( fdnReseter(), m_fdn );
            break;
        default:
            break;
    }
}



//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//template< typename INTERPOLATION >
//void sjf_verb_lateProcessor<INTERPOLATION>::setMixType( sjf::rev::mixers mixType )
//{
//    m_fdnMixer = mixType;
//}


//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//template< typename INTERPOLATION >
//void sjf_verb_lateProcessor<INTERPOLATION>::setFBLimit( bool shouldLimitFeedback )
//{
//    m_fbLimit = shouldLimitFeedback;
//}


//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template class sjf_verb_lateProcessor<sjf::interpolation::noneInterpolate<float> >;
template class sjf_verb_lateProcessor<sjf::interpolation::linearInterpolate<float> >;
template class sjf_verb_lateProcessor<sjf::interpolation::cubicInterpolate<float> >;
template class sjf_verb_lateProcessor<sjf::interpolation::fourPointInterpolatePD<float> >;
template class sjf_verb_lateProcessor<sjf::interpolation::fourPointFourthOrderOptimal<float> >;
template class sjf_verb_lateProcessor<sjf::interpolation::cubicInterpolateGodot<float> >;
template class sjf_verb_lateProcessor<sjf::interpolation::cubicInterpolateHermite<float> >;
