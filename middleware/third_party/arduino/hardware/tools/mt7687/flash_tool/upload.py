#!/usr/bin/env python2.7

import os
import sys
import imp
from optparse import OptionParser
import serial
import xmodem
import time
import logging
import pyprind
import platform
import struct


def main_is_frozen():
   return (hasattr(sys, "frozen") or # new py2exe
           hasattr(sys, "importers") # old py2exe
           or imp.is_frozen("__main__")) # tools/freeze


def get_main_dir():
   if main_is_frozen():
       return os.path.dirname(sys.executable)
   return os.path.dirname(os.path.realpath(__file__))

da87_path = './da87.bin'
da97_path = './da97.bin'
n9_fw_path = os.path.join(get_main_dir(), "WIFI_RAM_CODE_MT76X7_in_flash.bin")


def get_tool_n9_firmware_version(name):
    with open(name, "rb") as f:
        header = struct.Struct("<I")
        data_size = header.unpack(f.read(4))[0]

        # The version info is stored at the end of the data
        # 72 is the size of fw_image_tailer_t in connsys_util.h
        # 16 is the offset to ram_built_date field.
        # 1 is the offset to the start of the version (builtdate) info.
        f.seek(data_size - 72 + 16 + 1)
        fw_version = f.read(14)
        return fw_version.rstrip()
    return ""

logging.basicConfig()
parser = OptionParser(usage="python %prog [options]")
parser.add_option("-c", dest="com_port", help="COM port, can be COM1, COM2, ..., COMx")
parser.add_option("-d", action="store_true", dest="debug")
parser.add_option("-f", dest="bin_path", help="path of the bin file to be uploaded")
parser.add_option("-n", dest="da_file", help="path of the DA file to be used")
parser.add_option("-p", dest="platform", help="platform to be flashed (mt7687 | mt7697)", default='mt7697')
parser.add_option("-t", dest="target", help="target to be flashed (cm4 | ldr | n9).", default='cm4')
(opt, args) = parser.parse_args()

if opt.target != 'cm4' and opt.target != 'n9' and opt.target != 'ldr':
    print >> sys.stderr, "\nError: Invalid parameter!! Please specify the target to be flashed.\n"
    parser.print_help()
    sys.exit(-1)
    pass

if opt.platform != 'mt7687' and opt.platform != 'mt7697':
    print >> sys.stderr, "\nError: Invalid platform is assigned. Only mt7687 and mt7697 are supported.\n"
    parser.print_help()
    sys.exit(-1)
    pass

debug = opt.debug

da_path = opt.da_file

if not opt.da_file:
    if opt.platform == 'mt7697':
        da_path = './da97.bin'
    elif opt.platform == 'mt7687':
        da_path = './da87.bin'
    pass

if not opt.bin_path or not opt.com_port:
    print >> sys.stderr, "\nError: Invalid parameter!! Please specify the COM port and the bin file.\n"
    parser.print_help()
    sys.exit(-1)

if not os.path.exists(opt.bin_path):
    print >> sys.stderr, "\nError: Bin file [ %s ] not found !!!\n" % opt.bin_path
    sys.exit(-1)

if not os.path.exists(da_path):
    print >> sys.stderr, "\nError: DA file [ %s ] not found !!!\n" % da_path
    sys.exit(-1)

s = serial.Serial()


def reset_to_bootloader():

    s.baudrate = 115200
    s.port = opt.com_port
    s.timeout = 1
    s.open()

    s.setRTS(True)
    s.setDTR(False)
    time.sleep(0.1)

    s.setDTR(True)
    s.setRTS(False)
    time.sleep(0.1)

    # init Com port to original state
    s.setRTS(False)
    s.setDTR(False)
    time.sleep(0.1)
    s.flushInput()

    pass

# print >> sys.stderr, "Please push the Reset button"

error_count = 0
c_count = 0
retry = 0
start_time = time.time()
reset_to_bootloader()
while 1:
    c = s.read()
    s.flushInput()

    if debug:
        print >> sys.stderr, "Read: "+c.encode('hex')
        pass
    if c == "C":
        c_count = c_count + 1
    if c != 0 and c != "C":
        error_count = error_count + 1
    if c_count > 1:
        print >> sys.stderr, "Start uploading the download agent"
        break
        pass
    if error_count > 3:
        print "Error - Not reading the start flag"
        retry = retry + 1
        error_count = 0
        c_count = 0
        start_time = time.time()
        s.close()
        time.sleep(0.3)
        reset_to_bootloader()
    if time.time() - start_time > 3.0:
        print "Error - Timeout"
        retry = retry + 1
        error_count = 0
        c_count = 0
        start_time = time.time()
        s.close()
        reset_to_bootloader()
        pass
    if retry > 3:
        print "Exiting"
        sys.exit(-1)
        pass

statinfo = os.stat(da_path)
# bar = pyprind.ProgBar(statinfo.st_size/1024+2, stream=sys.stderr)


# noinspection PyUnusedLocal
def getc(size, timeout=1):
    return s.read(size)


# noinspection PyUnusedLocal
def putc(data, timeout=1):
    # bar.update()
    return s.write(data)


# noinspection PyUnusedLocal
def putc_user(data, timeout=1):
    # bar_user.update()
    return s.write(data)


def pgupdate(read, total):
    print "\r%d/%d bytes (%2.f%%) ..." % (read, total, read*100/total)

m = xmodem.XMODEM(getc, putc, mode='xmodem1k')

stream = open(da_path, 'rb')
m.send(stream)
s.baudrate = 115200*8

# Parse DA output to get on-board N9 firmware version string.
# Note that the DA behavior is now coupled with this Python script.
print >> sys.stderr, "\nWaiting for DA output..."
line = s.readline()
while not line.startswith("@FWVER="):
    line = s.readline()
onboard_fw_version = line.lstrip("@FWVER=").rstrip()

print >> sys.stderr, "DA uploaded, start uploading user binary..."
time.sleep(1)
if opt.target == 'ldr':
    s.write("1\r")
    pass
if opt.target == 'n9':
    s.write("3\r")
    pass
if opt.target == 'cm4':
    s.write("2\r")
    pass
s.flush()
s.flushInput()

flag = 3
while flag > 0:
    c = s.read()
    # wait for 5 consecutive "C"
    if c == 'C':
        flag -= 1
        pass
    pass
s.flush()
s.flushInput()

statinfo_bin = os.stat(opt.bin_path)
# bar_user = pyprind.ProgBar(statinfo_bin.st_size/1024+2, stream=sys.stderr)
stream = open(opt.bin_path, 'rb')
m = xmodem.XMODEM(getc, putc_user, mode='xmodem1k')
m.send(stream)
m = None
stream = None

# check if we need to update firmware
current_fw_version = get_tool_n9_firmware_version(n9_fw_path)
if onboard_fw_version != current_fw_version:
    print >> sys.stderr, \
        "\nOnboard Wi-Fi firmware version(%s) mismatch current version(%s). Update Wi-Fi firmware." % \
        (onboard_fw_version, current_fw_version)
    time.sleep(3)
    s.flush()
    s.flushInput()
    s.write("3\r")
    time.sleep(3)
    s.flush()
    s.flushInput()

    statinfo_bin = os.stat(n9_fw_path)
    # bar_user = pyprind.ProgBar(statinfo_bin.st_size / 1024 + 2, stream=sys.stderr)
    fw_stream = open(n9_fw_path, 'rb')
    m = xmodem.XMODEM(getc, putc_user, mode='xmodem1k')
    m.send(fw_stream)
    m = None
    fw_stream = None
else:
    print >> sys.stderr, "\nWi-Fi Firmware version check ok."

print >> sys.stderr, "\nThe board reboots now."
time.sleep(1)
s.write("C\r")
s.flush()
s.flushInput()

# Reset Board
s.setRTS(True)
s.setDTR(False)

# Init Com port to original state
#   Workaround for COM port behavior on different platforms:
#
#   On Windows, fail to reset s.RTS causes RST pin to be pull-down
#   (and fail to re-boot the board) until COM port is re-opened again.
#
#   On macOS, the COM port cannot be re-opened again
#   if we change RTS/DTS state before closing it.
if 'Windows' == platform.system():
    time.sleep(0.1)
    s.setRTS(False)
    s.setDTR(False)
    time.sleep(0.1)
else:
    time.sleep(0.2)

s.close()
