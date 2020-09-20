//==============================================================================

#include "MainApplication.h"
#include "MainWindow.h"
#include "MainComponent.h"

//==============================================================================
// MainApplication members

MainApplication& MainApplication::getApp() {
    MainApplication* const app = dynamic_cast<MainApplication*>(JUCEApplication::getInstance());
    jassert(app != nullptr);
    return *app;
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
    mediaManager = std::make_unique<MediaManager>();
    mainWindow = std::make_unique<MainWindow>(getApplicationName());
}

void MainApplication::shutdown() {
    mediaManager = nullptr;
    mainWindow = nullptr;
}

void MainApplication::systemRequestedQuit() {
    closeAllAlertAndDialogWindows();
    quit();
}

void MainApplication::anotherInstanceStarted(const String& commandLine) {
}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (MainApplication)
