#!/usr/bin/python
import sys
import re
import xml.etree.ElementTree as ET

""" Global set of variables. Entries can be repeated in the xml file and
    we do not want to generate duplicate get_ and set_ methods in the C++
    header file. """
var_names = set();

class ScalarVariable:
	def __init__(self, node):
		"""This class is responsible for generating strings from .xml <ScalarVariable> tags."""
		self.attributes = self.__createAttributes(node)
	def __createAttributes(self, node): 
		attributes = {}
		name = node.get("name")
		name = name.replace(".","_")
		name = name.replace(",","_")
		name = name.replace("]","_")
		name = name.replace("[","_")
		name = name.replace("(","_")
		name = name.replace(")","_")
		attributes["name"] = name
		attributes["index"] = node.get("valueReference")
		if node.find("Real") is not None:
			attributes["type"] = "double"
		elif node.find("Boolean") is not None:
			attributes["type"] = "boolean"
		elif node.find("Integer") is not None:
			attributes["type"] = "int"
		elif node.find("String") is not None:
			attributes["type"] = "std::string"
		elif node.find("Enumeration") is not None:
			attributes["type"] = "int"
		else:
			print ("Unknown attribute"+name)
		return attributes
	@property
	def name(self):
		return self.attributes["name"];

	def getCPPString(self):
		rs = "" # Return string
		if self.attributes['type']=="double":
			rs += "\t\t{0} get_{1}() {{ return get_real({2}); }}\n\t\tvoid set_{1}({0} val) {{ set_real({2},val); }}\n".format(self.attributes['type'], self.attributes['name'], self.attributes['index'])
		elif self.attributes['type']=="boolean":
			rs += "\t\tbool get_{1}() {{ return get_bool({2}); }}\n\t\tvoid set_{1}(bool val) {{ set_bool({2},val); }}\n".format(self.attributes['type'], self.attributes['name'], self.attributes['index'])
		elif self.attributes['type']=="int":
			rs += "\t\t{0} get_{1}() {{ return get_int({2}); }}\n\t\tvoid set_{1}({0} val) {{ set_int({2},val); }}\n".format(self.attributes['type'], self.attributes['name'], self.attributes['index'])
		elif self.attributes['type']=="std::string":
			rs += "\t\t{0} get_{1}() {{ return get_string({2}); }}\n\t\tvoid set_{1}({0} val) {{ set_string({2},val); }}\n".format(self.attributes['type'], self.attributes['name'], self.attributes['index'])
		else:
			rs += "\t\t{0} get_{1}() {{ return get_{0}({2}); }}\n\t\tvoid set_{1}({0} val) {{ set_{0}({2},val); }}\n".format(self.attributes['type'], self.attributes['name'], self.attributes['index'])
			print ("WARNING. UNRECOGNIZED TYPE: {0}. Parsing as: \n{1}".format(self.attributes['type'], rs))
		return rs

def write_file(filename, string):
	f = open(filename, 'w')
	f.write(string)
	f.close()

def interpret(filename): # Go through file and pick out important information.
	legend = {}
	variables = []
	index_num = 0
	doc = ET.parse(filename)
	legend["modelName"] = doc.getroot().get("modelName")
	print ("Model name = ",legend["modelName"])
	legend["guid"] = doc.getroot().get("guid")
	print ("GUID = ",legend["guid"])
	legend["indicatorNum"] = doc.getroot().get("numberOfEventIndicators")
	print ("eventIndicators =",legend["indicatorNum"])
	scalar_vars_list = doc.findall("ModelVariables/ScalarVariable")
	for var in scalar_vars_list:
		new_var = ScalarVariable(var);
		if not new_var.name in var_names:
			variables.append(new_var);
			var_names.add(new_var.name);
	return legend, variables

def providesJacobian(filename):
	doc = ET.parse(filename);
	providesDirDerivative  = doc.findall("ModelExchange/[@providesDirectionalDerivative='true']");
	return providesDirDerivative != None and len(providesDirDerivative) > 0;

def countDerivatives(filename):
	"""This function is responsible for counting the state variables"""
	doc = ET.parse(filename)
	index_num = 0
	derivatives_list = doc.findall("ModelStructure/Derivatives/Unknown")
	for var in derivatives_list:
		index_num=index_num+1
	return index_num

def compile_str(legend, variables, using_str, provides_jacobian):
	"""This function is responsible for creating the header file"""
	jacobian_init_str = "false";
	if provides_jacobian:
		jacobian_init_str = "true";
	name = legend['modelName']
	name = name.replace(".","")
	legend['modelName'] = name
	rs = "" # Return string variable.
	if using_str:
		rs += '#ifndef {0}_h_\n#define {0}_h_\n#include "adevs.h"\n#include "adevs_fmi.h"\n#include <string>\n\n'.format(legend['modelName']) # Format header default information
	else:
		rs += '#ifndef {0}_h_\n#define {0}_h_\n#include "adevs.h"\n#include "adevs_fmi.h"\n\n'.format(legend['modelName']) # Format header default information
	rs += 'class {0}:\n\tpublic adevs::FMI<{1}>\n{{\n\tpublic:\n\t\t{0}():\n'.format(legend['modelName'], legend['convType']) # Format first part of class
	rs += '\t\t\tadevs::FMI<{1}>\n\t\t\t(\n\t\t\t\t"{0}",\n\t\t\t\t"{2}",\n\t\t\t\t"{3}",\n\t\t\t\t{4},\n\t\t\t\t{5},\n\t\t\t\t"{6}",\n\t\t\t\t1E-8,0,0.0,\n\t\t\t\t{7}\n\t\t\t)\n\n\t\t{{\n\t\t}}\n'.format(legend['modelName'], legend['convType'], legend['guid'], legend['resourceLocation'], legend['derNum'], legend['indicatorNum'], legend['sharedLocation'], jacobian_init_str)  # Format FMI constructor call
	for var in variables:
		rs += var.getCPPString()
	rs += '};\n\n#endif'
	return rs

def print_help():
			print ("This program converts .xml files to .h files under the FMI standard found at www.fmi-standard.org")
			print ("Example: 'xml2cpp -x target_xml -type type -f shared_object_file -r resource_location -o output_name'")
			print ("-h will open up this screen")
			print ("-o is optional")

"""This part builds the header file"""
if __name__=="__main__":
	args = sys.argv
	output_file = ""
	USING_STRING = False
	if len(args) == 1:
		print_help()
		sys.exit(0)
	for i, arg in enumerate(args):
		if arg == "-x":
			filename = args[i+1]
		elif arg == "-r":
			res_loc = args[i+1]
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
	i = countDerivatives(filename)
	for var in variables:
		#if var.isDer():
			#der_num += 1
		if var.attributes['type']=="std::string":
			USING_STRING = True
	#if der_num > 0:
	#	legend['derNum'] = der_num-1
	#else:
	#	legend['derNum'] = der_num
	legend['derNum'] = i
	legend['convType'] = conv_type
	legend['sharedLocation'] = file_loc # This is the location of the shared object file produced by omc
	legend['resourceLocation'] = res_loc # This is the location of json files produced by omc

	if output_file == "":
		output_file = filename[:-4] + ".h"
	else:
		output_file = output_file + ".h"

	write_file(output_file, compile_str(legend, variables, USING_STRING, providesJacobian(filename)))

