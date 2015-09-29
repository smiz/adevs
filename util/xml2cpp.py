#!/usr/bin/python
import sys
import re

class ScalarVariable:
	def __init__(self, input_lines):
		"""This class is responsible for generating strings from .xml <ScalarVariable> tags."""
		self.xml_input = input_lines
		self.attributes = self.__createAttributes(input_lines)
	def __createAttributes(self, inputs): # Pick out information about this scalar variable from the XML file string.
		attributes = {}
		for line in inputs:
			if "name" in line:
				if "der(" in line:
					attributes["name"] = "der_" + line[14:-3]
				else:
					attributes["name"] = line[10:-2]
				if "." in attributes["name"]:
					attributes["name"] = re.sub('[.]', '_', attributes["name"])
				if "[" in attributes["name"]:
					attributes["name"] = re.sub('[[]', '', attributes["name"])
					attributes["name"] = re.sub('[]]', '', attributes["name"])
			elif "valueReference" in line: # Use index instead because that coincides with the index_num calculated in get_legend()
				attributes["index"] = line[20:-2]
			elif "variability" in line:
				attributes["var"] = line[17:-2]
			elif "causality" in line:
				attributes["cause"] = line[15:-2]
			elif "initial" in line:
				attributes["init"] = line[13:-3]
			elif "<Real" in line:
				attributes["type"] = "double"
			elif "<Bool" in line:
				attributes["type"] = "boolean"
			elif "<Integer" in line:
				attributes["type"] = "int"
			elif "<String" in line:
				attributes["type"] = "std::string"
			elif not "Scalar" in line:
				attributes["type"] = line.split("<")[1].split(" ")[0] # This will capture any odd types of variables that are not currently in the FMI standard
		return attributes
	def getXMLString(self):
		return self.xml_input

	def getCPPString(self):
		rs = "" # Return string
		if self.attributes['type']=="double":
			rs += "\t\t{0} get_{1}() {{ return get_real({2}); }}\n\t\tvoid set_{1}({0} val) {{ set_real({2},val); }}\n".format(self.attributes['type'], self.attributes['name'], self.attributes['index'])
		elif self.attributes['type']=="boolean":
			rs += "\t\t{0} get_{1}() {{ return get_bool({2}); }}\n\t\tvoid set_{1}({0} val) {{ set_bool({2},val); }}\n".format(self.attributes['type'], self.attributes['name'], self.attributes['index'])
		elif self.attributes['type']=="int":
			rs += "\t\t{0} get_{1}() {{ return get_int({2}); }}\n\t\tvoid set_{1}({0} val) {{ set_int({2},val); }}\n".format(self.attributes['type'], self.attributes['name'], self.attributes['index'])
		elif self.attributes['type']=="std::string":
			rs += "\t\t{0} get_{1}() {{ return get_string({2}); }}\n\t\tvoid set_{1}({0} val) {{ set_string({2},val); }}\n".format(self.attributes['type'], self.attributes['name'], self.attributes['index'])
		else:
			rs += "\t\t{0} get_{1}() {{ return get_{0}({2}); }}\n\t\tvoid set_{1}({0} val) {{ set_{0}({2},val); }}\n".format(self.attributes['type'], self.attributes['name'], self.attributes['index'])
			print "WARNING. UNRECOGNIZED TYPE: {0}. Parsing as: \n{1}".format(self.attributes['type'], rs)
		return rs

	def isDer(self): # Returns whether the variable is a derivative or not.
		return "der" in self.attributes['name']


def read_file(filename): # Open a file, copy it into list of lines. return
	f = open(filename, 'r')
	lines = f.readlines()
	f.close()
	return lines

def write_file(filename, string):
	f = open(filename, 'w')
	f.write(string)
	f.close()
def interpret(line_arr): # Go through file and pick out important information.
	legend = {}
	variables = []
	index_num = 0
	for i, line in enumerate(line_arr):
		if "modelName" in line:
			legend["modelName"] = line[13:-2]
		elif "guid" in line:
			legend["guid"] = line[8:-2]
		elif "Index" in line:
			index_num += 1
		elif "numberOfEventIndicators" in line:
			legend["indicatorNum"] = line[27:-3]
		elif "<ScalarVariable" in line:
			variables.append(ScalarVariable(line_arr[i:i+8]))

	legend["indexNum"] = index_num
	return legend, variables

def compile_str(legend, variables, using_str):
	if using_str:
		rs = "" # Return string variable.
		rs += '#ifndef {0}_h_\n#define {0}_h_\n#include "adevs.h"\n#include "adevs_fmi.h"\n#include <string>\n\n'.format(legend['modelName']) # Format header default information
		rs += 'class {0}:\n\tpublic adevs::FMI<{1}>\n{{\n\tpublic:\n\t\t{0}():\n'.format(legend['modelName'], legend['convType']) # Format first part of class
		rs += '\t\t\tadevs::FMI<{1}>\n\t\t\t(\n\t\t\t\t"{0}",\n\t\t\t\t"{2}",\n\t\t\t\t{3},\n\t\t\t\t{4},\n\t\t\t\t"{5}"\n\t\t\t)\n\t\t{{\n\t\t}}\n'.format(legend['modelName'], legend['convType'], legend['guid'], legend['derNum'], legend['indicatorNum'], legend['sharedLocation'])  # Format FMI constructor call
		for var in variables:
			rs += var.getCPPString()
		rs += '};\n\n#endif'
	else:
		rs = "" # Return string variable.
		rs += '#ifndef {0}_h_\n#define {0}_h_\n#include "adevs.h"\n#include "adevs_fmi.h"\n\n'.format(legend['modelName']) # Format header default information
		rs += 'class {0}:\n\tpublic adevs::FMI<{1}>\n{{\n\tpublic:\n\t\t{0}():\n'.format(legend['modelName'], legend['convType']) # Format first part of class
		rs += '\t\t\tadevs::FMI<{1}>\n\t\t\t(\n\t\t\t\t"{0}",\n\t\t\t\t"{2}",\n\t\t\t\t{3},\n\t\t\t\t{4},\n\t\t\t\t"{5}"\n\t\t\t)\n\t\t{{\n\t\t}}\n'.format(legend['modelName'], legend['convType'], legend['guid'], legend['derNum'], legend['indicatorNum'], legend['sharedLocation'])  # Format FMI constructor call
		for var in variables:
			rs += var.getCPPString()
		rs += '};\n\n#endif'
	return rs

def print_help():
			print "This program converts .xml files to .h files under the FMI standard found at www.fmi-standard.org"
			print "Example: 'xml2cpp -r target_xml -type type -f shared_object_file -o output_name'"
			print "-h will open up this screen"
			print "-o is optional"

if __name__=="__main__":
	args = sys.argv
	output_file = ""
	USING_STRING = False
	if len(args) == 1:
		print_help()
		sys.exit(0)
	for i, arg in enumerate(args):
		if arg == "-r":
			filename = args[i+1]
		elif arg == "-type":
			conv_type = args[i+1]
		elif arg == "-f":
			file_loc = args[i+1]
		elif arg == "-o":
			output_file = args[i+1]
		elif arg == "-h":
			print_help()
			sys.exit(0)
	legend, variables = interpret(read_file(filename))
	der_num = 0
	for var in variables:
		if var.isDer():
			der_num += 1
		if var.attributes['type']=="std::string":
			USING_STRING = True
	legend['derNum'] = der_num
	legend['convType'] = conv_type
	legend['sharedLocation'] = file_loc # This is the location of the shared object file produced by omc

	if output_file == "":
		output_file = filename[:-4] + ".h"
	else:
		output_file = output_file + ".h"

	write_file(output_file, compile_str(legend, variables, USING_STRING))
