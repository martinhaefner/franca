#include <boost/config/warning_disable.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/at_c.hpp>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>

#include "parser.h"


using boost::spirit::qi::lit;
using boost::spirit::qi::char_;
using boost::spirit::qi::lexeme;
using boost::spirit::qi::int_;
using boost::spirit::qi::hex;
using boost::spirit::qi::_val;
using boost::spirit::qi::_pass;
using boost::spirit::qi::eol;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;
namespace fp = franca::parser;


// ---------------------------------------------------------------------


namespace
{
   
bool is_keyword(const std::string& token)
{
   static std::set<std::string> keywords({
      "package",
      "interface",
      "typecollection",
      "method",
      "attribute",
      "in",
      "out",
      "version",
      "error",
      "broadcast",
      "struct",
      "extends",
      "enumeration",
      "array",
      "of",
      "union",
      "map",
      "to",
      "typedef",
      "is",
      "major",
      "minor",
      "import",
      "model"
   });
   
   return keywords.find(token) != keywords.end();   
}

bool is_intrinsic_type(const std::string& token)
{
   static std::set<std::string> intrinsics({
      "UInt8",
      "Int8",
      "UInt16",
      "Int16",
      "UInt32",
      "Int32",
      "UInt64",
      "Int64",
      "Boolean",
      "Float",
      "Double",
      "String",
      "ByteBuffer"      
   });
   
   return intrinsics.find(token) != intrinsics.end();   
}


struct name_extractor : public boost::static_visitor<std::string>
{   
   template<typename T>
   inline
   const std::string& operator()(const T& t) const
   {
      return t.name_;
   }
};


template<typename ContainerT, typename ItemT>
bool eval(const ContainerT& container, const ItemT& new_item)
{
   std::string name = boost::apply_visitor(name_extractor(), new_item);
   
   return !is_keyword(name) 
      && !is_intrinsic_type(name)
      && std::find_if(container.begin(), container.end(), [&name](const ItemT& item){
            return name == boost::apply_visitor(name_extractor(), item);
         }) == container.end();
}


template<typename ContainerT>
bool eval(const ContainerT& container, const std::string& name)
{
   typedef typename ContainerT::value_type item_type;
   
   return !is_keyword(name) 
      && !is_intrinsic_type(name)
      && std::find_if(container.begin(), container.end(), [&name](const item_type& item){
            return name == item.name_;
         }) == container.end();
}


}   // namespace


bool fp::interface::eval(const interface_item_type& new_item)
{
   return ::eval(parseitems_, new_item);
}


bool fp::typecollection::eval(const tc_item_type& new_item)
{
   return ::eval(parseitems_, new_item);
}


bool fp::document::eval(const doc_item_type& new_item)
{
   return ::eval(parseitems_, new_item);   
}


bool fp::method::eval_in(const arg& argument)
{
   return ::eval(in_, argument.name_);   
}


bool fp::method::eval_out(const arg& argument)
{
   return ::eval(out_, argument.name_);
}


bool fp::fire_and_forget_method::eval_in(const arg& argument)
{
   return ::eval(in_, argument.name_);
}


bool fp::broadcast::eval_out(const arg& argument)
{
   return ::eval(args_, argument.name_);
}


bool fp::struct_::eval(const struct_entry& item)
{
   return ::eval(values_, item.name_);   
}


bool fp::enumeration::eval(const enum_value& value)
{
   return ::eval(values_, value.name_);   
}


bool fp::extended_error::eval(const enum_value& value)
{
   return ::eval(values_, value.name_);
}


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


// FIXME looks the same like struct_ since inherited, can we skip?
BOOST_FUSION_ADAPT_STRUCT(
   fp::union_,
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
    (std::vector<fp::import_type>, imports_)
    (std::vector<fp::doc_item_type>, parseitems_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::typedef_,   
   (std::string, name_)
   (std::string, type_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::array,
   (std::string, name_)
   (std::string, type_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::namespace_import,
   (std::vector<std::string>, items_)
   (std::string, file_)
)


BOOST_FUSION_ADAPT_STRUCT(
   fp::map,
   (std::string, name_)   
   (std::string, key_)
   (std::string, value_)
)


// ---------------------------------------------------------------------


#define identifier      lexeme[+char_("a-zA-Z0-9_")]
#define type_identifier lexeme[+char_("a-zA-Z0-9_")]
#define type_reference  lexeme[+char_("a-zA-Z0-9_.")]

#define inargs(where)    (lit("in") >>  '{' >> *arg_[_pass = phx::bind(&fp::where::eval_in,  _val, qi::_1)] >> '}') 
#define outargs(where)   (lit("out") >> '{' >> *arg_[_pass = phx::bind(&fp::where::eval_out, _val, qi::_1)] >> '}')      

#define SKIPPER_PARSE_PHRASE (ascii::space | "//" >> *(char_ - eol) >> eol | "/*" >> *(char_ - "*/") >> "*/")

typedef BOOST_TYPEOF(SKIPPER_PARSE_PHRASE) skipper_type;


template <typename IteratorT>
struct grammar : qi::grammar<IteratorT, fp::document(), skipper_type>
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
            >> '{' >> -inargs(method) >> -outargs(method) >> -errors_ >> '}';
               
      fireforget_method_ %=
         lit("method") >> identifier 
            >> lit("fireAndForget") >> '{' >> -inargs(fire_and_forget_method) >> '}';
         
      attribute_ %=
         lit("attribute") 
            >> type_reference >> identifier 
            >> -( lit("readonly")[phx::at_c<2>(_val) = true] 
                 ^ lit("noSubscriptions")[phx::at_c<3>(_val) = true] );
            
      broadcast_ %=
         lit("broadcast") >> identifier
            >> '{' 
               >> -outargs(broadcast) 
            >> '}';
      
      extended_error_ %= 
         -( lit("extends") >> type_reference ) 
            >> '{' 
               >> *enumeration_decl_[_pass = phx::bind(&fp::extended_error::eval, _val, qi::_1)]
            >> '}';         
         
      errors_ %= lit("error") 
         >> type_reference | extended_error_;
            
      enumerator_value_ %=
         lit('=') >> '"' >> (int_|hex) >> '"';
      
      enumeration_decl_ %= 
         type_identifier >> -enumerator_value_;
            
      structentry_ %= 
         type_reference >> identifier
         ;
         
      structure_ %=
         lit("struct") >> identifier
            >> -( lit("extends") >> type_reference )
            >> '{' 
               >> *structentry_[_pass = phx::bind(&fp::struct_::eval, _val, qi::_1)]
            >> '}';
            
      union_ %=
         lit("union") >> identifier
            >> -( lit("extends") >> type_reference )
            >> '{' 
               >> *structentry_
            >> '}';
            
      enumeration_ %= 
         lit("enumeration") >> identifier 
            >> -( lit("extends") >> type_reference )
            >> '{' 
               >> *enumeration_decl_[_pass = phx::bind(&fp::enumeration::eval, _val, qi::_1)]
            >> '}';
        
      array_ %= 
         lit("array") >> identifier >> "of" >> type_reference;
         
      map_ %= 
         lit("map") >> identifier >> '{' >> type_reference >> "to" >> type_reference >> '}';
        
      typedef_ %= 
         lit("typedef") >> identifier >> "is" >> type_reference;
         
      interface_ %=
         lit("interface") >> identifier
            >> '{'
               >> version_
               >> *( method_ 
                   | fireforget_method_ 
                   | broadcast_ 
                   | attribute_ 
                   | enumeration_ 
                   | structure_
                   | union_
                   | array_
                   | map_
                   | typedef_ )[_pass = phx::bind(&fp::interface::eval, _val, qi::_1)]
            >> '}';
       
      typecollection_ %=
         lit("typecollection") >> identifier
            >> '{'
               >> *( enumeration_ 
                   | structure_
                   | union_
                   | array_
                   | map_
                   | typedef_ )[_pass = phx::bind(&fp::typecollection::eval, _val, qi::_1)]
            >> '}';
            
      model_import_ %= 
         -lit("model") >> lit('"') >> lexeme[+(char_ - '"')] > '"';
         
      namespace_import_ %=
         lexeme[+char_("a-zA-Z0-9_*") % '.'] >> "from" >> lit('"') >> lexeme[+(char_ - '"')] >> '"';
         
      import_ %=
         lit("import") >> ( model_import_ | namespace_import_ );
      
      franca_ %= 
         package_ 
            >> *import_
            >> *( interface_ 
             | typecollection_ )[_pass = phx::bind(&fp::document::eval, _val, qi::_1)];
      
      package_.name("package");
      interface_.name("interface");
      version_.name("version");  
      method_.name("method");
      model_import_.name("import");            
   }
      
   qi::rule<IteratorT, fp::document(),                  skipper_type> franca_;
   qi::rule<IteratorT, std::vector<std::string>(),      skipper_type> package_;
   qi::rule<IteratorT, fp::interface(),                 skipper_type> interface_;
   qi::rule<IteratorT, fp::version(),                   skipper_type> version_;
   qi::rule<IteratorT, fp::method(),                    skipper_type> method_;
   qi::rule<IteratorT, fp::fire_and_forget_method(),    skipper_type> fireforget_method_;
   qi::rule<IteratorT, fp::broadcast(),                 skipper_type> broadcast_;
   qi::rule<IteratorT, fp::arg(),                       skipper_type> arg_;
   qi::rule<IteratorT, fp::typecollection(),            skipper_type> typecollection_;
   qi::rule<IteratorT, fp::attribute(),                 skipper_type> attribute_;
   qi::rule<IteratorT, fp::enumeration(),               skipper_type> enumeration_;
   qi::rule<IteratorT, fp::enum_value(),                skipper_type> enumeration_decl_;
   qi::rule<IteratorT, int(),                           skipper_type> enumerator_value_;
   qi::rule<IteratorT, fp::extended_error(),            skipper_type> extended_error_;
   qi::rule<IteratorT, fp::method_error(),              skipper_type> errors_;
   qi::rule<IteratorT, fp::struct_entry(),              skipper_type> structentry_;   
   qi::rule<IteratorT, fp::struct_(),                   skipper_type> structure_;   
   qi::rule<IteratorT, fp::union_(),                    skipper_type> union_;   
   qi::rule<IteratorT, fp::array(),                     skipper_type> array_;
   qi::rule<IteratorT, fp::map(),                       skipper_type> map_;
   qi::rule<IteratorT, fp::typedef_(),                  skipper_type> typedef_;
   qi::rule<IteratorT, std::string(),                   skipper_type> model_import_;
   qi::rule<IteratorT, fp::namespace_import(),          skipper_type> namespace_import_;
   qi::rule<IteratorT, fp::import_type(),               skipper_type> import_;
};


// ---------------------------------------------------------------------


template<typename IteratorT>
size_t get_line(IteratorT begin, IteratorT cur)
{                 
   size_t line = 1;
   
   while(begin != cur)
   {
      switch(*begin)
      {
      case '\n':
         ++line;
      default:
         break;
      }
      
      ++begin;
   }
   
   return line;
}


struct printer 
{
   typedef boost::spirit::utf8_string string;

   void element(string const& tag, string const& value, int depth) const 
   {
      for (int i = 0; i < (depth*4); ++i) 
         std::cout << ' '; // indent to depth    
           
      std::cout << "\"" << value << "\"";      
   }
};

void print_info(boost::spirit::info const& what) 
{
   using boost::spirit::basic_info_walker;

   printer pr;
   basic_info_walker<printer> walker(pr, what.tag, 0);
   boost::apply_visitor(walker, what.value);
}


fp::document fp::parse(const char* filename, const std::vector<std::string>& includes)
{
   std::ifstream in(filename, std::ios_base::in);

   if (!in && filename[0] != '/')    // only on relative paths
   {
      for (auto iter = includes.begin(); iter != includes.end() && !in; ++iter)
      {
         std::ostringstream oss;
         oss << *iter << '/' << filename;
         
         in.open(oss.str().c_str(), std::ios_base::in);
      }
   }
   
   if (!in)
      throw std::runtime_error(std::string("file not found: ") + filename);
   
   in.unsetf(std::ios::skipws); // No white space skipping!
 
   std::string str;  
   
   std::copy(
      std::istream_iterator<char>(in),
      std::istream_iterator<char>(),
      std::back_inserter(str));         
 
   //typedef boost::spirit::istream_iterator/*<char>*/ string_iterator_type;
   typedef std::string::iterator string_iterator_type;     
   //typedef boost::spirit::line_pos_iterator<string_iterator_type> iterator_type;
   
   grammar<string_iterator_type> grammar;
   fp::document doc;   
   
   string_iterator_type iter = str.begin();
   string_iterator_type end = str.end();
   
   //iterator_type iter(siter);
   //iterator_type end(send);   

   try
   {
      if (!qi::phrase_parse(iter, end, grammar, SKIPPER_PARSE_PHRASE, doc) || iter != end)
      {
         std::cout << "Parser failure at " << filename << ":" << get_line(str.begin(), iter) << std::endl;
         throw std::runtime_error("parser failure");
      }
   }
   catch(const boost::spirit::qi::expectation_failure<string_iterator_type>& f)
   {
      std::cout << "Parser failure at " << filename << ":" << get_line(str.begin(), f.first) << std::endl;
      std::cout << "   expected: "; 
      print_info(f.what_);
      std::cout << ", got:      " << std::string(f.first, f.first+20) << "..." << std::endl;
      
      throw std::runtime_error("parser failure");
   }
      
   return doc;
}
