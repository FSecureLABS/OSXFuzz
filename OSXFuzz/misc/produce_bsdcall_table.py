
import json


""" This script is used to create the bsd system call table from the JSON output """

# Map BSD types to bughunt types
typemap = {
	"int" : "_INT32",
	"void" : "_VOID_PTR",
	"user_size_t" : "_UINT64",
	"user_addr_t" : "_VOID_PTR",
	#"msghdr" : "_STRUCT_MSGHDR",
	"struct" : "_VOID_PTR", #temp fix
	"uid" : "_UINT64",
	"gid" : "_UINT64",
	"uid_t" : "_UINT64",
	"gid_t" : "_UINT64",
	"caddr_t" : "_VOID_PTR",
	"uint_t" : "_UINT64",
	"char" : "_CHAR16_PTR"

}

def convert_arg_types(args):
	arg_str = ""

	if len(args) == 0:
		return "NIL"

	count = 0
	
	for a in args:
		
		bughunt_type = None

		# split on the space
		t = a.split(" ")[0]

		# special cases for fd and socket 
		if "fd" in a:
			bughunt_type = "_FD"
			arg_str += bughunt_type
		elif "socket" in a:
			bughunt_type = "_SOCKET"
			arg_str += bughunt_type
		elif "msghdr" in a:
			bughunt_type = "_STRUCT_MSGHDR"
			arg_str += bughunt_type

		if bughunt_type == None:
			try:
				bughunt_type = typemap[t]
				#print(bughunt_type)
				arg_str += bughunt_type
			except:
				#print("Cant find argtype " + str(t))
				arg_str += "_VOID_PTR" #If we don't know, just hope for the best :)

		#print(count)
		if len(args) >= 1:
			arg_str += ", "

		# Add the last argument of NIL if its the end
		if (count == len(args)-1):
			arg_str += "NIL"

		count += 1
		
	return arg_str

def convert_ret_val(ret_type):
	ret_val = ""
	try:
		ret_val = typemap[ret_type]
	except:
		ret_val = "NIL"
	return ret_val


if __name__ == "__main__":
	fp = open("bsdcalls.json","r")
	data = json.load(fp)

	for dp in data:
	#dp = data[7]
		try:
			callnum = int(dp[0])
		except:
			#print("Not a syscall")
			continue

		name = dp[2]
		if (name == "nosys" or name == "enosys"):
			continue

		ret_type = dp[1]
		num_args = int(dp[3])
		args = dp[4:4+num_args]

	#print(callnum)
	#print(ret_type)
	#print(num_args)
	#print(args)

		str_args = convert_arg_types(args)

		print("{ " + str(callnum) + ", { " + str_args + " }, " + convert_ret_val(ret_type) + " }, ")




