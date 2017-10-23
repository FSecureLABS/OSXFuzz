
# -*- coding: utf-8 -*-

""" 

This module is used to perform VM Monitoring on the host. 

The PanicMonitor class is used to determine of a guest VM has kernel panic'd.

The VMManager class is used to perform virtual machine automation tasks such as deployment, reverting state etc. 

The VMMonitor class exposes this via HTTP REST methods. 

<alex.plaskett@mwrinfosecurity.com> 

"""

import time
import os
import uuid
import glob
import shutil
import urllib2
import json
import re
import subprocess
import thread
import sys
import stat
import argparse

sys.path.append("../reproducer/")
from repro import Reproducer

from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer

from threading import Thread
from threading import Event

from receive_logs import * 

class PanicMonitor(Thread):

	"""

	Panic Monitoring Class

	This class is used to monitor the panic directory on the host and determine
	if any panic have occured. If a panic occurs, the crash data, testcase and
	other reproduction data is saved. 

	"""

	def __init__(self, config_name,interval=1):
		super(PanicMonitor, self).__init__()
		self.interval = interval
		self.config_name = config_name
		self.init_config()

		self.check_panic_path_permissions()
		self.do_panic_logs_already_exist()

		HOST, PORT = self.logger_addr, self.logger_port
		# The panic monitor runs the network logger instance. 
		self.network_logger = ThreadedTCPServer((HOST, PORT), ThreadedTCPRequestHandler)
		server_thread = threading.Thread(target=self.network_logger.serve_forever)
		server_thread.daemon = True
		server_thread.start()
		#server_thread.join()

	def init_config(self):
		""" Read the panic related JSON settings """
		with open(self.config_name) as data_file:
			data = json.load(data_file)
		self.logger_addr = data['host_settings']['logger_addr']
		self.logger_port = int(data['host_settings']['logger_port'])
		self.crash_path = data['host_settings']['crash_path']
		self.panic_path = data['host_settings']['panic_path']
		self.vmmanager_port = int(data['host_settings']['vmmanager_port'])	
		self.bughunt_start_src = data['host_settings']['bughunt_start_src']
		self.bughunt_binary_src = data['host_settings']['bughunt_binary_src']

	def check_panic_path_permissions(self):
		""" SIP resets the permissions on this directory """
		mode = "777"
		filemode = stat.S_IMODE(os.stat(self.panic_path).st_mode)
		if (int(mode,8) != 511):
			print("++ ERR: Please make sure permission mode (777) is set on panic dir correctly!! ++")
			sys.exit(1)
		print("++ Panic path permission check passed ++")

	def do_panic_logs_already_exist(self):
		files = os.listdir(self.panic_path)
		if (len(files) != 0):
			print("++ ERR: Panic logs already exist, you should clear them out first before the fuzzer run ++")
			sys.exit(1)
		print("++ Clean panic logs directory ++")

	def revert_vm(self,ip):
		""" Query the API to get the UUID to revert""" 
		data = {
			"action" : "revert_vm_ip", 
			"uuid" : "None",
			"ip_addr" : ip
		}
		# VMManager always runs on localhost. 
		vmmanager_url = "http://127.0.0.1:" + str(self.vmmanager_port)
		req = urllib2.Request(vmmanager_url)
		req.add_header('Content-Type','application/json')
		response = urllib2.urlopen(req,json.dumps(data))	

	def get_network_logger_data(self,ip_addr):
		""" Get the network data from the logger """ 
		log_lines = self.network_logger.get_logdata(ip_addr)
		return log_lines

	def create_repro(self,path):
		base_dir = os.path.dirname(path)
		dest_path = os.path.join(base_dir,"repro.c")
		print("++ Input Path: %s " % path)
		print("++ Output Path: %s " % dest_path)
		repro = Reproducer(path,dest_path)
		repro.create_repro()
		repro.create_makefile()

	def panic_has_occured(self,fn):
		""" Called when a panic occurs """
		print("++ A panic has occured! ++", fn) 

		# Extract the IP address from the panic log.
		found = re.findall(r'(?:[\d]{1,3})\.(?:[\d]{1,3})\.(?:[\d]{1,3})\.(?:[\d]{1,3})',fn)
		if found:
			panic_client = found[len(found)-1] 	# The host which crashed
			print(panic_client)			

		# Save the panic log to the file system. 
		print("++ Backing up panic log ++") 
		x = str(uuid.uuid4())

		crash_path = os.path.join(self.crash_path,x)
		panic_path = os.path.join(self.panic_path,fn)

		os.makedirs(crash_path)

		try:
			shutil.move(panic_path,crash_path)
		except:
			print("++ ERR: Could not move panic log - This is fatal!")
			print("++ Check permissions on the panic directory")
			sys.exit(1)

		# Look up associated log file from the network logger
		print("++ Getting associated fuzzer log file ++")
		lines = self.get_network_logger_data(found[len(found)-1])
		
		# If we have found log data
		if lines:
			print(len(lines))

			# Save the fuzzer logfile within the crash dir. 
			repro_path = os.path.join(crash_path,x + ".c")
			fd = open(repro_path,"w")

			# Format the log file properly. 
			for l in lines:
				fd.write(l + "\n")
			fd.close()

			# Try create a repro file.
			self.create_repro(repro_path) 

		# Try to backup the binary and python script used, incase we cannot repro from log file only. 
		try:
			shutil.copy(self.bughunt_start_src,crash_path)
		except:
			print("++ Could not copy bughunt python script to crash dir ++")
		try:
			shutil.copy(self.bughunt_binary_src,crash_path)
		except:
			print("++ Could not copy bughunt binary to crash dir ++")
			
		# TODO: Send it off to database 

		# Revert the state back to the snapshot. 
		self.revert_vm(panic_client)

		# Start the fuzzer running again. 

	def run(self):
		""" Main monitoring thread body """
		while True:
			time.sleep(self.interval)
			fn = self.is_paniclog_created()
			if (fn):
				time.sleep(4)
				self.panic_has_occured(fn)
				#print(fn)
				
	def is_paniclog_created(self):
		""" Determine if a panic log is created """
		logs = os.listdir(self.panic_path)
		if (len(logs) > 0):
			print("++ Panic log found")
			fn = logs[0]
			return fn
		return None

class VirtualMachine:

	"""

	Virtual Machine Abstraction

	This class represents a Virtual Machine and stores the associated details.

	"""

	def __init__(self,uuid,ip,vmx_path):
		self.uuid = uuid
		self.ip = ip
		self.vmx_path = vmx_path
		self.vm_state = "inactive"

		# Set UUID from VMX file
		self.parse_vmx_file()

	def parse_vmx_file(self):
		""" Parse a VMX file and extract the UUID for the BIOS """
		fd = open(self.vmx_path,"r")
		data = fd.readlines()
		for line in data:
			if "uuid.bios" in line:
				# Convert this into a UUID
				temp = line.split(" = ")[1].lstrip().strip().replace(" ","-").replace("\"","")
				#print(temp)
				uuid_obj = uuid.UUID(temp)
				print(uuid_obj)
				self.uuid = uuid_obj
		fd.close()

class VMManager(HTTPServer):


	"""

	Virtual Machine Manager Server

	This class manages VMWare Fusion virtual machines and performs the VM automation. 

	"""

	def __init__(self,config_name,*kargs):
		HTTPServer.__init__(self,*kargs)
		self.virtual_machines = {}  # UUID -> VM
		self.config_name = config_name
		self.init_config()

		# First bring all the VMs online
		print("++ Booting all the fuzzing VMs ++")

		self.create_vms()

		print("++ All VMs should now be online!")

	def init_config(self):
		""" Read the Virtual Machine related config details """
		with open(self.config_name) as data_file:
			data = json.load(data_file)
		self.vmrun_path  = "'" + data['host_settings']["vmrun_path"] + "'"
		self.bughunt_start_src = data['host_settings']['bughunt_start_src']
		self.launch_path = data['guest_settings']["launch_path"].replace(" ","\\ ")
		self.vm_username = data['guest_settings']['vm_username']
		self.vm_manager_url = "http://" + data['host_settings']['vmmanager_addr'] + ":" + data['host_settings']['vmmanager_port']
		self.fuzz_binary_path = data['host_settings']['bughunt_binary_src']
		self.logger_addr = data['host_settings']['logger_addr']
		self.logger_port = data['host_settings']['logger_port'] 

		# Extract all the VMX paths.
		self.vmx_files = data['host_settings']['vmx_files']

		# Get the name of the prepared snapshot name which we will revert too always. 
		self.snapshot_name = data['host_settings']['prepd_snapshot_name']

		try:
			self.dylib_path = data['guest_settings']['dylib_path']
		except:
			self.dylib_path = None

		self.init_password()

	def create_vms(self):
		""" Creates all the VMs from the VMX paths in the config """

		for vmx_path in self.vmx_files:
			#vm = self.create_vm("/Users/alex/Documents/Virtual Machines.localized/macOS 10.12.vmwarevm/macOS 10.12.vmx")
			vm = self.create_vm(vmx_path)
			self.start_vm(vm)
			self.virtual_machines[str(vm.uuid)] = vm

		time.sleep(5)

	def init_password(self):
		""" Read the VM password from user input at runtime """
		self.vm_password = raw_input("Please enter vm password ")

	def create_vm(self,vmx_path):
		""" Create a virtual machine object """
		print("++ Creating a VM %s ++" % vmx_path) 
		vm = VirtualMachine(None,None,vmx_path)
		return vm

	def start_vm(self,vm):
		""" Start a virtual machine running """
		vmx_path = vm.vmx_path
		cmd = self.vmrun_path + " -T fusion start '" + vmx_path + "' nogui" 
		os.system(cmd)

	def revert(self,vm):
		""" We perform the revert and fuzzer kick off in seporate thread to stop prevent blocking main thread """
		print("++ Setting the VM state to reverting ++")
		vm.state = "reverting"

		print("++ Reverting Virtual Machine thread running ++")
		cmd = self.vmrun_path + " -T fusion revertToSnapshot '" + vm.vmx_path + "' \"" + self.snapshot_name + "\""
		print(cmd)
		os.system(cmd)

		print("++ Starting the VM again ++")
		self.start_vm(vm)

		print("++ Starting the fuzzer running against ++")
		self.run_program_in_vm(vm)

	def revert_vm(self,uuid=None,ip=None):
		""" This is used when a fuzz run finishes or a panic occurs to revert the VM and start it fuzzer running against """
		vm = None 
		if uuid:
			vm = self.vm_from_uuid(uuid)
			if not vm:
				print("++ Could not find VM to revert from UUID ++")
		elif ip:
			vm = self.vm_from_ip(ip)
			if not vm:
				print("++ Could not find VM to revert from IP ++")

		print("++ Current VM state %s " % vm.state)

		if vm:
			if vm.state != "reverting":
				# Run the actual revert and wait in another thread. 
				thread.start_new_thread(self.revert,(vm,))
			else:
				print("++ VM is already reverting skipping ++")

	def restart_vm(self,vm):
		""" This is used to restart a virtual machine """
		print("++ Restarting Virtual Machine ++")
		vmx_path = vm.vmx_path
		cmd = self.vmrun_path + " -T fusion reset '" + vmx_path + "' hard" 
		os.system(cmd)

	def wait_for_boot(self,vm):
		""" Used to determine when a virtual machine has booted """
		while True:
			# Get the process list
			cmd  = self.vmrun_path + " -T fusion -gu " + self.vm_username + " -gp " + self.vm_password + " listProcessesInGuest '" + vm.vmx_path + "' \"Prepd\""
			proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True)
			(out, err) = proc.communicate()
			#print "program output:", out
			if "Process list" in out:
				print("++ Got process list, vm running ++")

				# Set the VM state to be active 
				vm.state = "active"
				#print(out)
				break
			else:
				time.sleep(1)

	def create_bughunt_dir(self,vm):
		pass 

	def copy_fuzzer_to_host(self,vm):
		""" Drop the start_bugunt.py on to the guest VM """
		print("++ Copying start_bughunt.py to the guest VM ++")
		src_file = self.bughunt_start_src
		dst_file = self.launch_path
		cmd = self.vmrun_path + " -T fusion -gu " + self.vm_username + " -gp " + self.vm_password + " copyFileFromHostToGuest '" + vm.vmx_path + "' " + src_file + " " + dst_file 
		print(cmd)
		os.system(cmd)

	def copy_fuzzer_binary_to_host(self,vm):
		""" Drop OSXFuzz binary to the guest VM """
		print("++ Copying OSXFuzz to the guest VM ++")
		src_file = self.fuzz_binary_path
		dst_file = "/tmp/OSXFuzz"
		cmd = self.vmrun_path + " -T fusion -gu " + self.vm_username + " -gp " + self.vm_password + " copyFileFromHostToGuest '" + vm.vmx_path + "' " + src_file + " " + dst_file 
		print(cmd)
		os.system(cmd)	

	def deploy_dylib_interpose_library(self,vm):
		""" Drop the dylib interpose library """
		print("++ Copying dylib interpose library to guest VM++")
		src_file = self.dylib_path
		dst_file = "/tmp/OSXFuzz.dylib"
		cmd = self.vmrun_path + " -T fusion -gu " + self.vm_username + " -gp " + self.vm_password + " copyFileFromHostToGuest '" + vm.vmx_path + "' " + src_file + " " + dst_file 
		print(cmd)
		os.system(cmd)			

	def run_program_in_vm(self,vm):
		""" Run a python script within the VM """

		print("++ Running a program in a VM ++") 
		# We need to wait until the VM is up. 
		self.wait_for_boot(vm)

		# Deploy the fuzzer start script.
		self.copy_fuzzer_to_host(vm)

		# Deploy the binary itself. 
		self.copy_fuzzer_binary_to_host(vm)

		if self.dylib_path:
			self.deploy_dylib_interpose_library(vm)

		script_file = self.launch_path

		# Construct the logger URL. 
		logger_url = self.logger_addr + ":" + self.logger_port

		program = "/usr/bin/python"
		cmd = self.vmrun_path + " -T fusion -gu " + self.vm_username + " -gp " + self.vm_password + " runProgramInGuest '" + vm.vmx_path + "' -nowait '" + program + "' '" + script_file + "' '" + str(vm.uuid) + "' '" + self.vm_manager_url + "' '" + logger_url + "'" 
		print(cmd)
		os.system(cmd)
		print("++ Fuzzer should be running ++")

	def vm_from_uuid(self,uuid):
		""" Get the virtual machine object from UUID """
		try:
			vm = self.virtual_machines[uuid]  
		except:
			vm = None
		return vm 

	def vm_from_ip(self,ip):
		""" Look up a virtual machine from the IP address """
		print("++ Looking up ip addr: ", ip)
		for k, v in self.virtual_machines.items():
			if v.ip == ip:
				return v

	def uuid_from_vm(self,vm):
		""" Get the UUID from the VM """
		return vm.uuid

	def register_vm(self,uuid,ip_addr):
		""" Called when registering a new VM and the client calls back to host """
		print("++ Registering VM ", uuid) 
		print("++ IP addr ", ip_addr)
		vm = self.vm_from_uuid(uuid)
		if not vm:
			print("++ Could not find VM to register ++")
		else:
			print("++ VM found, registering and starting fuzzing ++")
			# Set its IP address
			vm.ip = ip_addr

	def start_fuzz_run(self):
		""" Kick off a fuzz run against multiple VMs """
		print("++ Starting a fuzzing run on %d nodes" % int(len(self.virtual_machines)))
		# First find all the servers which are running. 

		# Now start the fuzzer on all the nodes
		for k,v in self.virtual_machines.items():
			self.run_program_in_vm(v)

	def get_vm_state(self,ip_addr):
		""" Check the VMs current state """
		vm = self.vm_from_ip(ip_addr)
		return vm.state

class VMMonitor(BaseHTTPRequestHandler):

	"""

	Virtual Machine Monitor Handler

	This class is the handler which is called when the VM Manager recieves a GET or POST from the guest VM client. 

	"""

	def __init__(self,*kargs):
		BaseHTTPRequestHandler.__init__(self, *kargs)

	def do_GET(self):
		if "START" in self.path:
			print("++ Starting fuzzing off! ++")
			self.server.start_fuzz_run()
		else:
			print("++ Some other get method")
		self.send_response(200)
		self.send_header("Content-type", "text/html")
		self.end_headers()
		self.wfile.write("<html><head><title>Fuzzing!</title></head>")

	def do_POST(self):
		print("++ Received HTTP POST ++")

		content_len = int(self.headers.getheader('content-length',0))
		post_body = self.rfile.read(content_len)
		data = json.loads(post_body)
		#print(data)
		action = data['action']
		uuid_str = data['uuid']
		ip_addr = data['ip_addr']

		print("++ VM Action " + action)
		print("++ VM UUID " + uuid_str)
		print("++ IP_ADDR " + ip_addr)

		data = "VM Manager Server"

		if action == "revert":
			self.server.revert_vm(uuid_str,None)
		elif action == "register":
			# We should pass config to fuzzer like this. 
			self.server.register_vm(uuid_str,ip_addr)
		elif action == "revert_vm_ip":
			self.server.revert_vm(None,ip_addr)
		elif action == "state":
			data = self.server.get_vm_state(ip_addr)

		self.send_response(200)
		self.end_headers()
		self.wfile.write(data)

if __name__ == "__main__":

	parser = argparse.ArgumentParser()
	parser.add_argument('--config', help='The config file to load', metavar='filename',dest='config_file',action='store',default='config.json')
	args = parser.parse_args()

	print("++ Config file: %s ++" % args.config_file)

	# Start the directory watcher thread first.
	print("++ Starting panic monitor thread ++")
	
	mt = PanicMonitor(args.config_file)
	mt.start()

	with open(args.config_file) as data_file:
		data = json.load(data_file)

	port = int(data['host_settings']['vmmanager_port'])

	server = VMManager(args.config_file,('', port), VMMonitor)
	print 'Started VMMonitor server on port ' , port
	server.serve_forever()

	mt.join()




