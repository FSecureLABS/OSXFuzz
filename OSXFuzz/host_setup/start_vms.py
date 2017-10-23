"""
This script is for launching a fuzz run when all the VNs are initially running. 
"""

import urllib2
req = urllib2.Request('http://127.0.0.1:8888/START')
response = urllib2.urlopen(req)	
