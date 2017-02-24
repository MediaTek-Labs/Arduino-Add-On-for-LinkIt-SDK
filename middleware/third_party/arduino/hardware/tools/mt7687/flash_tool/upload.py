#!/usr/bin/env python2.7

from optparse import OptionParser
import serial
import xmodem
import os, sys, time
import logging
import pyprind
import platform

da87_path = './da87.bin'
da97_path = './da97.bin'

logging.basicConfig()
parser = OptionParser(usage="python %prog [options]")
parser.add_option("-c", dest="com_port", help="COM port, can be COM1, COM2, ..., COMx")
parser.add_option("-d", action="store_true", dest="debug")
parser.add_option("-f", dest="bin_path", help="path of the bin file to be uploaded")
parser.add_option("-n", dest="da_file", help="path of the DA file to be used")
parser.add_option("-p", dest="platform", help="patform to be flashed (mt7687 | mt7697)", default='mt7697')
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
    print >> sys.stderr, "\nError: Bin file [ %s ] not found !!!\n" % (opt.bin_path)
    sys.exit(-1)

if not os.path.exists(da_path):
    print >> sys.stderr, "\nError: DA file [ %s ] not found !!!\n" % (da_path)
    sys.exit(-1)

s = serial.Serial()

def resetIntoBootloader():

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

    #init Com port to orginal state
    s.setRTS(False)
    s.setDTR(False)
    time.sleep(0.1)
    s.flushInput()

    pass

#print >> sys.stderr, "Please push the Reset button"

error_count = 0
c_count = 0
retry = 0
start_time = time.time()
resetIntoBootloader()
while 1:
    c = s.read()
    s.flushInput()

    if debug:
        print >> sys.stderr, "Read: "+c.encode('hex')
        pass
    if c == "C":
        c_count  = c_count +1
    if c!=0 and c!="C":
        error_count = error_count +1
    if c_count>1:
        print >> sys.stderr, "Start uploading the download agent"
        break
        pass
    if error_count>3:
        print "Error - Not reading the start flag"
        retry  = retry +1
        error_count = 0
        c_count = 0
        start_time = time.time()
        s.close()
        time.sleep(0.3)
        resetIntoBootloader()
    if time.time() - start_time > 3.0:
        print "Error - Timeout"
        retry  = retry +1
        error_count = 0
        c_count = 0
        start_time = time.time()
        s.close()
        resetIntoBootloader()
        pass
    if retry>3:
        print "Exiting"
        exit()
        pass

statinfo = os.stat(da_path)
bar = pyprind.ProgBar(statinfo.st_size/1024+2)

def getc(size, timeout=1):
    return s.read(size)

def putc(data, timeout=1):
    bar.update()
    return s.write(data)

def putc_user(data, timeout=1):
    bar_user.update()
    return s.write(data)

def pgupdate(read, total):
    print "\r%d/%d bytes (%2.f%%) ..." % (read, total, read*100/total)

m = xmodem.XMODEM(getc, putc, mode='xmodem1k')

stream = open(da_path, 'rb')
m.send(stream)
s.baudrate = 115200*2

print >> sys.stderr, "DA uploaded, start uploading the user bin"
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

flag = 1
while flag:
    c = s.read()
    if c =='C':
        flag = 0
        pass
    pass
s.flush()
s.flushInput()

statinfo_bin = os.stat(opt.bin_path)
bar_user = pyprind.ProgBar(statinfo_bin.st_size/1024+2)
stream = open(opt.bin_path, 'rb')
m = xmodem.XMODEM(getc, putc_user, mode='xmodem1k')
m.send(stream)

print >> sys.stderr, "\nBin file uploaded. The board reboots now."
time.sleep(1)
s.write("C\r")
s.flush()
s.flushInput()

#Resetx
s.setRTS(True)
s.setDTR(False)

#init Com port to orginal state
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
