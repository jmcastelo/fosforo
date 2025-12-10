#ifndef MIDIINPUTPORT_H
#define MIDIINPUTPORT_H



#include <libremidi/libremidi.hpp>

#include <QString>



class MidiInputPort
{
public:
    MidiInputPort(libremidi::input_port port, int set);

    int set();
    void setSet(int set);
    QString portName();

private:
    int mSet = 0;
    libremidi::input_port mInPort;
};



#endif // MIDIINPUTPORT_H
