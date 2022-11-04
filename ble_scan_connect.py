# ble_scan_connect.py:
import time
from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner, DefaultDelegate
import sys

MAC_ADDRESS = sys.argv[1] if len(sys.argv) > 1 else "fc:95:65:65:8a:7c"

def uint8s_to_int(data):
    num = data[0] << 8 | data[1]
    if num & (1 << 15):
        num -= 1 << 16
    return num

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)
    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
                print ("Discovered device", dev.addr)
        elif isNewData:
            print ("Received new data from", dev.addr)

def enable_notify(ch):
    setup_data = b"\x01\x00"
    notify_handle = ch.getHandle() + 1
    res = dev.writeCharacteristic(notify_handle, setup_data, withResponse=True)
    print(res)

print ("Connecting...")
dev = Peripheral(MAC_ADDRESS, 'random')
print ("Services...")
for svc in dev.services:
    print (str(svc))
try:
    testService = dev.getServiceByUUID(UUID(0xA000))
    mag = dev.getCharacteristics(uuid=UUID(0xA001))[0]

    heartRateService = dev.getServiceByUUID(UUID(0xA002))
    hr = dev.getCharacteristics(uuid=UUID(0xA003))[0]

    if ("NOTIFY" in mag.propertiesToString()):
        print("start notifying")
        enable_notify(mag)
        enable_notify(hr)
        while True:
            if dev.waitForNotifications(1.0):
                mag_data = mag.read()
                mag_xyz = [uint8s_to_int(mag_data[i:i+2]) for i in range(0, len(mag_data), 2)]
                print(f"x: {mag_xyz[0]:>4}, y: {mag_xyz[1]:>4}, z: {mag_xyz[2]:>4}")
                hr_data = hr.read()
                print(f"heart: {ord(hr_data)}")
finally:
    dev.disconnect()