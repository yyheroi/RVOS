//NOTLINTBEGIN

#include <gtkmm.h>
#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include "Core/InstFactory.hh"

class RISCVInstructionWindow : public Gtk::Window {
public:
    RISCVInstructionWindow();

private:
    // 数据成员（按规范：私有成员使用大驼峰+下划线后缀）
    std::vector<Gtk::Label*> BitLabels_;
    Gtk::Box* BinaryBox_ = nullptr;
    Gtk::Box* AssemblyBox_ = nullptr;

    Gtk::Box* OpcodeBitsBox_ = nullptr;    // 位 [6:0]  - 最低7位
    Gtk::Box* Funct3BitsBox_ = nullptr;    // 位 [14:12]
    Gtk::Box* Funct7BitsBox_ = nullptr;    // 位 [31:25] - 最高7位
    Gtk::Box* RdBitsBox_ = nullptr;        // 位 [11:7]
    Gtk::Box* Rs1BitsBox_ = nullptr;       // 位 [19:15]
    Gtk::Box* Rs2BitsBox_ = nullptr;       // 位 [24:20]
    
    Gtk::Box* InstructionBox_ = nullptr; 
    Gtk::Box* RdSymbolsBox_ = nullptr;        // 位 [11:7]
    Gtk::Box* Rs1SymbolsBox_ = nullptr;       // 位 [19:15]
    Gtk::Box* Rs2SymbolsBox_ = nullptr;       // 位 [24:20]

    Gtk::Label* InstructionLabel_ = nullptr; // 用于悬停检测的指令标签
    Gtk::Label* RdLabel_ = nullptr; // 目标寄存器标签
    Gtk::Label* Rs1Label_ = nullptr; // 源寄存器1
    Gtk::Label* Rs2Label_ = nullptr; // 源寄存器2

    Gtk::Entry* InsEntryHex_ = nullptr;
    Gtk::Button* InsButtonParse_ = nullptr;
    Gtk::TextView* InsTextView_ = nullptr;
    std::unique_ptr<IBaseTypeInst> pInst_ = nullptr;
    
    // 用于存储每个位域的tooltip信息
    std::map<std::string, std::string> FieldTooltips_ = {
        {"opcode", "opcode"},
        {"rd", "rd"},
        {"funct3", "funct3"},
        {"rs1", "rs1"},
        {"rs2", "rs2"},
        {"funct7", "funct7"}
    };

    // 函数成员
    void setupCss();
    void setupInstructionController();
    void setupFieldController(Gtk::Box* pFieldBox, const std::string& fieldName, int startBit, int endBit);
    void highlightFieldInMouse(const std::string& fieldName, int startBit, int endBit);
    void highlightField(const std::string& fieldName, int startBit, int endBit);
    void unhighlightField(const std::string& fieldName, int startBit, int endBit);
    void unhighlightFieldInMouse(const std::string& fieldName, int startBit, int endBit);
    void setInstruction(const std::string& instructionStr);
    void onGenerateRandom();
    void onInsButtonParseClicked();
    void uiLabelHide();
    void uiLabelShow();
    bool onInsTextViewKeyPressEvent(guint keyval, guint keycode, Gdk::ModifierType state);
    void showError(const std::string& msg);
    void showInsResult(std::unique_ptr<IBaseTypeInst> pInst);
    void updateAssemblyDisplay(std::unique_ptr<IBaseTypeInst> pInst);
    void updateBinaryDisplay(std::unique_ptr<IBaseTypeInst> pInst);
};

RISCVInstructionWindow::RISCVInstructionWindow() {
    set_title("RISC-V指令格式可视化");
    set_default_size(900, 300);
    
    // 创建主布局
    auto pMainBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 10);
    pMainBox->set_margin(15);
    
    // 输入框和按钮
    InsEntryHex_ = Gtk::make_managed<Gtk::Entry>();
    InsEntryHex_->set_placeholder_text("请输入指令");
    InsEntryHex_->signal_activate().connect(sigc::mem_fun(*this, &RISCVInstructionWindow::onInsButtonParseClicked));
    InsButtonParse_ = Gtk::make_managed<Gtk::Button>("解析指令");
    InsButtonParse_->signal_clicked().connect(sigc::mem_fun(*this, &RISCVInstructionWindow::onInsButtonParseClicked));

    InsTextView_ = Gtk::make_managed<Gtk::TextView>();
    InsTextView_->set_margin(10);
    InsTextView_->set_vexpand(true);
    InsTextView_->set_hexpand(true);
    InsTextView_->set_editable(false);
    InsTextView_->set_cursor_visible(false);

    pMainBox->append(*InsEntryHex_);
    pMainBox->append(*InsButtonParse_);
        
    AssemblyBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    AssemblyBox_->set_halign(Gtk::Align::START);
    AssemblyBox_->set_valign(Gtk::Align::CENTER);
    auto pAssemblyTitleLabel = Gtk::make_managed<Gtk::Label>("Assembly =");
    pAssemblyTitleLabel->set_margin_end(8);

    // 保存指令标签指针，以便后续添加事件控制器
    InstructionBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    InstructionLabel_ = Gtk::make_managed<Gtk::Label>("add");
    InstructionLabel_->set_margin_end(8);
    InstructionLabel_->set_css_classes({"instruction-label"});
    InstructionBox_->set_css_classes({"instruction-container"});
    InstructionBox_->append(*InstructionLabel_);

    RdSymbolsBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    RdSymbolsBox_->set_css_classes({"instruction-container"});
    Rs1SymbolsBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    Rs1SymbolsBox_->set_css_classes({"instruction-container"});
    Rs2SymbolsBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    Rs2SymbolsBox_->set_css_classes({"instruction-container"});
    
    RdLabel_ = Gtk::make_managed<Gtk::Label>("x5");
    RdLabel_->set_css_classes({"register-label"});
    Rs1Label_ = Gtk::make_managed<Gtk::Label>("x6");
    Rs1Label_->set_css_classes({"register-label"});
    Rs2Label_ = Gtk::make_managed<Gtk::Label>("x7");
    Rs2Label_->set_css_classes({"register-label"});
    auto pComma1Label = Gtk::make_managed<Gtk::Label>(",");
    pComma1Label->set_margin_start(4);
    pComma1Label->set_margin_end(4);
    auto pComma2Label = Gtk::make_managed<Gtk::Label>(",");
    pComma2Label->set_margin_start(4);
    pComma2Label->set_margin_end(4);
    
    RdSymbolsBox_->append(*RdLabel_);
    RdSymbolsBox_->append(*pComma1Label);
    Rs1SymbolsBox_->append(*Rs1Label_);
    Rs1SymbolsBox_->append(*pComma2Label);
    Rs2SymbolsBox_->append(*Rs2Label_);

    AssemblyBox_->append(*pAssemblyTitleLabel);
    AssemblyBox_->append(*InstructionBox_); // 使用成员变量
    AssemblyBox_->append(*RdSymbolsBox_);
    // AssemblyBox_->append(*pComma1Label);
    AssemblyBox_->append(*Rs1SymbolsBox_);
    // AssemblyBox_->append(*pComma2Label);
    AssemblyBox_->append(*Rs2SymbolsBox_);
    pMainBox->append(*AssemblyBox_);

    // 创建二进制位显示区域 - 从高位到低位 (31 到 0)
    BinaryBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    BinaryBox_->set_halign(Gtk::Align::START);
    BinaryBox_->set_valign(Gtk::Align::CENTER);
    
    // 创建不同的位域容器
    Funct7BitsBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    Funct7BitsBox_->set_css_classes({"bit-field-container"});
    
    Rs2BitsBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    Rs2BitsBox_->set_css_classes({"bit-field-container"});
    
    Rs1BitsBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    Rs1BitsBox_->set_css_classes({"bit-field-container"});
    
    Funct3BitsBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    Funct3BitsBox_->set_css_classes({"bit-field-container"});
    
    RdBitsBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    RdBitsBox_->set_css_classes({"bit-field-container"});
    
    OpcodeBitsBox_ = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    OpcodeBitsBox_->set_css_classes({"bit-field-container"});
    
    // 为各个功能字段添加事件控制器
    setupFieldController(Funct7BitsBox_, "funct7", 25, 31);
    setupFieldController(Rs2BitsBox_, "rs2", 20, 24);
    setupFieldController(Rs1BitsBox_, "rs1", 15, 19);
    setupFieldController(Funct3BitsBox_, "funct3", 12, 14);
    setupFieldController(RdBitsBox_, "rd", 7, 11);
    setupFieldController(OpcodeBitsBox_, "opcode", 0, 6);
    
    // 为指令标签添加特殊控制器
    setupInstructionController();
    
    // 创建32个位的标签 - 从高位31到低位0
    for (int i = 31; i >= 0; --i) {
        auto pBitLabel = Gtk::make_managed<Gtk::Label>("0");
        pBitLabel->set_width_chars(1);
        pBitLabel->set_halign(Gtk::Align::CENTER);
        pBitLabel->set_valign(Gtk::Align::CENTER);
        pBitLabel->set_size_request(18, 30);
        
        // 添加边框以便看清每个位
        pBitLabel->set_css_classes({"binary-bit"});
        if(i > 0 && i % 4 == 0)
            pBitLabel->set_margin_end(8);

        BitLabels_.push_back(pBitLabel);
        
        // 根据位位置分配到不同的字段容器
        if (i >= 25 && i <= 31) { // funct7: 31-25
            Funct7BitsBox_->append(*pBitLabel);
        } 
        else if (i >= 20 && i <= 24) { // rs2: 24-20
            Rs2BitsBox_->append(*pBitLabel);
        }
        else if (i >= 15 && i <= 19) { // rs1: 19-15
            Rs1BitsBox_->append(*pBitLabel);
        }
        else if (i >= 12 && i <= 14) { // funct3: 14-12
            Funct3BitsBox_->append(*pBitLabel);
        }
        else if (i >= 7 && i <= 11) { // rd: 11-7
            RdBitsBox_->append(*pBitLabel);
        }
        else if (i >= 0 && i <= 6) { // opcode: 6-0
            OpcodeBitsBox_->append(*pBitLabel);
        }
    }
    auto pBinaryHeadBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    auto pBinaryTitleLabel = Gtk::make_managed<Gtk::Label>("Binary =");
    pBinaryTitleLabel->set_margin_end(8);
    pBinaryHeadBox->append(*pBinaryTitleLabel);

    BinaryBox_->append(*pBinaryHeadBox);

    // 按RISC-V格式排列各个字段 (从高位到低位)
    BinaryBox_->append(*Funct7BitsBox_);
    BinaryBox_->append(*Rs2BitsBox_);
    BinaryBox_->append(*Rs1BitsBox_);
    BinaryBox_->append(*Funct3BitsBox_);
    BinaryBox_->append(*RdBitsBox_);
    BinaryBox_->append(*OpcodeBitsBox_);
    
    // 添加二进制位区域到主布局
    pMainBox->append(*BinaryBox_);

    pMainBox->append(*InsTextView_);
    // 添加四位分组指示器
    auto pGroupIndicatorBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    pGroupIndicatorBox->set_halign(Gtk::Align::CENTER);
    pGroupIndicatorBox->set_margin_top(2);
    
    // 32位分为8组，每组4位
    for (int i = 0; i < 8; ++i) {
        auto pGroupLabel = Gtk::make_managed<Gtk::Label>("    ");
        pGroupLabel->set_halign(Gtk::Align::CENTER);
        pGroupLabel->set_valign(Gtk::Align::START);
        pGroupIndicatorBox->append(*pGroupLabel);
        
        if (i < 7) {
            pGroupIndicatorBox->append(*Gtk::make_managed<Gtk::Label>("  "));
        }
    }
    
    pMainBox->append(*pGroupIndicatorBox);
    
    
    // 添加说明文本
    auto pDescription = Gtk::make_managed<Gtk::Label>(
        "说明: 鼠标悬停在不同指令字段上时，相应位域将被高亮显示并显示详细信息 (四位一组格式)"
    );
    pDescription->set_halign(Gtk::Align::CENTER);
    pDescription->set_margin_top(10);
    pMainBox->append(*pDescription);
  
    set_child(*pMainBox);
    
    // 设置CSS样式
    setupCss();
    uiLabelHide();
    // 初始设置为ADD指令
    setInstruction("0000000_00011_00010_000_00001_0110011");
}

#include "Core/InstFactory.hh"

bool RISCVInstructionWindow::onInsTextViewKeyPressEvent(guint keyval, guint keycode, Gdk::ModifierType state) {
     std::cout << "Key pressed: keyval=" << keyval 
              << ", keycode=" << keycode 
              << ", state=" << static_cast<int>(state) << std::endl;
    if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) {
        onInsButtonParseClicked();
        return true;
    }
    // 其他按键不处理
    return false;
}

void RISCVInstructionWindow::uiLabelHide() {
    AssemblyBox_->hide();    
    BinaryBox_->hide();
}

void RISCVInstructionWindow::uiLabelShow() {
    AssemblyBox_->show();
    BinaryBox_->show();
}
void RISCVInstructionWindow::onInsButtonParseClicked() {
    uiLabelShow();
    auto hexStr = InsEntryHex_->get_text();
    if (hexStr.empty()) {
        showError("请输入有效的十六进制指令");
        return;
    }
    std::cout << "Parsing hex: " << hexStr << std::endl;

    try {
        // 清理输入（移除0x/0X前缀）
        std::string cleanStr = hexStr;
        if (cleanStr.size() >= 2 && 
            (cleanStr.substr(0, 2) == "0x" || cleanStr.substr(0, 2) == "0X")) {
            cleanStr = cleanStr.substr(2);
        }

        // 转换为32位无符号整数
        uint32_t instructionNum = std::stoul(cleanStr, nullptr, 16);
        pInst_ = InstFactory::CreateInst(instructionNum);
        assert(pInst_ != nullptr);
        pInst_->SetBitField();
        pInst_->ProcessFunct();
        pInst_->Decode();
        printf("instruction 0x%08x\n", instructionNum);
        
        showInsResult(std::move(pInst_));

    } catch (const std::invalid_argument& e) {
        showError("输入格式错误：请输入有效的十六进制数");
    } catch (const std::out_of_range& e) {
        showError("数值超出范围：仅支持32位指令（0x00000000 ~ 0xFFFFFFFF）");
    } catch (const std::exception& e) {
        showError(std::string("解码失败：") + e.what());
    } catch (...) {
        showError("未知错误");
    }

    InsEntryHex_->grab_focus();
}

void RISCVInstructionWindow::showInsResult(std::unique_ptr<IBaseTypeInst> pInst) {    
    auto buffer = InsTextView_->get_buffer();
    if (!buffer) {
        std::cerr << "Error: TextView buffer is null." << std::endl;
        return;
    }
    
    pInst->ToAssemblyString();
    std::ostringstream oss;
    // oss << "Assembly      = " << pInst->ToAssemblyString() << "\n";
    // oss << "Binary        = " << pInst->bitField_  << "\n";
    oss << "Hexadecimal   = " << pInst->insHexStr_ << "\n";
    oss << "Format        = " << "R-Type" << "\n";
    oss << "Instruction set = " << "RV32I" << "\n";
    oss << "Manual        = " << "add";
    buffer->set_text(oss.str());

    updateAssemblyDisplay(std::move(pInst));
    updateBinaryDisplay(std::move(pInst));

    // std::string temp(1, '0');
    // for(int i = 0; i < 32; ++i) {
    //     temp[0] = pInst->bitField_[i] ? '1' : '0';
    //     BitLabels_[31 - i]->set_text(temp);
    // }

}

void RISCVInstructionWindow::updateAssemblyDisplay(std::unique_ptr<IBaseTypeInst> pInst) {
    switch (pInst->format_) {
    case InstFormat::R:
        InstructionLabel_->set_text(pInst->insStrVector_[0]);
        RdLabel_->set_text(pInst->insStrVector_[1]);
        Rs1Label_->set_text(pInst->insStrVector_[2]);
        Rs2Label_->set_text(pInst->insStrVector_[3]);

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
}

void RISCVInstructionWindow::updateBinaryDisplay(std::unique_ptr<IBaseTypeInst> pInst) {
    std::string temp(1, '0');

    switch (pInst->format_) {
    case InstFormat::R: {
        for(int i = 0; i < 32; ++i) {
            temp[0] = pInst->bitField_[i] ? '1' : '0';
            BitLabels_[31 - i]->set_text(temp);
        }
        break;
    }
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
}

void RISCVInstructionWindow::showError(const std::string& msg) {
    auto buffer = InsTextView_->get_buffer();
    if (buffer) {
        buffer->set_text("错误：" + msg + "\n\n请输入有效的32位RISC-V指令（如0x00310093）");
    }
}

void RISCVInstructionWindow::setupCss() {
    auto cssProvider = Gtk::CssProvider::create();
    cssProvider->load_from_data(R"(
        .binary-bit {
            background-color: #f0f0f0;
            border: 1px solid #ccc;
            border-radius: 3px;
            font-family: monospace;
            font-weight: bold;
            margin: 1px;
            padding: 0 2px;
        }
        
        .binary-bit.highlighted {
            background-color: #4a86e8;
            color: white;
            border: 1px solid #2a5298;
        }

        .binary-bit.highlighted-in-mouse {
            background-color: #b9e84aff;
            color: white;
            border: 1px solid #61982aff;
        }
        
        .bit-field-container {
            padding: 1px;
            border-radius: 3px;
        }
        
        .field-label {
            font-size: 0.85em;
            color: #555;
            margin-top: 3px;
        }
        
        .instruction-container { 
            padding: 0 3px;
            border-radius: 4px;
        }
        
        .instruction-label {
            background-color: #f0f0f0;
            border: 1px solid #ccc;
            border-radius: 4px;
            font-family: monospace;
            font-weight: bold;
            margin: 1px;
            padding: 0 3px;
            color: #10151aff;
        }
        .instruction-label.highlighted {
            background-color: #4a86e8;
            color: white;
            border: 1px solid #2a5298;
        }
        .instruction-label.highlighted-in-mouse {
            background-color: #b9e84aff;
            color: white;
            border: 1px solid #61982aff;
        }

        .register-label {
            background-color: #f0f0f0;
            border: 1px solid  #ccc;
            border-radius: 4px;
            font-family: monospace;
            font-weight: bold;
            margin: 1px;
            padding: 0 3px;
            color: #10151aff;
        }
        .register-label.highlighted {
            background-color: #4a86e8;
            color: white;
            border: 1px solid #2a5298;
        }
        .register-label.highlighted-in-mouse {
            background-color: #b9e84aff;
            color: white;
            border: 1px solid #61982aff;
        }
    )");
    
    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        cssProvider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

void RISCVInstructionWindow::setupInstructionController() {
    if (!InstructionLabel_) return;
    if (!RdLabel_) return;
    if (!Rs1Label_) return;
    if (!Rs2Label_) return;

    auto pMotionControllerIns = Gtk::EventControllerMotion::create();
    auto pMotionControllerRd = Gtk::EventControllerMotion::create();
    auto pMotionControllerRs1 = Gtk::EventControllerMotion::create();
    auto pMotionControllerRs2 = Gtk::EventControllerMotion::create();
   
    // 鼠标进入时高亮opcode, funct3, funct7
    pMotionControllerIns->signal_enter().connect([this](double, double) {
        InstructionLabel_->add_css_class("highlighted-in-mouse");
        highlightField("opcode", 0, 6);
        highlightField("funct3", 12, 14);
        highlightField("funct7", 25, 31);
    });
    
    // 鼠标离开时取消高亮
    pMotionControllerIns->signal_leave().connect([this]() {
        InstructionLabel_->remove_css_class("highlighted-in-mouse");
        unhighlightField("opcode", 0, 6);
        unhighlightField("funct3", 12, 14);
        unhighlightField("funct7", 25, 31);
    });
    
    InstructionBox_->add_controller(pMotionControllerIns);

    pMotionControllerRd->signal_motion().connect([this](double, double) {
        RdLabel_->add_css_class("highlighted-in-mouse");
        highlightField("rd", 7, 11);
    });
    pMotionControllerRd->signal_leave().connect([this]() {
        RdLabel_->remove_css_class("highlighted-in-mouse");
        unhighlightField("rd", 7, 11);
    });
    RdSymbolsBox_->add_controller(pMotionControllerRd);

    pMotionControllerRs1->signal_motion().connect([this](double, double) {
        Rs1Label_->add_css_class("highlighted-in-mouse");
        highlightField("rs1", 15, 19);
    });
    pMotionControllerRs1->signal_leave().connect([this]() {
        Rs1Label_->remove_css_class("highlighted-in-mouse");
        unhighlightField("rs1", 15, 19);
    });
    Rs1SymbolsBox_->add_controller(pMotionControllerRs1);

    pMotionControllerRs2->signal_motion().connect([this](double, double) {
        Rs2Label_->add_css_class("highlighted-in-mouse");
        highlightField("rs2", 20, 24);
    });
    pMotionControllerRs2->signal_leave().connect([this]() {
        Rs2Label_->remove_css_class("highlighted-in-mouse");
        unhighlightField("rs2", 20, 24);
    });
    Rs2SymbolsBox_->add_controller(pMotionControllerRs2);
}

void RISCVInstructionWindow::setupFieldController(Gtk::Box* pFieldBox, const std::string& fieldName, int startBit, int endBit) {
    auto pMotionController = Gtk::EventControllerMotion::create();
    
    // 鼠标进入时高亮整个字段
    pMotionController->signal_enter().connect([this, pFieldBox, fieldName, startBit, endBit](double, double) {
        if(fieldName == "opcode") {
            highlightFieldInMouse(fieldName, startBit, endBit);
            highlightField("funct3", 12, 14);
            highlightField("funct7", 25, 31);
            InstructionLabel_->add_css_class("highlighted");
        } else if(fieldName == "funct3") {
            highlightFieldInMouse(fieldName, startBit, endBit);
            highlightField("opcode", 0, 6);
            highlightField("funct7", 25, 31);
            InstructionLabel_->add_css_class("highlighted");
        } else if(fieldName == "funct7") {
            highlightFieldInMouse(fieldName, startBit, endBit);
            highlightField("opcode", 0, 6);
            highlightField("funct3", 12, 14);
            InstructionLabel_->add_css_class("highlighted");
        } else if(fieldName == "rd") {
            highlightFieldInMouse(fieldName, startBit, endBit);
            RdLabel_->add_css_class("highlighted");
        } else if(fieldName == "rs1") {
            highlightFieldInMouse(fieldName, startBit, endBit);
            Rs1Label_->add_css_class("highlighted");
        } else if(fieldName == "rs2") {
            highlightFieldInMouse(fieldName, startBit, endBit);
            Rs2Label_->add_css_class("highlighted");
        }

        // 直接使用捕获的field_box设置tooltip
        pFieldBox->set_tooltip_text(FieldTooltips_[fieldName]);
    });
    
    // 鼠标离开时移除高亮
    pMotionController->signal_leave().connect([this, fieldName, startBit, endBit]() {
        if(fieldName == "opcode") {
            unhighlightFieldInMouse(fieldName, startBit, endBit);
            unhighlightField("funct3", 12, 14);
            unhighlightField("funct7", 25, 31);
            InstructionLabel_->remove_css_class("highlighted");
        } else if(fieldName == "funct3") {
            unhighlightFieldInMouse(fieldName, startBit, endBit);
            unhighlightField("opcode", 0, 6);
            unhighlightField("funct7", 25, 31);
            InstructionLabel_->remove_css_class("highlighted");
        } else if(fieldName == "funct7") {
            unhighlightFieldInMouse(fieldName, startBit, endBit);
            unhighlightField("opcode", 0, 6);
            unhighlightField("funct3", 12, 14);
            InstructionLabel_->remove_css_class("highlighted");
        } else if(fieldName == "rd") {
            unhighlightFieldInMouse(fieldName, startBit, endBit);
            RdLabel_->remove_css_class("highlighted");
        } else if(fieldName == "rs1") {
            unhighlightFieldInMouse(fieldName, startBit, endBit);
            Rs1Label_->remove_css_class("highlighted");
        } else if(fieldName == "rs2") {
            unhighlightFieldInMouse(fieldName, startBit, endBit);
            Rs2Label_->remove_css_class("highlighted");
        }
    });
    
    pFieldBox->add_controller(pMotionController);
}

void RISCVInstructionWindow::highlightFieldInMouse(const std::string& fieldName, int startBit, int endBit) {
    int arrayStart = 31 - endBit;
    int arrayEnd = 31 - startBit;
    for (int i = arrayStart; i <= arrayEnd; i++) {
        if (i >= 0 && i < static_cast<int>(BitLabels_.size())) {
            BitLabels_[i]->add_css_class("highlighted-in-mouse");
        }
    } 
}

void RISCVInstructionWindow::highlightField(const std::string& fieldName, int startBit, int endBit) {
    // 计算在BitLabels_数组中的实际位置
    // 注意：BitLabels_[0] 对应位31, BitLabels_[31] 对应位0
    int arrayStart = 31 - endBit;
    int arrayEnd = 31 - startBit;
    
    for (int i = arrayStart; i <= arrayEnd; ++i) {
        if (i >= 0 && i < static_cast<int>(BitLabels_.size())) {
            BitLabels_[i]->add_css_class("highlighted");
        }
    }
}

void RISCVInstructionWindow::unhighlightField(const std::string& fieldName, int startBit, int endBit) {
    int arrayStart = 31 - endBit;
    int arrayEnd = 31 - startBit;
    
    for (int i = arrayStart; i <= arrayEnd; ++i) {
        if (i >= 0 && i < static_cast<int>(BitLabels_.size())) {
            BitLabels_[i]->remove_css_class("highlighted");
        }
    }
}

void RISCVInstructionWindow::unhighlightFieldInMouse(const std::string& fieldName, int startBit, int endBit) {
    int arrayStart = 31 - endBit;
    int arrayEnd = 31 - startBit;
    for (int i = arrayStart; i <= arrayEnd; i++) {
        if (i >= 0 && i < static_cast<int>(BitLabels_.size())) {
            BitLabels_[i]->remove_css_class("highlighted-in-mouse");
        }
    } 
}

void RISCVInstructionWindow::setInstruction(const std::string& instructionStr) {
    // 清除所有高亮
    for (auto pLabel : BitLabels_) {
        pLabel->remove_css_class("highlighted");
        pLabel->remove_css_class("highlighted-in-mouse");
    }
    
    // 移除下划线和空格
    std::string binaryStr;
    for (char c : instructionStr) {
        if (c == '0' || c == '1') {
            binaryStr += c;
        }
    }
    
    // 确保字符串长度为32
    if (binaryStr.length() < 32) {
        binaryStr = std::string(32 - binaryStr.length(), '0') + binaryStr;
    } else if (binaryStr.length() > 32) {
        binaryStr = binaryStr.substr(0, 32);
    }
    
    // 设置每个位的值
    for (int i = 0; i < 32; ++i) {
        // 注意：BitLabels_[0] 对应位31, BitLabels_[31] 对应位0
        int bitValue = binaryStr[i] - '0';
        BitLabels_[i]->set_text(bitValue == 0 ? "0" : "1");
    }
}

void RISCVInstructionWindow::onGenerateRandom() {
    // 生成随机R-Type指令
    // opcode 固定为 0110011 (0x33)
    // 随机生成其他位
    
    std::string instruction = "";
    
    // funct7 (7 bits) - 随机 0 或 1 (0 代表 ADD, 1 代表 SUB)
    bool isSub = (rand() % 2 == 0);
    instruction += isSub ? "0100000" : "0000000";
    
    // rs2 (5 bits) - 随机寄存器
    for (int i = 0; i < 5; ++i) {
        instruction += (rand() % 2 == 0) ? '0' : '1';
    }
    
    // rs1 (5 bits) - 随机寄存器
    for (int i = 0; i < 5; ++i) {
        instruction += (rand() % 2 == 0) ? '0' : '1';
    }
    
    // funct3 (3 bits) - 随机操作类型
    std::vector<std::string> funct3Options = {"000", "001", "010", "011", "100", "101", "110", "111"};
    instruction += funct3Options[rand() % funct3Options.size()];
    
    // rd (5 bits) - 随机寄存器
    for (int i = 0; i < 5; ++i) {
        instruction += (rand() % 2 == 0) ? '0' : '1';
    }
    
    // opcode (7 bits) - R-type固定为0110011
    instruction += "0110011";
    
    // 设置指令
    setInstruction(instruction);
}

class RISCVApplication : public Gtk::Application {
public:
    RISCVApplication() : Gtk::Application("org.gtkmm.example.riscvdemo", Gio::Application::Flags::NONE) {}
    
    static Glib::RefPtr<RISCVApplication> Create() {
        return Glib::RefPtr<RISCVApplication>(new RISCVApplication());
    }
    
protected:
    void on_activate() override {
        auto pWindow = new RISCVInstructionWindow();
        add_window(*pWindow);
        
        pWindow->signal_close_request().connect([pWindow]() -> bool {
            delete pWindow;
            return true;
        }, false);
        
        pWindow->show();
    }
};

int main(int argc, char* argv[]) {
    srand(time(nullptr)); // 初始化随机数种子
    auto app = RISCVApplication::Create();
    return app->run(argc, argv);
}

//NOTLINTEND