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
    template<typename Sample>
    using twoDArray = std::vector< std::vector < Sample > >;
    template<typename Sample>
    using randArray = const sjf::ctr::rArray< Sample, 4096, UNIX_TIMESTAMP +'e'+'a'+'r'+'l'+'y' >;
    template <typename Sample>
    using phasor = sjf::oscillators::phasor< Sample >;
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
    template< typename Sample >
    struct rddWrapper
    {
        rddWrapper( size_t nChannels, size_t nStages, randArray<Sample>& rArr, Sample sampleRate ) : NCHANNELS(nChannels), NSTAGES(nStages), rdd(nChannels,nStages)
        {
            auto randCount = 0;
            static constexpr Sample MAXDELAYTIMEMS = 100;
            const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
            rdd.initialise( sampleRate, maxDtSamps*2 );
            
            m_DTs.resize( NSTAGES );
            m_modulators.resize( NSTAGES );
            
            Sample stageLen = maxDtSamps / ( std::pow( 2, NSTAGES ) - 1 );
            auto moffset = 1.0 / static_cast< Sample >( NSTAGES * NCHANNELS );
            std::vector< size_t > channelShuffle( NCHANNELS );
            for ( auto i = 0; i < NSTAGES; i++ )
            {
                /* SHUFFLE */
                std::iota( channelShuffle.begin(), channelShuffle.end(), 0 );
                for ( auto j = 0; j < NSTAGES-1; j++ )
                    std::swap( channelShuffle[ j ], channelShuffle[ (static_cast< int >( rArr[ ++randCount ] * ( NSTAGES - j ) ) + j) ] );
                rdd.setRotationMatrix( channelShuffle, i );
                Sample chanLen = stageLen / NCHANNELS;
    //            DO MODULATORS
                m_modulators[ i ].resize( NCHANNELS );
                m_DTs[ i ].resize( NCHANNELS );
                for ( auto j = 0; j < NCHANNELS; j++ )
                {
                    /* POLARITY FLIPS */
                    rdd.setPolarityFlip( (rArr[ ++randCount ] >= 0.7), i, j );
                    
                    /* VELVET NOISE DELAY TIMES */
                    m_DTs[ i ][ j ] = ( rArr[ ++randCount ] * chanLen ) + ( chanLen * j );
                    rdd.setDelayTime( m_DTs[ i ][ j ], i, j );
                    m_modulators[ i ][ j ].initialise( moffset * (j + i*NCHANNELS), m_DTs[ i ][ j ] );;
                }
                stageLen *= 2.0;
            }
        }
        void processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize, varHolder<Sample>& vars )
        {
            std::vector< Sample > samps( NCHANNELS, 0 );
            Sample dt = 0;
            for ( auto i = 0; i < blockSize; i++ )
            {
                vars.process();
                for ( auto s = 0; s < NSTAGES; s++ )
                    for ( auto c= 0; c < NCHANNELS; c++ )
                    {
                        dt = m_modulators[s][c].process( m_DTs[s][c], vars.mPhase, vars.mDepth, vars.mDamp );
                        rdd.setDelayTime( dt * vars.size, s, c );
                    }
                for( auto c = 0; c < NCHANNELS; c++ )
                    samps[ c ] = buffer.getSample( c, i );
                rdd.processInPlace( samps );
                for ( auto c = 0; c < NCHANNELS; c++ )
                    buffer.setSample( c, i, samps[ c ] );
            }
        }
        
    private:
        const size_t NCHANNELS, NSTAGES;
        twoDArray<Sample> m_DTs;
        sjf::rev::rotDelDif< Sample > rdd;
        std::vector< std::vector< sjf::rev::dtModulatorVoice< Sample > > > m_modulators;
    };
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template< typename Sample >
    struct mtWrapper
    {
        mtWrapper( size_t nChannels, size_t nTaps, randArray<Sample>& rArr, Sample sampleRate ) : NCHANNELS(nChannels), NTAPS(nTaps), mt( NCHANNELS, NTAPS )
        {
            auto randCount = 0;
            static constexpr Sample MAXDELAYTIMEMS = 250;
            const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
            m_DTs.resize( NCHANNELS );
            auto vnBand = maxDtSamps / NTAPS;
            for ( auto i = 0; i < NCHANNELS; i++ )
            {
                mt[ i ].initialise( static_cast< int >( maxDtSamps ) );
                m_DTs[ i ].resize( NTAPS );
                for ( auto j = 0; j < NTAPS; j++ )
                {
                    m_DTs[ i ][ j ] = ( rArr[ ++randCount ]*vnBand ) + ( vnBand * j );
                    mt[ i ].setDelayTimeSamps( m_DTs[ i ][ j ], j );
                    
                    auto g = static_cast< Sample > ( ( NTAPS + 1 ) - j ) / static_cast< Sample > ( NTAPS + 1 );
                    g *= rArr[ ++randCount ] < 0.666 ? g : -g; // exponential decay
                    mt[ i ].setGain( g, j );
                    
                }
            }
        }
        void processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize, varHolder<Sample>& vars )
        {
            std::vector< Sample > samps( NCHANNELS, 0 );
            for ( auto i = 0; i < blockSize; i++ )
            {
                vars.process();
                for ( auto c = 0; c < NCHANNELS; c++ )
                {
                    for ( auto t = 0; t < NTAPS; t++ )
                        mt[ c ].setDelayTimeSamps( m_DTs[ c ][ t ] * vars.size, t );
                    buffer.setSample( c, i, mt[ c ].process( buffer.getSample( c, i ) ) );
                }
            }
        }
    private:
        const size_t NCHANNELS, NTAPS;
        twoDArray<Sample> m_DTs;
        std::vector< sjf::rev::multiTap< Sample > > mt;
    };
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template< typename Sample >
    struct sapWrapper
    {
        sapWrapper( size_t nChannels, size_t nStages, randArray<Sample>& rArr, Sample sampleRate ) : NCHANNELS(nChannels), NSTAGES(nStages), sap( NCHANNELS, NSTAGES )
        {
            auto randCount = 0;
            static constexpr Sample MAXDELAYTIMEMS = 20;
            const auto maxDtSamps = MAXDELAYTIMEMS * 0.001 * sampleRate;
            auto moffset = 1.0 / static_cast< Sample >( NCHANNELS * NSTAGES );
            m_modulators.resize( NCHANNELS );
            m_DTs.resize( NCHANNELS );
            for ( auto i = 0; i < NCHANNELS; i++ )
            {
                sap[ i ].initialise( static_cast<int>(maxDtSamps*2) );
                m_DTs[ i ].resize( NSTAGES );
                m_modulators[ i ].resize( NSTAGES );
                for ( auto j = 0; j < NSTAGES; j++ )
                {
                    m_DTs[ i ][ j ] = (rArr[ ++randCount ] * 0.75 + 0.25 )* maxDtSamps;
                    sap[ i ].setDelayTime( m_DTs[ i ][ j ], j );
                    m_modulators[ i ][ j ].initialise( moffset * ( i*NSTAGES + j ), m_DTs[ i ][ j ] );
                }
            }
        }
        
        
        void processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize, varHolder<Sample>& vars )
        {
            Sample dt = 0;
            for ( auto i = 0; i < blockSize; i++ )
            {
                vars.process();
                for ( auto c = 0; c < NCHANNELS; c++ )
                {
                    sap[ c ].setCoefs( vars.diffusion );
                    for ( auto s = 0; s < NSTAGES; s++ )
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
        std::vector< sjf::rev::seriesAllpass< Sample > > sap;
        std::vector< std::vector< sjf::rev::dtModulatorVoice< Sample > > > m_modulators;
    };
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    template< typename Sample >
    struct mtsapWrapper
    {
        mtsapWrapper( size_t nChannels, size_t nSAPStages, size_t nTaps, randArray<Sample>& rArr, Sample sampleRate ) :
                    NCHANNELS(nChannels), NSTAGES(nSAPStages), NTAPS(nTaps), sap( NCHANNELS, NSTAGES ), mt(NCHANNELS, NTAPS)
        {
            auto randCount = 0;
            /* INITIALISE MULTITAP */
            static constexpr Sample MAXDELAYTIMEMSMT = 250;
            const auto maxDtSampsMT = MAXDELAYTIMEMSMT * 0.001 * sampleRate;
            m_mtDTs.resize( NCHANNELS );
            auto vnBand = maxDtSampsMT / NTAPS;
            for ( auto i = 0; i < NCHANNELS; i++ )
            {
                mt[ i ].initialise( static_cast< int >( maxDtSampsMT ) );
                m_mtDTs[ i ].resize( NTAPS );
                for ( auto j = 0; j < NTAPS; j++ )
                {
                    m_mtDTs[ i ][ j ] = ( rArr[ ++randCount ]*vnBand ) + ( vnBand * j );
                    mt[ i ].setDelayTimeSamps( m_mtDTs[ i ][ j ], j );
                    
                    auto g = static_cast< Sample > ( ( NTAPS + 1 ) - j ) / static_cast< Sample > ( NTAPS + 1 );
                    g *= rArr[ ++randCount ] < 0.666 ? g : -g; // exponential decay
                    mt[ i ].setGain( g, j );
                    
                }
            }
            
            /* INITIALISE SERIES ALLPASS */
            static constexpr Sample MAXDELAYTIMEMSSAP = 20;
            const auto maxDtSampsSAP = MAXDELAYTIMEMSSAP * 0.001 * sampleRate;
            auto moffset = 1.0 / static_cast< Sample >( NCHANNELS * NSTAGES );
            m_modulators.resize( NCHANNELS );
            m_sapDTs.resize( NCHANNELS );
            for ( auto i = 0; i < NCHANNELS; i++ )
            {
                sap[ i ].initialise( static_cast<int>(maxDtSampsSAP*2) );
                m_sapDTs[ i ].resize( NSTAGES );
                m_modulators[ i ].resize( NSTAGES );
                for ( auto j = 0; j < NSTAGES; j++ )
                {
                    m_sapDTs[ i ][ j ] = (rArr[ ++randCount ] * 0.75 + 0.25 )* maxDtSampsSAP;
                    sap[ i ].setDelayTime( m_sapDTs[ i ][ j ], j );
                    m_modulators[ i ][ j ].initialise( moffset * ( i*NSTAGES + j ), m_sapDTs[ i ][ j ] );
                }
            }
        }
        
        void processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize, varHolder<Sample>& vars )
        {
            Sample samp = 0, dt = 0;
            for ( auto i = 0; i < blockSize; i++ )
            {
                vars.process();
                for ( auto c = 0; c < NCHANNELS; c++ )
                {
                    for ( auto t = 0; t < NTAPS; t++ )
                        mt[ c ].setDelayTimeSamps( m_mtDTs[ c ][ t ] * vars.size, t );
                    samp = mt[ c ].process( buffer.getSample( c, i ) );
                    
                    sap[ c ].setCoefs( vars.diffusion );
                    for ( auto s = 0; s < NSTAGES; s++ )
                    {
                        dt = m_modulators[c][s].process( m_sapDTs[c][s], vars.mPhase, vars.mDepth, vars.mDamp );
                        sap[ c ].setDelayTime( dt * vars.size, s );
                    }
                    buffer.setSample( c, i, sap[ c ].process( samp ) );
                }
            }
        }
    private:
        const size_t NCHANNELS, NSTAGES, NTAPS;
        twoDArray<Sample> m_sapDTs, m_mtDTs;
        std::vector< sjf::rev::seriesAllpass< Sample > > sap;
        std::vector< sjf::rev::multiTap< Sample > > mt;
        std::vector< std::vector< sjf::rev::dtModulatorVoice< Sample > > > m_modulators;
    };
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================
    //======================//======================//======================//======================//======================

};
#endif /* earlyProcessor_DSP_Wrappers_h */
