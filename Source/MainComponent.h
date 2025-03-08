#include "aubio/pitch/pitch.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>
#include <aubio/aubio.h>
#include <vector>

class MainComponent : public juce::AudioAppComponent, public juce::Timer {
public:
  MainComponent();
  ~MainComponent() override;

  void paint(juce::Graphics &) override;
  void resized() override;
  void mouseDown(const juce::MouseEvent &event) override;
  bool keyPressed(const juce::KeyPress &key) override;
  void timerCallback() override;
  void
  getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;
  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
  void releaseResources() override;
  void shutdownAudio();

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
  static constexpr int bufferSize = 2048;
  std::vector<float> audioBuffer;
  int bufferIndex = 0;
  aubio_pitch_t *aubioPitch;
  fvec_t *aubioInputBuffer;
  void initializeAubio();
  float detectPitch();
};
