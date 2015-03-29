#include "builder.h"

#include "simppl/typelist.h"


namespace fp = franca::parser;
namespace fm = franca::model;


template<typename... T>
struct package_builder : public boost::static_visitor<void>
{
   typedef typename std::tuple<std::reference_wrapper<T>...> tuple_type;
   
   package_builder(tuple_type&& tup)
    : tup_(tup)
   {
      // NOOP
   }
   
   template<typename U>
   void operator()(const U& u) const
   {
      typedef typename simppl::make_typelist<T...>::type list_type;
      std::get<simppl::Find<std::vector<U>, list_type>::value>(tup_).get().push_back(u);
   }
   
   tuple_type tup_;
};


// ---------------------------------------------------------------------


/*static*/ 
fm::package& franca::builder::build(fm::package& root, const fp::document& parsetree)
{
   fm::package* parent = &root;
   
   std::for_each(parsetree.package_.begin(), parsetree.package_.end(), [&parent](const std::string& str) {
      parent = new fm::package(str, *parent);
   });
   
   std::for_each(parsetree.parseitems_.begin(), parsetree.parseitems_.end(), [parent]( const doc_item_type& item) {
      boost::apply_visitor(package_builder(), item);
   });
   
   return root;
}
