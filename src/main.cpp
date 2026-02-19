#include "MainFrame.hpp"

class NoahhInstallerApp : public wxApp {
public:
    virtual bool OnInit();
};

// #ifndef _DEBUG
// wxIMPLEMENT_APP_CONSOLE(NoahhInstallerApp);
// #else
wxIMPLEMENT_APP(NoahhInstallerApp);
// #endif

bool NoahhInstallerApp::OnInit() {
    auto frame = new MainFrame();
    frame->Show(true);
    return true;
}