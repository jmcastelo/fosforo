#ifndef MIDILISTWIDGET_H
#define MIDILISTWIDGET_H



#include "midiinputport.h"

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>



class MidiListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MidiListWidget(QWidget *parent = nullptr);

signals:
    void multiLinkButtonChecked(bool checked);
    void clearLinksButtonClicked();

public slots:
    void addPortEntry(MidiInputPort* inPort);
    void removePortEntry(MidiInputPort* inPort);
    void toggleMultiLinkButton(bool checked);

private:
    QTableWidget* mPortsTable;
    QPushButton* mMultiLinkButton;

    QList<MidiInputPort*> mMidiInPorts;
};



#endif // MIDILISTWIDGET_H
