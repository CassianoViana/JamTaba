#include "ChordLabel.h"
#include <QVariant>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>

ChordLabel *ChordLabel::currentChordLabel = nullptr;
const QColor ChordLabel::BEAT_PROGRESS_COLOR = QColor(0, 255, 0, 35); //transparent green

// ++++++++++++++++++++++++++++++++++++++++++++=

ChordLabel::ChordLabel(const Chord &chord, int chordBeats) :
    chord(chord),
    currentBeat(0),
    beatsCount(chordBeats)//how many beats per chord?
{
    setText(chordToHtmlText(chord));
    setStyleSheetPropertyStatus(false);
}

ChordLabel::~ChordLabel()
{
    if (ChordLabel::currentChordLabel && ChordLabel::currentChordLabel == this)
        ChordLabel::currentChordLabel = nullptr;
}

void ChordLabel::paintEvent(QPaintEvent *ev)
{
    QLabel::paintEvent(ev);
    if(beatsCount > 0 && currentBeat > 0){
        QPainter painter(this);
        int position = currentBeat * width()/beatsCount;
        //fill the green rectangle, the beat progress indicator
        painter.fillRect(1, 1, position, height()-2, BEAT_PROGRESS_COLOR);

        //draw a two pixel line to highlight the end of above rectangle
        painter.fillRect(position-2, 1, 2, height()-2, BEAT_PROGRESS_COLOR);
    }
}

void ChordLabel::incrementIntervalBeat()
{
    if(currentBeat < beatsCount){
        currentBeat++;
        repaint();
    }
}

ChordLabel *ChordLabel::setAsCurrentChord()
{
    if (currentChordLabel)
        currentChordLabel->unsetAsCurrentChord();
    currentChordLabel = this;
    incrementIntervalBeat();
    setStyleSheetPropertyStatus(true);
    return this;
}

void ChordLabel::unsetAsCurrentChord()
{
    if(ChordLabel::currentChordLabel)
        ChordLabel::currentChordLabel->currentBeat = 0;

    ChordLabel::currentChordLabel = nullptr;
    setStyleSheetPropertyStatus(false);
}

void ChordLabel::setStyleSheetPropertyStatus(bool status)
{
    setProperty("current", QVariant(status));
    style()->unpolish(this);
    style()->polish(this);
}

QString ChordLabel::chordToHtmlText(const Chord &chord)
{
    QString finalString = "<strong>" + chord.getRootKey().left(1).toUpper();// the first chord letter
    if (chord.isFlat() || chord.isSharp()) {
        if (chord.getRootKey().size() > 1)// just in case
            finalString += "<sup>" + chord.getRootKey().mid(1, 1) + "</sup>";
    }
    finalString += "</strong>";
    if (chord.hasLettersAfterRoot())
        finalString += chord.getLettersAfterRoot().toLower();
    if (chord.hasLastPart())
        finalString += chord.getLastPart();
    if (chord.hasBassInversion()) {
        finalString += "/<strong>" + chord.getBassInversion().left(1);
        if (chord.getBassInversion().size() > 1) // chord inversion is flat or sharp?
            finalString += "<sup>" + chord.getBassInversion().mid(1, 1) + "</sup>";
        finalString += "</strong>";
    }
    return finalString;
}
