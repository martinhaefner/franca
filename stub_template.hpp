@require(interface)


#ifndef @{interface.fqn('_').upper()}STUB_HPP
#define @{interface.fqn('_').upper()}STUB_HPP


#include "@{interface.fqn("/")[1:]}.hpp"


@{interface.package().namespaces_open()}

class @{interface.name()}Stub : public public dbus::Stub<I@{interface.name()}>
{
public:
   typedef std::shared_ptr<@{interface.name()}Stub> Ptr;

   @{interface.name()}Stub(const dbus::Bus::Ptr& bus, const char* role) 
    : dbus::Stub<I@{interface.name()}>(bus)
    , object(access_service()->object_for_path(dbus::types::ObjectPath("@{interface.fqn("/")}" + stub)))
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
    dbus::Object::Ptr object;
};


@{interface.package().namespaces_close()}

#endif   // @{interface.fqn('_').upper()}STUB_HPP
