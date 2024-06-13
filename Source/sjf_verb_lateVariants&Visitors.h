//
//  sjf_verb_lateVariants&Visitors.h
//  sjf_verb
//
//  Created by Simon Fay on 12/06/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_lateVariants_Visitors_h
#define sjf_verb_lateVariants_Visitors_h
using Sample = float;
using noLimit = sjf::rev::fbLimiters::nolimit<Sample>;
using limit = sjf::rev::fbLimiters::limit<Sample>;

template< sjf::interpolation::interpolatorTypes interpType >
struct fdnVariantStruct
{        
        using noMix = sjf::mixers::None<Sample>;
        using houseMix = sjf::mixers::Householder<Sample>;
        using hadMix = sjf::mixers::Hadamard<Sample>;
    
        using fdnWrapNoMix = lateDSP::fdnWrapper< Sample, noMix, noLimit, interpType >;
        using fdnWrapHouse = lateDSP::fdnWrapper< Sample, houseMix, noLimit, interpType >;
        using fdnWrapHad = lateDSP::fdnWrapper< Sample, hadMix, noLimit, interpType >;
        using fdnWrapNoMixFB = lateDSP::fdnWrapper< Sample, noMix, limit, interpType >;
        using fdnWrapHouseFB = lateDSP::fdnWrapper< Sample, houseMix, limit, interpType >;
        using fdnWrapHadFB = lateDSP::fdnWrapper< Sample, hadMix, limit, interpType >;
    
        using fdnWrapNoMixPtr = std::unique_ptr< fdnWrapNoMix >;
        using fdnWrapHousePtr = std::unique_ptr< fdnWrapHouse >;
        using fdnWrapHadPtr = std::unique_ptr< fdnWrapHad >;
        using fdnWrapNoMixFBPtr = std::unique_ptr< fdnWrapNoMixFB >;
        using fdnWrapHouseFBPtr = std::unique_ptr< fdnWrapHouseFB >;
        using fdnWrapHadFBPtr = std::unique_ptr< fdnWrapHadFB >;
    
        using fdnVariant = std::variant< fdnWrapNoMixPtr, fdnWrapHousePtr, fdnWrapHadPtr, fdnWrapNoMixFBPtr,fdnWrapHouseFBPtr,fdnWrapHadFBPtr >;
    
};

template< sjf::interpolation::interpolatorTypes interpType >
struct fdnVisitor
{
    fdnVisitor( juce::AudioBuffer< Sample >& b, size_t bS, lateDSP::varHolder<Sample>& vH ) : buffer(b), blockSize(bS), vars(vH)
    {}

    void operator()( typename fdnVariantStruct<interpType>::fdnWrapNoMixPtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
    void operator()( typename fdnVariantStruct<interpType>::fdnWrapHousePtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
    void operator()( typename fdnVariantStruct<interpType>::fdnWrapHadPtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
    void operator()( typename fdnVariantStruct<interpType>::fdnWrapNoMixFBPtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
    void operator()( typename fdnVariantStruct<interpType>::fdnWrapHouseFBPtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
    void operator()( typename fdnVariantStruct<interpType>::fdnWrapHadFBPtr& fdn ){ fdn->processBlock(buffer, blockSize, vars); }
private:
        juce::AudioBuffer< Sample >& buffer;
        size_t blockSize;
        lateDSP::varHolder<Sample>& vars;

};

template< sjf::interpolation::interpolatorTypes interpType >
struct fdnReseter
{
    void operator()( typename fdnVariantStruct<interpType>::fdnWrapNoMixPtr& fdn ){ fdn.reset(); }
    void operator()( typename fdnVariantStruct<interpType>::fdnWrapHousePtr& fdn ){ fdn.reset(); }
    void operator()( typename fdnVariantStruct<interpType>::fdnWrapHadPtr& fdn ){ fdn.reset(); }
    void operator()( typename fdnVariantStruct<interpType>::fdnWrapNoMixFBPtr& fdn ){ fdn.reset(); }
    void operator()( typename fdnVariantStruct<interpType>::fdnWrapHouseFBPtr& fdn ){ fdn.reset(); }
    void operator()( typename fdnVariantStruct<interpType>::fdnWrapHadFBPtr& fdn ){ fdn.reset(); }
};

//==========//==========//==========//==========//==========//==========
//==========//==========//==========//==========//==========//==========
//==========//==========//==========//==========//==========//==========
//==========//==========//==========//==========//==========//==========

template< sjf::interpolation::interpolatorTypes interpType >
struct apLoopVariantStruct
{
    using apLoopWrap = lateDSP::apLoopWrapper<Sample, noLimit, interpType>;
    using apLoopWrapFB = lateDSP::apLoopWrapper<Sample, limit, interpType>;

    using apLoopWrapPtr = std::unique_ptr< apLoopWrap >;
    using apLoopWrapFBPtr = std::unique_ptr< apLoopWrapFB >;


    using apLoopVariant = std::variant< apLoopWrapPtr, apLoopWrapFBPtr >;
};

template< sjf::interpolation::interpolatorTypes interpType >
struct apLoopVisitor
{
    apLoopVisitor( juce::AudioBuffer< Sample >& b, size_t bS, lateDSP::varHolder<Sample>& vH ) : buffer(b), blockSize(bS), vars(vH)
    {}

    void operator()( typename apLoopVariantStruct<interpType>::apLoopWrapPtr& apl ){ apl->processBlock(buffer, blockSize, vars); }
    void operator()( typename apLoopVariantStruct<interpType>::apLoopWrapFBPtr& apl ){ apl->processBlock(buffer, blockSize, vars); }
private:
        juce::AudioBuffer< Sample >& buffer;
        size_t blockSize;
        lateDSP::varHolder<Sample>& vars;
};

template< sjf::interpolation::interpolatorTypes interpType >
struct apLoopReseter
{
    void operator()( typename apLoopVariantStruct<interpType>::apLoopWrapPtr& apl ){ apl.reset(); }
    void operator()( typename apLoopVariantStruct<interpType>::apLoopWrapFBPtr& apl ){ apl.reset(); }
};
#endif /* sjf_verb_lateVariants_Visitors_h */
