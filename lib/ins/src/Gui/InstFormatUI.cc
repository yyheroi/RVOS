#include "Gui/InstFormatUI.hh"

InstFormatUI::InstFormatUI(const InstFormatST& format) 
    : Gtk::Box(Gtk::Orientation::VERTICAL, 10), format_(format) {
    setupAssemblyDisplay();
    setupBinaryDisplay();
    setupFieldControllers();
}

void InstFormatUI::setupAssemblyDisplay() {
    auto asmArea = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    auto asmTitle = Gtk::make_managed<Gtk::Label>("Assembly =");
    asmTitle->set_margin_end(8);
    asmArea->append(*asmTitle);

    for(const std::string& iStr : format_.instTypeV_) {
        AsmMnemonicWidgetPairST *asmMnemonic_ = new AsmMnemonicWidgetPairST(iStr, asmArea);
        if(iStr.compare(",") == 0)
            continue;
        asmFieldWidgets_[iStr] = asmMnemonic_;
    }
    
    append(*asmArea);
}

void InstFormatUI::setupBinaryDisplay() {
    auto binaryArea = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
    auto binaryTitle = Gtk::make_managed<Gtk::Label>("Binary =");
    binaryTitle->set_margin_end(8);
    binaryArea->append(*binaryTitle);
    binaryArea->set_halign(Gtk::Align::START);
    binaryArea->set_valign(Gtk::Align::CENTER);
    binaryArea->set_hexpand(false);
    // for(int i = 0; i < 32; i++) {
    //     auto tmpLabel = Gtk::make_managed<Gtk::Label>("0");
    //     tmpLabel->set_width_chars(2);
    //     tmpLabel->set_halign(Gtk::Align::CENTER);
    //     tmpLabel->set_valign(Gtk::Align::CENTER);
    //     tmpLabel->set_size_request(18, 30);
    //     tmpLabel->set_css_classes({"binary-bit"});
    //     if(i > 0 && i % 4 == 0)
    //         tmpLabel->set_margin_end(8);
    //     binaryLabelsV_.push_back(tmpLabel);
    // }

    std::cout << "size of format_.binaryLabelsV_: " << binaryLabelsV_.size() << std::endl;
    for(const auto &binaryElem : format_.binaryV_) {
        BinaryFieldWidgetPairST *tmpBinaryField_ = new BinaryFieldWidgetPairST(binaryLabelsV_ ,binaryElem);
        std::cout << "binaryElem.name_: " << binaryElem.name_ << std::endl;
        binaryFieldWidgets_[binaryElem.name_] = tmpBinaryField_;
        binaryArea->append(*tmpBinaryField_->box);
    }

    append(*binaryArea);
}
void InstFormatUI::setupFieldControllers() {

    for (const auto& asmElem : asmFieldWidgets_) {
        asmElem.second->setupController(asmElem.first, format_, binaryFieldWidgets_);
        std::cout << "InstFormatUI::setupFieldControllers(): " << asmElem.first << std::endl;
    }

    for (const auto& binaryElem : binaryFieldWidgets_) {
        binaryElem.second->setupController(binaryElem.first, format_, binaryFieldWidgets_, asmFieldWidgets_);
        std::cout << "InstFormatUI::setupFieldControllers(): " << binaryElem.first << std::endl;
    }

}
void InstFormatUI::updateDisplay(Instruction& inst) {
    updateAssemblyDisplay(inst);
    updateBinaryDisplay(inst);
}

void InstFormatUI::updateAssemblyDisplay(Instruction& inst) {
    const std::vector<std::string> &instAssembly = inst.GetType().GetInstAssembly();
    const std::vector<uint32_t> &instBitsField = inst.GetType().GetInstBitsField();
    if(inst.GetType().GetInstFormat() == InstFormat::R) {
        asmFieldWidgets_["mnemonic"]->label->set_text(instAssembly.at(0));
        asmFieldWidgets_["rd"]->label->set_text(instAssembly.at(1));
        asmFieldWidgets_["rs1"]->label->set_text(instAssembly.at(2));
        asmFieldWidgets_["rs2"]->label->set_text(instAssembly.at(3));
        binaryFieldWidgets_["opcode"]->updateControlLables(instBitsField.at(0));
        binaryFieldWidgets_["rd"]->updateControlLables(instBitsField.at(1));
        binaryFieldWidgets_["funct3"]->updateControlLables(instBitsField.at(2));
        binaryFieldWidgets_["rs1"]->updateControlLables(instBitsField.at(3));
        binaryFieldWidgets_["rs2"]->updateControlLables(instBitsField.at(4));
        binaryFieldWidgets_["funct7"]->updateControlLables(instBitsField.at(5));
    }
}

void InstFormatUI::updateBinaryDisplay(Instruction& inst) {

}

enum binaryIndex {
    OPCODE = 0,
    FUNCT3 = 1,
    FUNCT7 = 2,
};

InstFormatST createRTypeFormat() {
    InstFormatST rFormat;

    rFormat.typeName_ = "R-Type";
    rFormat.fmt_ = InstFormat::R;
    rFormat.instTypeV_ = {"mnemonic", "rd", ",", "rs1", ",", "rs2"};
    rFormat.binaryV_ = {
        {FUNCT7, "funct7", 25, 31, "funct7"},
        {-1, "rs2",   20, 24, "rs2"},
        {-1, "rs1",   15, 19, "rs1"},
        {FUNCT3, "funct3", 12, 14, "funct3"},
        {-1, "rd",     7, 11, "rd"},
        {OPCODE, "opcode", 0,  6, "opcode"},
    };
    
    rFormat.binaryFieldRelations_ = {
        {"opcode", {"mnemonic", "funct3", "funct7"}},
        {"funct3", {"mnemonic", "opcode", "funct7"}},
        {"funct7", {"mnemonic", "opcode", "funct3"}},
        {"rd", {"rd"}},
        {"rs1", {"rs1"}},
        {"rs2", {"rs2"}},
    };
    rFormat.asmFieldRelations_ = {
        {"mnemonic", {"opcode", "funct3", "funct7"}},
        {"rd", {"rd"}},
        {"rs1", {"rs1"}},
        {"rs2", {"rs2"}},
    };
    
    return rFormat;
}

