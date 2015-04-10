#ifndef FRANCA_PARSER_H
#define FRANCA_PARSER_H


#include <string>
#include <vector>

#include <boost/variant.hpp>
#include <boost/optional.hpp>


namespace franca
{
   
namespace parser
{


struct version
{
   int major_;
   int minor_;
};


struct arg
{
   std::string type_;
   std::string name_;
};


struct attribute
{
   attribute()
    : readonly_(false)
    , no_subscriptions_(false)    
   {
      // NOOP
   }
   
   std::string type_;
   std::string name_;
   
   bool readonly_;   
   bool no_subscriptions_;  
};


struct enum_value
{
   enum_value()
    : value_(0xFFFFFFDD)
   {
      // NOOP
   }
   
   std::string name_;
   int value_;
};


struct enumeration
{
   bool eval(const enum_value& value);
   
   std::string name_;
   boost::optional<std::string> base_;
   
   std::vector<enum_value> values_;
};


struct typedef_
{   
   std::string name_;
   std::string type_;
};


struct array
{
   std::string name_;
   std::string type_;
};


struct map
{
   std::string name_;
   
   std::string key_;
   std::string value_;
};


struct struct_entry
{
   std::string type_;
   std::string name_;
};


struct struct_
{
   bool eval(const struct_entry& item);   
   
   std::string name_;
   boost::optional<std::string> base_;
   
   std::vector<struct_entry> values_;
};


struct union_ : struct_
{
   inline
   bool eval(const struct_entry& item)
   {
      return true;
   }
};


// TODO maybe merge with enumeration, the only difference is the missing name here
struct extended_error
{
   bool eval(const enum_value& value);
   
   boost::optional<std::string> base_;
   std::vector<enum_value> values_;
};


typedef boost::variant<std::string, extended_error> method_error;


struct method
{
   bool eval_in(const arg& argument);
   bool eval_out(const arg& argument);
   
   std::string name_;
   
   std::vector<arg> in_;
   std::vector<arg> out_;
   boost::optional<method_error> error_;
};


struct fire_and_forget_method
{
   bool eval_in(const arg& argument);
   
   std::string name_;
   
   std::vector<arg> in_;
};


struct broadcast
{
   bool eval_out(const arg& argument);
   
   std::string name_;   
   std::vector<arg> args_;
};


typedef boost::variant<method, 
                       fire_and_forget_method, 
                       broadcast, 
                       attribute, 
                       enumeration, 
                       struct_,
                       union_, 
                       array,
                       map,
                       typedef_
   > interface_item_type;


struct interface
{
   bool eval(const interface_item_type& new_item);
   
   std::string name_;
   version version_;
   
   std::vector<interface_item_type> parseitems_;
};


typedef boost::variant<enumeration, 
                       struct_, 
                       union_, 
                       array,
                       map,
                       typedef_
   > tc_item_type;


struct typecollection
{
   bool eval(const tc_item_type& new_item);
   
   std::string name_;
   std::vector<tc_item_type> parseitems_;
};


struct namespace_import
{
   std::vector<std::string> items_;
   std::string file_;
};


typedef boost::variant<interface, typecollection> doc_item_type;
typedef boost::variant<std::string, namespace_import> import_type;


struct document
{
   bool eval(const doc_item_type& new_item);
   
   std::vector<std::string> package_;
   std::vector<import_type> imports_;
   std::vector<doc_item_type> parseitems_;
};


document parse(const char* filename, const std::vector<std::string>& includes);

}   // parser

}   // franca


#endif   // FRANCA_PARSER_H
