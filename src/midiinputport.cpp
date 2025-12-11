#include "midiinputport.h"

MidiInputPort::MidiInputPort(libremidi::input_port port, int map) :
    mMap { map },
    mInPort { port }
{}



int MidiInputPort::map()
{
    return mMap;
}



void MidiInputPort::setMap(int map)
{
    mMap = map;
}



QString MidiInputPort::portName()
{
    return QString::fromStdString(mInPort.display_name);
}
