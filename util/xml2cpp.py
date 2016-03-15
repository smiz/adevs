#!/usr/bin/python
import sys
import re
from lxml import etree

class ScalarVariable:
	def __init__(self, node):
		"""This class is responsible for generating strings from .xml <ScalarVariable> tags."""
		self.attributes = self.__createAttributes(node)
	def __createAttributes(self, node): 
		attributes = {}
		name = node.get("name")
		name = name.replace(".","_")
		name = name.replace("]","_")
		name = name.replace("[","_")
		name = name.replace("(","_")
		name = name.replace(")","_")
		attributes["name"] = name
		attributes["index"] = node.get("valueReference")
		if node.find("Real") is not None:
			attributes["type"] = "double"
		elif node.find("Bool") is not None:
			attributes["type"] = "boolean"
		elif node.find("Boolean") is not None:
			attributes["type"] = "boolean"
		elif node.find("Integer") is not None:
			attributes["type"] = "int"
		elif node.find("String") is not None:
			attributes["type"] = "std::string"
		else:
			print "Unknown attribute"
		return attributes

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

def write_file(filename, string):
	f = open(filename, 'w')
	f.write(string)
	f.close()

def interpret(filename): # Go through file and pick out important information.
	legend = {}
	variables = []
	index_num = 0
	doc = etree.parse(filename)
	legend["modelName"] = doc.getroot().get("modelName")
	print "Model name = ",legend["modelName"]
	legend["guid"] = doc.getroot().get("guid")
	print "GUID = ",legend["guid"]
	legend["indicatorNum"] = doc.getroot().get("numberOfEventIndicators")
	print "eventIndicators =",legend["indicatorNum"]
	scalar_vars_list = doc.findall("ModelVariables/ScalarVariable")
	for var in scalar_vars_list:
		variables.append(ScalarVariable(var))
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
	legend, variables = interpret(filename)
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
