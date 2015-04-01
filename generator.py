from wheezy.template.engine import Engine
from wheezy.template.ext.core import CoreExtension
from wheezy.template.loader import FileLoader

import sys
import os
from types import MethodType

sys.path.append(os.getcwd() + "/build/lib.linux-i686-2.7")

import franca


# extend package
def namespaces_open(self):
   rc = ""
   p = self;
   
   while not p.is_root() :
      rc = "namespace " + p.name + " {\n" + rc
      p = p.package()
      
   return rc
   
   
def namespaces_close(self):
   rc = ""
   
   p = self;
   
   while not p.is_root() :
      rc += "}   // namespace " + p.name + "\n"
      p = p.package()
      
   return rc


franca.package.namespaces_open  = MethodType(namespaces_open, None, franca.package);
franca.package.namespaces_close = MethodType(namespaces_close, None, franca.package);

intrinsic_types = [ 'Int32', 'Int64', 'Float' ]

intrinsic_types_mapping = { 
   'Int32':'int', 
   'Int64':'int64_t', 
   'Float':'double'
}
   

# extend type
def in_signature(self) :
   if self.name in intrinsic_types :
      return intrinsic_types_mapping[self.name]
   elif self.name == "String" :      
      return "const std::string&"
   else :
      return "const " + self.name + "&"


def out_signature(self) :
   if self.name in intrinsic_types :
      return intrinsic_types_mapping[self.name] + "&"
   else :
      return self.name + "&"

   
franca.type.in_signature  = MethodType(in_signature, None, franca.type);
franca.type.out_signature  = MethodType(out_signature, None, franca.type);
   

# extend argument list
def for_method_decl_in(self) :
   rc = ""
   
   for i in range(0, len(self)) :
      rc += self[i].type().in_signature() + " " + self[i].name 
      if i < len(self)-1 :
         rc += ", "

   return rc


def for_method_decl_out(self) :
   rc = ""
   
   for i in range(0, len(self)) :
      rc += self[i].type().out_signature() + " " + self[i].name 
      if i < len(self)-1 :
         rc += ", "
         
   return rc


franca.arg_vector.for_method_decl_in  = MethodType(for_method_decl_in, None, franca.arg_vector);
franca.arg_vector.for_method_decl_out = MethodType(for_method_decl_out, None, franca.arg_vector);


def list_args(list) :
   rc = ""
   
   for i in range(0, len(list)) :
      rc += list[i].type().name + " " + list[i].name 
      if i < len(list)-1 :
         rc += ", "
      
   return rc


# load template
searchpath = ['.']

engine = Engine(
    loader=FileLoader(searchpath),
    extensions=[CoreExtension()]
)

engine.global_vars.update({'list_args':list_args})
template = engine.get_template('client_template.hpp')

# load franca
root = franca.package()
franca.builder.parse_and_build(root, "hello.fidl")

# generate output
print(template.render({'interface': root.packages[0].packages[0].interfaces[0]}))
