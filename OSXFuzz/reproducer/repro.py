

import sys
import os

"""
This script is used to take a testcase log and turn it into a standalone testcase and a makefile to compile it. 
Outputs the standalone testcase and Makefile to build it (in a zip). 
gcc main.c -o main  -w -framework IOKit -framework IOSurface -framework CoreFoundation -I/Users/alex/Documents/OSXFuzz/OSXFuzz/OSXFuzz/
"""

class Reproducer:

	def __init__(self,log_input_path,testcase_output_path):
		self.log_path = log_input_path
		self.testcase_output_path = testcase_output_path
		self.read_logfile()

	def read_logfile(self):
		fd = open(self.log_path,"r")
		self.data = fd.read()
		fd.close()

	def get_seed_line(self,data):
		""" Extract the seed line so we can seed before we setup the object database again """
		# Seed line should be top line of file always.
		temp = data.split("\n")
		seed_line = temp[0]
		rest_of_data = temp[1:]
		print("Seed line: " + seed_line)
		return (seed_line,rest_of_data)

	""" Convert the logfile to a standalone testcase """
	def convert_logfile(self):
		# Get the seed line and strip it off the output. 
		t = self.get_seed_line(self.data)
		seed_line = t[0]
		print("Seed line: " + seed_line)
		rest_of_data = "\n".join(t[1])

		# The headers here are a bit of a hack until we fix the deps. 
		template = """
#include "object_database.h"

int main(int argc, char *argv[])
{
%s
make_OBJECTS(); 
make_fd();
%s
}
""" % (seed_line, rest_of_data)
		#print(template)
		return template

	def create_repro(self):
		repro = self.convert_logfile()
		fd = open(self.testcase_output_path,"w")
		fd.write(repro)
		fd.close()

	""" TODO: How do we ensure the makefile paths use the fuzzer code """
	def create_makefile(self):
		source_path = "repro.c"
		dest_binary = "repro"
		template = """
INC=-I/Users/mwr/Documents/OSXFuzz/OSXFuzz
LDFLAGS=-framework IOKit -framework IOSurface -framework CoreFoundation -framework Hypervisor

all:
	clang %s -o %s -w -g $(INC) $(LDFLAGS)
""" % (source_path,dest_binary)
	
		base_dir = os.path.dirname(self.testcase_output_path)
		makefile_path = os.path.join(base_dir,"Makefile")
		fd = open(makefile_path,"w")
		fd.write(template)
		fd.close()

if __name__ == "__main__":
	input_path = sys.argv[1]
	output_path = sys.argv[2]
	print("++ Log Input Path: " + input_path)
	print("++ Output Path: " + output_path)
	repro = Reproducer(input_path,output_path)
	repro.create_repro()