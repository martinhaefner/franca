#include <iostream>

#include "parser.h"
#include "model.h"
#include "builder.h"


namespace fm = franca::model;
namespace fp = franca::parser;


void check_depends(fm::interface& if_)
{
   for (auto& dep : if_.dependencies_)
   {
      std::cout << std::hex << dep << " -> " << dep->fqn("/") << std::endl;
   }
}


void check_depends(fm::typecollection& coll)
{
   for (auto& dep : coll.dependencies_)
   {
      std::cout << std::hex << dep << " -> " << dep->fqn("/") << std::endl;
   }
}


void check_depends(fm::package& p)
{
   for (auto& tc : p.collections_)
      check_depends(tc);
   
   for (auto& if_ : p.interfaces_)
      check_depends(if_);
   
   for(auto& pck : p.packages_)
      check_depends(*pck);
}


int main(int argc, const char** argv)
{
   if (argc < 2)
   {
      std::cerr << "no filename given" << std::endl;
      return EXIT_FAILURE;
   }
   
   fm::package root;
   franca::builder::parse_and_build(root, argv[1]);
   
   for(auto& p : root.packages_)
      check_depends(*p);
      
   std::cout << root.name() << root.packages_.front()->name() << "." << root.packages_.front()->packages_.front()->name() << std::endl;
   
   return EXIT_SUCCESS;
}
