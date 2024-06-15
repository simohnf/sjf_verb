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
    template<typename T>
    using vect = std::vector<T>;
    template<typename T>
    using twoDArray = vect< vect<T> >;
    template<typename Sample>
    using randArray = sjf::ctr::rArray< Sample, 4096, UNIX_TIMESTAMP +'l'+'a'+'t'+'e' >;
    template <typename Sample>
    using phasor = sjf::oscillators::phasor< Sample >;
    template <typename Sample>
    using modulator = sjf::modulator::modVoice< Sample >;
    static constexpr size_t PRIME_MAX{10000};
    using primeArray = sjf::utilities::primes<PRIME_MAX>;

    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    struct primeDelayTimes
    {
        primeDelayTimes() : nPrimes(m_primes.getNPrimes()), m_used(m_primes.getNPrimes(), false), m_powers(m_primes.getNPrimes(), 0){}
        ~primeDelayTimes(){}
        
        size_t closestPower( const size_t target )
        {
            auto output = 0;
            auto n = 0;
            auto pn = static_cast<size_t>(0);
            auto pow = 1;
            auto diff = target;
            for ( auto i = 0; i < nPrimes; ++i )
            {
                if( !m_used[i] )
                {
                    pow = 1;
                    pn = m_primes.nthPrime( i );
                    while ( std::pow(pn, pow) < target )
                        ++pow;
                    pow = (pow == 1) ? 1 : target - std::pow(pn, pow-1) < std::pow(pn, pow) -target ? pow-1 : pow;
                    auto newVal = std::pow(pn, pow);
                    auto newDiff = target > newVal ? target - newVal : newVal - target;
                    if(  newDiff < diff )
                    {
                        output = newVal;
                        diff = newDiff;
                        n = i;
                    }
                }
            }
            m_used[ n ] = true;
            return output > 0 ? output : target;
        }
    private:
        static constexpr primeArray m_primes;
        const size_t nPrimes;
        std::vector<bool> m_used;
        std::vector<size_t> m_powers;
    };



    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template<typename Sample>
    struct varHolder
    {
        varHolder(){ DBG("created late varHolder"); }
        Sample size, diffusion, decay, lpfCO, hpfCO, mDepth, mRate, mDamp, mPhase, sampleRate{44100};
        LSV<Sample> m_sizeSmoother, m_decaySmoother, m_lpfSmoother, m_hpfSmoother, m_diffusionSmoother, m_modDSmoother, m_modRSmoother, m_modDampSmoother;
        sjf::oscillators::phasor< Sample > m_modPhasor{ 1, sampleRate };
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
    template<typename Sample, sjf::mixers::mixerTypes mixType = sjf::mixers::mixerTypes::householder, sjf::rev::fbLimiters::fbLimiterTypes limitType  = sjf::rev::fbLimiters::fbLimiterTypes::none, sjf::interpolation::interpolatorTypes interpType = sjf::interpolation::interpolatorTypes::pureData >
    struct fdnWrapper
    {
        fdnWrapper( const size_t nChannels, const randArray<Sample>& rArr, const Sample sampleRate ) : NCHANNELS(nChannels), fdn(nChannels)
        {
            primeDelayTimes pdt;
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
            for ( auto i = 0; i < NCHANNELS; ++i )
            {
                /* Delay */
                m_DTs[ i ] = pdt.closestPower( ( (rArr[ ++randCount ] * 0.4) + 0.6 ) * maxDtSamps );
                fdn.setDelayTime( m_DTs[ i ], i );
                m_modulators[ i ].initialise( moffset * i, m_DTs[ i ] );
                /* Allpass */
                m_apDTs[ i ] = pdt.closestPower( ( (rArr[ ++randCount ] * 0.5) + 0.5 ) * maxDtAPSamps );
                fdn.setAPTime( m_apDTs[ i + NCHANNELS ], i );
                m_apModulators[ i ].initialise( moffset * ( i + NCHANNELS ), m_apDTs[ i ] );
            }
        }
        void processBlock( juce::AudioBuffer< Sample >& buffer, const size_t blockSize, varHolder<Sample>& vars )
        {
            vect<Sample> samps ( NCHANNELS, 0 );
            Sample dt = 0;
            for( auto i = 0; i < blockSize; ++i )
            {
                for ( auto c = 0; c < NCHANNELS; ++c )
                    samps[ c ] = buffer.getSample( c, i );
                vars.process();
                fdn.setDiffusion( vars.diffusion );
                fdn.setDamping( vars.lpfCO );
                fdn.setDampingLow( vars.hpfCO );
                for ( auto c = 0; c < NCHANNELS; ++c )
                {
                    dt = m_modulators[c].process( m_DTs[c], vars.mPhase, vars.mDepth, vars.mDamp );
                    fdn.setDelayTime( dt * vars.size, c );
                    dt = m_apModulators[c].process( m_apDTs[c], vars.mPhase, vars.mDepth, vars.mDamp );
                    fdn.setAPTime( dt * vars.size, c );
                }
                fdn.setDecay( vars.decay );
                fdn.processInPlace( samps );
                
                for ( auto c = 0; c < NCHANNELS; ++c )
                    buffer.setSample( c, i, samps[ c ] );
            }
        }
        
    private:
        const size_t NCHANNELS;
        sjf::rev::fdn<Sample, mixType, limitType, interpType> fdn;
        vect<Sample> m_DTs, m_apDTs;
        vect< modulator<Sample> > m_modulators, m_apModulators;
    };
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template<typename Sample, sjf::rev::fbLimiters::fbLimiterTypes limitType  = sjf::rev::fbLimiters::fbLimiterTypes::none, sjf::interpolation::interpolatorTypes interpType = sjf::interpolation::interpolatorTypes::pureData >
    struct apLoopWrapper
    {
        apLoopWrapper( const size_t nChannels, const size_t nStages, const size_t apPerStage, const randArray<Sample>& rArr, const Sample sampleRate ) : NCHANNELS(nChannels), NSTAGES(nStages), APPERSTAGE(apPerStage), apLoop(nStages, apPerStage)
        {
            primeDelayTimes pdt;
            auto randCount = 0;
            static constexpr Sample MAXDELAYTIMEMS = 100;
            const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
            sjf::utilities::vectorResize( m_DTs, NSTAGES, APPERSTAGE+1, static_cast<Sample>(0) );
            sjf::utilities::vectorResize( m_modulators, NSTAGES, APPERSTAGE+1 );
            apLoop.initialise( sampleRate );
            auto moffset = 1.0 / static_cast< Sample >( NSTAGES*( APPERSTAGE+1 ) );
            for ( auto i = 0; i < NSTAGES; ++i )
            {
                auto targetTime = pdt.closestPower( ((rArr[ ++randCount ] * 0.6) + 0.4 ) * maxDtSamps );
                m_DTs[ i ][ APPERSTAGE ] = targetTime; // decay time needs to be a little less than sum of preceeding APs
    //            DO MODULATOR
                m_modulators[ i ][ APPERSTAGE ].initialise( moffset*(i*(APPERSTAGE+1) + APPERSTAGE ), targetTime );
                apLoop.setDelayTimeSamples( targetTime, i, APPERSTAGE );
                auto approxAPTime = m_DTs[ i ][ APPERSTAGE ] / APPERSTAGE;
                for ( auto j = 0; j < APPERSTAGE; ++j )
                {
                    auto APTime = ((rArr[ ++randCount ] * 0.75) + 0.5 ) * approxAPTime;
                    if ( j == APPERSTAGE-1)
                        while ( APTime <= targetTime )
                            APTime *= 1.5;
                    APTime = pdt.closestPower( APTime );
                    m_DTs[ i ][ j ] = APTime;
                    apLoop.setDelayTimeSamples( APTime, i, j );
                    m_modulators[ i ][ j ].initialise( moffset*(i*(APPERSTAGE+1) + j ), APTime );
                    targetTime -= APTime;
                }
            }

        }
        void processBlock( juce::AudioBuffer< Sample >& buffer, const size_t blockSize, varHolder<Sample>& vars )
        {
            Sample dt = 0;
            vect< Sample > samps( NCHANNELS, 0 );
            for ( auto i = 0; i < blockSize; ++i )
            {
                for ( auto c = 0; c < NCHANNELS; ++c )
                    samps[ c ] = buffer.getSample( c, i );
                vars.process();
                apLoop.setDiffusion( vars.diffusion );
                apLoop.setDamping( vars.lpfCO );
                apLoop.setDampingLow( vars.hpfCO );
                for ( auto s = 0; s < NSTAGES; ++s )
                {
                    for ( auto d = 0; d < APPERSTAGE+1; ++d )
                    {
                        dt = m_modulators[s][d].process( m_DTs[s][d], vars.mPhase, vars.mDepth, vars.mDamp );
                        apLoop.setDelayTimeSamples( dt * vars.size, s, d );
                    }
                }
                apLoop.setDecay( vars.decay );
                apLoop.processInPlace( samps );
                
                for ( auto c = 0; c < NCHANNELS; ++c )
                    buffer.setSample( c, i, samps[ c ] );
            }
        }
        
    private:
        const size_t NCHANNELS, NSTAGES, APPERSTAGE;
        sjf::rev::allpassLoop<Sample, limitType, interpType> apLoop;
        twoDArray<Sample> m_DTs;
        twoDArray< modulator<Sample> > m_modulators;
    };
}

#endif /* sjf_verb_lateProcessor_DSP_Wrappers_h */
