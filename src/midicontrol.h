#ifndef MIDICONTROL_H
#define MIDICONTROL_H


#include "midiinputport.h"

#include <QObject>
#include <QPair>
#include <QMap>
#include <QMultiMap>
#include <libremidi/libremidi.hpp>



class MidiControl : public QObject
{
    Q_OBJECT

public:
    explicit MidiControl(QObject *parent = nullptr);
    ~MidiControl();

    void setObserver();

signals:
    void inputPortAdded(MidiInputPort* inPort);
    void inputPortRemoved(MidiInputPort* inPort);

    void ccInputMessageReceived(int map, int key, int value);

    void midiEnabled(bool enabled);

private:
    libremidi::observer observer;

    QMap<libremidi::input_port, libremidi::midi_in*> mInPortToMidiInMap;
    QMap<libremidi::input_port, MidiInputPort*> mInPortsMap;

    libremidi::midi_in* createMidiIn(MidiInputPort* inPort);
    void addInputPort(const libremidi::input_port& port);
    void removeInputPort(const libremidi::input_port& port);
    bool anyPortOpen();
};



#endif // MIDICONTROL_H
