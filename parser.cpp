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
#include <fstream>

#include "parser.h"


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

namespace fp = franca::parser;


// ---------------------------------------------------------------------


BOOST_FUSION_ADAPT_STRUCT(
    fp::version,
    (int, major_)
    (int, minor_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::struct_entry,
   (std::string, type_)
   (std::string, name_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::struct_,
   (std::string, name_)
   (boost::optional<std::string>, base_)
   (std::vector<fp::struct_entry>, values_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::arg,
   (std::string, type_)
   (std::string, name_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::attribute,
   (std::string, type_)
   (std::string, name_)
   (bool, readonly_)
   (bool, no_subscriptions_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::enum_value,
   (std::string, name_)
   (int, value_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::extended_error,
   (boost::optional<std::string>, base_)
   (std::vector<fp::enum_value>, values_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::enumeration,
   (std::string, name_)
   (boost::optional<std::string>, base_)
   (std::vector<fp::enum_value>, values_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::method,
   (std::string, name_)   
   (std::vector<fp::arg>, in_)
   (std::vector<fp::arg>, out_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::fire_and_forget_method,
   (std::string, name_)   
   (std::vector<fp::arg>, in_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::broadcast,
   (std::string, name_)   
   (std::vector<fp::arg>, args_)   
)


BOOST_FUSION_ADAPT_STRUCT(
    fp::interface,
    (std::string, name_)
    (fp::version, version_)
    (std::vector<fp::interface_item_type>, parseitems_)    
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::typecollection,
   (std::string, name_)
   (std::vector<fp::tc_item_type>, parseitems_)
)


BOOST_FUSION_ADAPT_STRUCT(
    fp::document,
    (std::vector<std::string>, package_)
    (std::vector<fp::doc_item_type>, parseitems_)
)


// ---------------------------------------------------------------------


#define identifier      lexeme[+char_("a-zA-Z0-9_")]
#define type_identifier lexeme[+char_("a-zA-Z0-9_")]
#define type_reference  lexeme[+char_("a-zA-Z0-9_.")]

#define inargs          (lit("in") >>  '{' >> *arg_ >> '}') 
#define outargs         (lit("out") >> '{' >> *arg_ >> '}')      


template <typename IteratorT>
struct grammar : qi::grammar<IteratorT, fp::document(), ascii::space_type>
{
   grammar() : grammar::base_type(franca_)
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
         type_reference >> identifier;
            
      method_ %=
         lit("method") >> identifier 
            >> '{' >> -inargs >> -outargs >> -errors_ >> '}';
               
      fireforget_method_ %=
         lit("method") >> identifier 
            >> lit("fireAndForget") >> '{' >> -inargs >> '}';
         
      attribute_ %=
         lit("attribute") 
            >> type_reference >> identifier 
            >> -( lit("readonly")[at_c<2>(qi::_val) = true] 
                 ^ lit("noSubscriptions")[at_c<3>(qi::_val) = true] );
            
      broadcast_ %=
         lit("broadcast") >> identifier
            >> '{' >> -outargs >> '}';
      
      errors_ %= lit("error") 
         >> ( type_reference | ( -( lit("extends") >> type_reference) >> enumeration_decl_ ) );
            
      enumerator_value_ %=
         lit('=') >> '"' >> (int_|boost::spirit::hex) >> '"';
      
      enumeration_decl_ %= 
         '{' >> *( type_identifier >> -enumerator_value_ ) >> '}';
      
      structure_decl_ %=
         '{' >> *( type_reference >> identifier ) >> '}';
      
      structure_ %=
         lit("struct") >> identifier
            >> -(lit("extends") >> type_reference)
            >> structure_decl_;
            
      enumeration_ %= 
         lit("enumeration") >> identifier 
            >> -(lit("extends") >> type_reference)
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
      
   rule<IteratorT, fp::document(), ascii::space_type> franca_;
   rule<IteratorT, std::vector<std::string>(), ascii::space_type> package_;
   rule<IteratorT, fp::interface(), ascii::space_type> interface_;
   rule<IteratorT, fp::version(), ascii::space_type> version_;
   rule<IteratorT, fp::method(), ascii::space_type> method_;
   rule<IteratorT, fp::fire_and_forget_method(), ascii::space_type> fireforget_method_;
   rule<IteratorT, fp::broadcast(), ascii::space_type> broadcast_;
   rule<IteratorT, fp::arg(), ascii::space_type> arg_;
   rule<IteratorT, fp::typecollection(), ascii::space_type> typecollection_;
   rule<IteratorT, fp::attribute(), ascii::space_type> attribute_;
   rule<IteratorT, fp::enumeration(), ascii::space_type> enumeration_;
   rule<IteratorT, std::vector<fp::enum_value>(), ascii::space_type> enumeration_decl_;
   rule<IteratorT, int(), ascii::space_type> enumerator_value_;
   rule<IteratorT, fp::method_error(), ascii::space_type> errors_;
   rule<IteratorT, fp::struct_(), ascii::space_type> structure_;
   rule<IteratorT, std::vector<fp::struct_entry>(), ascii::space_type> structure_decl_;
};


// ---------------------------------------------------------------------


fp::document fp::parse(const char* filename)
{
   std::ifstream in(filename, std::ios_base::in);

   if (!in)
      throw std::runtime_error("file not found");
   
   in.unsetf(std::ios::skipws); // No white space skipping!
 
   std::string str;  
   
   std::copy(
      std::istream_iterator<char>(in),
      std::istream_iterator<char>(),
      std::back_inserter(str));         
   
   grammar<std::string::const_iterator> grammar;
   fp::document doc;
   
   std::string::const_iterator iter = str.begin();
   std::string::const_iterator end = str.end();

   if (!phrase_parse(iter, end, grammar, ascii::space, doc) || iter != end)
      throw std::runtime_error("parser failure");
      
   return doc;
}
