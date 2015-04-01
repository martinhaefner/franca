from wheezy.template.engine import Engine
from wheezy.template.ext.core import CoreExtension
from wheezy.template.loader import FileLoader

import sys
import os

sys.path.append(os.getcwd() + "/build/lib.linux-i686-2.7")

import franca


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

print list_args(root.packages[0].packages[0].interfaces[0].methods[0].in_args)

# generate output
print(template.render({'interface': root.packages[0].packages[0].interfaces[0]}))
