import socket
import threading
import SocketServer
import time
import thread
import urllib2
import json

""" 
This logger needs to be multi-threaded to handle multiple VMs
"""

DEBUG = True
LOG_TIMEOUT = 50

class ThreadedTCPRequestHandler(SocketServer.BaseRequestHandler):

    def handle(self):
        print("++ Incoming connection ++")
        # Get the socket details.
        peer_name = self.request.getpeername()
        self.server.new_connection(peer_name[0])
        while 1:
            try:
                data = self.request.recv(1024)
            except:
                print("++ Connection reset by peer ++")
                return 
            if not data:
                print("++ Connection terminated from VM side ++") 
                # Pause the monitoring thread.  
                #self.server.reset_monitoring(peer_name[0])
                break

            # Add the log line to the memory buffer. 
            # Buffer is initialized to zeros, so strip null bytes from recv buffer. 
            statement = data.rstrip("\0")
            self.server.add_statement(peer_name[0],statement)
            cur_thread = threading.current_thread()
            response = "{}: {}".format(cur_thread.name, statement)
            
            if DEBUG:
                print(response)
            
            self.request.sendall("A")

class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer, object):
    
    def __init__(self, sockname=("0.0.0.0", 1337), handler=None):
        print("++ Starting network logging server ++")
        super(ThreadedTCPServer, self).__init__(sockname, handler)
        self.vmmanager_port = 8888
        self.log_data = {} # IP -> Log Lines Buffer
        self.log_recv_time = {} # IP -> Last recv time
        self.monitoring_threads = {} # IP -> Active Thread Object

    def reset_monitoring(self,ip_addr):
        print("++ Resetting elapse time to None")
        self.log_recv_time[ip_addr] = None
 
    def recv_monitor_thread(self,ip_addr):

        cur_thread = threading.current_thread()
        statement = "{}: Monitoring recv buffer for {}".format(cur_thread.name, ip_addr)
        print(statement)

        while 1:

            # Get the last time the log was received. 
            last_update_time = self.log_recv_time[ip_addr]

            #print("++ Last update time = " + str(last_update_time))
            # Get the elapse time between the log being received.
            elapsed_time = time.time() - last_update_time

            statement = "{}:{} Time since last log statement {}".format(ip_addr,str(cur_thread.name),str(elapsed_time))
            print(statement)

            # This should only occur when the start_bughunt script has died for some reason. 
            # There's a bit of a race here if the VMs take longer than 100s to revert. 
            # As a revert might be occuring when another revert is triggered or a VM is booting. 
            # The state is therefore checked before a revert occurs. 
            if (elapsed_time > LOG_TIMEOUT):

                # Check to see if the VM is already reverting
                if not self.check_vm_state(ip_addr):
                    print("++ VM already reverting, sleeping and testing again ++")
                    time.sleep(2)
                    continue

                print("++ No log statements received from %s reverting fuzzer ++" % str(ip_addr))

                # Delete the key so we start a new monitoring thread after revert.
                if ip_addr in self.monitoring_threads:
                    del self.monitoring_threads[ip_addr]

                # Try do a revert if its not already reverting. 
                self.revert_vm(ip_addr)
                break

            time.sleep(2)

    def check_vm_state(self,ip):
        data = {
            "action" : "state", 
            "uuid" : "None",
            "ip_addr" : ip
        }
        # VMManager always runs on localhost. 
        vmmanager_url = "http://127.0.0.1:" + str(self.vmmanager_port)
        req = urllib2.Request(vmmanager_url)
        req.add_header('Content-Type','application/json')
        response = urllib2.urlopen(req,json.dumps(data)) 
        data = response.read()
        print("++ VM State is: %s " % data)      
        if "active" in data:
            return True
        return False

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

    def new_connection(self,ip_addr):
        print("++ New connection from peer: ", ip_addr)
        self.log_data[ip_addr] = []
        self.log_recv_time[ip_addr] = time.time() 
        
        # See if we have an active monitoring thread already. 
        # This should only be called one time when the fuzzing run starts. 
        if ip_addr not in self.monitoring_threads:
            # Start a thread monitoring the last recv time.
            th = thread.start_new_thread(self.recv_monitor_thread,(ip_addr,))
            self.monitoring_threads[ip_addr] = th
        else:
            print("++ There is already a log buffer monitoring thread running - reusing ++ ")

    def add_statement(self,ip_addr,statement):
        self.log_data[ip_addr].append(str(statement))
        # Update the last recv'd time to be the current time..
        self.log_recv_time[ip_addr] = time.time()

    def get_logdata(self,ip_addr):
        log_data = None
        try:
            log_data = self.log_data[ip_addr]
        except:
            print("++ Could not find log data")
        return log_data

if __name__ == "__main__":
	HOST, PORT = "0.0.0.0", 1337
	server = ThreadedTCPServer((HOST, PORT), ThreadedTCPRequestHandler)
	ip, port = server.server_address
	server_thread = threading.Thread(target=server.serve_forever)
	server_thread.daemon = True
	server_thread.start()
	print "Server loop running in thread:", server_thread.name
	server_thread.join()
