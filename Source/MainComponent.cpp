#include "MainComponent.h"
#include "juce_graphics/juce_graphics.h"

MainComponent::MainComponent() {
  setSize(800, 600);
  setWantsKeyboardFocus(true);
  startTimer(1000);
}

MainComponent::~MainComponent() {}

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

void MainComponent::resized() {}
