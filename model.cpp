#include "model.h"


namespace franca
{

namespace model
{


std::set<type> intrinsic_types = {
     type("Int8")
   , type("Int16")
   , type("Int32")
   , type("Int64")
   , type("UInt8")
   , type("UInt16")
   , type("UInt32")
   , type("UInt64")
   , type("Float")
   , type("String")
};


type::type(const std::string& name, typecollection& parent)
 : named_element(name)
 , parent_(&parent) 
{
   parent.types_.push_back(this);
}


type::type(const std::string& name)
 : named_element(name)
 , parent_(0)   // intrinsic types
{
   // NOOP
}


type::~type()
{
   // NOOP
}


bool type::operator<(const type& rhs) const
{
   return name_ < rhs.name_;
}


// ---------------------------------------------------------------------


interface::interface(const std::string& name, int major, int minor)
 : typecollection(name)
 , major_(major)
 , minor_(minor)
{
   // NOOP
}


// ---------------------------------------------------------------------


typecollection::typecollection(const std::string& name)
 : named_element(name)
 , parent_(0)
{
   // NOOP
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
      
      //std::cout << "Searching for " << type_name << " in collection " << typecoll;
      // namespace lookup
      type* rc = parent_->resolve(tokens.begin(), tokens.end(), typecoll, type_name);      
      
      if (rc != nullptr)         
         return rc;      
      
      // root lookup
      return parent_->root().resolve(tokens.begin(), tokens.end(), typecoll, type_name);                           
   }   
   
   return nullptr;
}


// ---------------------------------------------------------------------


package::package(const std::string& name)
 : named_element(name)
 , parent_(0)
{
   assert(name.find(".") == std::string::npos);      
}


package::package()
 : named_element("")
 , parent_(0)
{
   // NOOP
}


package& package::root()
{   
   package* rc = this;
   
   while(rc->parent_)
      rc = rc->parent_;
      
   return *rc;
}   


package& package::add_package(const package& pck)
{      
   packages_.push_back(pck);
   packages_.back().parent_ = this;
   
   return packages_.back();
}


interface& package::add_interface(const interface& iface)
{
   interfaces_.push_back(iface);
   interfaces_.back().parent_ = this;
   
   return interfaces_.back();
}


typecollection& package::add_typecollection(const typecollection& coll)
{
   collections_.push_back(coll);
   collections_.back().parent_ = this;
   
   return collections_.back();
}


}   // model

}   // franca
