

"""

This script automates the snapshotting of all running VMs 

"""

import argparse
import os
import subprocess

class SnapshotVM:
	def __init__(self):
		self.vmrun_path = "/Applications/VMware Fusion.app/Contents/Library/vmrun"
		self.snapshot_name = "prepd"

	def get_running_vms(self):
		output = subprocess.check_output([self.vmrun_path, "-T","fusion","list"],stderr=subprocess.STDOUT)
		vmx_files = output.split("\n")[1:]
		self.clone_vms(vmx_files)

	def clone_vms(self,vmx_files):
		for f in vmx_files:
			if "vmx" in f:
				print(f)
				# Create the snapshot. 
				output = subprocess.check_output([self.vmrun_path, "-T","fusion","snapshot",f,self.snapshot_name],stderr=subprocess.STDOUT)
				print(output)



if __name__ == "__main__":
	snapshot = SnapshotVM()
	snapshot.get_running_vms()