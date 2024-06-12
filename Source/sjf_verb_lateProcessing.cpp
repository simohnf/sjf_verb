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
    m_paramHandler.triggerCallbacks();
    if( m_stateChanged )
        setLateType(m_lateType);
    m_stateChanged = false;
    switch (m_lateType) {
        case parameterIDs::lateTypesEnum::fdn:
            std::visit( fdnVisitor{ buffer,blockSize,m_varHolder}, m_fdn );
            break;
        case parameterIDs::lateTypesEnum::apLoop:
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
template< typename INTERPOLATION >
void sjf_verb_lateProcessor<INTERPOLATION>::setMixType( sjf::rev::mixers mixType )
{
    m_fdnMixer = mixType;
    m_stateChanged = true;
}


//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename INTERPOLATION >
void sjf_verb_lateProcessor<INTERPOLATION>::setFBLimit( bool shouldLimitFeedback )
{
    m_fbLimit = shouldLimitFeedback;
    m_stateChanged = true;
}


//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
//=============================//=============================//=============================//=============================
template< typename INTERPOLATION >
void sjf_verb_lateProcessor<INTERPOLATION>::addParametersToHandler( juce::AudioProcessorValueTreeState& vts )
{
    
      auto p = vts.getParameter( parameterIDs::mainName + parameterIDs::lateReflectionType );
      auto val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
      m_paramHandler.addParameter(p, [ this ]( Sample v )
                                  {
                                          jassert( parameterIDs::lateTypeMap.find( v ) != parameterIDs::lateTypeMap.end() );
                                          setLateType( parameterIDs::lateTypeMap.find( v )->second );
                                  } );
      jassert( parameterIDs::lateTypeMap.find( val ) != parameterIDs::lateTypeMap.end() );
      setLateType( parameterIDs::lateTypeMap.find( static_cast<int>( val ) )->second );

      p = vts.getParameter( parameterIDs::mainName + parameterIDs::lateDiffusion );
      val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
      m_paramHandler.addParameter(p, [this]( Sample v ) { m_varHolder.m_diffusionSmoother.setTargetValue((v*0.006)+0.2); } );
      m_varHolder.m_diffusionSmoother.setCurrentAndTargetValue( ( val * 0.006 ) + 0.2 );

      p = vts.getParameter( parameterIDs::mainName + parameterIDs::lateHPFCutoff );
      val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
      m_paramHandler.addParameter(p, [this](Sample v) {m_varHolder.m_hpfSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
      m_varHolder.m_hpfSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));

      p = vts.getParameter( parameterIDs::mainName + parameterIDs::lateLPFCutoff );
      val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
      m_paramHandler.addParameter(p, [this](Sample v) {m_varHolder.m_lpfSmoother.setTargetValue(1.0-calculateLPFCoefficient(v,m_SR));});
      m_varHolder.m_lpfSmoother.setTargetValue(1.0-calculateLPFCoefficient(val,m_SR));

      p = vts.getParameter( parameterIDs::mainName + parameterIDs::decay );
      val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
      m_paramHandler.addParameter(p, [this](Sample v) { m_varHolder.m_decaySmoother.setTargetValue( v * 1000.0 ); });
      m_varHolder.m_decaySmoother.setCurrentAndTargetValue( val * 1000.0 );

      p = vts.getParameter( parameterIDs::mainName + parameterIDs::fdnMixType );
      val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
      m_paramHandler.addParameter(p, [this](Sample v)
                                  {
                                      jassert( parameterIDs::fdnMixMap.find( v ) != parameterIDs::fdnMixMap.end() );
                                      setMixType(parameterIDs::fdnMixMap.find(v)->second);
                                  });
      jassert( parameterIDs::fdnMixMap.find( val ) != parameterIDs::fdnMixMap.end() );
      setMixType(parameterIDs::fdnMixMap.find(val)->second);

      p = vts.getParameter( parameterIDs::mainName + parameterIDs::feedbackLimit );
      val = sjf::juceStuff::getUnNormalisedParameterValue< float >( p );
      m_paramHandler.addParameter(p, [this](Sample v) { setFBLimit( static_cast< bool >( v ) ); });
      setFBLimit( static_cast< bool >( val ) );

    
    
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
