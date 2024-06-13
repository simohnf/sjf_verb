//
//  sjf_verb_earlyProcessingWrapper.h
//  sjf_verb
//
//  Created by Simon Fay on 12/06/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verb_earlyProcessingWrapper_h
#define sjf_verb_earlyProcessingWrapper_h
//
//using Sample = float;
//template< sjf::interpolation::interpolatorTypes interpType = sjf::interpolation::interpolatorTypes::pureData >
//struct earlyWrapper
//{
//    enum class modTypes{ random, sin };
//    using earlyRandMod = sjf_verb_earlyProcessor<sjf::modulator::randMod<Sample>, interpType>;
//    using earlySinMod = sjf_verb_earlyProcessor<sjf::modulator::sinMod<Sample>, interpType>;
//    using earlyRandModPtr = std::unique_ptr< sjf_verb_earlyProcessor<sjf::modulator::randMod<Sample>, interpType> >;
//    using earlySinModPtr = std::unique_ptr< sjf_verb_earlyProcessor<sjf::modulator::sinMod<Sample>, interpType> >;
//
//    using earlyVariant = std::variant< earlyRandModPtr, earlySinModPtr >;
//
//    
//    void processBlock( juce::AudioBuffer< Sample >& buffer, size_t blockSize )
//    {
//        std::visit( earlyVisitor{ buffer, blockSize }, m_processor );
//    }
//    
//    void setModType( modTypes mt, juce::AudioProcessorValueTreeState& vts )
//    {
//        std::visit( earlyReseter{}, m_processor );
//        switch (mt) {
//            case modTypes::random:
//                m_processor = std::make_unique< earlyRandMod >( vts );
//                break;
//            case modTypes::sin:
//                m_processor = std::make_unique< earlySinMod >( vts );
//                break;
//            default:
//                m_processor = std::make_unique< earlyRandMod >( vts );
//                break;
//        }
//    }
//private:
//        struct earlyReseter
//        {
//            void operator()( earlyRandModPtr& e ){ e.reset(); }
//            void operator()( earlySinModPtr& e ){ e.reset(); }
//        };
//    
//        struct earlyVisitor
//        {
//            earlyVisitor( juce::AudioBuffer< Sample >& revBuffer, size_t blockSize ) : m_revBuffer(revBuffer), m_blockSize(blockSize){}
//    
//            void operator()( earlyRandModPtr& e ){ e->processBlock( m_revBuffer, m_blockSize ); }
//            void operator()( earlySinModPtr& e ){ e->processBlock( m_revBuffer, m_blockSize ); }
//        private:
//            juce::AudioBuffer< Sample >& m_revBuffer;
//            size_t m_blockSize;
//        };
//    
//
//    earlyVariant m_processor;
//}

#endif /* sjf_verb_earlyProcessingWrapper_h */

