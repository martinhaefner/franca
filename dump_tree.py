import sys
import os
import platform

# set path to franca module before import
sys.path.append(os.getcwd() + "/build/lib." 
   + platform.system().lower() + "-"
   + platform.machine() + "-" 
   + ".".join(platform.python_version_tuple()[0:2]))

import franca


root = franca.package()
franca.builder.parse_and_build(root, "hello.fidl")


def print_typecollection(tc, indent) :
   indent += "   "
   print indent + "typecollection " + tc.name()
   for t in tc.types:      
      #
      # XXX beware to always ask if the type is a union before trying struct
      #     else this seems to yield wrong results
      #
      if isinstance(t, franca.union) :
         print(indent + "   " + t.name() + " is union")
         if t.has_base() :
            print indent + "      is based on " + t.base().name() + " (" + t.base().fqn("::") + ")"

      elif isinstance(t, franca.struct) :
         print(indent + "   " + t.name() + " is struct")
         if t.has_base() :
            print indent + "      is based on " + t.base().name() + " (" + t.base().fqn("::") + ")"
      
      elif isinstance(t, franca.enumeration) :
         print(indent + "   " + t.name() + " is enum")
         if t.has_base() :
            print indent + "      is based on " + t.base().name() + " (" + t.base().fqn("::") + ")"
      
      elif isinstance(t, franca.typedef) :
         print(indent + "   " + t.name() + " is typedef to real type " + t.real_type().name())
      
      elif isinstance(t, franca.array) :
         print(indent + "   " + t.name() + " is array of " + t.element_type().name())
      
      elif isinstance(t, franca.map) :
         print(indent + "   " + t.name() + " is map of [" + t.key_type().name() + ", " + t.value_type().name() + "]")
      
      else :
         print "XXXXXXXXXXXXX UNKNOWN TYPE XXXXXXXXXXXXXXXXXXXXXXXXXX"
         
      print(indent + "   " + "   fqn " + t.fqn("::"))         
      print(indent + "   " + "   typeid = " + t.type_id())


def print_package(pck,indent) :
   if pck.name() :
      print indent + "package " + pck.name()
   
   for tc in pck.typecollections :
      print_typecollection(tc, indent)
      
   for p in pck.packages :
      print_package(p, indent + "   ")      


print_package(root,"")

