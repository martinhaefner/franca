#ifndef FRANCA_BUILDER_H
#define FRANCA_BUILDER_H


#include "model.h"
#include "parser.h"


namespace franca
{


/// model builder
struct builder
{
   /**
    * This is the main parser function. Use this function for creating a
    * franca model.
    * 
    * @param root     the (empty) root to parse the file to
    * @param includes a list of search directories for imported files' lookup
    */
   static 
   void parse_and_build(model::package& root, const char* franca_file, const std::vector<std::string>& includes);
   
   /// @param root the (empty) root to parse the file to
   static 
   void parse_and_build(model::package& root, const char* franca_file);
   
   /**
    * @param filter The filter to apply for building the model (from namespace import statement). 
    *        Beware that filter does currently not implement a full regular expression. Moreover, the
    *        filter may only be applied until the depth of interface or typecollection, i.e. it is 
    *        not possible to import a distinct type from a typecollection.
    * @return the currently parsed package
    */
   static 
   model::package& build(model::package& root, const parser::document& parsetree, const std::string& filter);
   
   /// resolve types that had trouble to be resolved after first build,
   /// throws exception if not possible
   static
   void resolve_all_symbols(model::package& root);
   
   /// sort all types within the given package recursively
   static
   void sort_types(model::package& pkg);
   
   /// create dependencies between type collections
   static
   void create_typecollection_dependencies(model::package& pkg);
};


}   // namespace franca


#endif   // FRANCA_BUILDER_H
