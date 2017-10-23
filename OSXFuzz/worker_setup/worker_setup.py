# -*- coding: utf-8 -*-

"""

This script is used to setup the guest VM for fuzzing - it should be ran as under sudo on a clean VM. 

The panicd server should be passed to this script arguments. 

"""

import os
import subprocess
import argparse

def check_is_sudod():
	""" Check the script is running as sudo """
	if os.geteuid() != 0:
		exit("You need to have root privileges to run this script.\nPlease try again, this time using 'sudo'. Exiting.")

def check_sip_is_disabled():
	""" Check to see if SIP is disabled """
	status = subprocess.check_output(["/usr/bin/csrutil","status"])
	if "disabled" in status:
		return True
	else:
		return False

def check_boot_args(hostname):
	""" Check to see if the boot-args NVRAM is configured properly """
	boot_args = subprocess.check_output(["/usr/sbin/nvram","boot-args"],stderr=subprocess.STDOUT)
	print(boot_args)
	if hostname in str(boot_args):
		print("++ Panicd already configured ++")
		return True
	print("++ Panicd not found in boot-args ++")
	return False

def setup_nvram_boot_args(hostname):
	""" Configure the NVRAM boot-args if not set """
	args = "boot-args=-v debug=0x2444 keepsyms=1 -zp -zc _panicd_ip=" + hostname
	print(args)
	boot_args = subprocess.check_output(["/usr/sbin/nvram",args])
	print(boot_args)

def disable_crashreport_dialog():
	""" Disable the crash reporting dialog box """
	os.system("defaults write com.apple.CrashReporter DialogType none")

def disable_system_updates():
	""" Prevent automatic system updates """
	os.system("/usr/sbin/softwareupdate --schedule off")

def disable_sleep():
	""" Prevent the VM from sleeping """
	os.system("/usr/sbin/systemsetup -setcomputersleep Never")

if __name__ == "__main__":

	parser = argparse.ArgumentParser(description='Parse the panicd settings')
	parser.add_argument('-p', '--panicd', dest='hostname', required=True, 
                    help='Panicd Host name, IP Address')

	args = parser.parse_args()
	panicd_hostname = args.hostname
	print("++ Using panicd address: " + panicd_hostname)

	check_is_sudod()

	if check_sip_is_disabled():
		print("++ SIP disabled already ++")
	else:
		print("++ SIP needs to be disabled manually using ctrl-R, recovery mode, then csrutil disable. \nWhen this is done please reboot and try again ++")
		exit("++ Please reboot and disable SIP ++")

	if check_boot_args(panicd_hostname):
		print("++ boot-args setup correctly ++")
	else:
		print("++ Setting boots-args")
		setup_nvram_boot_args(panicd_hostname)
		print("++ Panicd can be tested using - dtrace -w -n \"BEGIN{ panic();}\"")
		exit("++ Please reboot now to ensure boot-args is set ++")

	print("++ Finalizing settings ++")
	disable_crashreport_dialog()
	disable_system_updates()
	disable_sleep()

	# Check the other settings
	print("++ VM SETUP COMPLETE - Please clone ++ ")


