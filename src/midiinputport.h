#ifndef MIDIINPUTPORT_H
#define MIDIINPUTPORT_H



#include <libremidi/libremidi.hpp>

#include <QString>



class MidiInputPort
{
public:
    MidiInputPort(libremidi::input_port port, int map);

    int map();
    void setMap(int map);
    QString portName();

private:
    int mMap = 0;
    libremidi::input_port mInPort;
};



#endif // MIDIINPUTPORT_H
