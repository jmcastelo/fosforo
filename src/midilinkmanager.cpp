


#include "midilinkmanager.h"



MidiLinkManager::MidiLinkManager(QObject *parent)
    : QObject{parent}
{
    // Init links with empty QMultiMaps

    for (int map = 0; map < 16; map++)
    {
        mFloatLinks[map] = QMultiMap<int, Number<float>*>();
        mIntLinks[map] = QMultiMap<int, Number<int>*>();
        mUintLinks[map] = QMultiMap<int, Number<unsigned int>*>();
    }
}



QMap<int, QMultiMap<int, Number<float>*>> MidiLinkManager::floatLinks()
{
    return mFloatLinks;
}



QMap<int, QMultiMap<int, Number<int>*>> MidiLinkManager::intLinks()
{
    return mIntLinks;
}



QMap<int, QMultiMap<int, Number<unsigned int>*>> MidiLinkManager::uintLinks()
{
    return mUintLinks;
}



bool MidiLinkManager::anyMidiLink()
{
    bool any = false;

    for (auto [map, links] : mFloatLinks.asKeyValueRange()) {
        any |= !links.empty();
    }
    for (auto [map, links] : mIntLinks.asKeyValueRange()) {
        any |= !links.empty();
    }
    for (auto [map, links] : mUintLinks.asKeyValueRange()) {
        any |= !links.empty();
    }

    return any;
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



void MidiLinkManager::updateMidiLinks(int map, int key, int value)
{
    if (mLinkingFloat != nullptr)
    {
        // Linking a float number

        setupMidiLink(map, key, mLinkingFloat);
        mLinkingFloat = nullptr;
    }
    else if (mLinkingInt != nullptr)
    {
        // Linking an int number

        setupMidiLink(map, key, mLinkingInt);
        mLinkingInt = nullptr;
    }
    else if (mLinkingUint != nullptr)
    {
        // Linking an uint number

        setupMidiLink(map, key, mLinkingUint);
        mLinkingUint = nullptr;
    }
    else
    {
        // No number being linked: set value of already linked number
        // For each QMultiMap, iterate over all QMultiMap items (multi-link)

        if (mFloatLinks.contains(map) && mFloatLinks[map].contains(key))
        {
            auto [it, end] = mFloatLinks[map].equal_range(key);
            while (it != end)
            {
                it.value()->setValueFromIndex(value);
                it.value()->setIndex();
                it++;
            }
        }
        if (mIntLinks.contains(map) && mIntLinks[map].contains(key))
        {
            auto [it, end] = mIntLinks[map].equal_range(key);
            while (it != end)
            {
                it.value()->setValueFromIndex(value);
                it.value()->setIndex();
                it++;
            }
        }
        if (mUintLinks.contains(map) && mUintLinks[map].contains(key))
        {
            auto [it, end] = mUintLinks[map].equal_range(key);
            while (it != end)
            {
                it.value()->setValueFromIndex(value);
                it.value()->setIndex();
                it++;
            }
        }
    }
}



void MidiLinkManager::setupMidiLink(int map, int key, Number<float>* number)
{
    // Init if no links map exists for this port

    if (!mFloatLinks.contains(map)) {
        mFloatLinks[map] = QMultiMap<int, Number<float>*>();
    }

    // Remove previously assigned key

    if (!mMultiLink) {
        removeKey(map, key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Remove link on number deletion

    connect(number, &Number<float>::deleting, this, [=, this]() {
        if (mFloatLinks[map].contains(key, number)) {
            mFloatLinks[map].remove(key, number);
        }
    });

    // Store link

    mFloatLinks[map].insert(key, number);
}



void MidiLinkManager::setupMidiLink(int map, int key, Number<int>* number)
{
    // Init if no links map exists for this port

    if (!mIntLinks.contains(map)) {
        mIntLinks[map] = QMultiMap<int, Number<int>*>();
    }

    // Remove previously assigned key

    if (!mMultiLink) {
        removeKey(map, key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Remove link on number deletion

    connect(number, &Number<int>::deleting, this, [=, this]() {
        if (mIntLinks[map].contains(key, number)) {
            mIntLinks[map].remove(key, number);
        }
    });

    // Store link

    mIntLinks[map].insert(key, number);
}



void MidiLinkManager::setupMidiLink(int map, int key, Number<unsigned int>* number)
{
    // Init if no links map exists for this port

    if (!mUintLinks.contains(map)) {
        mUintLinks[map] = QMultiMap<int, Number<unsigned int>*>();
    }

    // Remove previously assigned key

    if (!mMultiLink) {
        removeKey(map, key);
    }

    // Adjust to midi range

    number->setIndexMax(127);
    number->setMidiLinked(true);

    // Remove link on number deletion

    connect(number, &Number<unsigned int>::deleting, this, [=, this]() {
        if (mUintLinks[map].contains(key, number)) {
            mUintLinks[map].remove(key, number);
        }
    });

    // Store link

    mUintLinks[map].insert(key, number);
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



void MidiLinkManager::removeKey(int map, int key)
{
    {
        auto [it, end] = mFloatLinks[map].equal_range(key);
        while (it != end)
        {
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
            it++;
        }
        mFloatLinks[map].remove(key);
    }
    {
        auto [it, end] = mIntLinks[map].equal_range(key);
        while (it != end)
        {
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
            it++;
        }
        mIntLinks[map].remove(key);
    }
    {
        auto [it, end] = mUintLinks[map].equal_range(key);
        while (it != end)
        {
            it.value()->setIndexMax(100'000);
            it.value()->setMidiLinked(false);
            it++;
        }
        mUintLinks[map].remove(key);
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

        for (auto [map, links] : mFloatLinks.asKeyValueRange())
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

        for (auto [map, links] : mIntLinks.asKeyValueRange())
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

        for (auto [map, links] : mUintLinks.asKeyValueRange())
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
