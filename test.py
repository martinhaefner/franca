import sys
import os

sys.path.append(os.getcwd() + "/build/lib.linux-i686-2.7")

import franca


root = franca.package()
franca.builder.parse_and_build(root, "hello.fidl")


def print_typecollection(tc, indent) :
   indent += "   "
   print indent + "typecollection " + tc.name
   for t in tc.types:      
      if isinstance(t, franca.struct) :
         print(indent + "   " + t.name + " is struct")
         if t.has_base :
            print indent + "      is based on " + t.base().name + " (" + t.base().fqn("::") + ")"
      else :
         print(indent + "   " + t.name + " is enum")
         
      print(indent + "   " + "   fqn " + t.fqn("::"))         


def print_package(pck,indent) :
   if pck.name :
      print indent + "package " + pck.name
   
   for tc in pck.typecollections :
      print_typecollection(tc, indent)
      
   for p in pck.packages :
      print_package(p, indent + "   ")      


print_package(root,"")

