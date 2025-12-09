#pragma once

#include <QMainWindow>
#include "astrorender.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private:
    AstroRender *astroRender;
};
