#include "MainFrame.hpp"

class NoahhInstallerApp : public wxApp {
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(NoahhInstallerApp);

bool NoahhInstallerApp::OnInit() {
    auto frame = new MainFrame();
    frame->Show(true);
    return true;
}