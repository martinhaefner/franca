#include <boost/config/warning_disable.hpp>

#include <boost/spirit/include/qi.hpp>

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/home/phoenix/bind/bind_function.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/at_c.hpp>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <functional>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "simppl/typelist.h"


using boost::spirit::qi::_1;
using boost::spirit::qi::lit;
using boost::spirit::qi::rule;
using boost::spirit::qi::char_;
using boost::spirit::qi::lexeme;
using boost::spirit::qi::int_;
using boost::spirit::qi::string;
using boost::spirit::qi::phrase_parse;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

using boost::phoenix::ref;
using boost::phoenix::val;
using boost::phoenix::construct;

using namespace qi::labels;

using boost::phoenix::at_c;
using boost::phoenix::push_back;


// FIXME all referenced identifiers may be '.' separated -> use appropriate franca type 
//       and use ident_ref instead of identifier for the parser grammar 


struct franca_version
{
   int major_;
   int minor_;
};


struct franca_arg
{
   std::string type_;
   std::string name_;
};


struct franca_attribute
{
   franca_attribute()
    : no_subscriptions_(false)
    , readonly_(false)
   {
      // NOOP
   }
   
   std::string type_;
   std::string name_;
   
   bool readonly_;   
   bool no_subscriptions_;  
};


struct franca_enum_value
{
   franca_enum_value()
    : value_(0xFFFFFFDD)
   {
      // NOOP
   }
   
   std::string name_;
   int value_;
};


struct franca_enumeration
{
   std::string name_;
   boost::optional<std::string> base_;
   
   std::vector<franca_enum_value> values_;
};


struct franca_struct_entry
{
   std::string type_;
   std::string name_;
};


struct franca_struct
{
   std::string name_;
   boost::optional<std::string> base_;
   
   std::vector<franca_struct_entry> values_;
};


struct franca_extended_error
{
   boost::optional<std::string> base_;
   std::vector<franca_enum_value> values_;
};


typedef boost::variant<std::string, franca_extended_error> franca_method_error;


struct franca_method
{
   std::string name_;
   
   std::vector<franca_arg> in_;
   std::vector<franca_arg> out_;
   boost::optional<franca_method_error> error_;
};


struct franca_fire_and_forget_method
{
   std::string name_;
   
   std::vector<franca_arg> in_;
};


struct franca_broadcast
{
   std::string name_;   
   std::vector<franca_arg> args_;
};


template<typename... T>
struct balancer : public boost::static_visitor<void>
{
   typedef typename std::tuple<std::reference_wrapper<T>...> tuple_type;
   
   balancer(tuple_type&& tup)
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


template<typename... T>
balancer<T...> make_balancer(T&... t)
{
   return balancer<T...>(std::tuple<std::reference_wrapper<T>...>(std::ref(t)...));
}


typedef boost::variant<franca_method, 
                       franca_fire_and_forget_method, 
                       franca_broadcast, 
                       franca_attribute, 
                       franca_enumeration, 
                       franca_struct> 
   interface_item_type;


struct franca_interface
{
   franca_interface& balance()
   {
      std::for_each(parseitems_.begin(), parseitems_.end(), [this](const interface_item_type& item) {
         boost::apply_visitor(make_balancer(methods_, 
                                            ff_methods_, 
                                            broadcasts_, 
                                            attributes_, 
                                            enumerations_, 
                                            structs_), 
                              item);
         });
      
      return *this;
   }
   
   std::string name_;
   franca_version version_;
   
   std::vector<interface_item_type> parseitems_;
   
   std::vector<franca_method> methods_;
   std::vector<franca_fire_and_forget_method> ff_methods_;
   std::vector<franca_broadcast> broadcasts_;
   std::vector<franca_attribute> attributes_;
   std::vector<franca_enumeration> enumerations_;
   std::vector<franca_struct> structs_;
};


typedef boost::variant<franca_enumeration, franca_struct> tc_item_type;


struct franca_typecollection
{
   std::string name_;
   std::vector<tc_item_type> parseitems_;
};


typedef boost::variant<franca_interface, franca_typecollection> doc_item_type;


struct doc_balancer : public boost::static_visitor<void>
{
   doc_balancer(std::vector<franca_interface>& ifs, std::vector<franca_typecollection>& tcs)
    : ifs_(ifs)
    , tcs_(tcs)
   {
      // NOOP
   }
   
   void operator()(const franca_interface& item) const
   {
      ifs_.push_back(item);
   }
   
   void operator()(const franca_typecollection& item) const
   {
      tcs_.push_back(item);
   }
   
   std::vector<franca_interface>& ifs_;
   std::vector<franca_typecollection>& tcs_;
};


struct franca_doc
{
   franca_doc& balance()
   {
      std::for_each(parseitems_.begin(), parseitems_.end(), [this](const doc_item_type& item) {
         boost::apply_visitor(doc_balancer(interfaces_, typecollections_), item);
      });
    
      std::transform(interfaces_.begin(), interfaces_.end(), interfaces_.begin(), std::bind(&franca_interface::balance, std::placeholders::_1));
        
      return *this;
   }
   
   std::vector<std::string> package_;
   std::vector<doc_item_type> parseitems_;
   
   std::vector<franca_interface> interfaces_;
   std::vector<franca_typecollection> typecollections_;
};


// ---------------------------------------------------------------------


BOOST_FUSION_ADAPT_STRUCT(
    franca_version,
    (int, major_)
    (int, minor_)
)


BOOST_FUSION_ADAPT_STRUCT(
   franca_struct_entry,
   (std::string, type_)
   (std::string, name_)
)


BOOST_FUSION_ADAPT_STRUCT(
   franca_struct,
   (std::string, name_)
   (boost::optional<std::string>, base_)
   (std::vector<franca_struct_entry>, values_)
)


BOOST_FUSION_ADAPT_STRUCT(
   franca_arg,
   (std::string, type_)
   (std::string, name_)
)


BOOST_FUSION_ADAPT_STRUCT(
   franca_attribute,
   (std::string, type_)
   (std::string, name_)
   (bool, readonly_)
   (bool, no_subscriptions_)
)


BOOST_FUSION_ADAPT_STRUCT(
   franca_enum_value,
   (std::string, name_)
   (int, value_)
)


BOOST_FUSION_ADAPT_STRUCT(
   franca_extended_error,
   (boost::optional<std::string>, base_)
   (std::vector<franca_enum_value>, values_)
)


BOOST_FUSION_ADAPT_STRUCT(
   franca_enumeration,
   (std::string, name_)
   (boost::optional<std::string>, base_)
   (std::vector<franca_enum_value>, values_)
)


BOOST_FUSION_ADAPT_STRUCT(
   franca_method,
   (std::string, name_)   
   (std::vector<franca_arg>, in_)
   (std::vector<franca_arg>, out_)
)


BOOST_FUSION_ADAPT_STRUCT(
   franca_fire_and_forget_method,
   (std::string, name_)   
   (std::vector<franca_arg>, in_)
)


BOOST_FUSION_ADAPT_STRUCT(
   franca_broadcast,
   (std::string, name_)   
   (std::vector<franca_arg>, args_)   
)


BOOST_FUSION_ADAPT_STRUCT(
    franca_interface,
    (std::string, name_)
    (franca_version, version_)
    (std::vector<interface_item_type>, parseitems_)    
)


BOOST_FUSION_ADAPT_STRUCT(
   franca_typecollection,
   (std::string, name_)
   (std::vector<tc_item_type>, parseitems_)
)


BOOST_FUSION_ADAPT_STRUCT(
    franca_doc,
    (std::vector<std::string>, package_)
    (std::vector<doc_item_type>, parseitems_)
)


// ---------------------------------------------------------------------


#define identifier      lexeme[+char_("a-zA-Z0-9_")]
#define type_identifier lexeme[+char_("a-zA-Z0-9_")]

#define inargs          (lit("in") >>  '{' >> *arg_ >> '}') 
#define outargs         (lit("out") >> '{' >> *arg_ >> '}')      


template <typename IteratorT>
struct franca_grammar : qi::grammar<IteratorT, franca_doc(), ascii::space_type>
{
   franca_grammar() : franca_grammar::base_type(franca_)
   {      
      package_ %= 
         lit("package") 
            >> lexeme[+char_("a-zA-Z0-9_") % '.'];
      
      version_ %= 
         lit("version") 
            >> '{' 
               >> "major" >> int_ 
               >> "minor" >> int_ 
            >> '}';
      
      arg_ %=
         type_identifier >> identifier;
            
      method_ %=
         lit("method") >> identifier 
            >> '{' >> -inargs >> -outargs >> -errors_ >> '}';
               
      fireforget_method_ %=
         lit("method") >> identifier 
            >> lit("fireAndForget") >> '{' >> -inargs >> '}';
         
      attribute_ %=
         lit("attribute") 
            >> type_identifier >> identifier 
            >> -( lit("readonly")[at_c<2>(qi::_val) = true] 
                 ^ lit("noSubscriptions")[at_c<3>(qi::_val) = true] );
            
      broadcast_ %=
         lit("broadcast") >> identifier
            >> '{' >> -outargs >> '}';
      
      errors_ %= lit("error") 
         >> ( identifier | ( -( lit("extends") >> identifier) >> enumeration_decl_ ) );
            
      enumerator_value_ %=
         lit('=') >> '"' >> (int_|boost::spirit::hex) >> '"';
      
      enumeration_decl_ %= 
         '{' >> *( type_identifier >> -enumerator_value_ ) >> '}';
      
      structure_decl_ %=
         '{' >> *( type_identifier >> identifier ) >> '}';
      
      structure_ %=
         lit("struct") >> identifier
            >> -(lit("extends") >> identifier)
            >> structure_decl_;
            
      enumeration_ %= 
         lit("enumeration") >> identifier 
            >> -(lit("extends") >> identifier)
            >> enumeration_decl_;
            
      interface_ %=
         lit("interface") >> identifier
            >> '{'
               >> version_
               >> *( method_ 
                   | fireforget_method_ 
                   | broadcast_ 
                   | attribute_ 
                   | enumeration_ 
                   | structure_ )
            >> '}';
       
      typecollection_ %=
         lit("typecollection") >> identifier
            >> '{'
               >> *( enumeration_ 
                   | structure_ )
            >> '}';
            
      franca_ %= package_ >> *( interface_ | typecollection_ );
      
      package_.name("package");
      interface_.name("interface");
      version_.name("version");  
      method_.name("method");
            
      qi::on_error<qi::fail>
      (
         franca_,
         std::cout
             << val("Error! Expecting ")
             << qi::_4                               // what failed?
             << val(" here: \"")
             << construct<std::string>(qi::_3, qi::_2)   // iterators to error-pos, end
             << val("\"")
             << std::endl
      );
   }
      
   rule<IteratorT, franca_doc(), ascii::space_type> franca_;
   rule<IteratorT, std::vector<std::string>(), ascii::space_type> package_;
   rule<IteratorT, franca_interface(), ascii::space_type> interface_;
   rule<IteratorT, franca_version(), ascii::space_type> version_;
   rule<IteratorT, franca_method(), ascii::space_type> method_;
   rule<IteratorT, franca_fire_and_forget_method(), ascii::space_type> fireforget_method_;
   rule<IteratorT, franca_broadcast(), ascii::space_type> broadcast_;
   rule<IteratorT, franca_arg(), ascii::space_type> arg_;
   rule<IteratorT, franca_typecollection(), ascii::space_type> typecollection_;
   rule<IteratorT, franca_attribute(), ascii::space_type> attribute_;
   rule<IteratorT, franca_enumeration(), ascii::space_type> enumeration_;
   rule<IteratorT, std::vector<franca_enum_value>(), ascii::space_type> enumeration_decl_;
   rule<IteratorT, int(), ascii::space_type> enumerator_value_;
   rule<IteratorT, franca_method_error(), ascii::space_type> errors_;
   rule<IteratorT, franca_struct(), ascii::space_type> structure_;
   rule<IteratorT, std::vector<franca_struct_entry>(), ascii::space_type> structure_decl_;
};


// ---------------------------------------------------------------------


int main(int argc, char** argv)
{
   std::string str = 
      "package my.test "
      ""
      "interface Hello {"
      "version { major 1 minor 2 }"
      ""
      "   method eval {"
      "      in {"
      "         int i"
      "      }"
      "      out {"
      "         double d"
      "      }"
      "   }"
      "}";
         
   if (argc > 1)
   {      
      str = "";
      
      std::ifstream in(argv[1], std::ios_base::in);

      if (!in)
      {
         std::cerr << "Error: Could not open input file: "
            << argv[1] << std::endl;
            
         return EXIT_FAILURE;
      }
      
      in.unsetf(std::ios::skipws); // No white space skipping!
      
      std::copy(
         std::istream_iterator<char>(in),
         std::istream_iterator<char>(),
         std::back_inserter(str));         
   }
   
   std::cout << "Parsing '" << str << "'" << std::endl;
       
   franca_grammar<std::string::const_iterator> grammar;
   franca_doc doc;
   
   std::string::const_iterator iter = str.begin();
   std::string::const_iterator end = str.end();

   bool r = phrase_parse(iter, end, grammar, ascii::space, doc);

   std::cout << std::boolalpha << r << std::endl;
   std::cout << (iter == end) << std::endl;
   
   doc.balance();
   
   std::cout 
      << "package: " << doc.package_[0] << std::endl
      << ", interface: " << doc.interfaces_[0].name_ << std::endl
      << ", version: " << doc.interfaces_[0].version_.major_ << "." << doc.interfaces_[0].version_.minor_ << std::endl
      << ", method: " << doc.interfaces_[0].methods_[0].name_ << std::endl
      << ", fireforget methods: " << doc.interfaces_[0].ff_methods_.size() << std::endl
      << ", in args: size = " << doc.interfaces_[0].methods_[0].in_.size() << ", " << doc.interfaces_[0].methods_[0].in_.operator[](0).type_ << ", " << doc.interfaces_[0].methods_[0].in_.operator[](1).type_ << std::endl
      << ", out arg: size = " << doc.interfaces_[0].methods_[0].out_.size() << ", " << doc.interfaces_[0].methods_[0].out_.operator[](0).type_ << std::endl
      << ", broadcast: " << doc.interfaces_[0].broadcasts_[0].name_ << std::endl
      << ", attribute: " << doc.interfaces_[0].attributes_[0].name_ << " (" << doc.interfaces_[0].attributes_[0].type_ << ") " 
      << (doc.interfaces_[0].attributes_[0].readonly_?"readonly ":" ")
      << (doc.interfaces_[0].attributes_[0].no_subscriptions_?"no subscriptions ":" ") << std::endl;
   
   return EXIT_SUCCESS;
}
