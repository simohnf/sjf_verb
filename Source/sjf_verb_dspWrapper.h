//
//  sjf_verb_dspWrapper.h
//  sjf_verb
//
//  Created by Simon Fay on 10/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_dspWrapper_h
#define sjf_verb_dspWrapper_h

#include "../sjf_audio/sjf_audioUtilities.h"
#include "../sjf_audio/sjf_compileTimeRandom.h"
#include "../sjf_audio/sjf_rev.h"
#include "parameterIDs.h"

template< typename Sample >
class sjf_verb_DSP_wrapper
{
public:
    sjf_verb_DSP_wrapper(){}
    ~sjf_verb_DSP_wrapper()
    {
        m_rotDelDif.reset();
        m_multiTap.clear();
        m_seriesAP.clear();
        m_apLoop.reset();
        m_fdn.reset();
    }
    
    unsigned initialise( Sample sampleRate, int numberOfChannels );
    
    void setEarlyType( parameterIDs::earlyTypesEnum type, Sample sampleRate );
    void setLateType( parameterIDs::lateTypesEnum type, Sample sampleRate );
    
    void setInterpolationType( sjf_interpolators::interpolatorTypes interpType );
    
    void initialiseEarlyDSP( Sample sampleRate );
    void initialiseLateDSP( Sample sampleRate );
    
    void setFdnMixType( sjf::rev::mixers type );
    
    sjf::utilities::classMemberFunctionPointer< sjf_verb_DSP_wrapper, void, std::vector< Sample >& >     processEarly{this,&sjf_verb_DSP_wrapper::er_default};
    sjf::utilities::classMemberFunctionPointer< sjf_verb_DSP_wrapper, void, std::vector< Sample >& >     processLate{this,&sjf_verb_DSP_wrapper::lr_default};
    
    
private:
    
    inline void er_rdd( std::vector< Sample >& );
    inline void er_mt( std::vector< Sample >& );
    inline void er_sap( std::vector< Sample >& );
    inline void er_mtsap( std::vector< Sample >& );
    inline void er_default( std::vector< Sample >& ){ return; };
    
    inline void lr_fdn( std::vector< Sample >& );
    inline void lr_apLoop( std::vector< Sample >& );
    inline void lr_default( std::vector< Sample >& ){ return; };
    
    void setDSPFunctions( );
    void initialiseDelayTimes( Sample sampleRate );
    
    
    
private:
    std::unique_ptr< sjf::rev::rotDelDif< Sample > >                                            m_rotDelDif;
    std::vector< std::unique_ptr< sjf::rev::multiTap< Sample > > >                              m_multiTap;
    std::vector< sjf::filters::damper< Sample > >                                               m_multiTapDamp;
    std::vector< std::unique_ptr< sjf::rev::seriesAllpass< Sample > > >                         m_seriesAP;
    std::unique_ptr< sjf::rev::allpassLoop< Sample > >                                          m_apLoop;
    std::unique_ptr< sjf::rev::fdn< Sample > >                                                  m_fdn;
    
    
    std::vector< std::vector< Sample > >                                                        m_rotDelDif_DT;
    std::vector< std::vector< Sample > >                                                        m_multiTap_DT;
    std::vector< std::vector< Sample > >                                                        m_seriesAP_DT;
    std::vector< std::vector< Sample > >                                                        m_apLoop_DT;
    std::vector< Sample >                                                                       m_fdn_DT;
    
    
    unsigned rdd_NCHANNELS{8}, rdd_NSTAGES{5};
    unsigned mt_NTAPS{32};
    unsigned sap_NSTAGES{8};
    unsigned apl_NSTAGES{6}, apl_NAP_PERSTAGE{2};
    unsigned fdn_NCHANNELS{8};
    
    unsigned NCHANNELS{2};
    
    static constexpr sjf::ctr::rArray< Sample, 4096, UNIX_TIMESTAMP > m_vn; // random values for delayTimes
    
    Sample m_SR{44100};
    
    std::vector< std::vector< sjf::rev::dtModulatorVoice< Sample > > >                          m_sap_modulators;
    std::vector< std::vector< sjf::rev::dtModulatorVoice< Sample > > >                          m_rdd_modulators;
    std::vector< sjf::rev::dtModulatorVoice< Sample > >                                         m_fdn_modulators;
    std::vector< std::vector< sjf::rev::dtModulatorVoice< Sample > > >                          m_apLoop_modulators;
    
    

public:
    // FOR EASY ACCESS BY PARENT CLASS
    Sample m_earlyDiff{0.7}, m_lateDiff{0.5}, m_erDamp{0.2}, m_lrDamp{0.2}, m_decay{1.0}, m_size{1.0}, m_modDepth{0.5}, m_modRate{1.0},
    m_modDamp{ static_cast<Sample>(1.0 - calculateLPFCoefficient(m_modRate, m_SR))}, m_modPhase{0};
    
    
    sjf::rev::phasor< Sample > m_modPhasor{ 1, m_SR };
};


#endif /* sjf_verb_dspWrapper_h */
