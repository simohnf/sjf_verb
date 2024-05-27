//
//  sjf_verb_lateProcessor_DSP_Wrappers.h
//  sjf_verb
//
//  Created by Simon Fay on 26/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_lateProcessor_DSP_Wrappers_h
#define sjf_verb_lateProcessor_DSP_Wrappers_h
namespace lateDSP
{
    template<typename Sample>
    using LSV = juce::LinearSmoothedValue< Sample >;
    template<typename Sample>
    using twoDArray = std::vector< std::vector < Sample > >;
    template<typename Sample>
    using randArray = sjf::ctr::rArray< Sample, 4096, UNIX_TIMESTAMP +'l'+'a'+'t'+'e' >;
    template <typename Sample>
    using phasor = sjf::oscillators::phasor< Sample >;
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template<typename Sample>
    struct varHolder
    {
        Sample size, diffusion, decay, lpfCO, hpfCO, mDepth, mRate, mDamp, mPhase, sampleRate{44100};
        LSV<Sample> m_sizeSmoother, m_decaySmoother, m_lpfSmoother, m_hpfSmoother, m_diffusionSmoother, m_modDSmoother, m_modRSmoother, m_modDampSmoother;
        sjf::oscillators::phasor< Sample > m_modPhasor{ 1, sampleRate };
        sjf::rev::mixers fdnMix{ sjf::rev::mixers::hadamard };
        bool ControlFB{false};
        void process()
        {
            size = m_sizeSmoother.getNextValue();
            diffusion = m_diffusionSmoother.getNextValue();
            decay = m_decaySmoother.getNextValue();
            lpfCO = m_lpfSmoother.getNextValue();
            hpfCO = m_hpfSmoother.getNextValue();
            mDepth = m_modDSmoother.getNextValue();
            mRate = m_modRSmoother.getNextValue();
            mDamp = m_modDampSmoother.getNextValue();
            m_modPhasor.setFrequency( mRate, sampleRate );
            mPhase = m_modPhasor.process();
        }  
    };
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template<typename Sample>
    struct fdnWrapper
    {
        fdnWrapper( size_t nChannels, const randArray<Sample>& rArr, Sample sampleRate ) : NCHANNELS(nChannels), fdn(nChannels)
        {
            auto randCount = 0;
            static constexpr Sample MAXDELAYTIMEMS = 100;
            const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
            const auto maxDtAPSamps = maxDtSamps * 0.25;
            m_DTs.resize( NCHANNELS );
            m_apDTs.resize( NCHANNELS );
            m_modulators.resize( NCHANNELS );
            m_apModulators.resize( NCHANNELS );
            fdn.initialise( maxDtSamps, maxDtAPSamps, maxDtSamps );
            auto moffset = 1.0 / static_cast< Sample >( NCHANNELS * 2 );
            for ( auto i = 0; i < NCHANNELS; i ++ )
            {
                /* Delay */
                m_DTs[ i ] = ( (rArr[ randCount++ ] * 0.5) + 0.5 ) * maxDtSamps;
                fdn.setDelayTime( m_DTs[ i ], i );
                m_modulators[ i ].initialise( moffset * i, m_DTs[ i ] );
                /* Allpass */
                m_apDTs[ i ] = ( (rArr[ randCount++ ] * 0.5) + 0.5 ) * maxDtAPSamps;
                fdn.setAPTime( m_apDTs[ i + NCHANNELS ], i );
                m_apModulators[ i ].initialise( moffset * ( i + NCHANNELS ), m_apDTs[ i ] );
            }
        }
        void processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize, varHolder<Sample>& vars )
        {
            std::vector<Sample> samps ( NCHANNELS, 0 );
            Sample dt = 0;
            fdn.setMixType( vars.fdnMix );
            fdn.setControlFB( vars.ControlFB );
            for( auto i = 0; i < blockSize; i++ )
            {
                for ( auto c = 0; c < NCHANNELS; c++ )
                    samps[ c ] = buffer.getSample( c, i );
                vars.process();
                fdn.setDiffusion( vars.diffusion );
                fdn.setDamping( vars.lpfCO );
                fdn.setDampingLow( vars.hpfCO );
                for ( auto c = 0; c < NCHANNELS; c++ )
                {
                    dt = m_modulators[c].process( m_DTs[c], vars.mPhase, vars.mDepth, vars.mDamp );
                    fdn.setDelayTime( dt * vars.size, c );
                    dt = m_apModulators[c].process( m_apDTs[c], vars.mPhase, vars.mDepth, vars.mDamp );
                    fdn.setAPTime( dt * vars.size, c );
                }
                fdn.setDecay( vars.decay );
                fdn.processInPlace( samps );
                
                for ( auto c = 0; c < NCHANNELS; c++ )
                    buffer.setSample( c, i, samps[ c ] );
            }
        }
        
    private:
        const size_t NCHANNELS;
        sjf::rev::fdn<Sample> fdn;
        std::vector<Sample> m_DTs, m_apDTs;
        std::vector< sjf::rev::dtModulatorVoice< Sample > > m_modulators, m_apModulators;
    };
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template<typename Sample>
    struct apLoopWrapper
    {
        apLoopWrapper( size_t nChannels, size_t nStages, size_t apPerStage, const randArray<Sample>& rArr, Sample sampleRate ) : NCHANNELS(nChannels), NSTAGES(nStages), APPERSTAGE(apPerStage), apLoop(nStages, apPerStage)
        {
            auto randCount = 0;
            static constexpr Sample MAXDELAYTIMEMS = 100;
            const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
            m_DTs.resize( NSTAGES );
            apLoop.initialise( sampleRate );
            auto moffset = 1.0 / static_cast< Sample >( NSTAGES );
            auto offsetCount = 0;
            m_modulators.resize( NSTAGES );
            for ( auto i = 0; i < NSTAGES; i++ )
            {
                m_DTs[ i ].resize( APPERSTAGE+1 );
                m_modulators[ i ].resize( APPERSTAGE+1 );
                auto targetTime = ((rArr[ randCount ] * 0.6) + 0.4 ) * maxDtSamps;
                m_DTs[ i ][ APPERSTAGE ] = targetTime; // decay time needs to be a little less than sum of preceeding APs
    //            DO MODULATOR
                m_modulators[ i ][ APPERSTAGE ].initialise( moffset*offsetCount, targetTime );
                apLoop.setDelayTimeSamples( targetTime, i, APPERSTAGE );
                randCount++;
                offsetCount++;
                auto approxAPTime = m_DTs[ i ][ APPERSTAGE ] / APPERSTAGE;
                for ( auto j = 0; j < APPERSTAGE; j++ )
                {
                    auto APTime = ((rArr[ randCount ] * 0.75) + 0.5 ) * approxAPTime;
                    if ( j == APPERSTAGE-1)
                        while ( APTime <= targetTime )
                            APTime *= 1.5;
                    m_DTs[ i ][ j ] = APTime;
                    apLoop.setDelayTimeSamples( APTime, i, j );
                    m_modulators[ i ][ j ].initialise( moffset*offsetCount, APTime );
                    targetTime -= APTime;
                    randCount++;
                    offsetCount++;
                }
            }

        }
        void processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize, varHolder<Sample>& vars )
        {
            Sample dt = 0;
            std::vector< Sample > samps( NCHANNELS, 0 );
            apLoop.setControlFB( vars.ControlFB );
            for ( auto i = 0; i < blockSize; i++ )
            {
                for ( auto c = 0; c < NCHANNELS; c++ )
                    samps[ c ] = buffer.getSample( c, i );
                vars.process();
                apLoop.setDiffusion( vars.diffusion );
                apLoop.setDamping( vars.lpfCO );
                apLoop.setDampingLow( vars.hpfCO );
                for ( auto s = 0; s < NSTAGES; s++ )
                {
                    for ( auto d = 0; d < APPERSTAGE+1; d++ )
                    {
                        dt = m_modulators[s][d].process( m_DTs[s][d], vars.mPhase, vars.mDepth, vars.mDamp );
                        apLoop.setDelayTimeSamples( dt * vars.size, s, d );
                    }
                }
                apLoop.setDecay( vars.decay );
                apLoop.processInPlace( samps );
                
                for ( auto c = 0; c < NCHANNELS; c++ )
                    buffer.setSample( c, i, samps[ c ] );
            }
        }
        
    private:
        const size_t NCHANNELS, NSTAGES, APPERSTAGE;
        sjf::rev::allpassLoop<Sample> apLoop;
        twoDArray<Sample> m_DTs;
        std::vector< std::vector< sjf::rev::dtModulatorVoice< Sample > > > m_modulators;
    };
}

#endif /* sjf_verb_lateProcessor_DSP_Wrappers_h */
