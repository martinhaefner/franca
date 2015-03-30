#ifndef FRANCA_MODEL_H
#define FRANCA_MODEL_H


#include <cassert>
#include <vector>
#include <set>

#include <tuple>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>


namespace franca
{
   
namespace model
{
   
struct package;
struct enumeration;
struct typecollection;
struct interface;


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
   
   /*struct_(const std::string& name, typecollection& parent, struct_& base)
    : type(name, parent)
    , base_(&base)
   {
      // NOOP
   }*/
   
   struct_* base_;
   
   std::vector<std::tuple<type*, std::string> > members_;
};


struct enumerator : named_element
{
   enumerator(const std::string& name, int value)
    : named_element(name)
    , value_(value)
   {
      // NOOP
   }
   
   int value_;
   
   enumeration* parent_;
};


struct enumeration : type
{
   enumeration(const std::string& name, typecollection& parent)
    : type(name, parent)
    , base_(0)
   {
      // NOOP
   }
   
   std::vector<enumerator> enumerators_;
   enumeration* base_;
};


struct arg : named_element
{
   arg(const std::string& name, type* t)
    : named_element(name)
    , type_(t)
   {
      // NOOP
   }
   
   type* type_;
};


struct method : named_element
{
   method(const std::string& name, interface& iface)
    : named_element(name)
    , interface_(iface)
   {
      // NOOP
   }
   
   interface& get_interface()
   {
      return interface_; 
   }
   
   std::vector<arg> in_;
   std::vector<arg> out_;
   
   enumeration* errors_;
   interface& interface_;
};


struct fire_and_forget_method : named_element
{
   fire_and_forget_method(const std::string& name)
    : named_element(name)
   {
      // NOOP
   }
   
   std::vector<arg> args_;
};


struct broadcast : named_element
{
   broadcast(const std::string& name)
    : named_element(name)
   {
      // NOOP
   }
   
   std::vector<arg> args_;
};


struct attribute : named_element
{
   attribute(const std::string& name, type& t, bool readonly, bool no_subscriptions)
    : named_element(name)
    , type_(&t)
    , readonly_(readonly)
    , no_subscriptions_(no_subscriptions)
   {
      // NOOP
   }
   
   type* type_;
   
   bool readonly_;   
   bool no_subscriptions_;
};


struct typecollection : named_element, parented<typecollection>
{
   typecollection(const std::string& name, package& parent, bool add = true);
   
   /// @name a (full)qualified type name
   type* resolve(const std::string& name);
   
   std::vector<type*> types_;   // FIXME make this a vector<type> instead
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
   std::vector<broadcast> broadcasts_;
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
      
   template<typename IteratorT>
   type* resolve(IteratorT begin, IteratorT end, const std::string& typecoll, const std::string& type_name);
   
   std::vector<typecollection*> collections_;
   std::vector<interface*> interfaces_;
   
   std::vector<package*> packages_;
   package* parent_;
};


// ---------------------------------------------------------------------


template<typename IteratorT>
inline
type* package::resolve(IteratorT begin, IteratorT end, const std::string& typecoll, const std::string& type_name)
{
   package* p = this;
   
   if (begin != end)
   {      
      auto iter = std::find_if(packages_.begin(), packages_.end(), [begin](const package* pack){ return *begin == pack->name(); });
      
      if (iter != packages_.end())      
         return (*iter)->resolve(++begin, end, typecoll, type_name);      
   }
   else
   {      
      auto iter = std::find_if(collections_.begin(), collections_.end(), [typecoll](const typecollection* coll){ return typecoll == coll->name(); });
      if (iter != collections_.end())
      {
         auto typeiter = std::find_if((*iter)->types_.begin(), (*iter)->types_.end(), [type_name](const type* t){ return type_name == t->name(); });
         if (typeiter != (*iter)->types_.end())
            return *typeiter;
      }
      else
      {      
         auto iter = std::find_if(interfaces_.begin(), interfaces_.end(), [typecoll](const interface* iface){ return typecoll == iface->name(); });
         if (iter != interfaces_.end())
         {
            auto typeiter = std::find_if((*iter)->types_.begin(), (*iter)->types_.end(), [type_name](const type* t){ return type_name == t->name(); });
            if (typeiter != (*iter)->types_.end())
               return *typeiter;
         }
      }
   }
   
   return nullptr;   
}


}   // model

}   // franca


#endif   // FRANCA_MODEL_H
