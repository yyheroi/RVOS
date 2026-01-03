//NOTLINTBEGIN
#include <gtkmm.h>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "Gui/InstCommST.hh"
#include "ISA/InstFormat.hh"
#include "Core/Instruction.hh"

class InstFormatUI : public Gtk::Box {
public:
    InstFormatST format_;
    InstFormatUI(const InstFormatST& format);
    void updateInstructionValue(uint32_t instructionValue);
    void updateDisplay(Instruction& inst);
    
protected:
    void setupFieldControllers();
    void setupAssemblyDisplay();
    void setupBinaryDisplay();
    void updateAssemblyDisplay(Instruction& inst);
    void updateBinaryDisplay(Instruction& inst);
    
    std::vector<Gtk::Label*> binaryLabelsV_;
    std::unordered_map<std::string, Gtk::Box*> fieldBoxes_;
    std::unordered_map<std::string, Gtk::Label*> fieldValueLabels_;
    BinaryFieldWidgetSTMap binaryFieldWidgets_;
    AsmMnemonicWidgetSTMap asmFieldWidgets_;
    
    uint32_t currentInstruction_;
};

InstFormatST createRTypeFormat();

//NOTLINTEND