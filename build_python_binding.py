#!/usr/bin/env python


from distutils.core import setup
from distutils.extension import Extension
from os import getcwd


setup(name="PackageName",
   ext_modules = [
      Extension("franca", ["python_binding.cpp"], 
         libraries = ["boost_python", "franca"],
         library_dirs = [getcwd() + "/build"],
         extra_compile_args = ["-std=c++11"]
      )
   ]
)
