#include "midiinputport.h"

MidiInputPort::MidiInputPort(libremidi::input_port port, int set) :
    mSet { set },
    mInPort { port }
{}



int MidiInputPort::set()
{
    return mSet;
}



void MidiInputPort::setSet(int set)
{
    mSet = set;
}



QString MidiInputPort::portName()
{
    return QString::fromStdString(mInPort.port_name);
}
