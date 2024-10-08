//
//  sjf_verb_earlyProcessing.h
//  sjf_verb
//
//  Created by Simon Fay on 24/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_earlyProcessing_h
#define sjf_verb_earlyProcessing_h

#include "../sjf_audio/sjf_audioUtilities.h"
#include "../sjf_audio/sjf_compileTimeRandom.h"
#include "../sjf_audio/sjf_rev.h"
#include "../sjf_audio/sjf_oscillators.h"
#include "../sjf_audio/sjf_modulator.h"
#include "../sjf_audio/sjf_parameterHandler.h"
#include "parameterIDs.h"
#include "sjf_verb_earlyProcessor_DSP_Wrappers.h"

template< sjf::interpolation::interpolatorTypes interpType = sjf::interpolation::interpolatorTypes::pureData >
class sjf_verb_earlyProcessor
{
    using Sample = float;
    using LSV = juce::LinearSmoothedValue< Sample >;
    using randArray = sjf::ctr::rArray< Sample, 4096, UNIX_TIMESTAMP +'e'+'a'+'r'+'l'+'y' >;
    
public:
    sjf_verb_earlyProcessor( juce::AudioProcessorValueTreeState& vts ) : m_paramHandler(vts)
        { addParametersToHandler( vts ); }
    ~sjf_verb_earlyProcessor(){}
    
    size_t initialise( Sample sampleRate, int numberOfChannels );
    
    void processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize );
    
    void setEarlyType( parameterIDs::earlyTypesEnum type );
    
private:
    LSV m_LPFSmoother, m_HPFSmoother;
    earlyDSP::varHolder<Sample> m_varHolder { };
    
    void addParametersToHandler( juce::AudioProcessorValueTreeState& vts );
    
    void filterBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize );

    static constexpr randArray m_randArray; // random values for delayTimes
    
    Sample m_SR{44100};

    std::unique_ptr< earlyDSP::rddWrapper<Sample, interpType> >         m_rdd;
    std::unique_ptr< earlyDSP::sapWrapper<Sample, interpType> >         m_sap;
    std::unique_ptr< earlyDSP::mtWrapper<Sample, interpType> >          m_mt;
    std::unique_ptr< earlyDSP::mtsapWrapper<Sample, interpType> >       m_mtSap;

    std::vector< sjf::filters::damper< Sample > >   m_earlyLPF, m_earlyHPF;

    size_t NCHANNELS{2};
    size_t rdd_NCHANNELS{8}, rdd_NSTAGES{5};
    size_t mt_NTAPS{64};
    size_t sap_NSTAGES{8};

    parameterIDs::earlyTypesEnum m_earlyType{ parameterIDs::earlyTypesEnum::rotDelDif };
    sjf::parameterHandler::paramHandlerVector m_paramHandler;
};

#endif /* sjf_verb_earlyProcessing_h */
