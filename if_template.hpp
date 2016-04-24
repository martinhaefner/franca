@require(interface)

#ifndef @{interface.fqn('_').upper()}_HPP
#define @{interface.fqn('_').upper()}_HPP

#include <chrono>

#include "core/dbus/traits/service.h"


@{interface.dependent_includes()}


@{interface.package().namespaces_open()}

class I@{interface.name()}
{
protected:
   @for m in interface.methods:
   struct @{m.name()}
   {
      typedef I@{interface.name()} Interface;
      
      inline static const std::string& name()
      {
         static const std::string s { "@{m.name()}" }; return s;
      }
      
      inline static const std::chrono::milliseconds default_timeout()
      {
         return std::chrono::seconds{1};
      }
      
      typedef @{m.in_args.typedef()}  ArgumentType;
      typedef @{m.out_args.typedef()} ResultType;

   };
   
   @end
};


@{interface.package().namespaces_close()}


namespace core {
namespace dbus {
namespace traits {

template<>
struct Service<@{interface.package().fqn("::")}::I@{interface.name()}>
{
    inline static const std::string& interface_name()
    {
        static const std::string s{"@{interface.fqn(".")[1:]}"};
        return s;
    }
};

} } }


#endif   // @{interface.fqn('_').upper()}_HPP
