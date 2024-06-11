//
//  sjf_verb_earlyProcessor_DSP_Wrappers.h
//  sjf_verb
//
//  Created by Simon Fay on 26/05/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_earlyProcessor_DSP_Wrappers_h
#define sjf_verb_earlyProcessor_DSP_Wrappers_h

namespace earlyDSP
{
    template<typename Sample>
    using LSV = juce::LinearSmoothedValue< Sample >;
    template<typename T>
    using vect = std::vector<T>;
    template<typename T>
    using twoDArray = vect< vect< T > >;
    template<typename Sample>
    using randArray = const sjf::ctr::rArray< Sample, 4096, UNIX_TIMESTAMP +'e'+'a'+'r'+'l'+'y' >;
    template <typename Sample>
    using phasor = sjf::oscillators::phasor< Sample >;
    template <typename Sample>
    using modulator = sjf::modulator::modVoice< Sample >;
//    template <typename Sample>
//    using modulator = sjf::rev::dtModulatorVoice< Sample >;
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template< typename Sample >
    struct varHolder
    {
        Sample size, diffusion, mDepth, mRate, mDamp, mPhase, sampleRate{44100};
        LSV<Sample> m_sizeSmoother, m_diffusionSmoother, m_modDSmoother, m_modRSmoother, m_modDampSmoother;
        sjf::oscillators::phasor< Sample > m_modPhasor{ 1, sampleRate };

        void process()
        {
            size = m_sizeSmoother.getNextValue();
            diffusion = m_diffusionSmoother.getNextValue();
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
    template< typename Sample, typename INTERPOLATION = sjf::interpolation::fourPointInterpolatePD< Sample > >
    struct rddWrapper
    {
        rddWrapper( const size_t nChannels, const size_t nStages, const randArray<Sample>& rArr, const Sample sampleRate ) : NCHANNELS(nChannels), NSTAGES(nStages), NMODCHANNELS(std::sqrt(NCHANNELS)), rdd(NCHANNELS,NSTAGES,NMODCHANNELS)
        {
            auto randCount = 0;
            static constexpr Sample MAXDELAYTIMEMS = 100;
            const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
            rdd.initialise( sampleRate, maxDtSamps*2 );
            
            sjf::utilities::vectorResize( m_DTs, NSTAGES, NCHANNELS, static_cast<Sample>(0) );
            sjf::utilities::vectorResize( m_modulators, NSTAGES, NMODCHANNELS );
            Sample stageLen = maxDtSamps / ( std::pow( 2, NSTAGES ) - 1 );
            auto moffset = 1.0 / static_cast< Sample >( NSTAGES * NMODCHANNELS );
            vect< size_t > channelShuffle( NCHANNELS );
            for ( auto i = 0; i < NSTAGES; ++i )
            {
                /* SHUFFLE */
                Sample chanLen = stageLen / NCHANNELS;
                // generate delay times and then shuffle
                for ( auto j = 0; j < NCHANNELS; ++j )
                    m_DTs[ i ][ j ] = ( rArr[ ++randCount ] * chanLen ) + ( chanLen * j );
                for ( auto j = 0; j < NCHANNELS-1; ++j )
                    std::swap( m_DTs[ i ][ j ], m_DTs[ i ][ (static_cast< int >( rArr[ ++randCount ] * ( NCHANNELS - j ) ) + j) ] );
                for ( auto j = 0; j < NCHANNELS; ++j )
                {
                    /* POLARITY FLIPS */
                    rdd.setPolarityFlip( (rArr[ ++randCount ] >= 0.7), i, j );
                    
                    /* VELVET NOISE DELAY TIMES */
                    rdd.setDelayTime( m_DTs[ i ][ j ], i, j );
                }
                for ( auto j = 0; j < NMODCHANNELS; ++j )
                {
                    m_modulators[ i ][ j ].initialise( moffset * (j + i*NMODCHANNELS), m_DTs[ i ][ j ] );
                }
                stageLen *= 2.0;
            }
        }
        void processBlock( juce::AudioBuffer< Sample >& buffer, const size_t blockSize, varHolder<Sample>& vars )
        {
            vect< Sample > samps( NCHANNELS, 0 );
            for ( auto i = 0; i < blockSize; ++i )
            {
                vars.process();
                for ( auto s = 0; s < NSTAGES; ++s )
                {
                    for ( auto c= 0; c < NMODCHANNELS; ++c )
                        rdd.setDelayTime( m_modulators[s][c].process( m_DTs[s][c], vars.mPhase, vars.mDepth, vars.mDamp ) * vars.size, s, c );
                    for ( auto c= NMODCHANNELS; c < NCHANNELS; ++c )
                        rdd.setDelayTime( m_DTs[s][c] * vars.size, s, c );
                }
                for( auto c = 0; c < NCHANNELS; ++c )
                    samps[ c ] = buffer.getSample( c, i );
                rdd.processInPlace( samps );
                for ( auto c = 0; c < NCHANNELS; ++c )
                    buffer.setSample( c, i, samps[ c ] );
            }
        }
        
    private:
        const size_t NCHANNELS, NSTAGES, NMODCHANNELS;
        twoDArray<Sample> m_DTs;
        sjf::rev::rotDelDif< Sample > rdd;
        twoDArray< modulator< Sample > > m_modulators;
    };
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template< typename Sample, typename INTERPOLATION = sjf::interpolation::fourPointInterpolatePD< Sample > >
    struct mtWrapper
    {
        mtWrapper( const size_t nChannels, const size_t nTaps, const randArray<Sample>& rArr, const Sample sampleRate ) : NCHANNELS(nChannels), NTAPS(nTaps), MODMAX( std::sqrt(NTAPS) ), mt( NCHANNELS, NTAPS )
        {
            auto randCount = 0;
            static constexpr Sample MAXDELAYTIMEMS = 250;
            const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
            const auto moffset = 1.0 / static_cast<Sample>( NCHANNELS * NTAPS );
            sjf::utilities::vectorResize( m_DTs, NCHANNELS, NTAPS, static_cast<Sample>(0) );
            auto vnBand = maxDtSamps / NTAPS;
            sjf::utilities::vectorResize( m_gains, NCHANNELS, NTAPS, static_cast<Sample>(0) );
            sjf::utilities::vectorResize( m_modulators, NCHANNELS, MODMAX );
            for ( auto i = 0; i < NCHANNELS; ++i )
            {
                auto sum = 0.0;
                mt[ i ].initialise( static_cast< long >( maxDtSamps ) );
                for ( auto j = 0; j < NTAPS; ++j )
                {
                    m_DTs[ i ][ j ] = ( rArr[ ++randCount ]*vnBand ) + ( vnBand * j );
                    mt[ i ].setDelayTimeSamps( m_DTs[ i ][ j ], j );
                    
                    auto g = static_cast< Sample > ( ( NTAPS + 1 ) - j ) / static_cast< Sample > ( NTAPS + 1 );
                    sum += g;
                    g *= (rArr[ ++randCount ] < 0.666) ? g : -g; // exponential decay
                    m_gains[ i ][ j ] = g;   
                }
                auto invSum { std::sqrt(1.0/sum) };
                for ( auto j = 0; j < NTAPS; ++j )
                {
                    m_gains[ i ][ j ] *= invSum * 0.5;
                    mt[ i ].setGain( m_gains[ i ][ j ], j );
                }
                for ( auto j = 0; j < MODMAX; ++j )
                    m_modulators[ i ][ j ].initialise( moffset*( NTAPS*i + j ), m_gains[ i ][ j ] );
            }
        }
        void processBlock( juce::AudioBuffer< Sample >& buffer, const size_t blockSize, varHolder<Sample>& vars )
        {
            for ( auto i = 0; i < blockSize; ++i )
            {
                vars.process();
                for ( auto c = 0; c < NCHANNELS; ++c )
                {
                    for ( auto t = 0; t < NTAPS; ++t )
                        mt[ c ].setDelayTimeSamps( m_DTs[ c ][ t ] * vars.size, t );
                    for ( auto t  = 0; t < MODMAX; ++t )
                        mt[ c ].setGain( m_modulators[ c ][ t ].process( m_gains[c][t], vars.mPhase, vars.mDepth, vars.mDamp ), t );
                    buffer.setSample( c, i, mt[ c ].process( buffer.getSample( c, i ) ) );
                }
            }
        }
    private:
        const size_t NCHANNELS, NTAPS, MODMAX;
        twoDArray<Sample> m_DTs, m_gains;
        vect< sjf::rev::multiTap< Sample > > mt;
        twoDArray< modulator< Sample > > m_modulators;
    };
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template< typename Sample, typename INTERPOLATION = sjf::interpolation::fourPointInterpolatePD< Sample > >
    struct sapWrapper
    {
        sapWrapper( const size_t nChannels, const size_t nStages, const randArray<Sample>& rArr, const Sample sampleRate ) : NCHANNELS(nChannels), NSTAGES(nStages), sap( NCHANNELS, NSTAGES )
        {
            auto randCount = 0;
            static constexpr Sample MAXDELAYTIMEMS = 20;
            const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
            auto moffset = 1.0 / static_cast< Sample >( NCHANNELS * NSTAGES );
            sjf::utilities::vectorResize( m_modulators, NCHANNELS, NSTAGES );
            sjf::utilities::vectorResize( m_DTs, NCHANNELS, NSTAGES, static_cast<Sample>(0) );
            for ( auto i = 0; i < NCHANNELS; ++i )
            {
                sap[ i ].initialise( static_cast<size_t>(maxDtSamps*2) );
                for ( auto j = 0; j < NSTAGES; ++j )
                {
                    m_DTs[ i ][ j ] = (rArr[ ++randCount ] * 0.75 + 0.25 )* maxDtSamps;
                    sap[ i ].setDelayTime( m_DTs[ i ][ j ], j );
                    m_modulators[ i ][ j ].initialise( moffset * ( i*NSTAGES + j ), m_DTs[ i ][ j ] );
                }
            }
        }
        
        
        void processBlock( juce::AudioBuffer< Sample >& buffer, const size_t blockSize, varHolder<Sample>& vars )
        {
            Sample dt = 0;
            for ( auto i = 0; i < blockSize; ++i )
            {
                vars.process();
                for ( auto c = 0; c < NCHANNELS; ++c )
                {
                    sap[ c ].setCoefs( vars.diffusion );
                    for ( auto s = 0; s < NSTAGES; ++s )
                    {
                        dt = m_modulators[c][s].process( m_DTs[c][s], vars.mPhase, vars.mDepth, vars.mDamp );
                        sap[ c ].setDelayTime( dt * vars.size, s );
                    }
                    buffer.setSample( c, i, sap[ c ].process( buffer.getSample( c, i ) ) );
                }
            }
        }
    private:
        const size_t NCHANNELS, NSTAGES;
        twoDArray<Sample> m_DTs;
        vect< sjf::rev::seriesAllpass< Sample > > sap;
        twoDArray< modulator< Sample > > m_modulators;
    };
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template< typename Sample, typename INTERPOLATION = sjf::interpolation::fourPointInterpolatePD< Sample > >
    struct mtsapWrapper
    {
        mtsapWrapper( const size_t nChannels, const size_t nSAPStages, const size_t nTaps, const randArray<Sample>& rArr, const Sample sampleRate ) :
                    NCHANNELS(nChannels), NSTAGES(nSAPStages), NTAPS(nTaps), sap( NCHANNELS, NSTAGES ), mt(NCHANNELS, NTAPS)
        {
            auto randCount = 0;
            /* INITIALISE MULTITAP */
            static constexpr Sample MAXDELAYTIMEMSMT = 250;
            const auto maxDtSampsMT = MAXDELAYTIMEMSMT * 0.001 * sampleRate;
            auto moffset = 1.0 / static_cast< Sample >( NCHANNELS*NTAPS + NCHANNELS * NSTAGES  );
            sjf::utilities::vectorResize( m_mtDTs, NCHANNELS, NTAPS, static_cast<Sample>(0) );
            sjf::utilities::vectorResize( m_mtGains, NCHANNELS, NTAPS, static_cast<Sample>(0) );
//            sjf::utilities::vectorResize( m_mtModulators, NCHANNELS, NTAPS );
            auto vnBand = maxDtSampsMT / NTAPS;
            for ( auto i = 0; i < NCHANNELS; ++i )
            {
                auto sum = 0.0;
                mt[ i ].initialise( static_cast< long >( maxDtSampsMT ) );
                for ( auto j = 0; j < NTAPS; ++j )
                {
                    m_mtDTs[ i ][ j ] = ( rArr[ ++randCount ]*vnBand ) + ( vnBand * j );
                    mt[ i ].setDelayTimeSamps( m_mtDTs[ i ][ j ], j );
                    
                    auto g = static_cast< Sample > ( ( NTAPS + 1 ) - j ) / static_cast< Sample > ( NTAPS + 1 );
                    sum += g;
                    g *= (rArr[ ++randCount ] < 0.666) ? g : -g; // exponential decay
                    m_mtGains[ i ][ j ] = g;
                }

                auto invSum { std::sqrt(1.0/sum) };
                for ( auto j = 0; j < NTAPS; ++j )
                {
                    m_mtGains[ i ][ j ] *= invSum * 0.5;
                    mt[ i ].setGain( m_mtGains[ i ][ j ], j );
//                    m_mtModulators[ i ][ j ].initialise( moffset*( NTAPS*i + j ), m_mtGains[ i ][ j ] );
                }
            }
            
            /* INITIALISE SERIES ALLPASS */
            static constexpr Sample MAXDELAYTIMEMSSAP = 20;
            const auto maxDtSampsSAP = MAXDELAYTIMEMSSAP * 0.001 * sampleRate;
            
            sjf::utilities::vectorResize( m_sapModulators, NCHANNELS, NSTAGES );
            sjf::utilities::vectorResize( m_sapDTs, NCHANNELS, NSTAGES, static_cast<Sample>(0) );
            for ( auto i = 0; i < NCHANNELS; ++i )
            {
                sap[ i ].initialise( static_cast<int>(maxDtSampsSAP*2) );
                for ( auto j = 0; j < NSTAGES; ++j )
                {
                    m_sapDTs[ i ][ j ] = (rArr[ ++randCount ] * 0.75 + 0.25 )* maxDtSampsSAP;
                    sap[ i ].setDelayTime( m_sapDTs[ i ][ j ], j );
                    m_sapModulators[ i ][ j ].initialise( moffset * ( NCHANNELS*NTAPS + i*NSTAGES + j ), m_sapDTs[ i ][ j ] );
                }
            }
        }
        
        void processBlock( juce::AudioBuffer< Sample >& buffer, const size_t blockSize, varHolder<Sample>& vars )
        {
            Sample samp = 0, dt = 0;
            for ( auto i = 0; i < blockSize; ++i )
            {
                vars.process();
                for ( auto c = 0; c < NCHANNELS; ++c )
                {
                    for ( auto t = 0; t < NTAPS; ++t )
                    {
                        mt[ c ].setDelayTimeSamps( m_mtDTs[ c ][ t ] * vars.size, t );
//                        mt[ c ].setGain( m_mtModulators[ c ][ t ].process( m_mtGains[c][t], vars.mPhase, vars.mDepth, vars.mDamp ), t );
                    }
                    samp = mt[ c ].process( buffer.getSample( c, i ) );
                    
                    sap[ c ].setCoefs( vars.diffusion );
                    for ( auto s = 0; s < NSTAGES; ++s )
                    {
                        dt = m_sapModulators[c][s].process( m_sapDTs[c][s], vars.mPhase, vars.mDepth, vars.mDamp );
                        sap[ c ].setDelayTime( dt * vars.size, s );
                    }
                    buffer.setSample( c, i, sap[ c ].process( samp ) );
                }
            }
        }
    private:
        const size_t NCHANNELS, NSTAGES, NTAPS;
        twoDArray<Sample> m_sapDTs, m_mtDTs, m_mtGains;
        vect< sjf::rev::seriesAllpass< Sample > > sap;
        vect< sjf::rev::multiTap< Sample > > mt;
        twoDArray< modulator< Sample > > /* m_mtModulators, */ m_sapModulators;
    };
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================

};
#endif /* earlyProcessor_DSP_Wrappers_h */
