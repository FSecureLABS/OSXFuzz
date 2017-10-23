# This wrapper script is ran from the VMware share from inside the VM.
# We assume this script and start_bughunt.py are in the shared folder location. 

# First copy start_bughunt.py from VM share
cd "/Volumes/VMware Shared Folders/shared/"
cp start_bughunt.py ~/Documents/

# Now start bughunt running from the filesystem
# This will do the deployment 
cd ~/Documents/
python start_bughunt.py $@
