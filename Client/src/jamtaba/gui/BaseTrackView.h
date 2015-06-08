#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <QWidget>

namespace Ui {
    class TrackView;
}

namespace Controller {
    class MainController;
}

class BaseTrackView : public QWidget
{
    Q_OBJECT

public:
    BaseTrackView(QWidget *parent, Controller::MainController* mainController, int trackID);
    ~BaseTrackView();
    void setPeaks(float left, float right);
    inline int getTrackID() const{return trackID;}
protected:
    Ui::TrackView *ui;
    Controller::MainController* mainController;
    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *source, QEvent *ev);
    int trackID;

private slots:
    void onMuteClicked();
    void onSoloClicked();
    void onFaderMoved(int value);
    void onPanSliderMoved(int value);
};



#endif // TRACKVIEW_H
