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
   
   std::cout << "Parsing..." << std::endl;
   std::vector<std::string> includes;
   fp::document doc = fp::parse(argv[1], includes);
   
   std::cout << "Building model..." << std::endl;
   fm::package root;
   fm::package& this_package = franca::builder::build(root, doc, "");
   
   std::cout << "Resolving..." << std::endl;
   franca::builder::resolve_all_symbols(root);
   
   std::cout << root.name() << root.packages_.front().name() << "." << root.packages_.front().packages_.front().name() << std::endl;
   
   return EXIT_SUCCESS;
}
