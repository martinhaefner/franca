#ifndef FRANCA_MODEL_H
#define FRANCA_MODEL_H


#include <cassert>
#include <vector>
#include <set>

#include <tuple>
#include <string>


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


}   // model

}   // franca


#endif   // FRANCA_MODEL_H
