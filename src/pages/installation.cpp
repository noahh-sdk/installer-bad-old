#include "Page.hpp"
#include "../MainFrame.hpp"
#include "../Manager.hpp"

class PageInstallGDPSInfo : public Page {
public:
    PageInstallGDPSInfo(MainFrame* parent) : Page(parent) {
        this->addText(
            "Please note that Noahh currently only supports "
            "Geometry Dash version 2.113. While support for "
            "other versions is planned in the future, for now "
            "it is advised not to try to install Noahh on an "
            "older version of GD as this may break the game."
        );
        m_canContinue = true;
    }
};
REGISTER_PAGE(InstallGDPSInfo);

/////////////////

class PageInstallSelectGD : public Page {
protected:
    wxTextCtrl* m_pathInput;
    ghc::filesystem::path m_path;
    
    void enter() override {
        this->updateContinue();
    }
    
    void leave() override {
        auto path = ghc::filesystem::path(
            m_pathInput->GetValue().ToStdWstring()
        );
        m_path = path;
    }

    void onBrowse(wxCommandEvent&) {
        wxFileDialog ofd(
            this, "Select Geometry Dash", "", "",
            "Executable files (*.exe)|*.exe",
            wxFD_OPEN | wxFD_FILE_MUST_EXIST 
        );
        if (ofd.ShowModal() == wxID_CANCEL) return;
        m_pathInput->SetValue(ofd.GetPath());
    }

    void onText(wxCommandEvent&) {
        this->updateContinue();
    }

    void updateContinue() {
        auto path = m_pathInput->GetValue().ToStdWstring();
        m_canContinue =
            ghc::filesystem::exists(path) &&
            ghc::filesystem::is_regular_file(path);
        m_frame->updateControls();
    }

public:
    PageInstallSelectGD(MainFrame* parent) : Page(parent) {
        ghc::filesystem::path path;

        if (Manager::get()->isFirstTime()) {
            auto gdPath = Manager::get()->findDefaultGDPath();
            if (gdPath.has_value()) {
                this->addText(
                    "Automatically detected Geometry Dash path! "
                    "Please verify that the path below is correct, and "
                    "select a different one if it is not."
                );
                path = gdPath.value();
            } else {
                this->addText(
                    "Unable to automatically detect Geometry Dash path. "
                    "Please enter the path below:"
                );
            }
        } else {
            this->addText(
                "Please enter the path to Geometry Dash. "
                "For the vanilla game, this should be the file "
                "called \"GeometryDash.exe\", however if you're "
                "installing on a GDPS, the name may differ."
            );
        }

        m_pathInput = this->addInput(path.wstring(), &PageInstallSelectGD::onText);

        this->addButton("Browse", &PageInstallSelectGD::onBrowse);
    }

    ghc::filesystem::path getPath() const { return m_path; }
};
REGISTER_PAGE(InstallSelectGD);

/////////////////

class PageInstallCheckMods : public Page {
public:
    PageInstallCheckMods(MainFrame* parent) : Page(parent) {
        auto path = GET_EARLIER_PAGE(InstallSelectGD)->getPath();
        auto others = Manager::get()->doesDirectoryContainOtherMods(path.parent_path());
        if (others) {
            if ((others & OMF_MHv6) || (others & OMF_MHv7)) {
                this->addText(
                    "Looks like you already have MegaHack " +
                    std::string((others & OMF_MHv6) ? "v7" : "v6") + 
                    " installed! This installer will uninstall it, "
                    "however you can get it back as a Noahh mod "
                    "through INSERT METHOD FOR GETTING MEGAHACK FOR "
                    "NOAHH"
                );
            } else if (others & OMF_GDHM) {
                this->addText(
                    "Looks like you already have GD HackerMode "
                    "installed! Noahh is not compatible with other "
                    "external mods or mod loaders, so it will be "
                    "uninstalled. Unfortunately, GDHM has no direct "
                    "Noahh-equivalent, but you can browse the Noahh "
                    "marketplace in-game to find alternatives."
                );
            } else if (others & OMF_Some) {
                this->addText(
                    "Looks like you already have some mods "
                    "installed! Noahh is not compatible with "
                    "other external mods or mod loaders, so "
                    "you will have to uninstall any other "
                    "mods / mod loaders you have. Check out "
                    "the Noahh marketplace in-game to find "
                    "ports && alternatives for your current "
                    "mods :)"
                );
            } else {
                this->addText("You should not see this teehehee");
            }
        } else {
            this->addText(
                "You are ready to install Noahh! Please "
                "note that Noahh is not compatible with "
                "other external mods or mod loaders, so "
                "make sure to uninstall any mods you may "
                "already have installed. Check out "
                "the Noahh marketplace in-game to find "
                "mods available for Noahh :)"
            );
        }
        this->addText("Press \"Next\" to begin installing Noahh.");
        m_canContinue = true;
    }
};
REGISTER_PAGE(InstallCheckMods);

/////////////////

class PageInstall : public Page {
protected:
    wxStaticText* m_status;
    wxGauge* m_gauge;

    void enter() override {
        Manager::get()->downloadLoader(
            [this](std::string const& str) -> void {
                wxMessageBox(
                    "Error downloading the Noahh loader: " + str + 
                    ". Try again, and if the problem persists, contact "
                    "the Noahh Development team for more help.",
                    "Error Installing",
                    wxICON_ERROR
                );
                this->setText(m_status, "Error: " + str);
            },
            [this](std::string const& text, int prog) -> void {
                this->setText(m_status, "Downloading Noahh: " + text);
                m_gauge->SetValue(prog / 2);
            },
            [this](wxWebResponse const& res) -> void {
                auto installRes = Manager::get()->installLoaderFor(
                    GET_EARLIER_PAGE(InstallSelectGD)->getPath(),
                    res.GetDataFile().ToStdWstring()
                );
                if (!installRes) {
                    wxMessageBox(
                        "Error installing Noahh: " + installRes.error() + ". Try "
                        "again, and if the problem persists, contact "
                        "the Noahh Development team for more help.",
                        "Error Installing",
                        wxICON_ERROR
                    );
                } else {
                    auto installation = installRes.value();
                    Manager::get()->downloadAPI(
                        [this](std::string const& str) -> void {
                            wxMessageBox(
                                "Error downloading the Noahh API: " + str + 
                                ". Try again, and if the problem persists, contact "
                                "the Noahh Development team for more help.",
                                "Error Installing",
                                wxICON_ERROR
                            );
                            this->setText(m_status, "Error: " + str);
                        },
                        [this](std::string const& text, int prog) -> void {
                            this->setText(m_status, "Downloading API: " + text);
                            m_gauge->SetValue(prog / 2 + 50);
                        },
                        [this, installation](wxWebResponse const& res) -> void {
                            auto apiInstallRes = Manager::get()->installAPIFor(
                                installation,
                                res.GetDataFile().ToStdWstring(),
                                res.GetSuggestedFileName()
                            );
                            if (!apiInstallRes) {
                                wxMessageBox(
                                    "Error installing Noahh API: " + apiInstallRes.error() + ". Try "
                                    "again, and if the problem persists, contact "
                                    "the Noahh Development team for more help.",
                                    "Error Installing",
                                    wxICON_ERROR
                                );
                            } else {
                                m_frame->nextPage();
                            }
                        }
                    );
                }
            }
        );
    }

public:
    PageInstall(MainFrame* frame) : Page(frame) {
        this->addText("Installing Noahh...");

        m_status = this->addText("Connecting..."); 
        m_gauge = this->addProgressBar();

        m_canContinue = false;
        m_canGoBack = false;
    }
};
REGISTER_PAGE(Install);

/////////////////

class PageInstallFinished : public Page {
protected:
    void enter() override {
        auto res = Manager::get()->saveData();
        if (!res) {
            wxMessageBox(
                "Unable to save installer data: " + res.error() + " - "
                "the installer will be unable to uninstall Noahh!",
                "Error Saving",
                wxICON_ERROR
            );
        }
    }

public:
    PageInstallFinished(MainFrame* frame) : Page(frame) {
        this->addText(
            "Installing finished! "
            "You can now close this installer && start up Geometry Dash :)"
        );
        this->addButton("Support Discord Server", &MainFrame::onDiscord, m_frame);

        m_canContinue = true;
        m_canGoBack = false;
    }
};
REGISTER_PAGE(InstallFinished);
