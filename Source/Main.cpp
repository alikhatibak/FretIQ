#include "MainComponent.h"
#include <JuceHeader.h>
#include <_inttypes.h>
#include <_types.h>
#include <aubio/fvec.h>
#include <aubio/types.h>

class MainWindow : public juce::DocumentWindow {
public:
  MainWindow(juce::String name)
      : DocumentWindow(
            name,
            juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                juce::ResizableWindow::backgroundColourId),
            juce::DocumentWindow::allButtons) {
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setContentOwned(new MainComponent(), true);
    centreWithSize(800, 600);
    setVisible(true);
  }

  void closeButtonPressed() override {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
  }
};

class FretIQApplication : public juce::JUCEApplication {
public:
  const juce::String getApplicationName() override { return "FretIQ"; }

  const juce::String getApplicationVersion() override { return "1.0.0"; }

  bool moreThanOneInstanceAllowed() override { return false; }

  void initialise(const juce::String &commandLine) override {
    mainWindow = std::make_unique<MainWindow>("FretIQ");
  }

  void shutdown() override { mainWindow = nullptr; }

private:
  // Store the main window as a smart pointer
  std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(FretIQApplication)
