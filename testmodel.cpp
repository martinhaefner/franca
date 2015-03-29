#include <iostream>

#include "parser.h"
#include "model.h"
#include "builder.h"


namespace fm = franca::model;
namespace fp = franca::parser;


int main(int argc, const char** argv)
{
   if (argc < 2)
   {
      std::cerr << "no filename given" << std::endl;
      return EXIT_FAILURE;
   }
   
   fp::document doc = fp::parse(argv[1]);
   
   fm::package root;
   fm::package this_package = franca::builder::build(root, doc);
   
   return EXIT_SUCCESS;
}