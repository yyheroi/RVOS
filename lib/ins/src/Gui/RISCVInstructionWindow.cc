//NOTLINTBEGIN
#include <gtkmm.h>
#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <sstream>
#include <iomanip> 
#include "Core/InstTypeFactory.hh"
#include "ISA/InstFormat.hh"
#include "Core/Instruction.hh"
#include "Gui/InstFormatUI.hh"

class RISCVInstructionWindow : public Gtk::Window {
public:
    Gtk::Box* uiContainer_ = nullptr;
    Gtk::Entry* InsEntry_ = nullptr;
    Gtk::Button* InsButtonParse_ = nullptr;
    Gtk::TextView* InsTextView_ = nullptr;
    Instruction *pInst_ = nullptr;
    RISCVInstructionWindow();
    InstFormatUI* rTypeUI_ = nullptr;
    InstFormatUI* iTypeUI_ = nullptr;
    InstFormatUI* sTypeUI_ = nullptr;
    InstFormatUI* bTypeUI_ = nullptr;
    InstFormatUI* uTypeUI_ = nullptr;
    InstFormatUI* jTypeUI_ = nullptr;
private:
    void initInstFormatUI();
    void onInsButtonParseClicked();
    void showInsResult(Instruction& inst);
    void updateDisplay(InstFormatUI& instUi,Instruction& inst);
    void updateAssemblyDisplay(InstFormatUI& instUi, Instruction& inst);
    void updateBinaryDisplay(InstFormatUI& instUi, Instruction& inst);
    void showError(const std::string& message);
    void loadCssFromFile(Gtk::Window& window);
};

RISCVInstructionWindow::RISCVInstructionWindow() {
    set_title("RISC-V指令格式可视化");
    set_default_size(900, 300);
    
    uiContainer_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 10);
    uiContainer_->set_margin(15);
    
    InsEntry_ = Gtk::make_managed<Gtk::Entry>();
    InsEntry_->set_placeholder_text("请输入指令");
    InsEntry_->signal_activate().connect(sigc::mem_fun(*this, &RISCVInstructionWindow::onInsButtonParseClicked));
    
    InsButtonParse_ = Gtk::make_managed<Gtk::Button>("解析指令");
    InsButtonParse_->signal_clicked().connect(sigc::mem_fun(*this, &RISCVInstructionWindow::onInsButtonParseClicked));

    uiContainer_->append(*InsEntry_);
    uiContainer_->append(*InsButtonParse_);
    
    InsTextView_ = Gtk::make_managed<Gtk::TextView>();
    InsTextView_->set_margin(10);
    InsTextView_->set_vexpand(true);
    InsTextView_->set_hexpand(true);
    InsTextView_->set_editable(false);
    InsTextView_->set_cursor_visible(false);
    loadCssFromFile(*this);

    initInstFormatUI();
    set_child(*uiContainer_);
}
void RISCVInstructionWindow::initInstFormatUI() {
    rTypeUI_ = new InstFormatUI(createRTypeFormat());
    uiContainer_->append(*rTypeUI_);
    rTypeUI_->set_visible(false);
}
void RISCVInstructionWindow::onInsButtonParseClicked() {
    auto hexStr = InsEntry_->get_text();
    if (hexStr.empty()) {
        showError("请输入有效的十六进制指令");
        return;
    }
    std::cout << "Parsing hex: " << hexStr << std::endl;

    try {
        std::string cleanStr = hexStr;
        if (cleanStr.size() >= 2 && 
            (cleanStr.substr(0, 2) == "0x" || cleanStr.substr(0, 2) == "0X")) {
            cleanStr = cleanStr.substr(2);
        }

        uint32_t instructionNum = std::stoul(cleanStr, nullptr, 16);
        pInst_ = new Instruction(instructionNum);
        if(pInst_ == nullptr) {
            throw std::invalid_argument("无法创建指令实例");
        }
        pInst_->Decode();

        printf("instruction11 0x%08x\n", instructionNum);
        showInsResult(*pInst_);

    } catch (const std::invalid_argument& e) {
        showError("输入格式错误：请输入有效的十六进制数");
    } catch (const std::out_of_range& e) {
        showError("数值超出范围：仅支持32位指令（0x00000000 ~ 0xFFFFFFFF）");
    } catch (const std::exception& e) {
        showError(std::string("解码失败：") + e.what());
    } catch (...) {
        showError("未知错误");
    }

    InsEntry_->grab_focus();
}

void RISCVInstructionWindow::showInsResult(Instruction& inst) {
    auto buffer = InsTextView_->get_buffer();
    if (!buffer) {
        std::cerr << "Error: TextView buffer is null." << std::endl;
        return;
    }
    InstFormatUI *currUi = nullptr;
    InstFormat fmt_ = inst.GetType().GetInstFormat();
    switch (fmt_) {
        case InstFormat::R:
            rTypeUI_->show();
            currUi = rTypeUI_;
            break;
        case InstFormat::I:
            break;
        case InstFormat::S:
            break;
        case InstFormat::B:
            break;
        case InstFormat::U:
            break;
        case InstFormat::J:
            break;
        default:
            break;
    }
    std::ostringstream oss;
    oss << "Hexadecimal   = 0x"<< std::hex << std::setfill('0') << std::setw(8) << static_cast<uint32_t>(inst) << "\n";
    oss << "Format          = "<< std::hex << inst.GetFormat() << '\n';
    oss << "Instruction set = " << inst.GetXLEN() << "\n";
    buffer->set_text(oss.str());

    if(currUi) {
        updateDisplay(*currUi, inst);
        // updateAssemblyDisplay(*currUi, inst);
        // updateBinaryDisplay(*currUi, inst);
    }


    uiContainer_->append(*InsTextView_);
}
void RISCVInstructionWindow::updateDisplay(InstFormatUI& instUi,Instruction& inst) {
    instUi.updateDisplay(inst); 
}
void RISCVInstructionWindow::updateAssemblyDisplay(InstFormatUI& instUi,Instruction& inst) {
    // instUi.updateAssemblyDisplay(inst); 
}

void RISCVInstructionWindow::updateBinaryDisplay(InstFormatUI& instUi,Instruction& inst) {
    // instUi.updateBinaryDisplay(inst); 
}

void RISCVInstructionWindow::showError(const std::string& message) {
    auto buffer = InsTextView_->get_buffer();
    if (buffer) {
        buffer->set_text("错误: " + message);
    }
    std::cerr << "Error: " << message << std::endl;
}


void RISCVInstructionWindow::loadCssFromFile(Gtk::Window& window) {
    try {
        auto cssProvider = Gtk::CssProvider::create();
        
        auto cssFile = Gio::File::create_for_path("/home/yyh/workspace_/c_test/RVOS_tmp/RVOS_1224/RVOS/lib/ins/src/Gui/InstStyle.css");
        
        cssProvider->load_from_file(cssFile);
        
        auto display = Gdk::Display::get_default();
        if (display) {
            Gtk::StyleContext::add_provider_for_display(
                display, cssProvider, 
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        
        std::cout << "CSS loaded successfully from file!" << std::endl;
    } catch (const Gio::ResourceError& ex) {
        std::cerr << "Resource error: " << ex.what() << std::endl;
    } catch (const Glib::Error& ex) {
        std::cerr << "GLib error: " << ex.what() << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}

//NOTLINTEND