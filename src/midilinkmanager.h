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

    QMap<int, QMultiMap<int, Number<float>*>> floatLinks();
    QMap<int, QMultiMap<int, Number<int>*>> intLinks();
    QMap<int, QMultiMap<int, Number<unsigned int>*>> uintLinks();

    bool anyMidiLink();

signals:
    void multiLinkSet(bool set);

public slots:
    void addMidiSignals(QUuid id, MidiSignals* midiSignals);
    void removeMidiSignals(QUuid id);

    void updateMidiLinks(int map, int key, int value);

    void setupMidiLink(int map, int key, Number<float>* number);
    void setupMidiLink(int map, int key, Number<int>* number);
    void setupMidiLink(int map, int key, Number<unsigned int>* number);

    void clearLinks();

    bool multiLink();
    void setMultiLink(bool enabled);

private:
    QMap<QUuid, MidiSignals*> mMidiSignalsMap;

    QMap<int, QMultiMap<int, Number<float>*>> mFloatLinks;
    QMap<int, QMultiMap<int, Number<int>*>> mIntLinks;
    QMap<int, QMultiMap<int, Number<unsigned int>*>> mUintLinks;

    Number<float>* mLinkingFloat = nullptr;
    Number<int>* mLinkingInt = nullptr;
    Number<unsigned int>* mLinkingUint = nullptr;

    bool mMultiLink = false;

    void removeKey(int map, int key);

    void setUpConnections(bool midiOn);

    void connectMidiSignals(QUuid id);
    void disconnectMidiSignals(QUuid id);
};



#endif // MIDILINKMANAGER_H
