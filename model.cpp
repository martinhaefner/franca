#include "model.h"


namespace franca
{

namespace model
{


type::type(const std::string& name, typecollection& parent)
 : named_element(name)
 , parent_(&parent)
{
   parent_->types_.push_back(this);
}


interface::interface(const std::string& name, int major, int minor, package& parent)
 : typecollection(name, parent, false)
 , major_(major)
 , minor_(minor)
{
   parent_->interfaces_.push_back(this);
}


typecollection::typecollection(const std::string& name, package& parent, bool add)
 : named_element(name)
 , parent_(&parent)
{
   if (add)
      parent_->collections_.push_back(this);
}


type* typecollection::resolve(const std::string& name)
{   
   std::vector<std::string> tokens;
   boost::algorithm::split(tokens, name, boost::algorithm::is_any_of("."));
   
   if (tokens.size() == 1)
   {
      // either local typecollection or intrinsic type

      auto iter = intrinsic_types.find(name);
      if (iter != intrinsic_types.end())
         return &const_cast<type&>(*iter);
      
      auto iter2 = std::find_if(types_.begin(), types_.end(), [name](const type* t){ return name == t->name(); });
      if (iter2 != types_.end())
         return *iter2;
   }
   else if (tokens.size() > 1)
   {      
      // qualified name, either full qualified or local type
      // if just one name prefix: local package, either in here or in 
      // sibling typecollection or interface
      
      std::string type_name = *tokens.rbegin();
      tokens.pop_back();
      
      std::string typecoll = *tokens.rbegin();
      tokens.pop_back();
      
      // namespace lookup
      type* rc = parent_->resolve(tokens.begin(), tokens.end(), typecoll, type_name);      
      
      if (rc != nullptr)      
         return rc;
      
      // root lookup
      return parent_->root().resolve(tokens.begin(), tokens.end(), typecoll, type_name);                     
   }   
   
   return nullptr;
}


}   // model

}   // franca
