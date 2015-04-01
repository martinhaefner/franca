#ifndef FRANCA_BUILDER_H
#define FRANCA_BUILDER_H


#include "model.h"
#include "parser.h"


namespace franca
{


/// model builder
struct builder
{
   /// @param root the (empty) root to parse the file to
   static 
   void parse_and_build(model::package& root, const char* franca_file);
   
   /// @return the currently parsed package
   static 
   model::package& build(model::package& root, const parser::document& parsetree);
   
   /// resolve types that had trouble to be resolved after first build,
   /// throws exception if not possible
   static
   void resolve_all_symbols(model::package& root);
};


}   // namespace franca


#endif   // FRANCA_BUILDER_H
