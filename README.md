# macOS Kernel Fuzzer

macOS 10.13 kernel fuzzer using multiple different methods. 

<h2>System Call Fuzzing</h2>

Both the BSD (SYSCALLS) and Mach (MACH_TRAPS) system dispatch tables are defined within the bughunt_syscalls.h file. A sample selection of both system calls and library calls have been provided to demonstrate how to implement additional calls. It is necessary to implement these additional system calls and library calls above the sample calls provided to increase coverage. 

<h2>macOS Host Configuration</h2>

1) Run host_setup/configure_panic_server.sh to setup the panic server on the host machine. 

2) Create a JSON config file to be used for the vm_manager.py script. An example default config can be found in host_setup/config.json. 

3) Follow the instructions for starting the VM Manager. 

<h2>macOS Guest Configuration</h2>

0) Deploy the latest macOS to a VM. 

1) Install the latest VMWare tools from https://my.vmware.com/web/vmware/details?downloadGroup=VMTOOLS1010&productId=491. At the time of writing this is VMware-Tools-10.1.0-other-4449150. 

2) Copy worker_setup/worker_setup.py to the initial VM and execute as follows:

```python worker_setup.py -p <panic_server_host_address>```

This step requires multiple reboots, to disable SIP and ensure nvram boot flags are set correctly. 

3) Clone the VM when its in its prepared state to scale up. A script is provided to do this and an example is as follows:

```
python clone_vmx.py --base_vmx "/Users/mwr/Documents/Virtual Machines.localized/macOS 10.12.vmwarevm/macOS 10.12.vmx" --clones 1
```

Where *--base-vmx* is the VMX file to use as a template for the clone and *--clones* is the number of clones to make. Note: the VM needs to be in a powered off state and prepd. 

This script will produce a list of VMX files which can then be taken and entered into the config file. 

5) Ensure there is a snapshot of each VM available called the same name as *prepd_snapshot_name* in the config file to revert back to. This will be automated at a future stage. 

6) Ensure vmx_files path is updated to list all VMX files to be used by the fuzz run. 

<h2>Usage</h2>

<h3>Running standalone</h3>

There are two methods for running the binary standalone. The first method is using remote **network logging**, which can be performed as follows:

```./OSXFuzz -s 1 -l "192.168.0.1" -p 1337 -b 1,2```

Where *-s* is the seed value. If this is set to 1 then PRNG will generate a new random seed, otherwise the seed value will be used. 

*-l* is the remote logger IP address. *-p* is the remote logger port. *-b* is the syscall numbers to blacklist. 

It is also possible to run the fuzzer with **disk based logging**. This can be performed as follows:

```./OSXFuzz -s 1 -d - -b 1,2```

*-d* is the directory in which the fuzzer log files will be stored in. If - is passed then the current working directory is used to createt these log files. 

*-D* can be used to enable *debug mode* which will print all log statements to standard out. 

<h3>Starting the VM Manager</h3>

The VM Manager and remote logger requires the configuration JSON being passed as `--config <filename>` option to vm_manager.py. If this is not passed the config will default to host_setup/config.json.  

An example of a configuration is as follows:

```json
{
	"host_settings" :
	{	
		"vmrun_path"   			:	"/Applications/VMware Fusion.app/Contents/Library/vmrun",
		"bughunt_start_src"		: 	"../start_bughunt.py",
		"bughunt_binary_src"	: 	"/Users/alex/Library/Developer/Xcode/DerivedData/OSXFuzz-aerykphcebbxjpfqusnxkdcbpkou/Build/Products/Debug/OSXFuzz", 
		"crash_path"   			: 	"/Users/alex/Documents/crashes",
		"panic_path"   			: 	"/PanicDumps/",
		"logger_addr"  			: 	"192.168.247.1",
		"logger_port"  			: 	"1337",
		"vmmanager_addr"		: 	"192.168.247.1",
		"vmmanager_port"		: 	"8888",	
		"vmx_files"				:	[
			"/Users/mwr/Documents/Virtual Machines.localized/macOS 10.12.vmwarevm/macOS 10.12.vmx"
		],
		"prepd_snapshot_name"	: "prepd"
	},
	"guest_settings" : 
	{
			"vm_username"  		: 	"alex",
			"launch_path"  		: 	"/tmp/start_bughunt.py"
	}
}
````

The settings are broken down into host settings (for the host system running VMWare) and guest settings which will be used on the VM guests. 

The *host settings* are as follows:

**vmrun_path** is the location of the vmrun binary used for VMWare automation. 

**bughunt_start_src** is the location on the host of the start_bughunt.py script which will be copied to the guests. 

**bughunt_binary_src** is the location of the OSXFuzz binary on the host which will be copied to the guests. 

**crash_path** is the location to store local crash data. 

**panic_path** is the location which is used to store kernel panic logs, configured on the host. 

**logger_addr** is the address which the remote log server should listen on. This needs to be exposed to the guest VMs. 

**logger_port** is the port which the remote log server should listen on.

**vmmanager_addr** is the host ip address which is exposed to the guest VMs where the VM manager HTTP server is running. 

**vmmanger_port** is the port which the VM Manager should listen on. Currently, the VM Manager binds to all interfaces. This also needs to be exposed to the guest VMs. 

**vmx_files** is an array of paths to the VMX files used for each VM. Currently this array has to be configured manually to add new VMs to the fuzzing process. 

**prepd_snapshot_name** is the snapshot name of when the VM is fully setup, which will be used when a revert occurs. This code assumes that all VMs have a snapshot with the same name setup. 

The *guest settings* are as follows:

**launch_path** is the location of the start_bughunt.py script which is executed within the guest VM to start the deployment and fuzzing process. Currently this is copied to the guest VM and the path created. 

**vm_username** is the username for the guest virtual machines. It is assumed that all VMs are configured using the same username/password combination currently. The vm password will be read in at runtime. 

When the VM Manager is configured, host_setup/vm_monitor.py is used to launch it and start the server listening. 

<h3>Starting a fuzzing run</h3>

When the VM Manager is running and all VMs are up and in a ready state a fuzzing run can be initiated by running host_setup/start_vms.py. This will launch the fuzzer on each node and you should see the VMs check-in with the VM Manager and the loggers call back. 

<h3>Performing library injection</h3>

If the config file contains a **dylib_path** option within guest_settings, this will be deployed to the host and can be used by a fuzzer script for dylib interpose based fuzzing. 

<h2>Crash Detection and Reproduction</h2>

Crash detection is currently being performed using the Panic Server and PanicMonitor python class. Crashes are being stored locally until database management code is created.  

In order to reproduce a crash, the repro.py script is ran to convert the log into the correct format to compile. A Makefile is also generated, which can be used to rebuild the repro testcase to binary. 
