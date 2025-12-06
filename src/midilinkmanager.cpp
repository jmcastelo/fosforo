


#include "midilinkmanager.h"



MidiLinkManager::MidiLinkManager(QObject *parent)
    : QObject{parent}
{}



bool MidiLinkManager::enabled()
{
    bool anyPortOpen = false;

    foreach (bool open, mPortOpen) {
        anyPortOpen |= open;
    }

    return anyPortOpen;
}



QMap<QPair<QString, int>, QMultiMap<int, Number<float>*>> MidiLinkManager::floatLinks()
{
    return mFloatLinks;
}



QMap<QPair<QString, int>, QMultiMap<int, Number<int>*>> MidiLinkManager::intLinks()
{
    return mIntLinks;
}



QMap<QPair<QString, int>, QMultiMap<int, Number<unsigned int>*>> MidiLinkManager::uintLinks()
{
    return mUintLinks;
}



void MidiLinkManager::addMidiSignals(QUuid id, MidiSignals* midiSignals)
{
    mMidiSignalsMap.insert(id, midiSignals);
    connectMidiSignals(id);
}



void MidiLinkManager::removeMidiSignals(QUuid id)
{
    disconnectMidiSignals(id);
    mMidiSignalsMap.remove(id);
}



void MidiLinkManager::setupMidi(QPair<QString, int> portId, bool open)
{
    if (open)
    {
        if (!mFloatLinks.contains(portId)) {
            mFloatLinks[portId] = QMultiMap<int, Number<float>*>();
        }

        if (!mIntLinks.contains(portId)) {
            mIntLinks[portId] = QMultiMap<int, Number<int>*>();
        }

        if (!mUintLinks.contains(portId)) {
            mUintLinks[portId] = QMultiMap<int, Number<unsigned int>*>();
        }
    }
    else
    {
        if (mFloatLinks.contains(portId))
        {
            for (auto key : mFloatLinks[portId].keys()) {
                removeKey(portId, key);
            }
            mFloatLinks.remove(portId);
        }

        if (mIntLinks.contains(portId))
        {
            for (auto key : mIntLinks[portId].keys()) {
                removeKey(portId, key);
            }
            mIntLinks.remove(portId);
        }

        if (mUintLinks.contains(portId))
        {
            for (auto key : mUintLinks[portId].keys()) {
                removeKey(portId, key);
            }
            mUintLinks.remove(portId);
        }
    }

    mPortOpen[portId] = open;

    // setUpConnections(enabled());

    emit midiEnabled(enabled());
}



void MidiLinkManager::updateMidiLinks(QPair<QString, int> portId, int key, int value)
{
    if (mLinkingFloat != nullptr)
    {
        // Linking a float number

        setupMidiLink(portId, key, mLinkingFloat);
        mLinkingFloat = nullptr;
    }
    else if (mLinkingInt != nullptr)
    {
        // Linking an int or uint number

        setupMidiLink(portId, key, mLinkingInt);
        mLinkingInt = nullptr;
    }
    else if (mLinkingUint != nullptr)
    {
        // Linking an int or uint number

        setupMidiLink(portId, key, mLinkingUint);
        mLinkingUint = nullptr;
    }
    else
    {
        // No number being linked: set value of already linked number
        // For each QMultiMap, iterate over all QMultiMap items

        if (mFloatLinks.contains(portId) && mFloatLinks[portId].contains(key))
        {
            auto [it, end] = mFloatLinks[portId].equal_range(key);
            while (it != end)
            {
                it.value()->setValueFromIndex(value);
                it.value()->setIndex();
                it++;
            }
        }
        if (mIntLinks.contains(portId) && mIntLinks[portId].contains(key))
        {
            auto [it, end] = mIntLinks[portId].equal_range(key);
            while (it != end)
            {
                it.value()->setValueFromIndex(value);
                it.value()->setIndex();
                it++;
            }
        }
        if (mUintLinks.contains(portId) && mUintLinks[portId].contains(key))
        {
            auto [it, end] = mUintLinks[portId].equal_range(key);
            while (it != end)
            {
                it.value()->setValueFromIndex(value);
                it.value()->setIndex();
                it++;
            }
        }
    }
}



void MidiLinkManager::setupMidiLink(QPair<QString, int> portId, int key, Number<float>* number)
{
    // Init if no links map exists for this port

    if (!mFloatLinks.contains(portId)) {
        mFloatLinks[portId] = QMultiMap<int, Number<float>*>();
    }

    // Remove previously assigned key

    if (!mMultiLink) {
        removeKey(portId, key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Remove link on number deletion

    connect(number, &Number<float>::deleting, this, [=, this]() {
        for (auto [id, map] : mFloatLinks.asKeyValueRange()) {
            if (mFloatLinks[id].contains(key, number)) {
                mFloatLinks[id].remove(key, number);
            }
        }
    });

    // Store link

    mFloatLinks[portId].insert(key, number);
}



void MidiLinkManager::setupMidiLink(QPair<QString, int> portId, int key, Number<int>* number)
{
    // Init if no links map exists for this port

    if (!mIntLinks.contains(portId)) {
        mIntLinks[portId] = QMultiMap<int, Number<int>*>();
    }

    // Remove previously assigned key

    if (!mMultiLink) {
        removeKey(portId, key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Remove link on number deletion

    connect(number, &Number<int>::deleting, this, [=, this]() {
        for (auto [id, map] : mFloatLinks.asKeyValueRange()) {
            if (mIntLinks[id].contains(key, number)) {
                mIntLinks[id].remove(key, number);
            }
        }
    });

    // Store link

    mIntLinks[portId].insert(key, number);
}



void MidiLinkManager::setupMidiLink(QPair<QString, int> portId, int key, Number<unsigned int>* number)
{
    // Init if no links map exists for this port

    if (!mUintLinks.contains(portId)) {
        mUintLinks[portId] = QMultiMap<int, Number<unsigned int>*>();
    }

    // Remove previously assigned key

    if (!mMultiLink) {
        removeKey(portId, key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Remove link on number deletion

    connect(number, &Number<unsigned int>::deleting, this, [=, this]() {
        for (auto [id, map] : mFloatLinks.asKeyValueRange()) {
            if (mUintLinks[id].contains(key, number)) {
                mUintLinks[id].remove(key, number);
            }
        }
    });

    // Store link

    mUintLinks[portId].insert(key, number);
}



void MidiLinkManager::clearLinks()
{
    for (auto [portId, links] : mFloatLinks.asKeyValueRange())
    {
        QMultiMapIterator<int, Number<float>*> it(links);
        while (it.hasNext())
        {
            it.next();
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
        }
    }

    for (auto [portId, links] : mIntLinks.asKeyValueRange())
    {
        QMultiMapIterator<int, Number<int>*> it(links);
        while (it.hasNext())
        {
            it.next();
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
        }
    }

    for (auto [portId, links] : mUintLinks.asKeyValueRange())
    {
        QMultiMapIterator<int, Number<unsigned int>*> it(links);
        while (it.hasNext())
        {
            it.next();
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
        }
    }

    mFloatLinks.clear();
    mIntLinks.clear();
    mUintLinks.clear();
}



bool MidiLinkManager::multiLink()
{
    return mMultiLink;
}



void MidiLinkManager::setMultiLink(bool enabled)
{
    mMultiLink = enabled;

    if (!enabled)
    {
        // Remove all links except the first set

        for (auto [portId, links] : mFloatLinks.asKeyValueRange())
        {
            foreach (int key, links.uniqueKeys())
            {
                QList<Number<float>*> numbers = links.values(key);
                for (int i = 0; i < numbers.size() - 1; i++)
                {
                    numbers[i]->setIndexMax(100'000);
                    numbers[i]->setMidiLinked(false);
                    links.remove(key, numbers[i]);
                }
            }
        }
        for (auto [portId, links] : mIntLinks.asKeyValueRange())
        {
            foreach (int key, links.uniqueKeys())
            {
                QList<Number<int>*> numbers = links.values(key);
                for (int i = 0; i < numbers.size() - 1; i++)
                {
                    numbers[i]->setIndexMax(100'000);
                    numbers[i]->setMidiLinked(false);
                    links.remove(key, numbers[i]);
                }
            }
        }
        for (auto [portId, links] : mUintLinks.asKeyValueRange())
        {
            foreach (int key, links.uniqueKeys())
            {
                QList<Number<unsigned int>*> numbers = links.values(key);
                for (int i = 0; i < numbers.size() - 1; i++)
                {
                    numbers[i]->setIndexMax(100'000);
                    numbers[i]->setMidiLinked(false);
                    links.remove(key, numbers[i]);
                }
            }
        }
    }

    emit multiLinkSet(enabled);
}



void MidiLinkManager::remapMidiLinks(QPair<QString, int> oldId, QPair<QString, int> newId)
{
    if (mFloatLinks.contains(oldId))
    {
        mFloatLinks.insert(newId, mFloatLinks[oldId]);
        mFloatLinks.remove(oldId);
    }

    if (mIntLinks.contains(oldId))
    {
        mIntLinks.insert(newId, mIntLinks[oldId]);
        mIntLinks.remove(oldId);
    }

    if (mUintLinks.contains(oldId))
    {
        mUintLinks.insert(newId, mUintLinks[oldId]);
        mUintLinks.remove(oldId);
    }
}



void MidiLinkManager::removeKey(QPair<QString, int> portId, int key)
{
    {
        auto [it, end] = mFloatLinks[portId].equal_range(key);
        while (it != end)
        {
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
            it++;
        }
        mFloatLinks[portId].remove(key);
    }
    {
        auto [it, end] = mIntLinks[portId].equal_range(key);
        while (it != end)
        {
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
            it++;
        }
        mIntLinks[portId].remove(key);
    }
    {
        auto [it, end] = mUintLinks[portId].equal_range(key);
        while (it != end)
        {
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
            it++;
        }
        mUintLinks[portId].remove(key);
    }
}



void MidiLinkManager::setUpConnections(bool midiOn)
{
    for (auto [id, widget] : mMidiSignalsMap.asKeyValueRange())
    {
        if (midiOn) {
            connectMidiSignals(id);
        } else {
            disconnectMidiSignals(id);
        }
    }
}



void MidiLinkManager::connectMidiSignals(QUuid id)
{
    connect(mMidiSignalsMap.value(id), QOverload<Number<float>*>::of(&MidiSignals::linkWait), this, [=, this](Number<float>* number) {
        mLinkingFloat = number;
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<int>*>::of(&MidiSignals::linkWait), this, [=, this](Number<int>* number) {
        mLinkingInt = number;
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<unsigned int>*>::of(&MidiSignals::linkWait), this, [=, this](Number<unsigned int>* number) {
        mLinkingUint = number;
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<float>*>::of(&MidiSignals::linkBreak), this, [=, this](Number<float>* number) {
        number->setIndexMax(100'000);
        number->setMidiLinked(false);

        for (auto [portId, links] : mFloatLinks.asKeyValueRange())
        {
            QList<int> keys = links.keys(number);
            foreach (int key, keys) {
                links.remove(key, number);
            }
        }
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<int>*>::of(&MidiSignals::linkBreak), this, [=, this](Number<int>* number) {
        number->setIndexMax(100'000);
        number->setMidiLinked(false);

        for (auto [portId, links] : mIntLinks.asKeyValueRange())
        {
            QList<int> keys = links.keys(number);
            foreach (int key, keys) {
                links.remove(key, number);
            }
        }
    });

    connect(mMidiSignalsMap.value(id), QOverload<Number<unsigned int>*>::of(&MidiSignals::linkBreak), this, [=, this](Number<unsigned int>* number) {
        number->setIndexMax(100'000);
        number->setMidiLinked(false);

        for (auto [portId, links] : mUintLinks.asKeyValueRange())
        {
            QList<int> keys = links.keys(number);
            foreach (int key, keys) {
                links.remove(key, number);
            }
        }
    });
}



void MidiLinkManager::disconnectMidiSignals(QUuid id)
{
    if (mMidiSignalsMap.contains(id))
    {
        disconnect(mMidiSignalsMap.value(id), QOverload<Number<float>*>::of(&MidiSignals::linkWait), this, nullptr);
        disconnect(mMidiSignalsMap.value(id), QOverload<Number<int>*>::of(&MidiSignals::linkWait), this, nullptr);
        disconnect(mMidiSignalsMap.value(id), QOverload<Number<unsigned int>*>::of(&MidiSignals::linkWait), this, nullptr);

        disconnect(mMidiSignalsMap.value(id), QOverload<Number<float>*>::of(&MidiSignals::linkBreak), this, nullptr);
        disconnect(mMidiSignalsMap.value(id), QOverload<Number<int>*>::of(&MidiSignals::linkBreak), this, nullptr);
        disconnect(mMidiSignalsMap.value(id), QOverload<Number<unsigned int>*>::of(&MidiSignals::linkBreak), this, nullptr);
    }
}
