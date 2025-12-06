#include "midicontrol.h"
#include <QDebug>



MidiControl::MidiControl(QObject *parent) : QObject(parent)
{}



MidiControl::~MidiControl()
{
    foreach (auto midiInput, idToMidiInMap) {
        delete midiInput;
    }
}



void MidiControl::setObserver()
{
    libremidi::observer_configuration config {
        .input_added = [&] (const libremidi::input_port& port) {
            addInputPort(port);
        },
        .input_removed = [&] (const libremidi::input_port& port) {
            removeInputPort(port);
        }
    };

    observer = libremidi::observer { std::move(config) };
}



libremidi::midi_in* MidiControl::createMidiIn(QPair<QString, int> id)
{
    libremidi::input_configuration config {
        .on_message = [=, this](const libremidi::message& message) {
            if (message.get_message_type() == libremidi::message_type::CONTROL_CHANGE)
            {
                // MIDI CC message bytes:
                // message[0] = channel, ranges from 176 (channel 1) to 191 (channel 16)
                // message[1] = controller, ranges from 0 to 127 -> knob or fader
                // message[2] = value, ranges from 0 to 127

                int key = message[0] * 128 + message[1];
                emit ccInputMessageReceived(id, key, message[2]);
            }
        }
    };

    libremidi::midi_in* midiIn = new libremidi::midi_in { std::move(config) };

    return midiIn;
}


void MidiControl::addInputPort(const libremidi::input_port& port)
{
    QString deviceName = QString::fromStdString(port.device_name);
    int deviceNumber = 0;

    if (!deviceToIndicesMap.contains(deviceName)) {
        deviceToIndicesMap[deviceName] = { 0 };
    }
    else {
        deviceNumber = deviceToIndicesMap[deviceName].back() + 1;
        deviceToIndicesMap[deviceName].append(deviceNumber);
    }

    QPair<QString, int> id = QPair { deviceName, deviceNumber };
    handleToIdMap.insert(port.port, id);

    libremidi::midi_in* midiIn = createMidiIn(id);
    idToMidiInMap.insert(id, midiIn);

    idToInputPortMap.insert(id, port);

    emit inputPortAdded(id);

    midiIn->open_port(port);

    emit inputPortOpen(id, midiIn->is_port_open());
}



void MidiControl::removeInputPort(const libremidi::input_port& port)
{
    auto handle = port.port;
    auto devId = handleToIdMap[handle];

    handleToIdMap.remove(handle);

    // Close port and delete midi input

    auto midiIn = idToMidiInMap.take(devId);
    midiIn->close_port();
    emit inputPortOpen(devId, midiIn->is_port_open());
    delete midiIn;

    idToInputPortMap.remove(devId);

    emit inputPortRemoved(devId);

    // Remap

    for (int index = deviceToIndicesMap[devId.first].back(); index > devId.second; index--)
    {
        QPair<QString, int> oldId = QPair { devId.first, index };
        QPair<QString, int> newId = QPair { devId.first, index - 1 };

        recreateMidiIn(oldId, newId);

        handleToIdMap[handleToIdMap.key(oldId)] = newId;

        deviceToIndicesMap[devId.first][index]--;
    }

    deviceToIndicesMap[devId.first].removeOne(devId.second);
}



void MidiControl::recreateMidiIn(QPair<QString, int> oldId, QPair<QString, int> newId)
{
    auto oldMidiIn = idToMidiInMap.take(oldId);
    oldMidiIn->close_port();
    delete oldMidiIn;

    emit inputPortRemoved(oldId);

    libremidi::midi_in* midiIn = createMidiIn(newId);
    idToMidiInMap.insert(newId, midiIn);

    emit inputPortAdded(newId);

    auto port = idToInputPortMap.value(oldId);
    midiIn->open_port(port);

    idToInputPortMap.insert(newId, port);
    idToInputPortMap.remove(oldId);

    emit inputPortIdChanged(oldId, newId);

    emit inputPortOpen(newId, midiIn->is_port_open());
}
