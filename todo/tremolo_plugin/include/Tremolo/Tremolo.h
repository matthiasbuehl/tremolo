#pragma once

namespace tremolo {
class Tremolo {
public:
  Tremolo() {
    lfo.setFrequency(5.f, true);
  }

  void prepare(double sampleRate, int expectedMaxFramesPerBlock) {
    const juce::dsp::ProcessSpec spec {
      .sampleRate = sampleRate, 
      .maximumBlockSize = static_cast<juce::uint32>(expectedMaxFramesPerBlock),
      .numChannels = juce::uint32{1},
    };
    
    lfo.prepare(spec);
    lfo.setFrequency(5.f);
  }

  void process(juce::AudioBuffer<float>& buffer) noexcept {
    // for each frame
    for (const auto frameIndex : std::views::iota(0, buffer.getNumSamples())) {
      // generate the LFO value
      // lfo values are between -1 and 1
      const auto lfoVal = lfo.processSample(0.f);
      // put lfo values between 0 and 1
      const auto lfoValUniPolar = (lfoVal + 1.f) * .5f;

      // calculate the modulation value
      constexpr auto modulationDepth = .4f;
      const auto modulationValue = 1.f - lfoValUniPolar * modulationDepth;

      // for each channel sample in the frame
      for (const auto channelIndex :
           std::views::iota(0, buffer.getNumChannels())) {
        // get the input sample
        const auto inputSample = buffer.getSample(channelIndex, frameIndex);

        // modulate the sample
        const auto outputSample = inputSample * modulationValue;

        // set the output sample
        buffer.setSample(channelIndex, frameIndex, outputSample);
      }
    }
  }

  void reset() noexcept {
    lfo.reset();
  }

private:
  // You should put class members and private functions here
  juce::dsp::Oscillator<float>lfo {[] (auto phase) { return std::sin(phase); }};
};
}  // namespace tremolo
