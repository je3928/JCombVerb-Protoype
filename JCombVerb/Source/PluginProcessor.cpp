/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
JCombVerbAudioProcessor::JCombVerbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), valuetree (*this, nullptr, "PARAMETERS",
                                     { std::make_unique<AudioParameterFloat> ("Rt60", "RT60", NormalisableRange<float> (0.0f, 10000.0f), 500), std::make_unique<AudioParameterFloat> ("Wet", "WET", NormalisableRange<float> (0.0f, 1.0f), 1)})
 
#endif
{
}

JCombVerbAudioProcessor::~JCombVerbAudioProcessor()
{
}

//==============================================================================
const juce::String JCombVerbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JCombVerbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool JCombVerbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool JCombVerbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double JCombVerbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JCombVerbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int JCombVerbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JCombVerbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String JCombVerbAudioProcessor::getProgramName (int index)
{
    return {};
}

void JCombVerbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void JCombVerbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    
    comb1l.reset(sampleRate);
    comb1r.reset(sampleRate);
    comb2l.reset(sampleRate);
    comb2r.reset(sampleRate);
    comb3r.reset(sampleRate);
    comb3l.reset(sampleRate);
    comb4l.reset(sampleRate);
    comb4r.reset(sampleRate);

    comb1l.createDelayBuffer(sampleRate, 100);
    comb2l.createDelayBuffer(sampleRate, 100);
    comb3l.createDelayBuffer(sampleRate, 100);
    comb4l.createDelayBuffer(sampleRate, 100);
    
    comb1r.createDelayBuffer(sampleRate, 100);
    comb2r.createDelayBuffer(sampleRate, 100);
    comb3r.createDelayBuffer(sampleRate, 100);
    comb4r.createDelayBuffer(sampleRate, 100);
    
    
}

void JCombVerbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool JCombVerbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void JCombVerbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    
    auto RT60 = valuetree.getRawParameterValue("Rt60");
    auto Wet = valuetree.getRawParameterValue("Wet");
    
    float rt60 = RT60->load();
    float wet = Wet->load();
    
    CombFilterParameters comb1params = comb1l.getParameters();
    CombFilterParameters comb2params = comb2l.getParameters();
    CombFilterParameters comb3params = comb3l.getParameters();
    CombFilterParameters comb4params = comb4l.getParameters();
    
    
    comb1params.RT60Time_mSec = rt60;
    comb2params.RT60Time_mSec = rt60;
    comb3params.RT60Time_mSec = rt60;
    comb4params.RT60Time_mSec = rt60;
    
    comb1params.delayTime_mSec = 45;
    comb2params.delayTime_mSec = 57;
    comb3params.delayTime_mSec = 77;
    comb4params.delayTime_mSec = 64;

    comb1params.enableLPF = false;
    comb2params.enableLPF = false;
    comb3params.enableLPF = false;
    comb4params.enableLPF = false;
    
    comb1params.interpolate = true;
    comb2params.interpolate = true;
    comb3params.interpolate = true;
    comb4params.interpolate = true;

    
    comb1l.setParameters(comb1params);
    comb2l.setParameters(comb2params);
    comb3l.setParameters(comb3params);
    comb4l.setParameters(comb4params);
    
    comb1r.setParameters(comb1params);
    comb2r.setParameters(comb2params);
    comb3r.setParameters(comb3params);
    comb4r.setParameters(comb4params);
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

 
    auto* channelDataL = buffer.getWritePointer(0);
    auto* channelDataR = buffer.getWritePointer(1);
    
    for (int i = 0; i < buffer.getNumSamples(); i++) {
        
        double xnl1 = channelDataL[i];
        double xnr1 = channelDataR[i];
        
        double xnldry = xnl1;
        double xnrdry = xnr1;
        
        double xnl2 = xnl1;
        double xnr2 = xnr1;
        
        double xnl3 = xnl1;
        double xnr3 = xnr1;
        
        double xnl4 = xnl1;
        double xnr4 = xnr1;
        
        xnl1 = comb1l.processAudioSample(xnl1);
        xnr1 = comb1r.processAudioSample(xnr1);
        
        xnl2 = comb2l.processAudioSample(xnl2);
        xnr2 = comb2r.processAudioSample(xnr2);
        
        xnl3 = comb3l.processAudioSample(xnl3);
        xnr3 = comb3r.processAudioSample(xnr3);
        
        xnl4 = comb4l.processAudioSample(xnl4);
        xnr4 = comb4r.processAudioSample(xnr4);
        
        xnl2 = xnl2 * -1.f;
        xnr2 = xnr2 * -1.f;
        
        xnl4 = xnl4 * -1.f;
        xnr4 = xnr4 * -1.f;
        
        double ynl = xnl1 + xnl2 + xnl3 + xnl4;
        double ynr = xnr1 + xnr2 + xnr3 + xnr4;
        
//        ynl = (ynl * wet) + (xnldry * (- wet));
//        ynr = (ynr * wet) + (xnrdry * (- wet));
        
        ynl = (xnldry * (1.0f - wet) + (ynl * wet));
        ynr = (xnrdry * (1.0f - wet) + (ynr * wet));
        
        channelDataL[i] = ynl;
        channelDataR[i] = ynr;
        
    }
    
    
}

//==============================================================================
bool JCombVerbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* JCombVerbAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void JCombVerbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void JCombVerbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JCombVerbAudioProcessor();
}
