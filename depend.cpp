#include "model.h"
#include "builder.h"


namespace fm = franca::model;


namespace {



void add_dependencies(fm::attribute& a)
{
   a.interface_.add_dependency(a.type_->parent_);
}


void add_dependencies(fm::method& m)
{
   std::for_each(m.in_.begin(), m.in_.end(), [&m](fm::arg& a){ 
      m.get_interface().add_dependency(a.type_->parent_);
   });
   
   std::for_each(m.out_.begin(), m.out_.end(), [&m](fm::arg& a){ 
      m.get_interface().add_dependency(a.type_->parent_);
   });
   
   if (m.has_errors())
      m.get_interface().add_dependency(m.errors_->parent_);
}


void add_dependencies(fm::fire_and_forget_method& m)
{
   std::for_each(m.args_.begin(), m.args_.end(), [&m](fm::arg& a){ 
      m.interface_.add_dependency(a.type_->parent_);
   });
}


void add_dependencies(fm::broadcast& b)
{
   std::for_each(b.args_.begin(), b.args_.end(), [&b](fm::arg& a){ 
      b.interface_.add_dependency(a.type_->parent_);
   });
}


void add_dependencies(fm::typecollection& tc)
{
   std::for_each(tc.types_.begin(), tc.types_.end(), [](const fm::type* t){
      
      std::set<const fm::typecollection*> colls = t->refers_to();
      
      /*
      std::cout << "type: " << t->name() << ", size=" << colls.size() << std::endl;
      
      std::for_each(colls.begin(), colls.end(), [](const fm::typecollection* coll){
         if (coll)
         {
            std::cout << "   ...depends on: " << std::flush;         
            std::cout << coll->name() << std::endl;
         }
         else
            std::cout << "Shit" << std::endl;
      });      
      
      std::cout << std::endl;
      */
      
      std::for_each(colls.begin(), colls.end(), [t](const fm::typecollection* coll){
         t->parent_->add_dependency(coll);         
      });   
   });
}


void add_dependencies(fm::interface& i)
{
   std::for_each(i.broadcasts_.begin(), i.broadcasts_.end(), [](fm::broadcast& b){
      add_dependencies(b);
   });
   
   std::for_each(i.methods_.begin(), i.methods_.end(), [](fm::method& m){
      add_dependencies(m);
   });
   
   std::for_each(i.ff_methods_.begin(), i.ff_methods_.end(), [](fm::fire_and_forget_method& m){
      add_dependencies(m);
   });
   
   std::for_each(i.attrs_.begin(), i.attrs_.end(), [](fm::attribute& a){
      add_dependencies(a);
   });
   
   add_dependencies(static_cast<fm::typecollection&>(i));
}


void traverse_for_dependency(fm::package& p)
{
   // all packages
   std::for_each(p.packages_.begin(), p.packages_.end(), [](fm::package& p){   
      traverse_for_dependency(p);
   });
   
   // ...and interaces
   std::for_each(p.interfaces_.begin(), p.interfaces_.end(), [](fm::interface& i){
      add_dependencies(i);
   });
   
   // ...and type collections
   std::for_each(p.collections_.begin(), p.collections_.end(), [](fm::typecollection& tc){
      add_dependencies(tc);
   });
}


}   // anonymous namespace


// ---------------------------------------------------------------------


/*static*/
void franca::builder::create_typecollection_dependencies(model::package& pkg)
{
   traverse_for_dependency(pkg);
}
