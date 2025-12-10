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
    void inputPortOpen(QPair<QString, int> id, bool open);

    void ccInputMessageReceived(int set, int key, int value);

private:
    libremidi::observer observer;

    QMap<QPair<QString, int>, libremidi::midi_in*> idToMidiInMap;
    QMap<QPair<QString, int>, libremidi::input_port> idToInputPortMap;
    QMap<libremidi::port_handle, QPair<QString, int>> handleToIdMap;
    QMap<QString, QList<int>> deviceToIndicesMap;

    QMap<libremidi::input_port, libremidi::midi_in*> mInPortToMidiInMap;
    QMap<libremidi::input_port, MidiInputPort*> mInPortsMap;

    libremidi::midi_in* createMidiIn(MidiInputPort* inPort);
    void addInputPort(const libremidi::input_port& port);
    void removeInputPort(const libremidi::input_port& port);
};



#endif // MIDICONTROL_H
