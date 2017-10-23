
""" 

This script is a helper to scale up VMs before a fuzzing run. 

The script creates a number of clones of a base VMX image.

This script produces a JSON output list of VMX files created.

The VM we are cloning needs to be turned off in a powered down state. 

"""

import argparse
import os
import subprocess

class CloneVMX:
	def __init__(self,base_vmx_path,clones):
		self.vmrun_path = "/Applications/VMware Fusion.app/Contents/Library/vmrun"
		self.base_vmx_path = base_vmx_path
		self.clones = clones
		self.determine_base_dir()
		self.vmx_files = []
		self.vmx_files.append(self.base_vmx_path)

	def determine_base_dir(self):
		self.base_dir = os.path.dirname(self.base_vmx_path)
		print("++ Base dir %s ++" % self.base_dir)

	def does_vmx_exist(self,path):
		if os.path.exists(path):
			return True
		else:
			return False

	def start_cloning(self):
		i = 0 
		while (i < self.clones):
			dest_folder = self.base_dir + "_clone_" + str(i+1)
			#print(dest_folder)
			dest_vmx = os.path.join(dest_folder,os.path.basename(self.base_vmx_path))
			print("Dest VMX %s " % dest_vmx)

			if not self.does_vmx_exist(dest_vmx):
				self.clone_vmx(self.base_vmx_path,dest_vmx)
			else:
				print("++ VMX file already exists - skipping ++")
			i += 1

	def clone_vmx(self,src_vmx,dest_vmx):
		output = subprocess.check_output([self.vmrun_path, "-T","fusion","clone",src_vmx,dest_vmx,"full"],stderr=subprocess.STDOUT)
		print(output)

	def dump_vmx(self):
		print(self.vmx_files)

if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument('--base_vmx',help='The base VMX file to clone',metavar='base_vmx',action='store',required=True)
	parser.add_argument('--clones',help='Number of clones to make',metavar='clones', type=int, action='store', required=True)
	args = parser.parse_args()

	print("++ Base VMX: %s ++" % args.base_vmx)
	print("++ Number of clones %s ++" % str(args.clones))

	cloner = CloneVMX(args.base_vmx,args.clones)
	cloner.start_cloning()
	cloner.dump_vmx()
