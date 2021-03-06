#ifndef INTERVALPROGRESSWINDOW_H
#define INTERVALPROGRESSWINDOW_H

#include <QFrame>
#include <QCloseEvent>
#include <QDialog>
#include "IntervalProgressDisplay.h"

class IntervalProgressWindow : public QDialog
{
    Q_OBJECT
public:
    explicit IntervalProgressWindow(QWidget *parent, IntervalProgressDisplay::PaintShape paintMode, int beatsPerInterval, int beatsPerAccent, bool showingAccents);
    void setIntervalProgressShape(IntervalProgressDisplay::PaintShape paintMode);
    void setCurrentBeat(int currentBeat);
    void setBeatsPerInterval(int beatsPerInterval);
    void setBeatsPerAccent(int beatsPerAccent);
    void setShowAccents(bool showAccents);

    //QSize minimumSizeHint() const;
    //QSize sizeHint() const;

signals:
    void windowClosed();

protected:
    void closeEvent(QCloseEvent *event);

private:
    IntervalProgressDisplay *progressDisplay;
};

#endif // INTERVALPROGRESSWINDOW_H
