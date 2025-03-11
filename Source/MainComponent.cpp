#include "MainComponent.h"

// Include Aubio headers
extern "C" {
#include <aubio/aubio.h>
#include <aubio/fvec.h>
#include <aubio/pitch/pitch.h>
#include <aubio/types.h>
}

#include <algorithm>
#include <cstdlib>
#include <random>
#include <vector>

#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"

#define COLOR_CYAN "\033[32m"
#define COLOR_GREEN "\033[36m"
#define COLOR_RESET "\033[35m"
#define COLOR_CORRECT "\033[38;2;144;238;144m"  // Pastel Lime Green
#define COLOR_INCORRECT "\033[38;2;255;105;97m" // Soft Pure Red

//==============================================================================
MainComponent::MainComponent() {
  setSize(800, 600);
  setWantsKeyboardFocus(true);
  startTimer(1000);

  // Set up the audio channels (1 input, 0 outputs)
  setAudioChannels(1, 0);

  // Initialize our circular audio buffer with the proper size.
  audioBuffer.resize(bufferSize);
  bufferIndex = 0;

  // Initialize aubio pitch detection.
  initializeAubio();
}

MainComponent::~MainComponent() { shutdownAudio(); }

void MainComponent::shutdownAudio() {
  juce::AudioAppComponent::shutdownAudio();
}

void MainComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::black);
  g.setColour(juce::Colours::white);
  g.drawText("Welcome to FretIQ!", getLocalBounds(),
             juce::Justification::centred, true);
}

void MainComponent::mouseDown(const juce::MouseEvent &event) {
  juce::Logger::writeToLog(
      COLOR_CYAN "[FretIQ]" COLOR_RESET " - Mouse clicked at: " +
      juce::String(event.x) + ", " + juce::String(event.y));
}

bool MainComponent::keyPressed(const juce::KeyPress &key) {
  juce::Logger::writeToLog(COLOR_CYAN "[FretIQ]" COLOR_RESET
                                      " - Key pressed: " +
                           juce::String(key.getTextCharacter()));
  return true;
}

void MainComponent::timerCallback() {
  juce::Logger::writeToLog(COLOR_CYAN "[FretIQ]" COLOR_RESET
                                      " - Timer triggered: Updating...");
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected,
                                  double sampleRate) {
  juce::Logger::writeToLog(COLOR_CYAN "[FretIQ]" COLOR_RESET
                                      " - Audio Initialized. Sample Rate: " +
                           juce::String(sampleRate));
}

void MainComponent::releaseResources() {
  juce::Logger::writeToLog(COLOR_CYAN "[FretIQ]" COLOR_RESET
                                      " - Audio Released.");
}

/*
  Updated getNextAudioBlock function:
  - Calculates peak and RMS values and logs them periodically.
  - Copies the incoming audio into our circular buffer safely.
  - Fills the aubio input buffer only up to its allocated length (hop size)
    and pads any remaining space with zeros.
  - Calls detectPitch() to process the aubio input buffer for pitch detection.
*/
void MainComponent::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill) {
  // Get a pointer to the incoming audio data.
  auto *channelData = bufferToFill.buffer->getReadPointer(0);
  const int numSamples = bufferToFill.numSamples;

  // Variables for computing the peak and RMS.
  float peak = 0.0f;
  float sum = 0.0f;

  // Process each sample in the incoming buffer.
  for (int i = 0; i < numSamples; ++i) {
    float sample = channelData[i];
    sum += sample * sample;
    if (std::abs(sample) > peak)
      peak = std::abs(sample);
  }

  // Calculate RMS for the current buffer.
  float rms = std::sqrt(sum / numSamples);

  if (rms < rmsThreshold) {
    stableFrameCount = 0;
    return;
  }

  // Copy incoming samples into the circular audio buffer.
  for (int i = 0; i < numSamples; ++i) {
    audioBuffer[bufferIndex] = channelData[i];
    bufferIndex = (bufferIndex + 1) % bufferSize;
  }

  // Fill the aubio input buffer safely.
  int aubioBufferLength = aubioInputBuffer->length; // e.g., 512 samples
  int samplesToProcess = std::min(numSamples, aubioBufferLength);

  for (int i = 0; i < samplesToProcess; ++i) {
    fvec_set_sample(aubioInputBuffer, channelData[i], i);
  }
  // Zero-pad any remaining samples if numSamples < aubioBufferLength.
  for (int i = samplesToProcess; i < aubioBufferLength; ++i) {
    fvec_set_sample(aubioInputBuffer, 0.0f, i);
  }

  // Update aubio state with the new buffer.
  float pitch = detectPitch();
  float confidence = aubio_pitch_get_confidence(aubioPitch);
  if (pitch > 0 && confidence >= pitchConfidenceThreshold) {
    int midiNote = frequencyToMidiNoteNumber(pitch);
    auto noteName = midiNoteNumberToNoteName(midiNote);

    bool isCorrect = (midiNote == targetMidiNote);
    if (isCorrect) {
      stableFrameCount++;
      if (stableFrameCount >= requiredStableFrames) {

        juce::Logger::writeToLog(COLOR_CYAN
                                 "[Fret IQ]" COLOR_RESET " - " COLOR_CORRECT
                                 " Correct! " COLOR_RESET
                                 "You played the correct note, " COLOR_CORRECT +
                                 noteName);
        setRandomTargetNote();
      }
    } else {
      stableFrameCount = 0;
      juce::Logger::writeToLog(
          COLOR_CYAN "[Fret IQ]" COLOR_RESET " - " COLOR_INCORRECT
                     " Try Again! " COLOR_RESET
                     "You played a " COLOR_INCORRECT +
          noteName + COLOR_RESET " instead of a " COLOR_CORRECT +
          midiNoteNumberToNoteName(targetMidiNote));
    }

    // Use a static counter to output logs every 50 buffers.
    static int logCounter = 0;
    if (++logCounter >= 5) {
      logCounter = 0;
      // Log the computed values.
      juce::Logger::writeToLog(
          COLOR_CYAN "[FretIQ]" COLOR_RESET " - Detected: " COLOR_GREEN +
          juce::String::formatted("%6.2f Hz", pitch) + COLOR_RESET +
          " | Note: " COLOR_GREEN + noteName + COLOR_RESET +
          " | RMS: " + juce::String::formatted("%.3f", rms) +
          " | Peak: " + juce::String::formatted("%6.2f", peak));
    }
  }
}

void MainComponent::resized() {}

// Initialize Aubio for pitch detection.
void MainComponent::initializeAubio() {
  // Define buffer sizes for aubio pitch detection.
  int bufferSizeAubio = 1024; // Used for initializing aubio_pitch
  int hopSize = 512;          // Number of samples per aubio processing block
  float sampleRate = 44100.0f;

  // Create the aubio pitch detection object using the "yin" algorithm.
  aubioPitch = new_aubio_pitch("yin", bufferSizeAubio, hopSize, sampleRate);
  if (!aubioPitch) {
    juce::Logger::writeToLog(COLOR_CYAN "[FretIQ]" COLOR_RESET
                                        " - Failed to initialize Aubio!");
    return;
  }
  // Set the tolerance parameter (adjust if needed).
  aubio_pitch_set_tolerance(aubioPitch, (smpl_t)0.8f);

  // Allocate the input and output vectors for aubio.
  aubioInputBuffer = new_fvec(hopSize);
  aubioOutputBuffer = new_fvec(1);
}

// Perform the pitch detection operation using aubio.
float MainComponent::detectPitch() {
  aubio_pitch_do(aubioPitch, aubioInputBuffer, aubioOutputBuffer);
  float pitch = fvec_get_sample(aubioOutputBuffer, 0);
  return pitch;
}

int MainComponent::frequencyToMidiNoteNumber(float frequencyHz) const {
  return juce::roundToInt(12.0f * std::log2(frequencyHz / 440.0f) + 69.0f);
}

juce::String MainComponent::midiNoteNumberToNoteName(int midiNoteNumber) const {
  static const juce::StringArray noteNames = {
      "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  int noteIndex = midiNoteNumber % 12;
  int octaveNumber = (midiNoteNumber / 12) - 1;
  return noteNames[noteNames.size() > 0
                       ? noteNames.indexOf(noteNames[noteNames.size() > 0
                                                         ? midiNoteNumber % 12
                                                         : 0])
                       : 0] +
         juce::String(octaveNumber);
}

void MainComponent::setRandomTargetNote() {
  static std::random_device rand;
  static std::mt19937 gen(rand());
  static std::uniform_int_distribution<int> noteDist(40, 88);

  targetMidiNote = noteDist(gen);
  juce::Logger::writeToLog(COLOR_CYAN "[FretIQ]" COLOR_RESET
                                      " - New Target Note: " COLOR_CYAN +
                           midiNoteNumberToNoteName(targetMidiNote));
}
