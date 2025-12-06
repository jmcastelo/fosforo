#ifndef MIDILINKMANAGER_H
#define MIDILINKMANAGER_H



#include "parameters/number.h"
#include "midisignals.h"

#include <QObject>
#include <QMap>
#include <QMultiMap>
#include <QUuid>
#include <QString>



class MidiLinkManager : public QObject
{
    Q_OBJECT

public:
    explicit MidiLinkManager(QObject *parent = nullptr);

    bool enabled();

    QMap<QPair<QString, int>, QMultiMap<int, Number<float>*>> floatLinks();
    QMap<QPair<QString, int>, QMultiMap<int, Number<int>*>> intLinks();
    QMap<QPair<QString, int>, QMultiMap<int, Number<unsigned int>*>> uintLinks();

signals:
    void midiEnabled(bool enabled);
    void multiLinkSet(bool set);

public slots:
    void addMidiSignals(QUuid id, MidiSignals* midiSignals);
    void removeMidiSignals(QUuid id);

    void setupMidi(QPair<QString, int> portId, bool open);

    void updateMidiLinks(QPair<QString, int> portId, int key, int value);

    void setupMidiLink(QPair<QString, int> portId, int key, Number<float>* number);
    void setupMidiLink(QPair<QString, int> portId, int key, Number<int>* number);
    void setupMidiLink(QPair<QString, int> portId, int key, Number<unsigned int>* number);

    void clearLinks();

    bool multiLink();
    void setMultiLink(bool enabled);

    void remapMidiLinks(QPair<QString, int> oldId, QPair<QString, int> newId);

private:
    QMap<QUuid, MidiSignals*> mMidiSignalsMap;

    QMap<QPair<QString, int>, QMultiMap<int, Number<float>*>> mFloatLinks;
    QMap<QPair<QString, int>, QMultiMap<int, Number<int>*>> mIntLinks;
    QMap<QPair<QString, int>, QMultiMap<int, Number<unsigned int>*>> mUintLinks;

    Number<float>* mLinkingFloat = nullptr;
    Number<int>* mLinkingInt = nullptr;
    Number<unsigned int>* mLinkingUint = nullptr;

    QMap<QPair<QString, int>, bool> mPortOpen;

    bool mMultiLink = false;

    void removeKey(QPair<QString, int> portId, int key);

    void setUpConnections(bool midiOn);

    void connectMidiSignals(QUuid id);
    void disconnectMidiSignals(QUuid id);
};



#endif // MIDILINKMANAGER_H
