//
//  sjf_verb_lateProcessing.h
//  sjf_verb
//
//  Created by Simon Fay on 26/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_lateProcessing_h
#define sjf_verb_lateProcessing_h

#include "../sjf_audio/sjf_audioUtilities.h"
#include "../sjf_audio/sjf_compileTimeRandom.h"
#include "../sjf_audio/sjf_rev.h"
#include "../sjf_audio/sjf_oscillators.h"
#include "../sjf_audio/sjf_modulator.h"
#include "../sjf_audio/sjf_parameterHandler.h"
#include "parameterIDs.h"
#include "sjf_verb_lateProcessor_DSP_Wrappers.h"

template< typename INTERPOLATION = sjf::interpolation::fourPointInterpolatePD< float > >
class sjf_verb_lateProcessor
{
    using Sample = float;
    using LSV = juce::LinearSmoothedValue< Sample >;
    using randArray = sjf::ctr::rArray< Sample, 4096, UNIX_TIMESTAMP +'l'+'a'+'t'+'e' >;
    
public:
    sjf_verb_lateProcessor( juce::AudioProcessorValueTreeState& vts ) : m_paramHandler(vts)
        { addParametersToHandler( vts ); }
    ~sjf_verb_lateProcessor(){}
    
    
    size_t initialise( Sample sampleRate, int numberOfChannels );
    
    void processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize );
    
    void setLateType( parameterIDs::lateTypesEnum type );
    
    void setMixType( sjf::rev::mixers mixType );
    
    void setFBLimit( bool shouldLimitFeedback );
    
    lateDSP::varHolder<Sample> m_varHolder;
    
private:
    
    void addParametersToHandler( juce::AudioProcessorValueTreeState& vts );
    
    Sample m_SR{ 44100 };
    parameterIDs::lateTypesEnum m_lateType{ parameterIDs::lateTypesEnum::fdn };
    size_t NCHANNELS{2};
    size_t apl_NSTAGES{6}, apl_NAP_PERSTAGE{2};
    size_t fdn_NCHANNELS{8};
    static constexpr randArray m_randArray; // random values for delayTimes
    bool m_fbLimit{false};
    bool m_stateChanged{false};
    using noLimit = sjf::rev::fbLimiters::nolimit<Sample>;
    using limit = sjf::rev::fbLimiters::limit<Sample>;
    
    //==========//==========//==========//==========//==========//==========
    //==========//==========//==========//==========//==========//==========
    //==========//==========//==========//==========//==========//==========
    //==========//==========//==========//==========//==========//==========
    sjf::rev::mixers m_fdnMixer{ sjf::rev::mixers::householder };
    using noMix = sjf::mixers::None<Sample>;
    using houseMix = sjf::mixers::Householder<Sample>;
    using hadMix = sjf::mixers::Hadamard<Sample>;
    
    using fdnWrapNoMix = lateDSP::fdnWrapper< Sample, noMix, noLimit, INTERPOLATION >;
    using fdnWrapHouse = lateDSP::fdnWrapper< Sample, houseMix, noLimit, INTERPOLATION >;
    using fdnWrapHad = lateDSP::fdnWrapper< Sample, hadMix, noLimit, INTERPOLATION >;
    using fdnWrapNoMixFB = lateDSP::fdnWrapper< Sample, noMix, limit, INTERPOLATION >;
    using fdnWrapHouseFB = lateDSP::fdnWrapper< Sample, houseMix, limit, INTERPOLATION >;
    using fdnWrapHadFB = lateDSP::fdnWrapper< Sample, hadMix, limit, INTERPOLATION >;
    
    using fdnWrapNoMixPtr = std::unique_ptr< fdnWrapNoMix >;
    using fdnWrapHousePtr = std::unique_ptr< fdnWrapHouse >;
    using fdnWrapHadPtr = std::unique_ptr< fdnWrapHad >;
    using fdnWrapNoMixFBPtr = std::unique_ptr< fdnWrapNoMixFB >;
    using fdnWrapHouseFBPtr = std::unique_ptr< fdnWrapHouseFB >;
    using fdnWrapHadFBPtr = std::unique_ptr< fdnWrapHadFB >;
    
    using fdnVariant = std::variant< fdnWrapNoMixPtr, fdnWrapHousePtr, fdnWrapHadPtr, fdnWrapNoMixFBPtr,fdnWrapHouseFBPtr,fdnWrapHadFBPtr >;
    
    struct fdnVisitor
    {
        fdnVisitor( juce::AudioBuffer< Sample >& b, size_t bS, lateDSP::varHolder<Sample>& vH ) : buffer(b), blockSize(bS), vars(vH)
        {}
        
        void operator()( fdnWrapNoMixPtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
        void operator()( fdnWrapHousePtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
        void operator()( fdnWrapHadPtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
        void operator()( fdnWrapNoMixFBPtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
        void operator()( fdnWrapHouseFBPtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
        void operator()( fdnWrapHadFBPtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
    private:
            juce::AudioBuffer< Sample >& buffer;
            size_t blockSize;
            lateDSP::varHolder<Sample>& vars;
            
    };
    
    struct fdnReseter
    {
        void operator()( fdnWrapNoMixPtr& fdn ){ fdn.reset(); }
        void operator()( fdnWrapHousePtr& fdn ){ fdn.reset(); }
        void operator()( fdnWrapHadPtr& fdn ){ fdn.reset(); }
        void operator()( fdnWrapNoMixFBPtr& fdn ){ fdn.reset(); }
        void operator()( fdnWrapHouseFBPtr& fdn ){ fdn.reset(); }
        void operator()( fdnWrapHadFBPtr& fdn ){ fdn.reset(); }
    };
    //==========//==========//==========//==========//==========//==========
    //==========//==========//==========//==========//==========//==========
    //==========//==========//==========//==========//==========//==========
    //==========//==========//==========//==========//==========//==========
    
    using apLoopWrap = lateDSP::apLoopWrapper<Sample, noLimit, INTERPOLATION>;
    using apLoopWrapFB = lateDSP::apLoopWrapper<Sample, limit, INTERPOLATION>;
    
    using apLoopWrapPtr = std::unique_ptr< apLoopWrap >;
    using apLoopWrapFBPtr = std::unique_ptr< apLoopWrapFB >;
    
    
    using apLoopVariant = std::variant< apLoopWrapPtr, apLoopWrapFBPtr >;
    
    struct apLoopVisitor
    {
        apLoopVisitor( juce::AudioBuffer< Sample >& b, size_t bS, lateDSP::varHolder<Sample>& vH ) : buffer(b), blockSize(bS), vars(vH)
        {}
        
        void operator()( apLoopWrapPtr& apl ){ apl->processBlock(buffer, blockSize, vars); }
        void operator()( apLoopWrapFBPtr& apl ){ apl->processBlock(buffer, blockSize, vars); }
    private:
            juce::AudioBuffer< Sample >& buffer;
            size_t blockSize;
            lateDSP::varHolder<Sample>& vars;
    };
    
    struct apLoopReseter
    {
        void operator()( apLoopWrapPtr& apl ){ apl.reset(); }
        void operator()( apLoopWrapFBPtr& apl ){ apl.reset(); }
    };
    //==========//==========//==========//==========//==========//==========
    //==========//==========//==========//==========//==========//==========
    //==========//==========//==========//==========//==========//==========
    //==========//==========//==========//==========//==========//==========
    
    
    
    fdnVariant m_fdn;
    apLoopVariant m_apLoop;
    
    
    
    sjf::parameterHandler::paramHandlerVector m_paramHandler;
};
#endif /* sjf_verb_lateProcessing_h */
