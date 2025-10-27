// VerticalToolbar.h
#ifndef VERTICALTOOLBAR_H
#define VERTICALTOOLBAR_H

#include <QWidget>

class VerticalToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit VerticalToolbar(QWidget *parent = nullptr);

private:
    void setupUI();
    void applyStyles();
};

#endif // VERTICALTOOLBAR_H

