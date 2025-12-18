#include <gtkmm.h>
#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>

class RISCVInstructionWindow : public Gtk::Window {
    std::vector<Gtk::Label*> m_bit_labels;
    Gtk::Box* m_opcode_bits_box = nullptr;    // 位 [6:0]  - 最低7位
    Gtk::Box* m_funct3_bits_box = nullptr;    // 位 [14:12]
    Gtk::Box* m_funct7_bits_box = nullptr;    // 位 [31:25] - 最高7位
    Gtk::Box* m_rd_bits_box = nullptr;        // 位 [11:7]
    Gtk::Box* m_rs1_bits_box = nullptr;       // 位 [19:15]
    Gtk::Box* m_rs2_bits_box = nullptr;       // 位 [24:20]
    Gtk::Box* m_instruction_box = nullptr; 
    Gtk::Label* m_instruction_label = nullptr; // 用于悬停检测的指令标签
    
    // 用于存储每个位域的tooltip信息
    std::map<std::string, std::string> m_field_tooltips = {
        {"opcode", "opcode (操作码): 决定指令的基本操作类型\nADD/SUB/AND/OR/XOR等基本R-Type指令的opcode为0110011"},
        {"rd", "rd (目标寄存器): 存储指令结果的寄存器编号"},
        {"funct3", "funct3 (功能码3位): 用于区分同一类指令的不同变体\n例如ADD/SUB指令使用相同opcode，但不同funct3"},
        {"rs1", "rs1 (源寄存器1): 第一个操作数的寄存器编号"},
        {"rs2", "rs2 (源寄存器2): 第二个操作数的寄存器编号"},
        {"funct7", "funct7 (功能码7位): 用于进一步细分指令类型\n例如ADD使用0000000，SUB使用0100000"}
    };
    
public:
    RISCVInstructionWindow() {
        set_title("RISC-V指令格式可视化");
        set_default_size(900, 300);
        
        // 创建主布局
        auto main_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 10);
        main_box->set_margin(15);
        
        // 标题
        auto title_label = Gtk::make_managed<Gtk::Label>(); 
        title_label->set_markup("<b>RISC-V R-Type 指令格式可视化 (四位一组)</b>");
        title_label->set_halign(Gtk::Align::CENTER);
        main_box->append(*title_label);
        
        // 示例指令说明
        auto example_label = Gtk::make_managed<Gtk::Label>();
        example_label->set_markup("<i>示例: ADD x1, x2, x3 指令: 00000000_00110001_00000000_10110011</i>");
        example_label->set_halign(Gtk::Align::CENTER);

        main_box->append(*example_label);

        auto assembly_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        assembly_box->set_halign(Gtk::Align::START);
        assembly_box->set_valign(Gtk::Align::CENTER);
        auto assembly_title_label = Gtk::make_managed<Gtk::Label>("Assembly =");
        assembly_title_label->set_margin_end(8);

        // 保存指令标签指针，以便后续添加事件控制器
        m_instruction_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        m_instruction_label = Gtk::make_managed<Gtk::Label>("add");
        m_instruction_label->set_margin_end(8);
        m_instruction_label->set_css_classes({"instruction-label"});
        m_instruction_box->set_css_classes({"instruction-container"});
        m_instruction_box->append(*m_instruction_label);

        auto rd_label = Gtk::make_managed<Gtk::Label>("x5");
        auto rs1_label = Gtk::make_managed<Gtk::Label>("x6");
        auto rs2_label = Gtk::make_managed<Gtk::Label>("x7");
        auto comma1_label = Gtk::make_managed<Gtk::Label>(",");
        comma1_label->set_margin_start(4);
        comma1_label->set_margin_end(4);
        auto comma2_label = Gtk::make_managed<Gtk::Label>(",");
        comma2_label->set_margin_start(4);
        comma2_label->set_margin_end(4);

        assembly_box->append(*assembly_title_label);
        assembly_box->append(*m_instruction_box); // 使用成员变量
        assembly_box->append(*rd_label);
        assembly_box->append(*comma1_label);
        assembly_box->append(*rs1_label);
        assembly_box->append(*comma2_label);
        assembly_box->append(*rs2_label);
        main_box->append(*assembly_box);

        // 创建二进制位显示区域 - 从高位到低位 (31 到 0)
        auto binary_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        binary_box->set_halign(Gtk::Align::START);
        binary_box->set_valign(Gtk::Align::CENTER);
        
        // 创建不同的位域容器
        m_funct7_bits_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        m_funct7_bits_box->set_css_classes({"bit-field-container"});
        
        m_rs2_bits_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        m_rs2_bits_box->set_css_classes({"bit-field-container"});
        
        m_rs1_bits_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        m_rs1_bits_box->set_css_classes({"bit-field-container"});
        
        m_funct3_bits_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        m_funct3_bits_box->set_css_classes({"bit-field-container"});
        
        m_rd_bits_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        m_rd_bits_box->set_css_classes({"bit-field-container"});
        
        m_opcode_bits_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        m_opcode_bits_box->set_css_classes({"bit-field-container"});
        
        // 为各个功能字段添加事件控制器
        setup_field_controller(m_funct7_bits_box, "funct7", 25, 31);
        setup_field_controller(m_rs2_bits_box, "rs2", 20, 24);
        setup_field_controller(m_rs1_bits_box, "rs1", 15, 19);
        setup_field_controller(m_funct3_bits_box, "funct3", 12, 14);
        setup_field_controller(m_rd_bits_box, "rd", 7, 11);
        setup_field_controller(m_opcode_bits_box, "opcode", 0, 6);
        
        // 为指令标签添加特殊控制器
        setup_instruction_controller();
        
        // 创建32个位的标签 - 从高位31到低位0
        for (int i = 31; i >= 0; --i) {
            auto bit_label = Gtk::make_managed<Gtk::Label>("0");
            bit_label->set_width_chars(1);
            bit_label->set_halign(Gtk::Align::CENTER);
            bit_label->set_valign(Gtk::Align::CENTER);
            bit_label->set_size_request(18, 30);
            
            // 添加边框以便看清每个位
            bit_label->set_css_classes({"binary-bit"});
            if(i > 0 && i % 4 == 0)
                bit_label->set_margin_end(8);

            m_bit_labels.push_back(bit_label);
            
            // 根据位位置分配到不同的字段容器
            if (i >= 25 && i <= 31) { // funct7: 31-25
                m_funct7_bits_box->append(*bit_label);
            } 
            else if (i >= 20 && i <= 24) { // rs2: 24-20
                m_rs2_bits_box->append(*bit_label);
            }
            else if (i >= 15 && i <= 19) { // rs1: 19-15
                m_rs1_bits_box->append(*bit_label);
            }
            else if (i >= 12 && i <= 14) { // funct3: 14-12
                m_funct3_bits_box->append(*bit_label);
            }
            else if (i >= 7 && i <= 11) { // rd: 11-7
                m_rd_bits_box->append(*bit_label);
            }
            else if (i >= 0 && i <= 6) { // opcode: 6-0
                m_opcode_bits_box->append(*bit_label);
            }

        }
        auto binary_head_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        auto binary_title_label = Gtk::make_managed<Gtk::Label>("Binary =");
        binary_title_label->set_margin_end(8);
        binary_head_box->append(*binary_title_label);

        binary_box->append(*binary_head_box);

        // 按RISC-V格式排列各个字段 (从高位到低位)
        binary_box->append(*m_funct7_bits_box);
        binary_box->append(*m_rs2_bits_box);
        binary_box->append(*m_rs1_bits_box);
        binary_box->append(*m_funct3_bits_box);
        binary_box->append(*m_rd_bits_box);
        binary_box->append(*m_opcode_bits_box);
        
        // 添加二进制位区域到主布局
        main_box->append(*binary_box);
        
        // 添加四位分组指示器
        auto group_indicator_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
        group_indicator_box->set_halign(Gtk::Align::CENTER);
        group_indicator_box->set_margin_top(2);
        
        // 32位分为8组，每组4位
        for (int i = 0; i < 8; ++i) {
            auto group_label = Gtk::make_managed<Gtk::Label>("    ");
            group_label->set_halign(Gtk::Align::CENTER);
            group_label->set_valign(Gtk::Align::START);
            group_indicator_box->append(*group_label);
            
            if (i < 7) {
                group_indicator_box->append(*Gtk::make_managed<Gtk::Label>("  "));
            }
        }
        
        main_box->append(*group_indicator_box);
        
        
        // 添加说明文本
        auto description = Gtk::make_managed<Gtk::Label>(
            "说明: 鼠标悬停在不同指令字段上时，相应位域将被高亮显示并显示详细信息 (四位一组格式)"
        );
        description->set_halign(Gtk::Align::CENTER);
        description->set_margin_top(10);
        main_box->append(*description);
        
        // 添加预设指令按钮
        auto button_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
        button_box->set_halign(Gtk::Align::CENTER);
        button_box->set_margin_top(10);
        
        auto add_button = Gtk::make_managed<Gtk::Button>("ADD 指令");
        add_button->signal_clicked().connect([this]() { set_instruction("0000000_00011_00010_000_00001_0110011"); });
        button_box->append(*add_button);
        
        auto sub_button = Gtk::make_managed<Gtk::Button>("SUB 指令");
        sub_button->signal_clicked().connect([this]() { set_instruction("0100000_00011_00010_000_00001_0110011"); });
        button_box->append(*sub_button);
        
        auto and_button = Gtk::make_managed<Gtk::Button>("AND 指令");
        and_button->signal_clicked().connect([this]() { set_instruction("0000000_00011_00010_111_00001_0110011"); });
        button_box->append(*and_button);
        
        auto or_button = Gtk::make_managed<Gtk::Button>("OR 指令");
        or_button->signal_clicked().connect([this]() { set_instruction("0000000_00011_00010_110_00001_0110011"); });
        button_box->append(*or_button);
        
        auto xor_button = Gtk::make_managed<Gtk::Button>("XOR 指令");
        xor_button->signal_clicked().connect([this]() { set_instruction("0000000_00011_00010_100_00001_0110011"); });
        button_box->append(*xor_button);
        
        auto random_button = Gtk::make_managed<Gtk::Button>("随机 R-Type 指令");
        random_button->signal_clicked().connect(sigc::mem_fun(*this, &RISCVInstructionWindow::on_generate_random));
        button_box->append(*random_button);
        
        main_box->append(*button_box);
        
        set_child(*main_box);
        
        // 设置CSS样式
        setup_css();
        
        // 初始设置为ADD指令
        set_instruction("0000000_00011_00010_000_00001_0110011");
    }
    
private:
    void setup_css() {
        auto css_provider = Gtk::CssProvider::create();
        css_provider->load_from_data(R"(
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

            .binary-bit.highlighted_in_mouse {
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
                color: #c586c0;
            }
            .instruction-label.highlighted {
                background-color: #4a86e8;
                color: white;
                border: 1px solid #2a5298;
            }
            .instruction-label.highlighted_in_mouse {
                background-color: #b9e84aff;
                color: white;
                border: 1px solid #61982aff;
            }
        )");
        
        Gtk::StyleContext::add_provider_for_display(
            Gdk::Display::get_default(),
            css_provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
    }
    
    void setup_instruction_controller() {
        if (!m_instruction_label) return;
        
        auto motion_controller = Gtk::EventControllerMotion::create();
        
        // 鼠标进入时高亮opcode, funct3, funct7
        motion_controller->signal_enter().connect([this](double x, double y) {
            m_instruction_label->add_css_class({"highlighted_in_mouse"});
            highlight_field("opcode", 0, 6);
            highlight_field("funct3", 12, 14);
            highlight_field("funct7", 25, 31);
        });
        
        // 鼠标离开时取消高亮
        motion_controller->signal_leave().connect([this]() {
            m_instruction_label->remove_css_class("highlighted_in_mouse");
            unhighlight_field("opcode", 0, 6);
            unhighlight_field("funct3", 12, 14);
            unhighlight_field("funct7", 25, 31);
        });
        
        // m_instruction_label->add_controller(motion_controller);
        m_instruction_box->add_controller(motion_controller);
    }
    
    void setup_field_controller(Gtk::Box* field_box, const std::string& field_name, int start_bit, int end_bit) {
        auto motion_controller = Gtk::EventControllerMotion::create();
        
        // 鼠标进入时高亮整个字段
        motion_controller->signal_enter().connect([this, field_box, field_name, start_bit, end_bit](double x, double y) {
            if(field_name == "opcode") {
                highlight_field_in_mouse(field_name, start_bit, end_bit);
                highlight_field("funct3", 12, 14);
                highlight_field("funct7", 25, 31);
                m_instruction_label->add_css_class("highlighted");
            } else if(field_name == "funct3") {
                highlight_field_in_mouse(field_name, start_bit, end_bit);
                highlight_field("opcode", 0, 6);
                highlight_field("funct7", 25, 31);
                m_instruction_label->add_css_class("highlighted");
            } else if(field_name == "funct7") {
                highlight_field_in_mouse(field_name, start_bit, end_bit);
                highlight_field("opcode", 0, 6);
                highlight_field("funct3", 12, 14);
                m_instruction_label->add_css_class("highlighted");
            } else {
                highlight_field_in_mouse(field_name, start_bit, end_bit);
            }

            // 直接使用捕获的field_box设置tooltip
            field_box->set_tooltip_text(m_field_tooltips[field_name]);
        });
        
        // 鼠标离开时移除高亮
        motion_controller->signal_leave().connect([this, field_name, start_bit, end_bit]() {
            if(field_name == "opcode") {
                unhighlight_field_in_mouse(field_name, start_bit, end_bit);
                unhighlight_field("funct3", 12, 14);
                unhighlight_field("funct7", 25, 31);
                m_instruction_label->remove_css_class("highlighted");
            } else if(field_name == "funct3") {
                unhighlight_field_in_mouse(field_name, start_bit, end_bit);
                unhighlight_field("opcode", 0, 6);
                unhighlight_field("funct7", 25, 31);
                m_instruction_label->remove_css_class("highlighted");
            } else if(field_name == "funct7") {
                unhighlight_field_in_mouse(field_name, start_bit, end_bit);
                unhighlight_field("opcode", 0, 6);
                unhighlight_field("funct3", 12, 14);
                m_instruction_label->remove_css_class("highlighted");
            } else {
                unhighlight_field_in_mouse(field_name, start_bit, end_bit);
            }

        });
        
        field_box->add_controller(motion_controller);
    }
    
    void highlight_field_in_mouse(const std::string& field_name, int start_bit, int end_bit) {
        int array_start = 31 - end_bit;
        int array_end = 31 - start_bit;
        for (int i = array_start; i <= array_end; i++) {
            m_bit_labels[i]->add_css_class("highlighted_in_mouse");
        } 
    }
    void highlight_field(const std::string& field_name, int start_bit, int end_bit) {
        // 计算在m_bit_labels数组中的实际位置
        // 注意：m_bit_labels[0] 对应位31, m_bit_labels[31] 对应位0
        int array_start = 31 - end_bit;
        int array_end = 31 - start_bit;
        
        for (int i = array_start; i <= array_end; ++i) {
            if (i >= 0 && i < 32) {
                m_bit_labels[i]->add_css_class("highlighted");
            }
        }
    }
    
    void unhighlight_field(const std::string& field_name, int start_bit, int end_bit) {
        int array_start = 31 - end_bit;
        int array_end = 31 - start_bit;
        
        for (int i = array_start; i <= array_end; ++i) {
            if (i >= 0 && i < 32) {
                m_bit_labels[i]->remove_css_class("highlighted");
            }
        }
    }
    void unhighlight_field_in_mouse(const std::string& field_name, int start_bit, int end_bit) {
        int array_start = 31 - end_bit;
        int array_end = 31 - start_bit;
        for (int i = array_start; i <= array_end; i++) {
            if (i >= 0 && i < 32) {
                m_bit_labels[i]->remove_css_class("highlighted_in_mouse");
            }
        } 
    }
    
    void set_instruction(const std::string& instruction_str) {
        // 清除所有高亮
        for (auto label : m_bit_labels) {
            label->remove_css_class("highlighted");
            label->remove_css_class("highlighted_in_mouse");
        }
        
        // 移除下划线和空格
        std::string binary_str;
        for (char c : instruction_str) {
            if (c == '0' || c == '1') {
                binary_str += c;
            }
        }
        
        // 确保字符串长度为32
        if (binary_str.length() < 32) {
            binary_str = std::string(32 - binary_str.length(), '0') + binary_str;
        } else if (binary_str.length() > 32) {
            binary_str = binary_str.substr(0, 32);
        }
        
        // 设置每个位的值
        for (int i = 0; i < 32; ++i) {
            // 注意：m_bit_labels[0] 对应位31, m_bit_labels[31] 对应位0
            int bit_value = binary_str[i] - '0';
            m_bit_labels[i]->set_text(bit_value == 0 ? "0" : "1");
        }
    }
    
    void on_generate_random() {
        // 生成随机R-Type指令
        // opcode 固定为 0110011 (0x33)
        // 随机生成其他位
        
        std::string instruction = "";
        
        // funct7 (7 bits) - 随机 0 或 1 (0 代表 ADD, 1 代表 SUB)
        bool is_sub = (rand() % 2 == 0);
        instruction += is_sub ? "0100000" : "0000000";
        
        // rs2 (5 bits) - 随机寄存器
        for (int i = 0; i < 5; ++i) {
            instruction += (rand() % 2 == 0) ? '0' : '1';
        }
        
        // rs1 (5 bits) - 随机寄存器
        for (int i = 0; i < 5; ++i) {
            instruction += (rand() % 2 == 0) ? '0' : '1';
        }
        
        // funct3 (3 bits) - 随机操作类型
        std::vector<std::string> funct3_options = {"000", "001", "010", "011", "100", "101", "110", "111"};
        instruction += funct3_options[rand() % funct3_options.size()];
        
        // rd (5 bits) - 随机寄存器
        for (int i = 0; i < 5; ++i) {
            instruction += (rand() % 2 == 0) ? '0' : '1';
        }
        
        // opcode (7 bits) - R-type固定为0110011
        instruction += "0110011";
        
        // 设置指令
        set_instruction(instruction);
    }
};

class RISCVApplication : public Gtk::Application {
public:
    RISCVApplication() : Gtk::Application("org.gtkmm.example.riscvdemo", Gio::Application::Flags::NONE) {}
    
    static Glib::RefPtr<RISCVApplication> create() {
        return Glib::RefPtr<RISCVApplication>(new RISCVApplication());
    }
    
protected:
    void on_activate() override {
        auto window = new RISCVInstructionWindow();
        add_window(*window);
        
        window->signal_close_request().connect([window]() -> bool {
            delete window;
            return true;
        }, false);
        
        window->show();
    }
};

int main(int argc, char* argv[]) {
    srand(time(nullptr)); // 初始化随机数种子
    auto app = RISCVApplication::create();
    return app->run(argc, argv);
}