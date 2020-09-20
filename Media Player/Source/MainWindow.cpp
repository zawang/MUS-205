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
    auto maincomp = std::make_unique<MainComponent>();
    setContentOwned(maincomp.release(), false);
    // UNSURE IF I HAVE TO DO ANYTHING TO MAKE THE MAIN COMPONENT'S SIZE CONFORM TO THE WINDOW SIZE???????????????????????????????????????????
    
    setResizable(false, false);
    centreWithSize(392, 120);
    setVisible(true);
}

//==============================================================================
// DocumentWindow overrides

void MainWindow::closeButtonPressed() {
    // when the main window is closed signal the app to exit
    JUCEApplication::getInstance()->systemRequestedQuit();
}

