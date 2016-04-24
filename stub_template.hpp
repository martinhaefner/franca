@require(interface)


#ifndef @{interface.fqn('_').upper()}STUB_HPP
#define @{interface.fqn('_').upper()}STUB_HPP

#include "core/dbus/stub.h"

#include "@{interface.fqn("/")[1:]}.hpp"


@{interface.package().namespaces_open()}

class @{interface.name()}Stub : public core::dbus::Stub<I@{interface.name()}>
{
public:
   typedef std::shared_ptr<@{interface.name()}Stub> Ptr;

   @{interface.name()}Stub(const core::dbus::Bus::Ptr& bus) 
    : core::dbus::Stub<I@{interface.name()}>(bus)
    , object(access_service()->object_for_path(core::dbus::types::ObjectPath("@{interface.fqn("/")}")))
   {
      // NOOP
   }

   ~@{interface.name()}Stub() noexcept = default;

   @for m in interface.methods :
   @{m.out_args.typedef()} @{m.name()}(@{m.in_args.for_method_decl_in()})
   {
      auto result =
         object->invoke_method_synchronously<
            I@{interface.name()}::@{m.name()}, 
            I@{interface.name()}::@{m.name()}::ResultType>(
      @for i in range(0, len(m.in_args)):
      @{i>0 and ', ' or '  '}@{m.in_args[i].name()}_in
      @end
      );
      
      if (result.is_error())
         throw std::runtime_error(result.error().print());

      @{len(m.out_args) > 0 and 'return std::move(result.value());' or ''}
   }
   
   @end
   
private:
    core::dbus::Object::Ptr object;
};


@{interface.package().namespaces_close()}

#endif   // @{interface.fqn('_').upper()}STUB_HPP
