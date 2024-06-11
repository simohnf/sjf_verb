//
//  sjf_verbWrapper.h
//  sjf_verb
//
//  Created by Simon Fay on 10/06/2024.
//  Copyright © 2024 sjf. All rights reserved.
//

#ifndef sjf_verbWrapper_h
#define sjf_verbWrapper_h
#include "sjf_verb.h"

class sjf_verbWrapper
{
    
    using verbNone = sjf_verb<sjf::interpolation::noneInterpolate<float> >;
    using verbLin = sjf_verb<sjf::interpolation::linearInterpolate<float> >;
    using verbCub = sjf_verb<sjf::interpolation::cubicInterpolate<float> >;
    using verbPD = sjf_verb<sjf::interpolation::fourPointInterpolatePD<float> >;
    using verb4p = sjf_verb<sjf::interpolation::fourPointFourthOrderOptimal<float> >;
    using verbGod = sjf_verb<sjf::interpolation::cubicInterpolateGodot<float> >;
    using verbHer = sjf_verb<sjf::interpolation::cubicInterpolateHermite<float> >;
    
    using verbNonePtr = std::unique_ptr<verbNone>;
    using verbLinPtr = std::unique_ptr<verbLin>;
    using verbCubPtr = std::unique_ptr<verbCub>;
    using verbPDPtr = std::unique_ptr<verbPD>;
    using verb4pPtr = std::unique_ptr<verb4p>;
    using verbGodPtr = std::unique_ptr<verbGod>;
    using verbHerPtr = std::unique_ptr<verbHer>;
    
    using verbVariant = std::variant< verbNonePtr, verbLinPtr, verbCubPtr, verbPDPtr, verb4pPtr, verbGodPtr, verbHerPtr >;
    
public:
    sjf_verbWrapper( juce::AudioProcessorValueTreeState &vts ) :
        m_vts(vts) , m_verb(std::make_unique<verbPD>(m_vts)) {}
    ~sjf_verbWrapper(){ std::visit( verbReseter{}, m_verb ); }
    
    void initialise( double sampleRate, int samplesPerBlock, int nChannels )
    {
        m_sampleRate = sampleRate;
        m_samplesPerBlock = samplesPerBlock;
        m_nChannels = nChannels;
        std::visit( verbInitialiser{ m_sampleRate, m_samplesPerBlock, m_nChannels }, m_verb );
    }
    
    void processBlock( juce::AudioBuffer<float>& buffer ){ std::visit( verbVisitor{ buffer }, m_verb ); }
    
    void setInterpolationType( sjf::interpolation::interpolatorTypes interpType, juce::AudioProcessorValueTreeState &vts,  const juce::Array<juce::AudioProcessorParameter*>& params )
    {
        std::visit( verbReseter{}, m_verb );
        switch (interpType) {
            case sjf::interpolation::interpolatorTypes::none:
                m_verb = std::make_unique<verbNone>( m_vts );
                break;
            case sjf::interpolation::interpolatorTypes::linear:
                m_verb = std::make_unique<verbLin>( m_vts );
                break;
            case sjf::interpolation::interpolatorTypes::cubic:
                m_verb = std::make_unique<verbCub>( m_vts );
                break;
            case sjf::interpolation::interpolatorTypes::pureData:
                m_verb = std::make_unique<verbPD>( m_vts );
                break;
            case sjf::interpolation::interpolatorTypes::fourthOrder:
                m_verb = std::make_unique<verb4p>( m_vts );
                break;
            case sjf::interpolation::interpolatorTypes::godot:
                m_verb = std::make_unique<verbGod>( m_vts );
                break;
            case sjf::interpolation::interpolatorTypes::hermite:
                m_verb = std::make_unique<verbHer>( m_vts );
                break;
            default:
                m_verb = std::make_unique<verbLin>( m_vts );
                break;
        }
//        vts.removeParameterListener(<#StringRef parameterID#>, <#Listener *listener#>)
        std::visit( verbParamAdder{ vts, params }, m_verb );
        std::visit( verbInitialiser{ m_sampleRate, m_samplesPerBlock, m_nChannels }, m_verb );
    }
   
private:
    double m_sampleRate{44100};
    int m_samplesPerBlock{1024}, m_nChannels{2};
    
    juce::AudioProcessorValueTreeState& m_vts;
    verbVariant m_verb;
    
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    struct verbInitialiser
    {
        verbInitialiser( double sr, int blockSize, int nChannels ) :
            sampleRate(sr), samplesPerBlock(blockSize), numberOfChannels(nChannels)
        {}
        void operator()( verbNonePtr& v ){ v->initialise( sampleRate, samplesPerBlock, numberOfChannels ); }
        void operator()( verbLinPtr& v ){ v->initialise( sampleRate, samplesPerBlock, numberOfChannels ); }
        void operator()( verbCubPtr& v ){ v->initialise( sampleRate, samplesPerBlock, numberOfChannels ); }
        void operator()( verbPDPtr& v ){ v->initialise( sampleRate, samplesPerBlock, numberOfChannels ); }
        void operator()( verb4pPtr& v ){ v->initialise( sampleRate, samplesPerBlock, numberOfChannels ); }
        void operator()( verbGodPtr& v ){ v->initialise( sampleRate, samplesPerBlock, numberOfChannels ); }
        void operator()( verbHerPtr& v ){ v->initialise( sampleRate, samplesPerBlock, numberOfChannels ); }
        
    private:
        double sampleRate;
        int samplesPerBlock, numberOfChannels;
    };
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    
    struct verbParamAdder
    {
        verbParamAdder( juce::AudioProcessorValueTreeState &vts, const juce::Array<juce::AudioProcessorParameter*>& params ) :
            m_vts(vts), m_params(params)
        {}
        void operator()( verbNonePtr& v ){ v->addParametersToHandler( /*m_vts,*/ m_params ); }
        void operator()( verbLinPtr& v ){ v->addParametersToHandler( /*m_vts,*/ m_params ); }
        void operator()( verbCubPtr& v ){ v->addParametersToHandler( /*m_vts,*/ m_params ); }
        void operator()( verbPDPtr& v ){ v->addParametersToHandler( /*m_vts,*/ m_params ); }
        void operator()( verb4pPtr& v ){ v->addParametersToHandler( /*m_vts,*/ m_params ); }
        void operator()( verbGodPtr& v ){ v->addParametersToHandler( /*m_vts,*/ m_params ); }
        void operator()( verbHerPtr& v ){ v->addParametersToHandler( /*m_vts,*/ m_params ); }
    private:
        juce::AudioProcessorValueTreeState& m_vts;
        const juce::Array<juce::AudioProcessorParameter*>& m_params;
    };
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    
    struct verbReseter
    {   
        void operator()( verbNonePtr& v ){ v.reset(); }
        void operator()( verbLinPtr& v ){ v.reset(); }
        void operator()( verbCubPtr& v ){ v.reset(); }
        void operator()( verbPDPtr& v ){ v.reset(); }
        void operator()( verb4pPtr& v ){ v.reset(); }
        void operator()( verbGodPtr& v ){ v.reset(); }
        void operator()( verbHerPtr& v ){ v.reset(); }
    };
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    
    struct verbVisitor
    {
        verbVisitor( juce::AudioBuffer<float>& buffer ) :
            m_buffer(buffer)
        {}
        void operator()( verbNonePtr& v ){ v->processBlock( m_buffer ); }
        void operator()( verbLinPtr& v ){ v->processBlock( m_buffer ); }
        void operator()( verbCubPtr& v ){ v->processBlock( m_buffer ); }
        void operator()( verbPDPtr& v ){ v->processBlock( m_buffer ); }
        void operator()( verb4pPtr& v ){ v->processBlock( m_buffer ); }
        void operator()( verbGodPtr& v ){ v->processBlock( m_buffer ); }
        void operator()( verbHerPtr& v ){ v->processBlock( m_buffer ); }
    private:
        juce::AudioBuffer<float>& m_buffer;
    };
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    //===================//===================//===================//===================//===================
    
    
    
};


#endif /* sjf_verbWrapper_h */
