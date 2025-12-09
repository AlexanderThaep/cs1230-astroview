#pragma once

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QMainWindow>
#include "astrorender.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void initialize();
    void finish();

private:
    AstroRender *astroRender;
};
