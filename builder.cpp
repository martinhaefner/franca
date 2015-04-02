#include "builder.h"

#include <memory>
#include <sstream>
#include <libgen.h>

#include "parser.h"


namespace fp = franca::parser;
namespace fm = franca::model;


namespace /* anonymous */
{
   
// forward decls
fm::type* internal_resolve_unresolved(fm::type*, fm::typecollection&);

void parse_recursive(std::vector<std::pair<std::string, fp::document> >& docs, const char* franca_file, const std::vector<std::string>& includes, const std::string& filter);


inline
int _stl_strncmp(const std::string& s1, const std::string& s2, size_t len)
{   
   return s1.compare(0, len, s2, 0, len);
}

   
/**
 * placeholder during model generation if requested type is not yet
 * available, will hopefully be replaced on second pass through model.
 */
struct unresolved : fm::type
{
   unresolved(const std::string& name)
    : fm::type(name)
   {
      // NOOP
   }
   
   
   std::string type_id() const
   {
      std::ostringstream os;
      os << "unresolved<" << name() << ">";
      
      return os.str();
   }
};


void internal_resolve_unresolved(fm::enumeration& e)
{   
   e.base_ = internal_resolve_unresolved(e.base_, *e.parent_);
}


void internal_resolve_unresolved(fm::union_& s)
{   
   std::for_each(s.members_.begin(), s.members_.end(), [&s](fm::union_::member_type& m){ 
      unresolved* udef = dynamic_cast<unresolved*>(m.first);
      if (udef)
      {
         fm::type* real = internal_resolve_unresolved(m.first, *s.parent_); 
         
         // check for duplicate type within union
         if (std::find_if(s.members_.begin(), s.members_.end(), [real, &m](const fm::union_::member_type& it) {
            return &m == &it ? false : real->type_id() == it.first->type_id();
         }) != s.members_.end())
            throw std::runtime_error("union type may not contain same type multiple times");
            
         m.first = real;
      }
   });   

   s.base_ = internal_resolve_unresolved(s.base_, *s.parent_);   
}


void internal_resolve_unresolved(fm::typedef_& t)
{   
   t.real_type_ = internal_resolve_unresolved(t.real_type_, *t.parent_);      
}


void internal_resolve_unresolved(fm::array& a)
{   
   a.element_type_ = internal_resolve_unresolved(a.element_type_, *a.parent_);      
}


void internal_resolve_unresolved(fm::map& m)
{   
   m.key_type_ = internal_resolve_unresolved(m.key_type_, *m.parent_);      
   m.value_type_ = internal_resolve_unresolved(m.value_type_, *m.parent_);      
}


void internal_resolve_unresolved(fm::struct_& s)
{   
   std::for_each(s.members_.begin(), s.members_.end(), [&s](fm::struct_::member_type& m){ m.first = internal_resolve_unresolved(m.first, *s.parent_); });   
      
   s.base_ = internal_resolve_unresolved(s.base_, *s.parent_);   
}


fm::type* internal_resolve_unresolved(fm::type* t, fm::typecollection& context)
{
   if (t)
   {
      unresolved* udef = dynamic_cast<unresolved*>(t);
      
      if (udef)
      {   
         fm::type* rc = context.resolve(udef->name());
         
         if (rc)
         {
            delete t;         
            return rc;
         }
         else
            throw std::runtime_error("cannot resolve type");
      }
   }
 
   return t;
}


void internal_resolve_unresolved(fm::attribute& a)
{
   a.type_ = internal_resolve_unresolved(a.type_, a.interface_);   
}


void internal_resolve_unresolved(fm::method& m)
{
   std::for_each(m.in_.begin(), m.in_.end(), [&m](fm::arg& a){ a.type_ = internal_resolve_unresolved(a.type_, m.get_interface()); });
   std::for_each(m.out_.begin(), m.out_.end(), [&m](fm::arg& a){ a.type_ = internal_resolve_unresolved(a.type_, m.get_interface()); });
   
   m.errors_ = internal_resolve_unresolved(m.errors_, m.get_interface());
}


void internal_resolve_unresolved(fm::fire_and_forget_method& m)
{
   std::for_each(m.args_.begin(), m.args_.end(), [&m](fm::arg& a){ a.type_ = internal_resolve_unresolved(a.type_, m.interface_); });
}


void internal_resolve_unresolved(fm::broadcast& b)
{
   std::for_each(b.args_.begin(), b.args_.end(), [&b](fm::arg& a){ a.type_ = internal_resolve_unresolved(a.type_, b.interface_); });
}


void internal_resolve_unresolved_tc(fm::typecollection& coll)
{
   std::for_each(coll.types_.begin(), coll.types_.end(), [](fm::type* t){ 
      
      union {
         fm::struct_* s;         
         fm::enumeration* e;
         fm::union_* u;
         fm::typedef_* t;
         fm::array* a;
         fm::map* m;
      } u;       
      
      if ((u.u = dynamic_cast<fm::union_*>(t)) != 0)      
      {                                 
         internal_resolve_unresolved(*u.u);                   
      }
      else if ((u.s = dynamic_cast<fm::struct_*>(t)) != 0)      
      {      
         internal_resolve_unresolved(*u.s);             
      }
      else if ((u.e = dynamic_cast<fm::enumeration*>(t)) != 0)      
      {                                 
         internal_resolve_unresolved(*u.e);                   
      }       
      else if ((u.t = dynamic_cast<fm::typedef_*>(t)) != 0)      
      {                                 
         internal_resolve_unresolved(*u.t);                   
      }       
      else if ((u.a = dynamic_cast<fm::array*>(t)) != 0)      
      {                                 
         internal_resolve_unresolved(*u.a);                   
      }       
      else if ((u.m = dynamic_cast<fm::map*>(t)) != 0)      
      {                                 
         internal_resolve_unresolved(*u.m);                   
      }       
      
      
   });
}


void internal_resolve_unresolved(fm::interface& iface)
{
   internal_resolve_unresolved_tc(iface);

   std::for_each(iface.attrs_.begin(), iface.attrs_.end(), [](fm::attribute& a){ internal_resolve_unresolved(a); });
   std::for_each(iface.methods_.begin(), iface.methods_.end(), [](fm::method& m){ internal_resolve_unresolved(m); });
   std::for_each(iface.ff_methods_.begin(), iface.ff_methods_.end(), [](fm::fire_and_forget_method& m){ internal_resolve_unresolved(m); });
   std::for_each(iface.broadcasts_.begin(), iface.broadcasts_.end(), [](fm::broadcast& b){ internal_resolve_unresolved(b); });         
}


void internal_resolve_unresolved(fm::package& pack)
{
   std::for_each(pack.collections_.begin(), pack.collections_.end(), [](fm::typecollection& coll){ internal_resolve_unresolved_tc(coll); });
   std::for_each(pack.interfaces_.begin(), pack.interfaces_.end(), [](fm::interface& iface){ internal_resolve_unresolved(iface); });
   
   std::for_each(pack.packages_.begin(), pack.packages_.end(), [](fm::package& pck){ internal_resolve_unresolved(pck); });
}


// ---------------------------------------------------------------------


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
            if (dynamic_cast<fm::enumeration*>(t) == 0)
               throw std::runtime_error("invalid base error type");                                 
         }
         else
            t = new unresolved(*err.base_);
         
         e->base_ = t;
      }
      
      // iterate over all enum values
      for (auto iter = err.values_.begin(); iter != err.values_.end(); ++iter)
      {
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
      std::unique_ptr<fm::struct_> new_s(new fm::struct_(s.name_, coll_));
      
      // search for parent if apropriate
      if (s.base_)
      {         
         fm::type* base = coll_.resolve(*s.base_);
                  
         if (base)
         {
            if (dynamic_cast<fm::struct_*>(base) == 0)
               throw std::runtime_error("invalid struct base type");
         }
         else         
            base = new unresolved(*s.base_);         
                     
         new_s->base_ = base;         
      }
      
      // iterate over all elements of struct. 
      for (auto iter = s.values_.begin(); iter != s.values_.end(); ++iter)
      {         
         // resolve element
         fm::type* t = coll_.resolve(iter->type_);
         if (!t)             
            t = new unresolved(iter->type_);            
                     
         // if can be resolved add the element to the struct
         new_s->members_.push_back(std::make_pair(t, iter->name_));
      }
   
      // keep it
      new_s.release();
   }
   
   
   void operator()(const fp::enumeration& e) const
   {
      std::unique_ptr<fm::enumeration> new_e(new fm::enumeration(e.name_, coll_));
      
      // search for parent if apropriate
      if (e.base_)
      {         
         fm::type* base = coll_.resolve(*e.base_);
         
         if (base)
         {
            if (dynamic_cast<fm::enumeration*>(base) == 0)
               throw std::runtime_error("invalid enum base type");
         }
         else
            base = new unresolved(*e.base_);            
                     
         new_e->base_ = base;         
      }
      
      // iterate over all enum values
      for (auto iter = e.values_.begin(); iter != e.values_.end(); ++iter)
      {
         // if can be resolved add the element to the struct
         new_e->enumerators_.push_back(fm::enumerator(iter->name_, iter->value_));         
      }
   
      // keep it
      new_e.release();
   }
   
   
   void operator()(const fp::map& m) const
   {
      std::unique_ptr<fm::map> new_m(new fm::map(m.name_, coll_));
      
      new_m->key_type_ = coll_.resolve(m.key_);
      
      if (!new_m->key_type_)
         new_m->key_type_ = new unresolved(m.key_);         
         
      new_m->value_type_ = coll_.resolve(m.value_);
      
      if (!new_m->value_type_)
         new_m->value_type_ = new unresolved(m.value_);         
         
      new_m.release();      
   }
   
   
   void operator()(const fp::array& a) const
   {
      std::unique_ptr<fm::array> new_a(new fm::array(a.name_, coll_));
      
      new_a->element_type_ = coll_.resolve(a.type_);
      
      if (!new_a->element_type_)
         new_a->element_type_ = new unresolved(a.type_);         
         
      new_a.release();      
   }
   
   
   void operator()(const fp::union_& s) const
   {
      std::unique_ptr<fm::union_> new_s(new fm::union_(s.name_, coll_));
      
      // search for parent if apropriate
      if (s.base_)
      {         
         fm::type* base = coll_.resolve(*s.base_);
                  
         if (base)
         {
            if (dynamic_cast<fm::union_*>(base) == 0)
               throw std::runtime_error("invalid struct base type");
         }
         else         
            base = new unresolved(*s.base_);         
                     
         new_s->base_ = base;         
      }
      
      // iterate over all elements of union. 
      for (auto iter = s.values_.begin(); iter != s.values_.end(); ++iter)
      {
         // resolve element
         fm::type* t = coll_.resolve(iter->type_);
         if (t)
         {             
            if (std::find_if(new_s->members_.begin(), new_s->members_.end(), [t](const fm::union_::member_type& m) {
               return t->type_id() == m.first->type_id();
            }) != new_s->members_.end())
               throw std::runtime_error("union type may not contain same type multiple times");
         }
         else
            t = new unresolved(iter->type_);            
                   
         new_s->members_.push_back(std::make_pair(t, iter->name_));
      }
      
      if (new_s->members_.size() == 0)
         throw std::runtime_error("empty union not allowed");
   
      // keep it
      new_s.release();
   }
   
   
   void operator()(const fp::typedef_& t) const
   {
      std::unique_ptr<fm::typedef_> new_t(new fm::typedef_(t.name_, coll_));
      
      new_t->real_type_ = coll_.resolve(t.type_);
      
      if (!new_t->real_type_)
         new_t->real_type_ = new unresolved(t.type_);      
         
      new_t.release();
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
            t = new unresolved(iter->type_);            
            
         meth.in_.push_back(fm::arg(iter->name_, t));
      }
      
      // out      
      for(auto iter = s.out_.begin(); iter != s.out_.end(); ++iter)
      {
         fm::type* t = i_.resolve(iter->type_);
         if (!t)
            t = new unresolved(iter->type_);    
            
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
      fm::fire_and_forget_method n_meth(s.name_, i_);
            
      for(auto iter = s.in_.begin(); iter != s.in_.end(); ++iter)
      {
         fm::type* t = i_.resolve(iter->type_);
         if (!t)
            t = new unresolved(iter->type_);
            
         n_meth.args_.push_back(fm::arg(iter->name_, t));
      }
      
      i_.ff_methods_.push_back(n_meth);
   }
   
   
   void operator()(const fp::broadcast& s) const
   {
      fm::broadcast bc(s.name_, i_);
            
      for(auto iter = s.args_.begin(); iter != s.args_.end(); ++iter)
      {
         fm::type* t = i_.resolve(iter->type_);
         if (!t)
            t = new unresolved(iter->type_);
            
         bc.args_.push_back(fm::arg(iter->name_, t));
      }
      
      i_.broadcasts_.push_back(bc);
   }
   
   
   void operator()(const fp::attribute& s) const
   {      
      fm::type* t = i_.resolve(s.type_);
      
      if (!t)
         t = new unresolved(s.type_);
      
      fm::attribute attr(s.name_, *t, i_, s.readonly_, s.no_subscriptions_);      
      
      i_.attrs_.push_back(attr);
   }
      
      
   fm::interface& i_;
};


// ---------------------------------------------------------------------


struct package_builder : public boost::static_visitor<void>
{
   package_builder(fm::package& package, const std::string& comparator)
    : package_(package)
    , comp_(comparator)
    , pkgname_(package_.fqn("."))
   {
      pkgname_ += ".";
   }
   
   void operator()(const fp::interface& i) const
   {            
      std::string ifname = pkgname_ + i.name_ 
         + ".";   // end-of-name recognition
      
      if (_stl_strncmp(ifname, comp_, std::min(ifname.size(), comp_.size())) == 0)
      {      
         fm::interface mi(i.name_, i.version_.major_, i.version_.minor_);      
         fm::interface& rmi = package_.add_interface(mi);
         
         std::for_each(i.parseitems_.begin(), i.parseitems_.end(), [this,&rmi]( const fp::interface_item_type& item) {
            boost::apply_visitor(interface_builder(rmi), item);
         });
      }
   }
   
   void operator()(const fp::typecollection& tc) const
   {
      std::string tcname = pkgname_ + tc.name_ 
         + ".";   // end-of-name recognition
      
      if (_stl_strncmp(tcname, comp_, std::min(tcname.size(), comp_.size())) == 0)
      {      
         fm::typecollection mtc(tc.name_);      
         fm::typecollection& rmtc = package_.add_typecollection(mtc);
         
         std::for_each(tc.parseitems_.begin(), tc.parseitems_.end(), [this,&rmtc]( const fp::tc_item_type& item) {
            boost::apply_visitor(typecollection_builder(rmtc), item);
         });
      }
   }
   
   fm::package& package_;
   
   // entity filtering
   std::string comp_;
   std::string pkgname_;
};


struct import_visitor : public boost::static_visitor<void>
{
   explicit
   import_visitor(std::vector<std::pair<std::string, fp::document> >& docs, const std::vector<std::string>& includes)
    : docs_(docs)
    , includes_(includes)
   {
      // NOOP
   }
   
   void operator()(const fp::namespace_import& import) const
   {
      std::ostringstream filter;
      
      std::for_each(import.items_.begin(), import.items_.end(), [&filter](const std::string& item){
         filter << ".";
         filter << item;         
      });
      
      parse_recursive(docs_, import.file_.c_str(), includes_, filter.str());
   }
   
   void operator()(const std::string& filename) const
   {
      parse_recursive(docs_, filename.c_str(), includes_, "*");
   }
   
   std::vector<std::pair<std::string, fp::document> >& docs_;
   const std::vector<std::string>& includes_;
};


struct namespace_extraction_visitor : public boost::static_visitor<std::string>
{
   std::string operator()(const fp::namespace_import& import) const
   {
      std::ostringstream filter;
      
      std::for_each(import.items_.begin(), import.items_.end()-1, [&filter](const std::string& item){
         filter << ".";
         filter << item;
      });
      
      return filter.str().substr(1);
   }
   
   std::string operator()(const std::string& filename) const
   {
      return "";
   }   
};

void parse_recursive(std::vector<std::pair<std::string, fp::document> >& docs, const char* franca_file, const std::vector<std::string>& includes, const std::string& filter)
{
   fp::document result = fp::parse(franca_file, includes);
   docs.push_back(std::make_pair(filter, result));

   if (!result.imports_.empty())   
   {
      // add current document's path to this search path
      std::vector<std::string> my_includes(includes);
      
      char doc_path[1024];
      strncpy(doc_path, franca_file, sizeof(doc_path));
      doc_path[sizeof(doc_path)-1] = '\0';
      
      char* dir = dirname(doc_path);
      
      if (strcmp(dir, "."))
         my_includes.push_back(dir);
      
      std::for_each(result.imports_.begin(), result.imports_.end(), [&docs, &my_includes](const fp::import_type& i){
         boost::apply_visitor(import_visitor(docs, my_includes), i);
      });
   }
}


}   // namespace


// ---------------------------------------------------------------------


/*static*/ 
fm::package& franca::builder::build(fm::package& root, const fp::document& parsetree, const std::string& filter)
{   
   // comparable string - may be empty for full model import
   std::string fqn_comp = filter;   
   
   // must add the "." to distinguish between complete and just parts of package names
   if (*fqn_comp.rbegin() == '*')
   {
      fqn_comp.erase(fqn_comp.end()-1);
   }
   else
      fqn_comp += ".";     // end-of-name recognition 
         
   fm::package* parent = &root;   
            
   for (auto iter = parsetree.package_.begin(); iter != parsetree.package_.end(); ++iter)
   {      
      std::string fqn = parent->fqn(".");
      fqn += ".";
      fqn += *iter;  
      fqn += ".";         // end-of-name recognition 
      
      if (_stl_strncmp(fqn, fqn_comp, std::min(fqn.size(), fqn_comp.size())) == 0)      
      {
         parent = &parent->add_package(*iter);            
      }      
      else
         return root;    // nothing more to do      
   }
   
   // add namespace imports
   std::for_each(parsetree.imports_.begin(), parsetree.imports_.end(), [&parent](const fp::import_type& imp){   
      parent->add_import(boost::apply_visitor(namespace_extraction_visitor(), imp));
   });
   
   if (_stl_strncmp(parent->fqn("."), fqn_comp, std::min(fqn_comp.size(), parent->fqn(".").size())) == 0)
   {
      std::for_each(parsetree.parseitems_.begin(), parsetree.parseitems_.end(), [parent, &fqn_comp]( const fp::doc_item_type& item) {
         boost::apply_visitor(package_builder(*parent, fqn_comp), item);
      });
   }      
   
   return root;
}


/*static*/
void franca::builder::resolve_all_symbols(model::package& root)
{
   internal_resolve_unresolved(root);
}


/*static*/
void franca::builder::parse_and_build(model::package& root, const char* franca_file)
{
   std::vector<std::string> dummy;
   parse_and_build(root, franca_file, dummy);
}


/*static*/
void franca::builder::parse_and_build(model::package& root, const char* franca_file, const std::vector<std::string>& includes)
{
   std::vector<std::pair<std::string, fp::document> > docs;
   parse_recursive(docs, franca_file, includes, "*");
   
   std::for_each(docs.begin(), docs.end(), [&root](const std::pair<std::string, fp::document>& doc){
      (void)franca::builder::build(root, doc.second, doc.first);
   });
   
   franca::builder::resolve_all_symbols(root);   
}
