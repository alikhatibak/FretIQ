#include "MainComponent.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include <cstdlib>

MainComponent::MainComponent() {
  setSize(800, 600);
  setWantsKeyboardFocus(true);
  startTimer(1000);
  setAudioChannels(1, 0);
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
      "[FretIQ] Mouse clicked at: " + juce::String(event.x) + ", " +
      juce::String(event.y));
}

bool MainComponent::keyPressed(const juce::KeyPress &key) {
  juce::Logger::writeToLog("[FretIQ] Key pressed: " +
                           juce::String(key.getTextCharacter()));
  return true;
}

void MainComponent::timerCallback() {
  juce::Logger::writeToLog("[FretIQ] Timer triggered: Updating...");
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected,
                                  double sampleRate) {
  juce::Logger::writeToLog("[FretIQ] Audio Initialized. Sample Rate: " +
                           juce::String(sampleRate));
}

void MainComponent::releaseResources() {
  juce::Logger::writeToLog("[FretIQ] Audio Released.");
}
/*Previous function that would output every sample: */

/*void MainComponent::getNextAudioBlock(*/
/*    const juce::AudioSourceChannelInfo &bufferToFill) {*/
/*  auto *inputChannelData = bufferToFill.buffer->getReadPointer(0);*/
/*  juce::Logger::writeToLog("Sample: " + juce::String(inputChannelData[0]));*/
/*  bufferToFill.clearActiveBufferRegion();*/
/*}*/

void MainComponent::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill) {
  auto *channelData = bufferToFill.buffer->getReadPointer(0);
  const int numSamples = bufferToFill.numSamples;

  static int logCounter = 0;
  float peak = 0.0f;
  float sum = 0.0f;

  for (int i = 0; i < numSamples; ++i) {
    float sample = channelData[i];
    sum += sample * sample;
    if (std::abs(sample) > peak)
      peak = std::abs(sample);
  }

  float rms = std::sqrt(sum / numSamples);
  if (++logCounter >= 50) {
    logCounter = 0;
    /*DBG("Peak: " << peak << " | RMS: " << rms);*/
    juce::Logger::writeToLog("[FretIQ] Peak: " + juce::String(peak) +
                             " | RMS: " + juce::String(rms));
  }
}

void MainComponent::resized() {}
