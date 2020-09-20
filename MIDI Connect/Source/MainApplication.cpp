//==============================================================================

#include "MainApplication.h"
#include "MainWindow.h"
#include "MainComponent.h"

//==============================================================================
// MainApplication members

MainApplication& MainApplication::getApp() {
  MainApplication* const app = dynamic_cast<MainApplication*>(JUCEApplication::getInstance());
  assert(app != nullptr);
  return *app;
}

const File MainApplication::getRuntimeResourceDirectory() {
    #if JUCE_MAC
        return File::getSpecialLocation(juce::File::currentApplicationFile).getChildFile("Contents/Resources");
    #endif
    #if JUCE_WINDOWS
        return File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getChildFile("Resources");
    #endif
    #if JUCE_IOS
        return File::getSpecialLocation(juce::File::currentApplicationFile);
    #endif
    #if JUCE_LINUX
        return File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getChildFile("Resources");
    #endif
        // If you hit this assert you need to add the application's resource directory on your OS.
        jassert(false);
}

AudioDeviceManager& MainApplication::getAudioDeviceManager() {
  return audioDeviceManager;
}

void MainApplication::closeAllAlertAndDialogWindows() {
    // delete any open alert or dialog windows.
    Desktop& desktop = Desktop::getInstance();
    std::vector <std::unique_ptr<DialogWindow>> openDialogs;
    std::vector <std::unique_ptr<AlertWindow>> openAlerts;
    for (int i = 0; i < desktop.getNumComponents(); ++i) {
        Component* component = desktop.getComponent(i);
        DialogWindow* dialogWindow = dynamic_cast<DialogWindow*>(component);
        AlertWindow* alertWindow = dynamic_cast<AlertWindow*>(component);
        
        if (dialogWindow)
            // Version 1: Manually delete raw pointer
            // delete dialogWindow;
            
            // Version 2: Instead of managing the raw pointer manually, turn it into a unique pointer so it manages itself. When the unique pointer goes out of scope, it's automatically deleted.
            openDialogs.push_back(std::unique_ptr<DialogWindow>(dialogWindow));
        else if (alertWindow)
            // Version 1
            // delete alertWindow;
            
            // Version 2
            openAlerts.push_back(std::unique_ptr<AlertWindow>(alertWindow));
    }
}

//==============================================================================
// JUCEApplication overrides

MainApplication::MainApplication() {
}

const String MainApplication::getApplicationName() {
  return ProjectInfo::projectName;
}

const String MainApplication::getApplicationVersion() {
  return ProjectInfo::versionString;
}

bool MainApplication::moreThanOneInstanceAllowed() {
  return false;
}

void MainApplication::initialise(const String& commandLine) {
    // initialize the audio device manager
    auto errors = audioDeviceManager.initialise(0, 2, nullptr, true);
    // use jassert to ensure audioError is empty
    jassert(errors.isEmpty());
    // Create the application window.
    mainWindow = std::make_unique<MainWindow>(getApplicationName());
}

void MainApplication::shutdown() {
    // Delete our main window
    mainWindow = nullptr;
}

void MainApplication::systemRequestedQuit() {
    // Set the main component's quitting member to true to prevent any additional midi input from being processed.
    MainContentComponent* mainComponent = dynamic_cast<MainContentComponent*>(mainWindow->getContentComponent());
    mainComponent->quitting = true;
    
    closeAllAlertAndDialogWindows();
    quit();
}

void MainApplication::anotherInstanceStarted(const String& commandLine) {
  // When another instance of the app is launched while this one is running,
  // this method is invoked, and the commandLine parameter tells you what
  // the other instance's command-line arguments were.
}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (MainApplication)
