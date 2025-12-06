#ifndef MIDICONTROL_H
#define MIDICONTROL_H


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
    void inputPortAdded(QPair<QString, int> id);
    void inputPortRemoved(QPair<QString, int> id);
    void inputPortIdChanged(QPair<QString, int> oldId, QPair<QString, int> newId);
    void inputPortOpen(QPair<QString, int> id, bool open);
    void ccInputMessageReceived(QPair<QString, int> id, int key, int value);

private:
    libremidi::observer observer;

    QMap<QPair<QString, int>, libremidi::midi_in*> idToMidiInMap;
    QMap<QPair<QString, int>, libremidi::input_port> idToInputPortMap;
    QMap<libremidi::port_handle, QPair<QString, int>> handleToIdMap;
    QMap<QString, QList<int>> deviceToIndicesMap;

    libremidi::midi_in* createMidiIn(QPair<QString, int> id);
    void addInputPort(const libremidi::input_port& port);
    void removeInputPort(const libremidi::input_port& port);
    void recreateMidiIn(QPair<QString, int> oldId, QPair<QString, int> newId);
};



#endif // MIDICONTROL_H
