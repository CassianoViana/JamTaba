#ifndef WAVE_PEAK_PANEL_H
#define WAVE_PEAK_PANEL_H

#include <QWidget>

class WavePeakPanel : public QWidget
{

public:
    explicit WavePeakPanel(QWidget *parent = 0);

    void addPeak(float peak);
    void clearPeaks();

    QSize minimumSizeHint() const;

    void setBufferingPercentage(uint percentage);
    void setShowBuffering(bool setShowBuffering);
protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    static const int peaksRectWidth;
    static const int peaksPad;

    bool showingBuffering;
    int bufferingPercentage;

    std::vector<float> peaksArray;

    uint maxPeaks;// change when widget resize

    int computeMaxPeaks();
    void recreatePeaksArray();

    void drawPeak(QPainter *g, int x, float peak, const QColor &color);
};

#endif
