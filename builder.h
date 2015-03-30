#ifndef FRANCA_BUILDER_H
#define FRANCA_BUILDER_H


#include "model.h"
#include "parser.h"


namespace franca
{


/// model builder
struct builder
{
   /// @return the currently parsed package
   static 
   model::package& build(model::package& root, const parser::document& parsetree);
};


}   // namespace franca


#endif   // FRANCA_BUILDER_H
