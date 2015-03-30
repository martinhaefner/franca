#include "builder.h"

#include <memory>


namespace fp = franca::parser;
namespace fm = franca::model;


struct method_error_builder : public boost::static_visitor<fm::enumeration*>
{
   method_error_builder(fm::method& m)
    : method_(m)
   {
      // NOOP
   }
   
   fm::enumeration* operator()(const std::string& err) const
   {
      fm::type* t = method_.get_interface().resolve(err);
      
      if (t)
         return dynamic_cast<fm::enumeration*>(t);      
      
      throw std::runtime_error("unknown error enum type");      
   }
   
   fm::enumeration* operator()(const fp::extended_error& err) const
   {
      std::unique_ptr<fm::enumeration> e(new fm::enumeration(method_.name() + "_errors", method_.get_interface()));
      
      if (err.base_)
      {
         fm::type* t = method_.get_interface().resolve(*err.base_);
         if (t)
         {
            e->base_ = dynamic_cast<fm::enumeration*>(t);
            
            if (e->base_ == 0)
               throw std::runtime_error("invalid base error type");      
         }
         else
            throw std::runtime_error("unknown base error enum type");      
      }
      
      // iterate over all enum values
      for (auto iter = err.values_.begin(); iter != err.values_.end(); ++iter)
      {
         // FIXME check value - maybe use set instead here!
         // if can be resolved add the element to the struct
         e->enumerators_.push_back(fm::enumerator(iter->name_, iter->value_));         
      }
      
      return e.release();
   }
      
   fm::method& method_;
};


// ---------------------------------------------------------------------


struct typecollection_builder : public boost::static_visitor<void>
{
   typecollection_builder(fm::typecollection& coll)
    : coll_(coll)
   {
      // NOOP
   }
   
   void operator()(const fp::struct_& s) const
   {       
      // FIXME must distinguish if type is given two times?!      
      
      // search for struct in model, if already there, do nothing
      {
         auto iter = std::find_if(coll_.types_.begin(), coll_.types_.end(), [&s](const fm::type* typ) {
            return s.name_ == typ->name();
         });
      
         if (iter != coll_.types_.end())
            return;
      }
            
      std::unique_ptr<fm::struct_> new_s(new fm::struct_(s.name_, coll_));
      
      // search for parent if apropriate
      if (s.base_)
      {
         // if not available drop struct again
         // FIXME distinguish first and second pass
         fm::type* base = coll_.resolve(*s.base_);
         if (!base)
            return;
            
         fm::struct_* base_struct = dynamic_cast<fm::struct_*>(base);
         if (base_struct)
         {
            new_s->base_ = base_struct;
         }
         else
            throw std::runtime_error("invalid struct base type");
      }
      
      // iterate over all elements of struct. 
      for (auto iter = s.values_.begin(); iter != s.values_.end(); ++iter)
      {
         std::cout << iter->type_ << std::endl;
         // resolve element
         fm::type* t = coll_.resolve(iter->type_);
         if (t)
         {
            // if can be resolved add the element to the struct
            new_s->members_.push_back(std::make_tuple(t, iter->name_));
         }
         else
            throw std::runtime_error("invalid struct member type");            
      }
   
      // keep it
      new_s.release();
   }
   
   
   void operator()(const fp::enumeration& e) const
   {
      // search for enum in model, if already there, do nothing
      {
         auto iter = std::find_if(coll_.types_.begin(), coll_.types_.end(), [&e](const fm::type* typ) {
            return e.name_ == typ->name();
         });
      
         if (iter != coll_.types_.end())
            return;
      }
      
      std::unique_ptr<fm::enumeration> new_e(new fm::enumeration(e.name_, coll_));
      
      // search for parent if apropriate
      if (e.base_)
      {
         // if not available drop struct again
         // FIXME distinguish first and second pass
         fm::type* base = coll_.resolve(*e.base_);
         if (!base)
            return;
            
         fm::enumeration* base_enum = dynamic_cast<fm::enumeration*>(base);
         if (base_enum)
         {
            new_e->base_ = base_enum;
         }
         else
            throw std::runtime_error("invalid enum base type");
      }
      
      // iterate over all enum values
      for (auto iter = e.values_.begin(); iter != e.values_.end(); ++iter)
      {
         // FIXME check value - maybe use set instead here!
         // if can be resolved add the element to the struct
         new_e->enumerators_.push_back(fm::enumerator(iter->name_, iter->value_));         
      }
   
      // keep it
      new_e.release();
   }
   
   fm::typecollection& coll_;
};


// ---------------------------------------------------------------------


struct interface_builder : typecollection_builder
{
   using typecollection_builder::operator();
   
   
   interface_builder(fm::interface& i)
    : typecollection_builder(i)
    , i_(i)
   {
      // NOOP
   }   
   
   
   void operator()(const fp::method& s) const
   {
      fm::method meth(s.name_, i_);
          
      // in
      for(auto iter = s.in_.begin(); iter != s.in_.end(); ++iter)
      {
         fm::type* t = i_.resolve(iter->type_);
         if (!t)
            return;
            
         meth.in_.push_back(fm::arg(iter->name_, t));
      }
      
      // out      
      for(auto iter = s.out_.begin(); iter != s.out_.end(); ++iter)
      {
         fm::type* t = i_.resolve(iter->type_);
         if (!t)
            return;
            
         meth.out_.push_back(fm::arg(iter->name_, t));
      }
      
      // errors
      if (s.error_)      
      {
         meth.errors_ = boost::apply_visitor(method_error_builder(meth), *s.error_);
         
         if (!meth.errors_)
            return;
      }
      
      i_.methods_.push_back(meth);
   }
   
   
   void operator()(const fp::fire_and_forget_method& s) const
   {
      fm::fire_and_forget_method n_meth(s.name_);
            
      for(auto iter = s.in_.begin(); iter != s.in_.end(); ++iter)
      {
         fm::type* t = i_.resolve(iter->type_);
         if (!t)
            return;
            
         n_meth.args_.push_back(fm::arg(iter->name_, t));
      }
      
      i_.ff_methods_.push_back(n_meth);
   }
   
   
   void operator()(const fp::broadcast& s) const
   {
      fm::broadcast bc(s.name_);
            
      for(auto iter = s.args_.begin(); iter != s.args_.end(); ++iter)
      {
         fm::type* t = i_.resolve(iter->type_);
         if (!t)
            return;
            
         bc.args_.push_back(fm::arg(iter->name_, t));
      }
      
      i_.broadcasts_.push_back(bc);
   }
   
   
   void operator()(const fp::attribute& s) const
   {
      // FIXME check double name
      
      fm::type* t = i_.resolve(s.type_);
      
      if (t)
      {
         fm::attribute attr(s.name_, *t, s.readonly_, s.no_subscriptions_);         
      }
      else
         throw std::runtime_error("invalid attribute type");      
   }
      
      
   fm::interface& i_;
};


// ---------------------------------------------------------------------


struct package_builder : public boost::static_visitor<void>
{
   package_builder(fm::package& package)
    : package_(package)
   {
      // NOOP
   }
   
   void operator()(const fp::interface& i) const
   {
      fm::interface mi(i.name_, i.version_.major_, i.version_.minor_);
      
      std::for_each(i.parseitems_.begin(), i.parseitems_.end(), [this,mi]( const fp::interface_item_type& item) {
         boost::apply_visitor(interface_builder(this->package_.add_interface(mi)), item);
      });
   }
   
   void operator()(const fp::typecollection& tc) const
   {
      fm::typecollection mtc(tc.name_);
      
      std::for_each(tc.parseitems_.begin(), tc.parseitems_.end(), [this,mtc]( const fp::tc_item_type& item) {
         boost::apply_visitor(typecollection_builder(this->package_.add_typecollection(mtc)), item);
      });
   }
   
   fm::package& package_;
};


// ---------------------------------------------------------------------


/*static*/ 
fm::package& franca::builder::build(fm::package& root, const fp::document& parsetree)
{
   fm::package* parent = &root;
   
   std::for_each(parsetree.package_.begin(), parsetree.package_.end(), [&parent](const std::string& str) {
      fm::package pck(str);
      parent = &parent->add_package(pck);      
   });
   
   std::for_each(parsetree.parseitems_.begin(), parsetree.parseitems_.end(), [parent]( const fp::doc_item_type& item) {
      boost::apply_visitor(package_builder(*parent), item);
   });
   
   return root;
}
