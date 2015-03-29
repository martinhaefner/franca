#ifndef FRANCA_BUILDER_H
#define FRANCA_BUILDER_H


#include "model.h"
#include "parser.h"


namespace franca
{


struct builder
{
   static 
   model::package& build(model::package& root, const parser::document& parsetree);
};


}   // namespace franca


#endif   // FRANCA_BUILDER_H
