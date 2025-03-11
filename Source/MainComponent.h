#pragma once

#include "juce_core/juce_core.h"
#include <JuceHeader.h>

// Include Aubio headers (make sure these are in your include path)
extern "C" {
#include <aubio/aubio.h>       // Main aubio header
#include <aubio/fvec.h>        // Vector type for aubio
#include <aubio/pitch/pitch.h> // Pitch detection functions
#include <aubio/types.h>       // Type definitions for aubio (e.g., smpl_t)
}

#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_gui_basics/juce_gui_basics.h"
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

  // Returns the latest block of audio samples from our circular buffer.
  std::vector<float> getLatestAudioBlock();

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)

  // Size of the circular audio buffer.
  static constexpr int bufferSize = 2048;
  // Circular buffer for storing audio samples.
  std::vector<float> audioBuffer;
  // Current index in the circular audio buffer.
  int bufferIndex = 0;

  // Aubio pitch detection objects.
  aubio_pitch_t *aubioPitch;
  fvec_t *aubioInputBuffer;
  fvec_t *aubioOutputBuffer;

  // Initializes aubio pitch detection components.
  void initializeAubio();
  // Processes the aubio pitch detection and returns the detected pitch.
  float detectPitch();
  int targetMidiNote = 60; // Midi Middle C
  int stableFrameCount = 0;
  int incorrectFrameCount = 0;
  static constexpr float requiredStableFrames = 5;
  static constexpr float requiredIncorrectFrames = 20;
  static constexpr float rmsThreshold = 0.02f;
  static constexpr float pitchConfidenceThreshold = 0.8f;

  void setRandomTargetNote();
  // Added note detection methods.
  int frequencyToMidiNoteNumber(float frequencyHZ) const;
  juce::String midiNoteNumberToNoteName(int midiNoteNumber) const;
  juce::String targetNoteText = "Target: C4";
  juce::String detectedNoteText = "You played: -";
  juce::String feedbackText = "";
  void updateUI();
  bool isWaitingForNewNote = false;
  void startNoteChangeDelay();
};
