// ControlButtonsWidget.h
#ifndef CONTROLBUTTONSWIDGET_H
#define CONTROLBUTTONSWIDGET_H

#include <QWidget>

class QPushButton;

/**
 * @brief Widget containing run, stop, and pause buttons for schematic control
 * 
 * This widget provides control buttons that are overlaid on the schematic scene
 * in the top-right corner. It manages the state and appearance of the buttons.
 */
class ControlButtonsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ControlButtonsWidget(QWidget *parent = nullptr);
    
    enum State {
        Stopped,  // No operation running
        Running,  // Operation is running
        Paused    // Operation is paused
    };
    
    State getState() const { return m_state; }
    void setState(State state);

signals:
    void runClicked();
    void stopClicked();
    void pauseClicked();

private slots:
    void onRunButtonClicked();
    void onStopButtonClicked();
    void onPauseButtonClicked();

private:
    void setupUI();
    void applyStyles();
    void updateButtonStates();
    QIcon createRunIcon();
    QIcon createPauseIcon();
    QIcon createStopIcon();
    
    QPushButton* m_runButton;
    QPushButton* m_stopButton;
    QPushButton* m_pauseButton;
    State m_state;
};

#endif // CONTROLBUTTONSWIDGET_H

