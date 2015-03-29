#ifndef FRANCA_MODEL_H
#define FRANCA_MODEL_H


#include <cassert>
#include <vector>
#include <set>

#include <tuple>
#include <string>

#include <boost/algorithm/string.hpp>


namespace franca
{
   
namespace model
{
   
struct package;
struct Enumeration;
struct typecollection;


struct named_element
{
   named_element(const std::string& name)
    : name_(name)
   {
      // NOOP
   }
   
   inline
   std::string name() const
   {
      return name_;
   }
   
protected:
   
   inline
   ~named_element() 
   {
      // NOOP
   }
   
   std::string name_;
};


template<typename ElementT>
struct parented
{
   inline
   std::string fqn(const char* sep = "::") const
   {
      std::string this_name = static_cast<const ElementT*>(this)->name();
      
      auto parent = static_cast<const ElementT*>(this)->parent_;
      
      if (parent)
         return parent->fqn(sep) + sep + this_name; 
      else
         return this_name;
   }
};


struct type : named_element, parented<type>
{
   /// user defined types
   type(const std::string& name, typecollection& parent);
   
   /// intrinsic types only
   type(const std::string& name)
    : named_element(name)
    , parent_(0)
   {
      // NOOP
   }
   
   virtual ~type()
   {
      // NOOP
   }
   
   bool operator<(const type& rhs) const
   {
      return name_ < rhs.name_;
   }
   
   typecollection* parent_;
};


static
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

   
struct struct_ : type
{
   struct_(const std::string& name, typecollection& parent)
    : type(name, parent)
    , base_(0)
   {
      // NOOP
   }
   
   struct_(const std::string& name, typecollection& parent, struct_& base)
    : type(name, parent)
    , base_(&base)
   {
      // NOOP
   }
   
   struct_* base_;
   
   std::vector<std::tuple<type*, std::string> > members_;
};


struct enumerator : named_element
{
   int value_;
   
   Enumeration* parent_;
};


struct enumeration : type
{
   std::vector<enumerator> enumerators_;
   enumeration* base_;
};


struct arg : named_element
{
   type* type_;
};


struct method : named_element
{
   std::vector<arg> in_;
   std::vector<arg> out_;
   
   enumeration errors_;
};


struct fire_and_forget_method : named_element
{
   std::vector<arg> args_;
};


struct broadcast : named_element
{
   std::vector<arg> args_;
};


struct attribute : named_element
{
   type type_;
   
   bool readonly_;   
   bool no_subscriptions_;
};


struct typecollection : named_element, parented<typecollection>
{
   typecollection(const std::string& name, package& parent, bool add = true);
   
   /// @name a (full)qualified type name
   type* resolve(const std::string& name);
   
   std::vector<type*> types_;
   package* parent_;
};
   
   
struct interface : typecollection
{
   interface(const std::string& name, int major, int minor, package& parent);
   
   int major_;
   int minor_;
   
   std::vector<attribute> attrs_;
   std::vector<method> methods_;
   std::vector<fire_and_forget_method> ff_methods_;
   std::vector<broadcast> events_;
};


struct package : named_element, parented<package>
{
   /// normal namespace item
   package(const std::string& name, package& parent)
    : named_element(name)
    , parent_(&parent)
   {
      assert(name.find(".") == std::string::npos);
      parent_->packages_.push_back(this);
   }
   
   /// root package
   package()
    : named_element("")
    , parent_(0)
   {
      // NOOP
   }
   
   /// get root node
   package& root()
   {
      package* rc = this;
      
      while(rc->parent_)
         rc = rc->parent_;
         
      return *rc;
   }
   
   type* find(const std::string& fqn) { /*FIXME*/ return nullptr; }
   
   std::vector<typecollection*> collections_;
   std::vector<interface*> interfaces_;
   
   std::vector<package*> packages_;
   package* parent_;
};


// ---------------------------------------------------------------------


inline
type::type(const std::string& name, typecollection& parent)
 : named_element(name)
 , parent_(&parent)
{
   parent_->types_.push_back(this);
}


inline
interface::interface(const std::string& name, int major, int minor, package& parent)
 : typecollection(name, parent, false)
 , major_(major)
 , minor_(minor)
{
   parent_->interfaces_.push_back(this);
}


inline
typecollection::typecollection(const std::string& name, package& parent, bool add)
 : named_element(name)
 , parent_(&parent)
{
   if (add)
      parent_->collections_.push_back(this);
}


#if 0
inline
Type* package::find(const std::string& type_name)
{
   std::vector<std::string> tokens = split(type_name, ".");
   
   // last token is type's name
   std::string type = tokens.rbegin();
   tokens.pop_back();
   
   // this token - if available - is typecollection or interface 
   std::string tc_name;
   if (tokens.size() > 0)
   {
      tc_name = tokens.rbegin();
      tokens.pop_back();
   }
   
   // rest of tokens are package names
   package* p = nullptr;
   
   if (tokens.size() > 0)
   {
      // first search in this namespace, then in root   
      for (int i=0; i<2 && p == nullptr; ++i)
      {
         p = (i==0 ? this : root());
         
         for(auto iter = tokens.begin(); iter != tokens.end(); ++iter)
         {
            auto rc = std::find(p->packages_.begin(), p->packages_->end(), [iter](const package& package) {
               return package->name_ == *iter;
            });
            
            if (rc == p->packages_.end())
            {
               p = nullptr;
            }
            else
               p = *rc;
         }
      }
      
      if (p == nullptr)
      throw std::runtime_error("type not found");
   }
   
   if (p == root() && tc_name.empty())
   {
      auto iter = intrinsic_types.find(type);
      if (iter != intrinsic_types.end())
         return &iter; 
   }
   
   if (tc.empty())
   {
   auto tc = std::find(p->packages_.begin(), p->packages_->end(), [iter](const package& package) {
      return package->name_ == *iter;
   });
}
#endif


inline
type* typecollection::resolve(const std::string& name)
{
   std::vector<std::string> tokens;
   boost::algorithm::split(tokens, name, boost::algorithm::is_any_of("."));
   
   if (tokens.size() == 0)
   {
      // either local typecollection or intrinsic type

      auto iter = intrinsic_types.find(name);
      if (iter != intrinsic_types.end())
         return &const_cast<type&>(*iter);
      
      auto iter2 = std::find_if(types_.begin(), types_.end(), [name](const type* t){ return name == t->name(); });
      if (iter2 != types_.end())
         return *iter2;
   }
   else
   {
      // qualified name, either full qualified or local type
      // if just one name prefix: local package, either in here or in 
      // sibling typecollection or interface
   }
   
   // now search global
   
   throw std::runtime_error("type not found");
}


}   // model

}   // franca


#endif   // FRANCA_MODEL_H
