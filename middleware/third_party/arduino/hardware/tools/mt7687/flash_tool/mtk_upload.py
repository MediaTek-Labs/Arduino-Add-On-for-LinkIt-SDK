#!/usr/bin/python3
import os
import sys
import time
import serial
import xmodem
import logging
import pyprind
import platform
import tempfile

from optparse import OptionParser

class pySerialDev(serial.Serial):
	def __init__(self, *args, **kwargs):
		serial.Serial.__init__(self, *args, **kwargs)
		self.__putc_hook = None

	def setup_putc_hook(self, putc_hook):
		self.__putc_hook = putc_hook

	def getc(self, size, timeout=1):
		# FIXME:
		# If reconfigure the serial on Windows, it will cause modem.send fail.
		#if self.timeout != timeout:
		#	self.timeout = timeout

		return self.read(size)

	def putc(self, data, timeout=1):
		# FIXME:
		# If reconfigure the serial on Windows, it will cause modem.send fail.
		#if self.write_timeout != timeout:
		#	self.write_timeout = timeout

		if self.__putc_hook != None:
			try:
				self.__putc_hook()
			except Exception as err:
				#logging.error()
				pass

		return self.write(data)

class mt7687_dev:
	def __init__(self, serial_dev, quite=None):
		if not serial_dev.readable():
			logging.critical("'%s' couldn't be read."%s_dev.name)
			raise IOError("'%s' couldn't be read."%s_dev.name)

		if not serial_dev.writable():
			logging.critical("'%s' couldn't be write."%s_dev.name)
			raise IOError("'%s' couldn't be write."%s_dev.name)

		self.__s_dev = serial_dev
		self.__modem = xmodem.XMODEM(serial_dev.getc, serial_dev.putc)

		if quite == True:
			self.__modem.log.setLevel(logging.CRITICAL)
		elif quite == False:
			self.__modem.log.setLevel(logging.INFO)
		else:
			self.__modem.log.setLevel(logging.ERROR)

	def wait_for_start(self, count=3, retries=0):
		if not isinstance(retries, int):
			raise TypeError("'retries' shoud be int.")

		if retries <= 0:
			retries = None

		cnt = count

		# If the device is in the waiting of transmitting, it will output 'C' every second.
		# So, If we want get count 'C', it maybe need count seconds at least.
		self.__s_dev.timeout = cnt * 2

		if retries == None:
			while True:
				# Clear the current receive buffer.
				self.__s_dev.reset_input_buffer()

				logging.debug("In the waiting until get 'C'.")
				data = self.__s_dev.read(cnt)
				logging.debug("Get cnt %s."%data)

				if data == cnt*b"C":
					return True
		else:
			while retries > 0:
				# Clear the current receive buffer.
				self.__s_dev.reset_input_buffer()

				logging.debug("In the waiting until get 'C'.")
				data = self.__s_dev.read(cnt)
				logging.debug("Get cnt %s."%data)

				if data == cnt*b"C":
					return True
				else:
					retries -= 1
			else:
				return False

	def send_file(self, file_path, quite=None):
		if not os.path.isfile(file_path):
			logging.critical("'%s' isn't a file."%file_path)
			raise ValueError("'%s' doesn't exist."%file_path)

		stream = open(file_path, 'rb')

		if quite:
			self.__s_dev.setup_putc_hook(None)
		else:
			logging.debug("Setup the progress bar.")
			statinfo = os.stat(file_path)
			bar = pyprind.ProgBar(statinfo.st_size/128+1, stream=sys.stdout)

			self.__s_dev.setup_putc_hook(bar.update)

		logging.debug("Send file: start modem send.")
		return self.__modem.send(stream)

	def check_and_send_file(self, file_path, quite=None, check_retries=0):
		if not os.path.isfile(file_path):
			logging.critical("'%s' isn't a file."%file_path)
			raise ValueError("'%s' doesn't exist."%file_path)

		stream = open(file_path, 'rb')

		if quite:
			self.__s_dev.setup_putc_hook(None)
		else:
			logging.debug("Setup the progress bar.")
			statinfo = os.stat(file_path)
			bar = pyprind.ProgBar(statinfo.st_size/128+1, stream=sys.stdout)

			self.__s_dev.setup_putc_hook(bar.update)

		logging.debug("Check the status: waiting for 'C'.")
		if not self.wait_for_start(retries=check_retries):
			logging.debug("Current status doesn't allow to send file.")
			return False

		logging.debug("Send file: start modem send.")
		return self.__modem.send(stream)

	def upload_bootstrap(self, bootstrap_path, timeout=None, quite=None):
		if (platform.system() == 'Windows'):
			try:
				self.__s_dev.sendBreak()
				if self.check_and_send_file(bootstrap_path, quite, check_retries=0):
					return True
			except Exception as err:
				logging.debug("Fail to execute sendBreak.")

		if quite == True:
			return False

		logging.info("Please push the Reset button")
		return self.check_and_send_file(bootstrap_path, quite, 0)

		# The following flow also did well.
		# logging.debug("Check the status: waiting for 'C'.")
		# self.wait_for_start(count=5, retries=0)
		# self.__s_dev.reset_input_buffer()
		# return self.send_file(bootstrap_path, quite)

	def __check_update_condition(self):
		def get_counter():
			try:
				counter = self.__s_dev.read_until(terminator=b'\r')
				logging.debug("Get byte: %s."%counter)

				counter = counter.decode('ascii').strip()
				logging.debug("Get string: %s."%counter)

				return int(counter)
			except:
				return None

		time.sleep(2)

		self.__s_dev.reset_input_buffer()
		self.__s_dev.timeout = 1

		start_num = None

		while start_num == None:
			start_num = get_counter()

		# Actualy, the start_num is in (60, 0]
		if (start_num <= 0) and (start_num >= 120):
			logging.debug("Menu: No!")
			return False

		max_cnt = 2
		while max_cnt:
			num = get_counter()
			if num == None:
				continue

			if (num >= start_num) or (num <= start_num-2):
				logging.debug("Menu: No!")
				return False

			start_num = num
			max_cnt   = max_cnt - 1

		logging.debug("Menu: yes!")
		return True

	def update_bootloader(self, bootloader_path, quite=None):
		logging.debug("Action: update bootloader.")
		if not self.__check_update_condition():
			return False

		logging.debug("Menu: choose 1")
		self.__s_dev.write(b"1")
		time.sleep(2)

		return self.check_and_send_file(bootloader_path, quite, check_retries=1)

	def update_xip(self, xip_path, quite=None):
		logging.debug("Action: update application.")
		if not self.__check_update_condition():
			return False

		logging.debug("Menu: choose 2")
		self.__s_dev.write(b"2")
		time.sleep(2)

		return self.check_and_send_file(xip_path, quite, check_retries=1)

	def update_fw(self, wifi_fw_path, quite):
		logging.debug("Action: update wifi firmware.")
		if not self.__check_update_condition():
			return False

		logging.debug("Menu: choose 3")
		self.__s_dev.write(b"3")
		time.sleep(2)

		return self.check_and_send_file(wifi_fw_path, quite, check_retries=1)

	def start_run(self):
		logging.debug("Action: start run")
		if not self.__check_update_condition():
			return False

		logging.debug("Menu: choose c")
		self.__s_dev.write(b"c")

def main():
	cur_dir = os.path.dirname(sys.argv[0])
	cur_dir = os.path.abspath(cur_dir)

	prog_name = os.path.basename(sys.argv[0])
	prog_name = os.path.splitext(prog_name)[0]

	parser = OptionParser(usage="""
Name:
       %prog - Update the Firmware of MT76x7

Synopsis:
       %prog  option ... [ACTION OPTION]... [ARGUMENT]...

Description:
       When MT76x7 HDK connect the PC host with USB, there is a serial interface in the USB device function.
       Based on the serial, this tool has been implemented to update the bootloader/Application Firmware/WiFi Firmware.

       There are four actions provided by this tool:
       1. Update the bootloader
       2. Update the Application Firmware
       3. Update the WiFi Firmware
       4. Set MCU run after update""")
	if (platform.system() == 'Windows'):
		serial_usage = "COM port, can be COM1, COM2, ..., COMx"
	else:
		serial_usage = "Serial Devices, /dev/ttyACMx,  /dev/tty.XXXX"
	parser.add_option("-s",	'--serial',     dest="serial_dev",	help=serial_usage)
	parser.add_option("-B", '--baud',       dest="baud_rate",       help="Baud rate, can be 9600, 115200, etc...", type="int", default=115200)
	parser.add_option("-x", '--xip',        dest="xip_path",	help="Update the application with a specified binary.")
	parser.add_option("-b", '--bootloader', dest="bootloader_path",	help="Update the bootloader with a specified binary.")
	parser.add_option("-w",	'--wififw',     dest="wifi_fw_path",	help="Update the WiFi Firmware with a specified binary.")
	parser.add_option("-g",	'--logfile',    dest="logfile",         help="Dump log the specified file.")
	parser.add_option("-V",	'--verbose',    dest="quite",	        help="Show some status and some hint/extra message.", action='store_false', default=None)
	parser.add_option("-Q",	'--quite',      dest="quite",	        help="Don't show any status and some hint/extra message.", action='store_true', default=None)
	parser.add_option("-v",	'--version',    dest="show_version",    help="Show the version of this tool.", action='store_true', default=False)

	(opt, args) = parser.parse_args()

	if opt.show_version:
		print("Version: 0.9.0")
		sys.exit(0)

	if not opt.serial_dev:
		print("The Serial Device should be specified.")
		parser.print_help()
		sys.exit(1)

	if (not opt.xip_path) and (not opt.bootloader_path) and (not opt.wifi_fw_path):
		print("A least, a update action should be specified.")
		parser.print_help()
		sys.exit(2)

	if opt.logfile:
		logfile = opt.logfile
	else:
		logfile = os.path.join(tempfile.gettempdir(), prog_name+".log")

	# set up logging to file
	logging.basicConfig(level=logging.DEBUG,
			format='%(asctime)s: %(levelname)-8s %(message)s',
			datefmt='%m-%d %H:%M',
			filename=logfile,
			filemode='w')

	# define a Handler which writes INFO messages or higher to the sys.stderr
	console = logging.StreamHandler()
	console.setLevel(logging.INFO)
	# set a format which is simpler for console use
	formatter = logging.Formatter('%(message)s')
	# tell the handler to use this format
	console.setFormatter(formatter)
	# add the handler to the root logger
	logging.getLogger('').addHandler(console)

	try:
		serial_dev = pySerialDev(opt.serial_dev, opt.baud_rate, 8, 'N', 1)
		serial_dev.rts = False
		serial_dev.dtr = False
		# NOTE: It will cause fail that operate 'break' on my Linux host.
		# serial_dev.break_condition
	except Exception as err:
		print("Fail to open '%s' with baud rate '%s'."%(opt.serial_dev, opt.baud_rate))
		print(err)
		sys.exit(3)

	try:
		mt7687 = mt7687_dev(serial_dev, opt.quite)
	except Exception as err:
		print("ERROR: '%s'."%err)
		sys.exit(4)

	mt7687.upload_bootstrap(cur_dir+'/bootstrap.bin', quite=opt.quite)

	if (opt.bootloader_path):
		mt7687.update_bootloader(opt.bootloader_path, quite=opt.quite)

	if (opt.xip_path):
		mt7687.update_xip(opt.xip_path, quite=opt.quite)

	if (opt.wifi_fw_path):
		mt7687.update_fw(opt.wifi_fw_path, quite=opt.quite)

	mt7687.start_run()

	sys.stdout.flush()

	if opt.quite != True:
		time.sleep(0.1)
		logging.info("Successful.")

if __name__ == "__main__":
	main()
	sys.exit(0)
