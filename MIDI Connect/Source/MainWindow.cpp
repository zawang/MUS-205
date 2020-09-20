//==============================================================================

#include "MainApplication.h"
#include "MainWindow.h"
#include "MainComponent.h"

MainWindow::MainWindow(String name)
: DocumentWindow(name, Colours::white, DocumentWindow::allButtons) {
    // Window title bar uses  the OS'es window style
    setUsingNativeTitleBar(true);
    
    // add our content component. owned means that when the window is
    // deleted it will automatically delete our content component
    auto maincomp = std::make_unique<MainContentComponent>();
    setContentOwned(maincomp.release(), false);
    
    setResizable(true, true);
    setResizeLimits(608, 412, 2*608, 2*412);
    centreWithSize (600, 400);
    setVisible(true);
}

//==============================================================================
// DocumentWindow overrides

void MainWindow::closeButtonPressed() {
    // when the main window is closed signal the app to exit
    JUCEApplication::getInstance()->systemRequestedQuit();
}

