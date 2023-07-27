import os
import can
from pydbus import SessionBus
from gi.repository import GLib

class MyService:
    '''
    DBus Service Example
    '''
    dbus = """
    <node>
        <interface name='com.example12.MyService'>
            <method name='hello'>
                <arg type='s' name='name' direction='in'/>
                <arg type='s' name='message' direction='out'/>
            </method>
        </interface>
    </node>
    """

    def __init__(self):
        os.system('sudo ifconfig can1 down')
        os.system('sudo ip link set can1 up type can bitrate 500000 dbitrate 8000000 restart-ms 1000 berr-reporting on fd on')
        self.can1 = can.interface.Bus(channel = 'can1', bustype = 'socketcan')
        self.count=0

    def hello(self, name: str) -> str:
        '''returns a greeting'''
        self.count += 1
        msg = self.can1.recv(10.0);
        rpm = 0
        distance = 0
        if msg is None:
            return "No message recieved"
        else:
            rpm=msg.data[0] + msg.data[1]*256
            distance=msg.data[2] + msg.data[3]*256
        return f"Hello, {name}! rpm:{rpm}, dis:{distance}, count:{self.count}"

    def getMSG(self) -> str:
        msg = self.can1.recv(10.0);
        return msg

bus = SessionBus()
bus.publish("com.example12.MyService", MyService())

loop = GLib.MainLoop()
loop.run()
