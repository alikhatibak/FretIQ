#include "MainComponent.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"

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
  juce::Logger::writeToLog("Mouse clicked at: " + juce::String(event.x) + ", " +
                           juce::String(event.y));
}

bool MainComponent::keyPressed(const juce::KeyPress &key) {
  juce::Logger::writeToLog("Key pressed: " +
                           juce::String(key.getTextCharacter()));
  return true;
}

void MainComponent::timerCallback() {
  juce::Logger::writeToLog("Timer triggered: Updating...");
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected,
                                  double sampleRate) {
  juce::Logger::writeToLog("Audio Initialized. Sample Rate: " +
                           juce::String(sampleRate));
}

void MainComponent::releaseResources() {
  juce::Logger::writeToLog("Audio Released.");
}

void MainComponent::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill) {
  auto *inputChannelData = bufferToFill.buffer->getReadPointer(0);
  juce::Logger::writeToLog("Sample: " + juce::String(inputChannelData[0]));
  bufferToFill.clearActiveBufferRegion();
}

void MainComponent::resized() {}
