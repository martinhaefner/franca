from wheezy.template.engine import Engine
from wheezy.template.ext.core import CoreExtension
from wheezy.template.loader import FileLoader
from types import MethodType

import sys
import os
import platform

# set path to franca module before import
sys.path.append(os.getcwd() + "/build/lib." 
   + platform.system().lower() + "-"
   + platform.machine() + "-" 
   + ".".join(platform.python_version_tuple()[0:2]))

import franca


# extend package
def namespaces_open(self):
   rc = ""
   p = self;
   
   while not p.is_root() :
      rc = "namespace " + p.name() + " {\n" + rc
      p = p.package()
      
   return rc
   
   
def namespaces_close(self):
   rc = ""
   
   p = self;
   
   while not p.is_root() :
      rc += "}   // namespace " + p.name() + "\n"
      p = p.package()
      
   return rc


franca.package.namespaces_open  = MethodType(namespaces_open, None, franca.package);
franca.package.namespaces_close = MethodType(namespaces_close, None, franca.package);
   

# extend type
intrinsic_types = [ 'UInt8', 'UInt16', 'UInt32', 'UInt64', 'Int8', 'Int16', 'Int32', 'Int64', 'Float' ]

intrinsic_types_mapping = { 
   'UInt8' :'unsigned char', 
   'UInt16':'unsigned short',
   'UInt32':'unsigned int', 
   'UInt64':'unsigned int64_t',
   'Int8'  :'signed char', 
   'Int16' :'short',
   'Int32' :'int', 
   'Int64' :'int64_t', 
   'Float' :'double'
}


def cpp_type(self) :
   if self.name() in intrinsic_types :
      return intrinsic_types_mapping[self.name()]
   elif self.name() == "String" :      
      return "std::string"
   else :
      return self.name()
   
   
franca.type.cpp_type = MethodType(cpp_type, None, franca.type);


def in_signature(self) :
   if self.name() in intrinsic_types :
      return self.cpp_type()
   else :
      return "const " + self.cpp_type() + "&"


def out_signature(self) :
   return self.cpp_type() + "&"

   
franca.type.in_signature  = MethodType(in_signature, None, franca.type);
franca.type.out_signature = MethodType(out_signature, None, franca.type);
   

# extend argument list
def for_method_decl_in(self) :
   rc = ""
   
   for i in range(0, len(self)) :
      rc += self[i].type().in_signature() + " " + self[i].name() + "_in"
      if i < len(self)-1 :
         rc += ", "

   return rc


def for_method_decl_out(self) :
   rc = ""
   
   for i in range(0, len(self)) :
      rc += self[i].type().out_signature() + " " + self[i].name() + "_out" 
      if i < len(self)-1 :
         rc += ", "
         
   return rc


franca.arg_vector.for_method_decl_in  = MethodType(for_method_decl_in, None, franca.arg_vector);
franca.arg_vector.for_method_decl_out = MethodType(for_method_decl_out, None, franca.arg_vector);


# ### START of main program ############################################ 

# load template
searchpath = ['.']

engine = Engine(
    loader=FileLoader(searchpath),
    extensions=[CoreExtension()]
)

template = engine.get_template('client_template.hpp')

# load franca
root = franca.package()
franca.builder.parse_and_build(root, "hello.fidl")

# generate output
print(template.render({'interface': root.packages[0].packages[0].interfaces[0]}))
