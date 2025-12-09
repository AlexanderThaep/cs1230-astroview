#pragma once

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QMainWindow>
#include "astrorender.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void initialize(const QString &sceneFilePath);
    void finish();

private:
    AstroRender *astroRender;
};
