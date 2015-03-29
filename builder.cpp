#include "builder.h"

#include <memory>


namespace fp = franca::parser;
namespace fm = franca::model;


struct interface_builder : public boost::static_visitor<void>
{
   interface_builder(fm::interface& i)
    : i_(i)
   {
      // NOOP
   }
   
   void operator()(const fp::method& s) const
   {
      // FIXME 
   }
   
   void operator()(const fp::fire_and_forget_method& s) const
   {
      // FIXME 
   }
   
   void operator()(const fp::broadcast& s) const
   {
      // FIXME 
   }
   
   void operator()(const fp::attribute& s) const
   {
      // FIXME 
   }
   
   void operator()(const fp::struct_& s) const
   {
      // FIXME 
   }
   
   void operator()(const fp::enumeration& e) const
   {
      // FIXME 
   }
   
   fm::interface& i_;
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
            throw std::runtime_error("invalid base type");
      }
      
      // iterate over all elements of struct. 
      for (auto iter = s.values_.begin(); iter != s.values_.end(); ++iter)
      {
         // resolve element
         fm::type* t = coll_.resolve(iter->type_);
         if (t)
         {
            // if can be resolved add the element to the struct
            new_s->members_.push_back(std::make_tuple(t, iter->name_));
         }
         else
            return;
      }
   
      // keep it
      new_s.release();
   }
   
   
   void operator()(const fp::enumeration& e) const
   {
      // FIXME 
   }
   
   fm::typecollection& coll_;
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
      auto mi = new fm::interface(i.name_, i.version_.major_, i.version_.minor_, package_);
      
      std::for_each(i.parseitems_.begin(), i.parseitems_.end(), [mi]( const fp::interface_item_type& item) {
         boost::apply_visitor(interface_builder(*mi), item);
      });
   }
   
   void operator()(const fp::typecollection& tc) const
   {
      auto mtc = new fm::typecollection(tc.name_, package_);
      
      std::for_each(tc.parseitems_.begin(), tc.parseitems_.end(), [mtc]( const fp::tc_item_type& item) {
         boost::apply_visitor(typecollection_builder(*mtc), item);
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
      parent = new fm::package(str, *parent);
   });
   
   std::for_each(parsetree.parseitems_.begin(), parsetree.parseitems_.end(), [parent]( const fp::doc_item_type& item) {
      boost::apply_visitor(package_builder(*parent), item);
   });
   
   return root;
}
