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


size_t sjf_verb_lateProcessor::initialise( Sample sampleRate, int numberOfChannels )
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


void sjf_verb_lateProcessor::processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize )
{
    switch (m_lateType) {
        case parameterIDs::lateTypesEnum::fdn:
            m_fdn->processBlock( buffer, blockSize, m_varHolder );
            break;
        case parameterIDs::lateTypesEnum::apLoop:
            m_apLoop->processBlock( buffer, blockSize, m_varHolder );
            break;
        default:
            break;
    }
}



//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================


void sjf_verb_lateProcessor::setLateType( parameterIDs::lateTypesEnum type )
{
    m_lateType = type;
    
    switch (m_lateType) {
        case parameterIDs::lateTypesEnum::fdn:
            m_apLoop.reset();
            m_fdn = std::make_unique< lateDSP::fdnWrapper<Sample> >(fdn_NCHANNELS, m_randArray, m_SR );
            break;
        case parameterIDs::lateTypesEnum::apLoop:
            m_apLoop = std::make_unique<lateDSP::apLoopWrapper<Sample> >(NCHANNELS, apl_NSTAGES, apl_NAP_PERSTAGE, m_randArray, m_SR);
            m_fdn.reset();
            break;
        default:
            break;
    }
}



//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

