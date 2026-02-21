#pragma once

namespace tremolo {
class Tremolo {
public:
  enum class LfoWaveform : size_t {
    sine = 0,
    triangle = 1
  };

  Tremolo() {
    for (auto& lfo : lfos) {
      lfo.setFrequency(5.f, true);
    }
  }

  void setLfoWaveform(LfoWaveform wf) {
    jassert(wf == LfoWaveform::sine || wf == LfoWaveform::triangle);
    nextWaveForm = wf;
  }

  void prepare(double sampleRate, int expectedMaxFramesPerBlock) {
    const juce::dsp::ProcessSpec spec {
      .sampleRate = sampleRate, 
      .maximumBlockSize = static_cast<juce::uint32>(expectedMaxFramesPerBlock),
      .numChannels = juce::uint32{1},
    };

    for (auto& lfo : lfos) {
      lfo.prepare(spec);
    }
  }

  void process(juce::AudioBuffer<float>& buffer) noexcept {
    updateWaveForm();

    // for each frame
    for (const auto frameIndex : std::views::iota(0, buffer.getNumSamples())) {
      // generate the LFO value
      // lfo values are between -1 and 1
      const auto lfoVal = getNextLfoValue();
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
    for (auto& lfo : lfos) {
      lfo.reset();
    }
  }

  static float triangle(float phase) noexcept {
    return std::abs(2 * phase / juce::MathConstants<float>::pi) - 1.f;
  }

private:
  // You should put class members and private functions here
  LfoWaveform currentWaveForm = LfoWaveform::triangle;
  LfoWaveform nextWaveForm = currentWaveForm;
  
  std::array<juce::dsp::Oscillator<float>, 2> lfos {
    juce::dsp::Oscillator<float> { [] (auto phase) { return std::sin(phase); } },
    juce::dsp::Oscillator<float> { triangle }
  };

  float getNextLfoValue() {
    return lfos[juce::toUnderlyingType(currentWaveForm)].processSample(0.f);
  }

  void updateWaveForm() {
    if (currentWaveForm == nextWaveForm) return;

    currentWaveForm = nextWaveForm;
  }

};
}  // namespace tremolo
