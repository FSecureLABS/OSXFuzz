
import subprocess
import fnmatch
import sys
import shutil
import os
import time
import glob
import sys
import json
import urllib2
import socket
import fcntl
import struct 

"""
Script used to start bughunt running from the guest VM. 

This script does the following:
1) Registers the fuzz node with the VM manager. 
2) launch_bughunt deploys the fuzzer to the disk. 
3) Starts execution of the fuzzer. 

This script communicates with the server using JSON. 

"""

# BSD System Call Blacklist, we pass this from python to save rebuiding the fuzzer every time. 
BSD_SYSCALL_BLACKLIST = [
1, # exit
2, # fork
59, # execve
66, # vfork
111, # sigsuspend
232, # waitevent
296, # vm_pressure_monitor
301, # psynch_mutexwait
306, # psynch_rw_rdlock
307, # psynch_rw_wrlock
308, # psynch_rw_unlock
309, # psynch_rw_unlock2
]

BUGHUNT_ORIG_PATH = "/Volumes/VMware Shared Folders/shared/OSXFuzz"
BUGHUNT_LOCK = "/tmp/BUGHUNT_LOCK"
TIMEOUT=75000

# We should pass these paths to the python script via the launchd config. 

class Bughunt():

	def __init__(self,uuid,vm_manager_url,logger_url):
		self.uuid = uuid
		self.vm_manager_url = vm_manager_url
		t = logger_url.split(":")
		self.logger_host = t[0]
		self.logger_port = t[1]
		self.create_logfile()

	""" Check to see if bughunt is running """
	def is_bughunt_running(self):
		if os.path.exists(BUGHUNT_LOCK):
			self.logger("++ Bughunt running exiting! ++")
			return True
		else:
			return False

	# The lockfile will be deleted by reverting the VM. 
	def create_lockfile(self):
		self.lockfd = open(BUGHUNT_LOCK,"w")
		self.lockfd.write("test")
		self.lockfd.close()

	def create_logfile(self):
		self.log_file = open("/tmp/bughunt.log","w")

	def logger(self,data):
		self.log_file.write(data + "\n")
		self.log_file.flush()
		print(data)

	""" Copy bughunt from the VM share to the VM main disk 
		Not currently used. 
	"""
	def deploy_bughunt(self):
		self.logger("++ Deploying bughunt ++")

		# Try make a directory as the user. 
		home_dir = os.getenv("HOME")
		bughunt_dir = os.path.join(home_dir,"bughunt")

		try:
			shutil.rmtree(bughunt_dir)
		except:
			print("++ Dir not present ++")

		try:
			os.mkdir(bughunt_dir)
		except:
			print("++ Created bughunt dir ++")

		# Now copy the binary. 
		shutil.copy(BUGHUNT_ORIG_PATH,bughunt_dir)
		os.chmod(bughunt_dir, 0777)

		self.bughunt_path = os.path.join(bughunt_dir,"OSXFuzz")

	"""
	Get the local IP address from ifconfig.  
	"""
	def get_local_ip_addr(self):
		proc = subprocess.Popen("ifconfig en0",stdout=subprocess.PIPE,shell=True)
		(out,err) = proc.communicate()
		# Now grep out the ip addr. 
		lines = out.split("\n")
		for l in lines:
			if "inet " in l:
				addr = l.split(" ")[1]
				return str(addr)

	""" Used to parse /REGISTER config data from the host to the fuzzers """
	def parse_fuzzer_config(self):
		pass

	""" This function is called when a fuzz node comes online 
	At some point we might want to provide a config object as response. 
	"""
	def register_fuzznode(self):
		ip_addr = self.get_local_ip_addr()
		data = {
			"action" : "register", 
			"uuid" : self.uuid,
			"ip_addr" : ip_addr
		} 
		print(data)

		url = self.vm_manager_url + "/REGISTER"
		req = urllib2.Request(url)
		req.add_header('Content-Type','application/json')
		response = urllib2.urlopen(req,json.dumps(data))
		return data 

	""" This function is called when a fuzz node needs reverted after a fuzzing run """
	def revert_fuzznode(self):
		ip_addr = self.get_local_ip_addr()
		data = {
			"action" : "revert", 
			"uuid" : self.uuid,
			"ip_addr" : ip_addr
		} 
		print(data)

		url = self.vm_manager_url + "/REVERT"
		req = urllib2.Request(url)
		req.add_header('Content-Type','application/json')
		response = urllib2.urlopen(req,json.dumps(data))

		return data 		

	""" Determine if dylib for interposing exists """
	def dylib_exist(self,dylib_path):
		if os.path.exists(dylib_path):
			return True
		else:
			return False

	""" Determine if we need to inject in library """
	def initialize_environment(self):
		my_env = os.environ.copy()
		if self.dylib_exist("/tmp/OSXFuzz.dylib"):
			self.logger("++ injecting in dylib ++")
			my_env['DYLD_INSERT_LIBRARIES'] = "/tmp/OSXFuzz.dylib"
		return my_env

	""" Start a run of the fuzzer """
	def start_bughunt(self):

		# Convert the blacklist into the correct form for the C binary args.
		blacklist = str(BSD_SYSCALL_BLACKLIST)
		blacklist = blacklist.replace(" ","")
		blacklist = blacklist.replace("[","")
		blacklist = blacklist.replace("]","")

		self.logger("++ Starting bughunt ++")
		# Get the current dir the python file is in. 
		# We know OSXFuzz is within the same directory. 
		self.bughunt_path = "/tmp/OSXFuzz"
		
		cmd = [self.bughunt_path,"-s","1","-l",self.logger_host,"-p",self.logger_port,"-b",blacklist,"-D"]
		#cmd = [self.bughunt_path,"1","-",blacklist]
		ps = subprocess.Popen(cmd,env=self.initialize_environment(),stdout = open("/tmp/output.txt","w"), stderr=subprocess.STDOUT, shell=False)
		logstr = "Starting process " + str(ps.pid)
		self.logger(logstr)
		self.create_lockfile()
		c = 0 
		self.logger("++ After lock file ++")
		while c < TIMEOUT:
			time.sleep(0.1)
			c += 100
			ps.poll()
			try:
				self.logger(str(ps.returncode))
			except:
				print("++ Error writing to log file ++")
			if ps.returncode != None:
				try:
					self.logger("++ Process exited ++")
				except:
					print("++ Error writing to log file ++")
				break

		try:
			self.logger("++ Timeout hit reverting VM ++")
		except:
			print("++ Error writing to log file ++")
		self.revert_fuzznode()

if __name__ == "__main__":

	# First parse UUID out of the startup. 
	uuid = sys.argv[1]

	# Now get the URL of the VM monitor so we can make the callback. 
	vm_monitor_url = sys.argv[2]

	# Pass the logger URL
	logger_url = sys.argv[3]

	#print(uuid)

	bh = Bughunt(uuid,vm_monitor_url,logger_url)

	if bh.is_bughunt_running():
		bh.revert_fuzznode()
		sys.exit(0)
	else:
		time.sleep(5)
		#bh.deploy_bughunt()
		bh.register_fuzznode()
		bh.start_bughunt()

