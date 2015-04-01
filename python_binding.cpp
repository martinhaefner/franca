#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <cstdlib>

#include "model.h"
#include "builder.h"


namespace fm = franca::model;

using namespace boost::python;
   

void IndexError() 
{ 
   PyErr_SetString(PyExc_IndexError, "Index out of range"); 
   throw_error_already_set();
}


template<typename T>
struct std_list_item
{
   typedef typename T::value_type value_type;
   
   static 
   const value_type& get(T const& x, int i)
   {
      if (i < 0) 
         i += x.size();
         
      if (i >= 0 && i < x.size()) 
      {
         auto iter = x.begin();
         
         while(--i >= 0)
            ++iter;
            
         return *iter;
      }         
      
      IndexError(); 
      
      // make compiler happy
      abort();        
   }   
};


template<typename T>
struct std_item
{
   typedef typename T::value_type value_type;
   
   static 
   const value_type& get(T const& x, int i)
   {
      if (i < 0) 
         i += x.size();
         
      if (i >= 0 && i < x.size()) 
         return x[i];
      
      IndexError(); 
      
      // make compiler happy
      abort();        
   }   
};


// ---------------------------------------------------------------------
   

BOOST_PYTHON_MODULE(franca)
{      
   class_<fm::package>("package")
      .def_readonly("name", &fm::package::name)      
      .def_readonly("typecollections", &fm::package::collections_)
      .def_readonly("interfaces", &fm::package::interfaces_)
      .def_readonly("packages", &fm::package::packages_)
      .def_readonly("parent", &fm::package::parent_)
      .def("fqn", &fm::package::fqn)
   ;      
      
   class_<franca::builder>("builder")
      .def("parse_and_build", &franca::builder::parse_and_build)
      .staticmethod("parse_and_build")
   ;
   
   class_<fm::arg>("arg", no_init)
      .def_readonly("name", &fm::arg::name)      
      .def_readonly("type", &fm::arg::type_)
   ;
   
   class_<fm::type>("type", no_init)
      .def_readonly("name", &fm::type::name)
      .def("fqn", &fm::type::fqn)      
   ;
   
   class_<fm::struct_, bases<fm::type> >("struct", no_init)      
      .def_readonly("has_base", &fm::struct_::has_base)            
      .def("members", &fm::struct_::members, return_value_policy<reference_existing_object>())
      .def("base", &fm::struct_::base, return_value_policy<reference_existing_object>())                  
   ;
   
   class_<fm::enumerator>("enumerator", no_init)
      .def_readonly("name", &fm::enumerator::name)
      .def_readonly("value", &fm::enumerator::value_)
      .def_readonly("parent", &fm::enumerator::parent_)
   ;
   
   class_<fm::enumeration, bases<fm::type> >("enumeration", no_init)      
      .def_readonly("has_base", &fm::enumeration::has_base)
      .def_readonly("enumerators", &fm::enumeration::enumerators_)
      .def("base", &fm::enumeration::base, return_value_policy<reference_existing_object>())      
   ;   
   
   class_<fm::method>("method", no_init)
      .def_readonly("name", &fm::method::name)          
      .def_readonly("in", &fm::method::in_)          
      .def_readonly("out", &fm::method::out_)                
      .def_readonly("has_errors", &fm::method::has_errors)      
      .def("errors", &fm::method::errors, return_value_policy<reference_existing_object>())      
      .def("interface", &fm::method::get_interface, return_value_policy<reference_existing_object>())          
   ;
   
   class_<fm::fire_and_forget_method>("fire_and_forget_method", no_init)
      .def_readonly("name", &fm::fire_and_forget_method::name)    
      .def_readonly("args", &fm::fire_and_forget_method::args_)    
      .def("interface", &fm::fire_and_forget_method::get_interface, return_value_policy<reference_existing_object>())      
   ;
   
   class_<fm::broadcast>("broadcast", no_init)
      .def_readonly("name", &fm::broadcast::name)      
      .def_readonly("args", &fm::broadcast::args_)      
      .def("interface", &fm::broadcast::get_interface, return_value_policy<reference_existing_object>())      
   ;
   
   class_<fm::attribute>("attribute", no_init)
      .def_readonly("name", &fm::attribute::name)      
      .def_readonly("type", &fm::attribute::type_)      
      .def_readonly("readonly", &fm::attribute::readonly_)   
      .def_readonly("no_subscriptions", &fm::attribute::no_subscriptions_)   
      .def("interface", &fm::attribute::get_interface, return_value_policy<reference_existing_object>())   
   ;   
   
   class_<fm::interface>("interface", no_init)    
      .def_readonly("name", &fm::interface::name)        
      .def_readonly("major", &fm::interface::major_)
      .def_readonly("minor", &fm::interface::minor_)
      .def_readonly("types", &fm::interface::types_)            
      .def_readonly("attributes", &fm::interface::attrs_)
      .def_readonly("methods", &fm::interface::methods_)
      .def_readonly("fire_and_forget_methods", &fm::interface::ff_methods_)
      .def_readonly("broadcasts", &fm::interface::broadcasts_)
      .def("fqn", &fm::interface::fqn)
   ;
      
   class_<fm::typecollection>("typecollection", no_init)            
      .def_readonly("name", &fm::typecollection::name)        
      .def_readonly("types", &fm::typecollection::types_)      
      .def("fqn", &fm::typecollection::fqn)
   ;
      
   // --- STL helpers --------------------------------------------------
   
   class_<std::list<fm::package> >("package_list")   
      .def("__len__", &std::list<fm::package>::size)  
      .def("__getitem__", &std_list_item<std::list<fm::package> >::get,
           return_value_policy<reference_existing_object>())        
   ;
   
   class_<std::list<fm::typecollection> >("typecollection_list")   
      .def("__len__", &std::list<fm::typecollection>::size)        
      .def("__getitem__", &std_list_item<std::list<fm::typecollection> >::get,
           return_value_policy<reference_existing_object>())        
   ;
   
   class_<std::list<fm::interface> >("interface_list")   
      .def("__len__", &std::list<fm::interface>::size)        
      .def("__getitem__", &std_list_item<std::list<fm::interface> >::get,
           return_value_policy<reference_existing_object>())        
   ;
   
   class_<std::vector<fm::type*> >("type_vector")   
      .def("__len__", &std::vector<fm::type*>::size)  
      .def("__getitem__", &std_item<std::vector<fm::type*> >::get,
           return_value_policy<reference_existing_object>())        
   ;
   
   class_<std::vector<fm::arg> >("arg_vector")   
      .def("__len__", &std::vector<fm::arg>::size)  
      .def("__getitem__", &std_item<std::vector<fm::arg> >::get,
           return_value_policy<reference_existing_object>())        
   ;   
   
   class_<std::vector<fm::attribute> >("attribute_vector")   
      .def("__len__", &std::vector<fm::attribute>::size)  
      .def("__getitem__", &std_item<std::vector<fm::attribute> >::get,
           return_value_policy<reference_existing_object>())        
   ;   
   
   class_<std::vector<fm::broadcast> >("broadcast_vector")   
      .def("__len__", &std::vector<fm::broadcast>::size)  
      .def("__getitem__", &std_item<std::vector<fm::broadcast> >::get,
           return_value_policy<reference_existing_object>())        
   ;   
   
   class_<std::vector<fm::method> >("method_vector")   
      .def("__len__", &std::vector<fm::method>::size)  
      .def("__getitem__", &std_item<std::vector<fm::method> >::get,
           return_value_policy<reference_existing_object>())        
   ;   
   
   class_<std::vector<fm::fire_and_forget_method> >("ffmethod_vector")   
      .def("__len__", &std::vector<fm::fire_and_forget_method>::size)  
      .def("__getitem__", &std_item<std::vector<fm::fire_and_forget_method> >::get,
           return_value_policy<reference_existing_object>())        
   ;      
   
   class_<std::vector<fm::enumerator> >("enumerator_vector")   
      .def("__len__", &std::vector<fm::enumerator>::size)  
      .def("__getitem__", &std_item<std::vector<fm::enumerator> >::get,
           return_value_policy<reference_existing_object>())        
   ;
   
   class_<fm::struct_::member_type>("member_pair")   
      .def_readonly("first", &fm::struct_::member_type::first)  
      .def_readonly("second", &fm::struct_::member_type::second)
   ; 
   
   class_<std::vector<fm::struct_::member_type> >("member_vector")   
      .def("__len__", &std::vector<fm::struct_::member_type>::size)  
      .def("__getitem__", &std_item<std::vector<fm::struct_::member_type> >::get,
           return_value_policy<reference_existing_object>())        
   ; 
}
