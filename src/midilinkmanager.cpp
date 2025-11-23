


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



QMap<QString, QMultiMap<int, Number<float>*>> MidiLinkManager::floatLinks()
{
    return mFloatLinks;
}



QMap<QString, QMultiMap<int, Number<int>*>> MidiLinkManager::intLinks()
{
    return mIntLinks;
}



QMap<QString, QMultiMap<int, Number<unsigned int>*>> MidiLinkManager::uintLinks()
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
    mMidiSignalsMap.remove(id);
    disconnectMidiSignals(id);
}



void MidiLinkManager::setupMidi(QString portName, bool open)
{
    if (open)
    {
        if (!mFloatLinks.contains(portName)) {
            mFloatLinks[portName] = QMultiMap<int, Number<float>*>();
        }

        if (!mIntLinks.contains(portName)) {
            mIntLinks[portName] = QMultiMap<int, Number<int>*>();
        }

        if (!mUintLinks.contains(portName)) {
            mUintLinks[portName] = QMultiMap<int, Number<unsigned int>*>();
        }
    }
    /*else
    {
        mFloatLinks.remove(portName);
        mIntLinks.remove(portName);
    }*/

    mPortOpen[portName] = open;

    // setUpConnections(enabled());

    emit midiEnabled(enabled());
}



void MidiLinkManager::updateMidiLinks(QString portName, int key, int value)
{
    if (mLinkingFloat != nullptr)
    {
        // Linking a float number

        setupMidiLink(portName, key, mLinkingFloat);
        mLinkingFloat = nullptr;
    }
    else if (mLinkingInt != nullptr)
    {
        // Linking an int or uint number

        setupMidiLink(portName, key, mLinkingInt);
        mLinkingInt = nullptr;
    }
    else if (mLinkingUint != nullptr)
    {
        // Linking an int or uint number

        setupMidiLink(portName, key, mLinkingUint);
        mLinkingUint = nullptr;
    }
    else
    {
        // No number being linked: set value of already linked number
        // For each QMultiMap, iterate over all QMultiMap items

        if (mFloatLinks.contains(portName) && mFloatLinks[portName].contains(key))
        {
            auto [it, end] = mFloatLinks[portName].equal_range(key);
            while (it != end)
            {
                it.value()->setValueFromIndex(value);
                it.value()->setIndex();
                it++;
            }
        }
        if (mIntLinks.contains(portName) && mIntLinks[portName].contains(key))
        {
            auto [it, end] = mIntLinks[portName].equal_range(key);
            while (it != end)
            {
                it.value()->setValueFromIndex(value);
                it.value()->setIndex();
                it++;
            }
        }
        if (mUintLinks.contains(portName) && mUintLinks[portName].contains(key))
        {
            auto [it, end] = mUintLinks[portName].equal_range(key);
            while (it != end)
            {
                it.value()->setValueFromIndex(value);
                it.value()->setIndex();
                it++;
            }
        }
    }
}



void MidiLinkManager::setupMidiLink(QString portName, int key, Number<float>* number)
{
    // Init if no links map exists for this port

    if (!mFloatLinks.contains(portName)) {
        mFloatLinks[portName] = QMultiMap<int, Number<float>*>();
    }

    // Remove previously assigned key

    if (!mMultiLink) {
        removeKey(key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Remove link on number deletion

    connect(number, &Number<float>::deleting, this, [=, this]() {
        mFloatLinks[portName].remove(key, number);
    });

    // Store link

    mFloatLinks[portName].insert(key, number);

    if (!mPortOpen[portName]) {
        emit midiLinkSet(portName);
    }
}



void MidiLinkManager::setupMidiLink(QString portName, int key, Number<int>* number)
{
    // Init if no links map exists for this port

    if (!mIntLinks.contains(portName)) {
        mIntLinks[portName] = QMultiMap<int, Number<int>*>();
    }

    // Remove previously assigned key

    if (!mMultiLink) {
        removeKey(key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Remove link on number deletion

    connect(number, &Number<int>::deleting, this, [=, this]() {
        mIntLinks[portName].remove(key, number);
    });

    // Store link

    mIntLinks[portName].insert(key, number);

    if (!mPortOpen[portName]) {
        emit midiLinkSet(portName);
    }
}



void MidiLinkManager::setupMidiLink(QString portName, int key, Number<unsigned int>* number)
{
    // Init if no links map exists for this port

    if (!mUintLinks.contains(portName)) {
        mUintLinks[portName] = QMultiMap<int, Number<unsigned int>*>();
    }

    // Remove previously assigned key

    if (!mMultiLink) {
        removeKey(key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Remove link on number deletion

    connect(number, &Number<unsigned int>::deleting, this, [=, this]() {
        mUintLinks[portName].remove(key, number);
    });

    // Store link

    mUintLinks[portName].insert(key, number);

    if (!mPortOpen[portName]) {
        emit midiLinkSet(portName);
    }
}



void MidiLinkManager::clearLinks()
{
    for (auto [portName, links] : mFloatLinks.asKeyValueRange())
    {
        QMultiMapIterator<int, Number<float>*> it(links);
        while (it.hasNext())
        {
            it.next();
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
        }
    }

    for (auto [portName, links] : mIntLinks.asKeyValueRange())
    {
        QMultiMapIterator<int, Number<int>*> it(links);
        while (it.hasNext())
        {
            it.next();
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
        }
    }

    for (auto [portName, links] : mUintLinks.asKeyValueRange())
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

        for (auto [portName, links] : mFloatLinks.asKeyValueRange())
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
        for (auto [portName, links] : mIntLinks.asKeyValueRange())
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
        for (auto [portName, links] : mUintLinks.asKeyValueRange())
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



void MidiLinkManager::removeKey(int key)
{
    for (auto [portName, links] : mFloatLinks.asKeyValueRange())
    {
        auto [it, end] = links.equal_range(key);
        while (it != end)
        {
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
            it++;
        }
        links.remove(key);
    }

    for (auto [portName, links] : mIntLinks.asKeyValueRange())
    {
        auto [it, end] = links.equal_range(key);
        while (it != end)
        {
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
            it++;
        }
        links.remove(key);
    }

    for (auto [portName, links] : mUintLinks.asKeyValueRange())
    {
        auto [it, end] = links.equal_range(key);
        while (it != end)
        {
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
            it++;
        }
        links.remove(key);
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

        for (auto [portName, links] : mFloatLinks.asKeyValueRange())
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

        for (auto [portName, links] : mIntLinks.asKeyValueRange())
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

        for (auto [portName, links] : mUintLinks.asKeyValueRange())
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
