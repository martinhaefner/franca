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
   std::string name_;
   boost::optional<std::string> base_;
   
   std::vector<enum_value> values_;
};


struct struct_entry
{
   std::string type_;
   std::string name_;
};


struct struct_
{
   std::string name_;
   boost::optional<std::string> base_;
   
   std::vector<struct_entry> values_;
};


struct extended_error
{
   boost::optional<std::string> base_;
   std::vector<enum_value> values_;
};


typedef boost::variant<std::string, extended_error> method_error;


struct method
{
   std::string name_;
   
   std::vector<arg> in_;
   std::vector<arg> out_;
   boost::optional<method_error> error_;
};


struct fire_and_forget_method
{
   std::string name_;
   
   std::vector<arg> in_;
};


struct broadcast
{
   std::string name_;   
   std::vector<arg> args_;
};


typedef boost::variant<method, 
                       fire_and_forget_method, 
                       broadcast, 
                       attribute, 
                       enumeration, 
                       struct_> 
   interface_item_type;


struct interface
{
   std::string name_;
   version version_;
   
   std::vector<interface_item_type> parseitems_;
};


typedef boost::variant<enumeration, struct_> tc_item_type;


struct typecollection
{
   std::string name_;
   std::vector<tc_item_type> parseitems_;
};


typedef boost::variant<interface, typecollection> doc_item_type;


struct document
{
   std::vector<std::string> package_;
   std::vector<doc_item_type> parseitems_;
};


document parse(const char* filename);

}   // parser

}   // franca


#endif   // FRANCA_PARSER_H
