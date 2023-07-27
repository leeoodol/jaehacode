from pydbus import SessionBus
import time

bus = SessionBus()
my_service = bus.get("com.example12.MyService", "/com/example12/MyService")

while (True):
    print(my_service.hello("Raspberry pi"))
    #print(my_service.getMSG())
    time.sleep(0.1)
