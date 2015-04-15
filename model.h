#ifndef FRANCA_MODEL_H
#define FRANCA_MODEL_H


#include <cassert>
#include <vector>
#include <set>
#include <list>

#include <string>

#include <boost/algorithm/string.hpp>


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
   std::string fqn(const char* sep) const
   {
      std::string this_name = static_cast<const ElementT*>(this)->name();
      
      auto parent = static_cast<const ElementT*>(this)->parent_;         
      if (parent != 0)    
      {  
         std::string p = parent->fqn(sep);         
         return p + sep + this_name;       
      }
      else
         return this_name;
   }
      

protected:

   virtual ~parented()
   {
      // NOOP
   }
};


struct type : named_element, parented<type>
{
   /// user defined types
   type(const std::string& name, typecollection& parent);
   
   /// intrinsic types only
   type(const std::string& name);   
   
   virtual ~type();
   
   /// uniquely identifying type id string with all typedefs resolved
   virtual std::string type_id() const;
   
   /// trivial ordering by name
   bool operator<(const type& rhs) const;
   
   /// allow ordering by internal dependencies
   /// FIXME move to cpp
   virtual bool depends(const type& rhs) const
   {
      return fqn(".") == rhs.fqn(".");   // type always depends on itself
   }
   
   /// @return a list of typecollections this types refers to
   virtual std::set<const typecollection*> refers_to() const;
   
   typecollection* parent_;
};


extern std::set<type> intrinsic_types;     

   
struct struct_ : type
{
   typedef std::pair<type*, std::string> member_type;
   
   struct_(const std::string& name, typecollection& parent)
    : type(name, parent)
    , base_(0)
   {
      // NOOP
   }
      
   inline
   struct_& base()
   {
      if (!has_base())
         throw std::runtime_error("no baseclass provided");
         
      return *dynamic_cast<struct_*>(base_);
   }
   
   inline
   bool has_base()
   {
      return base_ != 0;
   }
   
   inline
   std::vector<member_type>& members()
   {
      return members_;
   }
   
   bool depends(const type& rhs) const
   {
      if (fqn(".") == rhs.fqn("."))
         return true;
         
      if (base_)
      {
         if (base_->fqn(".") == rhs.fqn("."))
            return true;
            
         if (base_->depends(rhs))
            return true;
      }
      
      for(auto iter = members_.begin(); iter != members_.end(); ++iter)
      {
         if (iter->first->fqn(".") == rhs.fqn(".") || iter->first->depends(rhs))
            return true;
      }
      
      return false;
   }
   
   std::set<const typecollection*> refers_to() const
   {
      std::set<const typecollection*> rc;
      
      if (base_ && base_->parent_ && base_->parent_ != parent_)      
         rc.insert(base_->parent_);      
      
      std::for_each(members_.begin(), members_.end(), [&rc, this](const member_type& mem){
         if (mem.first->parent_ && mem.first->parent_ != this->parent_)
            rc.insert(mem.first->parent_);              
      });      
      
      return rc;
   }
   
   type* base_;
   
   std::vector<member_type> members_;
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
   
   inline
   enumeration& base()
   {
      if (!has_base())
         throw std::runtime_error("no baseclass provided");
         
      return *dynamic_cast<enumeration*>(base_);
   }
   
   bool depends(const type& rhs) const
   {
      if (fqn(".") == rhs.fqn("."))
         return true;
         
      if (base_)
      {
         if (base_->fqn(".") == rhs.fqn("."))
            return true;
            
         return base_->depends(rhs);         
      }
         
      return false;
   }
   
   std::set<const typecollection*> refers_to() const
   {
      std::set<const typecollection*> rc;
      
      if (base_ && base_->parent_ && base_->parent_ != parent_)      
         rc.insert(base_->parent_);      
      
      return rc;
   }
   
   inline
   bool has_base()
   {
      return base_ != 0;
   }
   
   std::vector<enumerator> enumerators_;
   type* base_;
};


struct union_ : struct_
{
   union_(const std::string& name, typecollection& parent)
    : struct_(name, parent)
   {
      // NOOP
   }
   
   inline
   struct_& base()
   {
      if (!has_base())
         throw std::runtime_error("no baseclass provided");
         
      return *dynamic_cast<union_*>(base_);
   }
   
   std::string type_id() const;
};


struct typedef_ : type
{
   typedef_(const std::string& name, typecollection& parent)
    : type(name, parent)
    , real_type_(0)
   {
      // NOOP
   }
   
   std::string type_id() const;
   
   bool depends(const type& rhs) const
   {
      if (fqn(".") == rhs.fqn("."))
         return true;
         
      if (real_type_->fqn(".") == rhs.fqn("."))
         return true;
         
      return real_type_->depends(rhs);      
   }
   
   std::set<const typecollection*> refers_to() const
   {
      std::set<const typecollection*> rc;         
      
      if (real_type_->parent_ && real_type_->parent_ != parent_)
         rc.insert(real_type_->parent_);            
         
      return rc;
   }
   
   inline
   type& real_type() const
   {
      return *real_type_;
   }
   
   type* real_type_;
};


struct array : type
{
   array(const std::string& name, typecollection& parent)
    : type(name, parent)
    , element_type_(0)
   {      
      // NOOP
   }
   
   std::string type_id() const;
   
   bool depends(const type& rhs) const
   {
      if (fqn(".") == rhs.fqn("."))
         return true;
      
      if (element_type_->fqn(".") == rhs.fqn("."))
         return true;
         
      return element_type_->depends(rhs);
   }
   
   std::set<const typecollection*> refers_to() const
   {
      std::set<const typecollection*> rc;         
      
      if (element_type_->parent_)
         rc.insert(element_type_->parent_);            
      
      return rc;
   }
   
   inline
   const type& element_type() const
   {
      return *element_type_;
   }
   
   type* element_type_;
};


struct map : type
{
   map(const std::string& name, typecollection& parent)
    : type(name, parent)
    , key_type_(0)
    , value_type_(0)
   {
      // NOOP
   }
   
   std::string type_id() const;
   
   bool depends(const type& rhs) const
   {
      if (fqn(".") == rhs.fqn("."))
         return true;
         
      if (key_type_->fqn(".") == rhs.fqn("."))
         return true;
         
      if (value_type_->fqn(".") == rhs.fqn("."))
         return true;
         
      return key_type_->depends(rhs) || value_type_->depends(rhs);
   }
   
   inline
   const type& key_type() const
   {
      return *key_type_;
   }
   
   inline
   const type& value_type() const
   {
      return *value_type_;
   }
   
   std::set<const typecollection*> refers_to() const
   {
      std::set<const typecollection*> rc;   
            
      if (key_type_->parent_)
         rc.insert(key_type_->parent_);
         
      if (value_type_->parent_)
         rc.insert(value_type_->parent_);
      
      return rc;
   }
   
   type* key_type_;
   type* value_type_;
};


struct arg : named_element
{
   arg(const std::string& name, type* t)
    : named_element(name)
    , type_(t)
   {
      // NOOP
   }
   
   type& get_type()
   {
      return *type_;
   }
   
   type* type_;
};


struct method : named_element
{
   inline
   method(const std::string& name, interface& iface)
    : named_element(name)
    , interface_(iface)
   {
      // NOOP
   }
   
   inline
   interface& get_interface()
   {
      return interface_; 
   }
   
   inline
   enumeration& errors()
   {
      if (!has_errors())
         throw std::runtime_error("method does not declare errors");
         
      return *dynamic_cast<enumeration*>(errors_);
   }
   
   inline
   bool has_errors()
   {
      return errors_ != 0;
   }
   
   std::vector<arg> in_;
   std::vector<arg> out_;
   
   type* errors_;
   
private:
   interface& interface_;
};


struct fire_and_forget_method : named_element
{
   fire_and_forget_method(const std::string& name, interface& iface)
    : named_element(name)
    , interface_(iface)
   {
      // NOOP
   }
   
   inline
   interface& get_interface()
   {
      return interface_; 
   }
   
   std::vector<arg> args_;
   interface& interface_;
};


struct broadcast : named_element
{
   broadcast(const std::string& name, interface& iface)
    : named_element(name)
    , interface_(iface)
   {
      // NOOP
   }
   
   inline
   interface& get_interface()
   {
      return interface_; 
   }
   
   std::vector<arg> args_;
   interface& interface_;
};


struct attribute : named_element
{
   attribute(const std::string& name, type& t, interface& iface, bool readonly, bool no_subscriptions)
    : named_element(name)
    , type_(&t)
    , interface_(iface)
    , readonly_(readonly)
    , no_subscriptions_(no_subscriptions)
   {
      // NOOP
   }
   
   inline
   interface& get_interface()
   {
      return interface_; 
   }
   
   inline
   type& get_type()
   {
      return *type_;
   }
      
   type* type_;
   interface& interface_;
   
   bool readonly_;   
   bool no_subscriptions_;
};


struct typecollection : named_element, parented<typecollection>
{
   typecollection(const std::string& name);
   
   /// @name a (full)qualified (dot-separated) type name
   type* resolve(const std::string& name);
   
   inline
   package& get_package()
   {
      return *parent_;
   }
   
   void add(type& t);
   
   void add_dependency(const typecollection* coll);
   
   // FIXME make this a list of shared_ptr's
   std::vector<type*> types_;    // beware this is polymorphic, therefore we store pointers
   package* parent_;
   
   std::vector<const typecollection*> dependencies_;
};
   
   
struct interface : typecollection
{
   interface(const std::string& name, int major, int minor);
   
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
   package(const std::string& name);
   
   /// root package
   package();
   
   /// get root node
   package& root();
      
   inline
   bool is_root() const
   {
      return !parent_;
   }
   
   /// @return the parent package.
   inline
   package& get_package()
   {
      return *parent_;
   }
      
   ///@return a reference to the newly insered or already available package
   package& add_package(const std::string& packagename);
   
   interface& add_interface(const interface& iface);
   
   void add_import(const std::string& import)
   {
      if (!import.empty())    
         imports_.push_back(import);
   }
   
   typecollection& add_typecollection(const typecollection& coll);
      
   ///@return 0 if type cannot be found within the model
   template<typename IteratorT>
   type* resolve(IteratorT begin, IteratorT end, const std::string& typecoll, const std::string& type_name);
   
   std::list<typecollection> collections_;
   std::list<interface> interfaces_;   
   
   std::list<package> packages_;
   package* parent_;
   
   std::vector<std::string> imports_;
};


// ---------------------------------------------------------------------


template<typename IteratorT>
inline
type* package::resolve(IteratorT begin, IteratorT end, const std::string& typecoll, const std::string& type_name)
{
   package* p = this;
   
   if (begin != end)
   {      
      auto iter = std::find_if(packages_.begin(), packages_.end(), [begin](const package& pack){ return *begin == pack.name(); });
      
      if (iter != packages_.end())      
         return iter->resolve(++begin, end, typecoll, type_name);      
   }
   else
   {         
      {
         // look into collections
         
         auto iter = std::find_if(collections_.begin(), collections_.end(), [typecoll](const typecollection& coll){ return typecoll == coll.name(); });
         if (iter != collections_.end())
         {
            auto typeiter = std::find_if(iter->types_.begin(), iter->types_.end(), [type_name](const type* t){ return type_name == t->name(); });
            if (typeiter != iter->types_.end())
               return *typeiter;
               
            return nullptr;
         }
      }
      
      {
         // look into interfaces
         
         auto iter = std::find_if(interfaces_.begin(), interfaces_.end(), [typecoll](const interface& iface){ return typecoll == iface.name(); });
         if (iter != interfaces_.end())
         {
            auto typeiter = std::find_if(iter->types_.begin(), iter->types_.end(), [type_name](const type* t){ return type_name == t->name(); });
            if (typeiter != iter->types_.end())
               return *typeiter;
               
            return nullptr;
         }         
      }         
   }
   
   return nullptr;   
}


}   // model

}   // franca


#endif   // FRANCA_MODEL_H
