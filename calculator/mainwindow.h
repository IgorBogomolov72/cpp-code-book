#pragma once

#include "calculator.h"
#include "enums.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void SetInputText(const std::string& text);
    void SetErrorText(const std::string& text);
    void SetFormulaText(const std::string& text);
    void SetMemText(const std::string& text);
    void SetExtraKey(const std::optional<std::string>& key);

    void SetDigitKeyCallback(std::function<void(int key)> cb) {
        digit_cb_ = cb;
    }

    void SetProcessOperationKeyCallback(std::function<void(Operation key)> cb){
        operation_cb_ = cb;
    }

    void SetProcessControlKeyCallback(std::function<void(ControlKey key)> cb) {
        control_cb_ = cb;
    }

    void SetControllerCallback(std::function<void(ControllerType controller)> cb){
        controller_cb_ = cb;
    }

private slots:

    void on_btn_mc_clicked()
    {
        if (control_cb_ == nullptr){
            return;
        }
        control_cb_(ControlKey::MEM_CLEAR);
    }

    void on_btn_mr_clicked()
    {
        if (control_cb_ == nullptr){
            return;
        }
        control_cb_(ControlKey::MEM_LOAD);
    }

    void on_btn_ms_clicked()
    {
        if (control_cb_ == nullptr){
            return;
        }
        control_cb_(ControlKey::MEM_SAVE);
    }

    void on_btn_clear_clicked()
    {
        if (control_cb_ == nullptr){
            return;
        }
        control_cb_(ControlKey::CLEAR);
    }

    void on_btn_change_sign_clicked()
    {
        if (control_cb_ == nullptr){
            return;
        }
        control_cb_(ControlKey::PLUS_MINUS);
    }

    void on_btn_bsp_clicked()
    {
        if (control_cb_ == nullptr){
            return;
        }
        control_cb_(ControlKey::BACKSPACE);
    }

    void on_tb_extra_clicked()
    {
        if (control_cb_ == nullptr){
            return;
        }
        control_cb_(ControlKey::EXTRA_KEY);
    }

    void on_btn_calculate_clicked()
    {
        if (control_cb_ == nullptr){
            return;
        }
        control_cb_(ControlKey::EQUALS);
    }

    void on_btn_0_clicked()
    {
        if (digit_cb_ == nullptr){
            return;
        }
        digit_cb_(0);
    }

    void on_btn_1_clicked()
    {
        if (digit_cb_ == nullptr){
            return;
        }
        digit_cb_(1);
    }

    void on_btn_2_clicked()
    {
        if (digit_cb_ == nullptr){
            return;
        }
        digit_cb_(2);
    }

    void on_btn_3_clicked()
    {
        if (digit_cb_ == nullptr){
            return;
        }
        digit_cb_(3);
    }

    void on_btn_4_clicked()
    {
        if (digit_cb_ == nullptr){
            return;
        }
        digit_cb_(4);
    }

    void on_btn_5_clicked()
    {
        if (digit_cb_ == nullptr){
            return;
        }
        digit_cb_(5);
    }

    void on_btn_6_clicked()
    {
        if (digit_cb_ == nullptr){
            return;
        }
        digit_cb_(6);
    }

    void on_btn_7_clicked()
    {
        if (digit_cb_ == nullptr){
            return;
        }
        digit_cb_(7);
    }

    void on_btn_8_clicked()
    {
        if (digit_cb_ == nullptr){
            return;
        }
        digit_cb_(8);
    }

    void on_btn_9_clicked()
    {
        if (digit_cb_ == nullptr){
            return;
        }
        digit_cb_(9);
    }

    void on_cmb_controller_currentIndexChanged(int index)
    {
        switch (index) {
        case 0:
            controller_cb_(ControllerType::UINT8_T);
            break;
        case 1:
            controller_cb_(ControllerType::INT);
            break;
        case 2:
            controller_cb_(ControllerType::INT64_T);
            break;
        case 3:
            controller_cb_(ControllerType::SIZE_T);
            break;
        case 4:
            controller_cb_(ControllerType::DOUBLE);
            break;
        case 5:
            controller_cb_(ControllerType::FLOAT);
            break;
        case 6:
            controller_cb_(ControllerType::RATIONAL);
            break;
        }
    }

    void on_btn_power_clicked()
    {
        if (operation_cb_ == nullptr){
            return;
        }
        operation_cb_(Operation::POWER);
    }

    void on_btn_division_clicked()
    {
        if (operation_cb_ == nullptr){
            return;
        }
        operation_cb_(Operation::DIVISION);
    }

    void on_btn_multiplication_clicked()
    {
        if (operation_cb_ == nullptr){
            return;
        }
        operation_cb_(Operation::MULTIPLICATION);
    }

    void on_btn_subtraction_clicked()
    {
        if (operation_cb_ == nullptr){
            return;
        }
        operation_cb_(Operation::SUBTRACTION);
    }

    void on_btn_addition_clicked()
    {
        if (operation_cb_ == nullptr){
            return;
        }
        operation_cb_(Operation::ADDITION);
    }

private:
    Ui::MainWindow* ui;
    std::function<void(int key)> digit_cb_;
    std::function<void(Operation key)> operation_cb_;
    std::function<void(ControlKey key)> control_cb_;
    std::function<void(ControllerType controller)> controller_cb_;

};
