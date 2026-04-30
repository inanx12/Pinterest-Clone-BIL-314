#pragma once
#include <QMainWindow>
#include "networkclient.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    NetworkClient* network_;
};
