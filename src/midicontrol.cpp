#include "midicontrol.h"



MidiControl::MidiControl(QObject *parent) : QObject(parent)
{}



MidiControl::~MidiControl()
{
    foreach (auto midiIn, mInPortToMidiInMap) {
        delete midiIn;
    }
    foreach (auto inPort, mInPortsMap) {
        delete inPort;
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



libremidi::midi_in* MidiControl::createMidiIn(MidiInputPort* inPort)
{
    libremidi::input_configuration config {
        .on_message = [=, this](const libremidi::message& message) {
            if (message.get_message_type() == libremidi::message_type::CONTROL_CHANGE)
            {
                // MIDI CC message bytes:
                // message[0] = channel, ranges from 176 (channel 1) to 191 (channel 16)
                // message[1] = controller, ranges from 0 to 127 -> knob or fader
                // message[2] = value, ranges from 0 to 127

                int key = (message[0] - 176) * 128 + message[1];

                emit ccInputMessageReceived(inPort->map(), key, message[2]);
            }
        }
    };

    libremidi::midi_in* midiIn = new libremidi::midi_in { std::move(config) };

    return midiIn;
}


void MidiControl::addInputPort(const libremidi::input_port& port)
{
    // Create midi input and open port

    int map = mInPortsMap.size();
    MidiInputPort* inPort = new MidiInputPort(port, map);
    mInPortsMap.insert(port, inPort);

    libremidi::midi_in* midiIn = createMidiIn(inPort);
    mInPortToMidiInMap.insert(port, midiIn);

    emit inputPortAdded(inPort);

    midiIn->open_port(port);

    emit midiEnabled(anyPortOpen());
}



void MidiControl::removeInputPort(const libremidi::input_port& port)
{
    // Close port and delete midi input

    auto midiIn = mInPortToMidiInMap.take(port);
    midiIn->close_port();
    delete midiIn;

    auto inPort = mInPortsMap.take(port);
    emit inputPortRemoved(inPort);
    delete inPort;

    emit midiEnabled(anyPortOpen());
}



bool MidiControl::anyPortOpen()
{
    bool anyOpen = false;
    for (auto [port, midiIn] : mInPortToMidiInMap.asKeyValueRange()) {
        anyOpen |= midiIn->is_port_open();
    }
    return anyOpen;
}
