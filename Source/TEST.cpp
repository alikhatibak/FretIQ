#include "MainComponent.h"
#include "juce_gui_basics/juce_gui_basics.h"

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
  setAudioChannels(1, 0);
  audioBuffer.resize(bufferSize);
  bufferIndex = 0;
  initializeAubio();
}

MainComponent::~MainComponent() { shutdownAudio(); }

void MainComponent::shutdownAudio() {
  juce::AudioAppComponent::shutdownAudio();
}

void MainComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::black);
  g.setColour(juce::Colours::white);

  g.drawText(juce::String("Mode: ") + juce::String(currentMode == FullFretboard
                                                       ? "Full Fretboard"
                                                       : "By String"),
             getLocalBounds().withHeight(50), juce::Justification::centred,
             true);
  g.drawText(targetNoteText, getLocalBounds().withHeight(100),
             juce::Justification::centred, true);
  g.drawText(detectedNoteText, getLocalBounds().withTrimmedTop(100),
             juce::Justification::centred, true);
  g.setColour(feedbackText.contains("+") ? juce::Colours::limegreen
                                         : juce::Colours::red);
  g.drawText(feedbackText, getLocalBounds().withTrimmedTop(200).withHeight(100),
             juce::Justification::centred, true);
}

void MainComponent::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill) {
  float pitch = detectPitch();
  float confidence = aubio_pitch_get_confidence(aubioPitch);

  if (pitch > 0 && confidence >= pitchConfidenceThreshold) {
    int midiNote = frequencyToMidiNoteNumber(pitch);
    auto fullNoteName = midiNoteNumberToNoteName(midiNote);

    // Ensure detected note displays correctly for each mode
    auto detectedNote =
        (currentMode == ByString)
            ? fullNoteName.upToFirstOccurrenceOf("0", false, false)
                  .upToFirstOccurrenceOf("1", false, false)
                  .upToFirstOccurrenceOf("2", false, false)
                  .upToFirstOccurrenceOf("3", false, false)
                  .upToFirstOccurrenceOf("4", false, false)
                  .upToFirstOccurrenceOf("5", false, false)
                  .upToFirstOccurrenceOf("6", false, false)
                  .upToFirstOccurrenceOf("7", false, false)
                  .upToFirstOccurrenceOf("8", false, false)
                  .upToFirstOccurrenceOf("9", false, false)
            : fullNoteName;

    detectedNoteText = "You played: " + detectedNote;

    if (currentMode == FullFretboard) {
      if (midiNote == targetMidiNote) {
        stableFrameCount++;
        incorrectFrameCount = 0;
        if (stableFrameCount >= requiredStableFrames) {
          feedbackText = "+ Correct!";
          isWaitingForNewNote = true;
          startNoteChangeDelay();
          stableFrameCount = 0;
          updateUI();
        }
      }
    } else {
      // Ensure target note is properly stripped of octaves
      juce::String targetNoteOnly = cleanTargetNote.trim().toLowerCase();
      detectedNote = detectedNote.trim().toLowerCase();

      if (detectedNote == targetNoteOnly) {
        stableFrameCount++;
        incorrectFrameCount = 0;

        if (stableFrameCount >= requiredStableFrames) {
          feedbackText = "+ Correct!";
          notePlayCount[detectedNote]++;

          if (notePlayCount[detectedNote] >= 2) {
            setNextString();
          } else {
            isWaitingForNewNote = true;
            startNoteChangeDelay();
          }

          stableFrameCount = 0;
          updateUI();
        }
      }
    }
  }
}

void MainComponent::setRandomTargetNote() {
  static std::random_device rand;
  static std::mt19937 gen(rand());

  if (currentMode == FullFretboard) {
    static std::uniform_int_distribution<int> noteDist(40, 88);
    targetMidiNote = noteDist(gen);
    targetNoteText = "Target: " + midiNoteNumberToNoteName(targetMidiNote);
  } else {
    static const juce::StringArray noteNames = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    static std::uniform_int_distribution<int> noteDist(0, noteNames.size() - 1);
    cleanTargetNote = noteNames[noteDist(gen)]; // Store without octave
    targetNoteText = "Target: " + cleanTargetNote + " (String " +
                     juce::String(selectedString) + ")";
  }
  updateUI();
}

void MainComponent::updateUI() {
  targetNoteText = "Target: " + (currentMode == FullFretboard
                                     ? midiNoteNumberToNoteName(targetMidiNote)
                                     : cleanTargetNote);
  repaint();
}
