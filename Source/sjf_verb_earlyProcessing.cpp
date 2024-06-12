//
//  sjf_verb_earlyProcessing.cpp
//  sjf_verb
//
//  Created by Simon Fay on 24/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#include "sjf_verb_earlyProcessing.h"
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename INTERPOLATION >
size_t sjf_verb_earlyProcessor<INTERPOLATION>::initialise( Sample sampleRate, int numberOfChannels )
{
    m_SR = sampleRate;
    NCHANNELS = numberOfChannels;
    m_varHolder.sampleRate = m_SR;
    m_LPFSmoother.reset( m_SR, 0.05 );
    m_HPFSmoother.reset( m_SR, 0.05 );
    m_varHolder.m_sizeSmoother.reset( m_SR, 0.2 );
    m_varHolder.m_modDSmoother.reset( m_SR, 0.05 );
    m_varHolder.m_modRSmoother.reset( m_SR, 0.05 );
    m_varHolder.m_modDampSmoother.reset( m_SR, 0.05 );
    m_varHolder.m_diffusionSmoother.reset( m_SR, 0.05 );
    
    m_varHolder.m_modPhasor.setFrequency( 0.001, m_SR );
    
    m_earlyHPF.resize( rdd_NCHANNELS );
    m_earlyLPF.resize( rdd_NCHANNELS );
    
    return rdd_NCHANNELS > numberOfChannels ? rdd_NCHANNELS : numberOfChannels;
}
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename INTERPOLATION >
void sjf_verb_earlyProcessor<INTERPOLATION>::processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize )
{
    m_paramHandler.triggerCallbacks();
    
    switch ( m_earlyType ) {
        case parameterIDs::earlyTypesEnum::multitap:
            m_mt->processBlock( buffer, blockSize, m_varHolder );
            break;
        case parameterIDs::earlyTypesEnum::seriesAP:
            m_sap->processBlock( buffer, blockSize, m_varHolder );
            break;
        case parameterIDs::earlyTypesEnum::mt_sAP:
            m_mtSap->processBlock( buffer, blockSize, m_varHolder );
            break;
        case parameterIDs::earlyTypesEnum::rotDelDif:
            m_rdd->processBlock( buffer, blockSize, m_varHolder );
            break;
            
        default:
            break;
    }
    
    filterBlock( buffer, blockSize );
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename INTERPOLATION >
void sjf_verb_earlyProcessor<INTERPOLATION>::filterBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize )
{
    jassert( buffer.getNumChannels() == rdd_NCHANNELS );
    Sample samp = 0, lpfCO = 0, hpfCO = 0;
    for ( auto i =0 ; i < blockSize; i++ )
    {
        lpfCO = m_LPFSmoother.getNextValue();
        hpfCO = m_HPFSmoother.getNextValue();
        for( auto c = 0; c < rdd_NCHANNELS; c++ )
        {
            samp = buffer.getSample( c, i );
            samp = m_earlyHPF[ c ].processHP( samp, hpfCO );
            samp = m_earlyLPF[ c ].process( samp, lpfCO );
            buffer.setSample( c, i, samp );
        }
    }
}

//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================

template< typename INTERPOLATION >
void sjf_verb_earlyProcessor<INTERPOLATION>::setEarlyType( parameterIDs::earlyTypesEnum type  )
{
    m_earlyType = type;
    switch ( m_earlyType ) {
        case parameterIDs::earlyTypesEnum::rotDelDif :
            m_rdd = std::make_unique< earlyDSP::rddWrapper<Sample, INTERPOLATION> >( rdd_NCHANNELS, rdd_NSTAGES, m_randArray, m_SR );
            m_mt.reset();
            m_sap.reset();
            break;
        case parameterIDs::earlyTypesEnum::multitap :
            m_rdd.reset();
            m_mt = std::make_unique< earlyDSP::mtWrapper<Sample, INTERPOLATION> >( NCHANNELS, mt_NTAPS, m_randArray, m_SR );
            m_sap.reset();
            break;
        case parameterIDs::earlyTypesEnum::seriesAP  :
            m_rdd.reset();
            m_mt.reset();
            m_sap = std::make_unique< earlyDSP::sapWrapper<Sample, INTERPOLATION> >( NCHANNELS, sap_NSTAGES, m_randArray, m_SR );
            break;
        case parameterIDs::earlyTypesEnum::mt_sAP :
            m_rdd.reset();
            m_mt.reset();
            m_sap.reset();
            m_mtSap = std::make_unique< earlyDSP::mtsapWrapper<Sample, INTERPOLATION>>(NCHANNELS, sap_NSTAGES, mt_NTAPS, m_randArray, m_SR );
            break;
        default:
            break;
    }
}
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
template< typename INTERPOLATION >
void sjf_verb_earlyProcessor<INTERPOLATION>::addParametersToHandler( juce::AudioProcessorValueTreeState& vts )
{
    auto p = vts.getParameter( parameterIDs::mainName + parameterIDs::earlyReflectionType );
    auto val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter(p, [this](Sample v)
                                {
                                    jassert( parameterIDs::earlyTypeMap.find( v ) != parameterIDs::earlyTypeMap.end() );
                                    setEarlyType( parameterIDs::earlyTypeMap.find( v )->second );
                                } );
    jassert( parameterIDs::earlyTypeMap.find( val ) != parameterIDs::earlyTypeMap.end() );
    setEarlyType( parameterIDs::earlyTypeMap.find( static_cast<int>( val ) )->second );

    p = vts.getParameter( parameterIDs::mainName + parameterIDs::earlyDiffusion );
    val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter(p, [this]( Sample v ){ m_varHolder.m_diffusionSmoother.setTargetValue((v*0.006)+0.2); });
    m_varHolder.m_diffusionSmoother.setCurrentAndTargetValue( ( val * 0.006 ) + 0.2 );
    
    p = vts.getParameter( parameterIDs::mainName + parameterIDs::earlyHPFCutoff );
    val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter(p, [this](Sample v){m_HPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
    m_HPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));

    p = vts.getParameter( parameterIDs::mainName + parameterIDs::earlyLPFCutoff );
    val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
    m_paramHandler.addParameter(p, [this](Sample v){m_LPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
    m_LPFSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));

    
    
       p = vts.getParameter( parameterIDs::mainName + parameterIDs::size );
       val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
       m_paramHandler.addParameter(p, [this](Sample v)
                                   {
                                       v *= 0.01;
                                       v =  (v <= 0.5) ? std::pow( 2.0, -( 0.5 - v )*3.0 ) : std::pow( 2.0, v - 0.5 );
                                       m_varHolder.m_sizeSmoother.setTargetValue( v );
                                   } );
       val *= 0.01;
       val = (val <= 0.5) ? std::pow( 2.0, -( 0.5 - val )*3.0 ) : std::pow( 2.0, val - 0.5 );
       m_varHolder.m_sizeSmoother.setCurrentAndTargetValue( val );

       p = vts.getParameter( parameterIDs::mainName + parameterIDs::modDepth );
       val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
       m_paramHandler.addParameter(p, [this](Sample v)
                                   {
                                       auto depth = std::pow( v * 0.008, 2 );
                                       m_varHolder.m_modDSmoother.setTargetValue( depth );
                                   } );
       auto depth = std::pow( val * 0.008, 2 );
       m_varHolder.m_modDSmoother.setCurrentAndTargetValue( depth );

       p = vts.getParameter( parameterIDs::mainName + parameterIDs::modRate );
       val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
       m_paramHandler.addParameter(p, [this](Sample v)
                                   {
                                       auto coef =  1.0 - calculateLPFCoefficient( v, m_SR );
                                       m_varHolder.m_modRSmoother.setTargetValue( v );
                                       m_varHolder.m_modDampSmoother.setTargetValue( coef );
                                   } );
       auto coef = 1.0 - calculateLPFCoefficient( val, m_SR );
       m_varHolder.m_modRSmoother.setCurrentAndTargetValue( val );
       m_varHolder.m_modDampSmoother.setCurrentAndTargetValue( coef );
}

//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================
//=======================================//=======================================//=======================================

template class sjf_verb_earlyProcessor<sjf::interpolation::noneInterpolate<float> >;
template class sjf_verb_earlyProcessor<sjf::interpolation::linearInterpolate<float> >;
template class sjf_verb_earlyProcessor<sjf::interpolation::cubicInterpolate<float> >;
template class sjf_verb_earlyProcessor<sjf::interpolation::fourPointInterpolatePD<float> >;
template class sjf_verb_earlyProcessor<sjf::interpolation::fourPointFourthOrderOptimal<float> >;
template class sjf_verb_earlyProcessor<sjf::interpolation::cubicInterpolateGodot<float> >;
template class sjf_verb_earlyProcessor<sjf::interpolation::cubicInterpolateHermite<float> >;
