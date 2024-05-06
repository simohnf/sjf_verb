/*
  ==============================================================================

    sjf_verb.h
    Created: 1 May 2024 3:38:49pm
    Author:  Simon Fay

  ==============================================================================
*/

#pragma once

#include "../sjf_audio/sjf_audioUtilities.h"
#include "../sjf_audio/sjf_compileTimeRandom.h"
#include "../sjf_audio/sjf_parameterHandler.h"
#include "../sjf_audio/sjf_rev.h"
#include "../sjf_audio/sjf_lpf.h"
#include "parameterIDs.h"



template < typename Sample >
class sjf_verb
{
public:
//    typedef <#type#> <#name#>;
    
    sjf_verb(){}
    ~sjf_verb() { m_smoothers.clear(); }
    
    inline void process( juce::AudioBuffer<Sample>& buffer );
    
    void addParametersToHandler( juce::AudioProcessorValueTreeState &vts, juce::Array<juce::AudioProcessorParameter*>& params  );
    
    void initialise( Sample sampleRate, int numberOfChannels );
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout( );
private:
    class DSP_wrapper
    {
    public:
        DSP_wrapper(){}
        ~DSP_wrapper()
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
        
        void setInterpolationType( parameterIDs::interpTypesEnum type );
        
        void initialiseEarlyDSP( Sample sampleRate );
        void initialiseLateDSP( Sample sampleRate );
        
//        std::function< void( juce::AudioBuffer< Sample >& ) > processBlock;
        
        std::function< void( std::vector< Sample >& samples ) > erFunc { [ this ]( std::vector< Sample >& samples ) { return; } };
        std::function< void( std::vector< Sample >& samples ) > lrFunc { [ this ]( std::vector< Sample >& samples ) { return; } };
        
        inline void processER( std::vector< Sample >& samples ){ (this->*earlyReflectionsF)( samples ); }
        inline void processLR( std::vector< Sample >& samples ){ (this->*lateReflectionsF)( samples ); }
        
        
    private:
        
        void ( DSP_wrapper::*earlyReflectionsF )( std::vector< Sample >& ){ &sjf_verb< Sample >::DSP_wrapper::er_default };
        void ( DSP_wrapper::*lateReflectionsF )( std::vector< Sample >& ){ &sjf_verb< Sample >::DSP_wrapper::lr_default };
        
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
        
        std::unique_ptr< sjf::rev::rotDelDif< Sample > > m_rotDelDif;
        std::vector< std::unique_ptr< sjf::rev::multiTap< Sample > > > m_multiTap;
        std::vector< std::unique_ptr< sjf::rev::seriesAllpass< Sample > > > m_seriesAP;
        std::unique_ptr< sjf::rev::allpassLoop< Sample > > m_apLoop;
        std::unique_ptr< sjf::rev::fdn< Sample > > m_fdn;
        
        
        std::vector< std::vector< Sample > > m_rotDelDif_DT;
        std::vector< std::vector< Sample > > m_multiTap_DT;
        std::vector< std::vector< Sample > > m_seriesAP_DT;
        std::vector< Sample > m_apLoop_DT;
        std::vector< Sample > m_fdn_dt;
        
        int m_interpType{1};
        unsigned rdd_NCHANNELS{8}, rdd_NSTAGES{5};
        unsigned mt_NTAPS{32};
        unsigned sap_NSTAGES{8};
        unsigned apl_NSTAGES{6}, apl_NAP_PERSTAGE{2};
        unsigned fdn_NCHANNELS{8};
        
        unsigned NCHANNELS = 2;
        
        static constexpr sjf::ctr::rArray< Sample, 4096, UNIX_TIMESTAMP > m_vn; // random values for delayTimes
        
        Sample m_SR = 44100;
    };
    
    
    
    sjf::parameterHandler::paramHandlerVector m_paramHandler;
    
    std::vector< juce::LinearSmoothedValue< Sample > > m_smoothers;
    std::unordered_map<  parameterIDs::idsenum, juce::LinearSmoothedValue< Sample >* > m_smootherMap;
    
    
    Sample m_SR = 44100;
    
    DSP_wrapper m_dspWrap;
    
    std::vector< Sample > m_samples;
    
//    static const std::array< int, 10000 > primes;
    
    
};

