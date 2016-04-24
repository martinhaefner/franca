@require(interface)


#ifndef @{interface.fqn('_').upper()}SKELETON_HPP
#define @{interface.fqn('_').upper()}SKELETON_HPP

#include "core/dbus/skeleton.h"
#include "@{interface.fqn("/")[1:]}.hpp"


@{interface.package().namespaces_open()}


class @{interface.name()}Skeleton : public core::dbus::Skeleton<I@{interface.name()}>
{
public:
    @{interface.name()}Skeleton(const core::dbus::Bus::Ptr& bus) 
     : core::dbus::Skeleton<I@{interface.name()}>(bus)
     , object(access_service()->add_object_for_path(core::dbus::types::ObjectPath("@{interface.fqn("/")}")))
    {
       @for m in interface.methods :
       object->install_method_handler<I@{interface.name()}::@{m.name()}>(
            std::bind(&@{interface.name()}Skeleton::handle_@{m.name()}, 
                      this, std::placeholders::_1));
                              
       @end
    }

    ~@{interface.name()}Skeleton() noexcept = default;
    
    @for m in interface.methods :
    virtual @{m.out_args.typedef()} @{m.name()}(@{m.in_args.for_method_decl_in()}) = 0; 
    @end

private:
    
    @for m in interface.methods :
    void handle_@{m.name()}(const core::dbus::Message::Ptr& msg)
    {
        @if len(m.in_args) > 0:
        @{m.in_args.typedef()} in;
        msg->reader() >> in;
        @end
        @{len(m.out_args) > 0 and 'auto out = ' or ''}@{m.name()}(
        @if len(m.in_args) > 1:
        @for i in range(0, len(m.in_args)):
        @{i>0 and ', ' or '  '}std::get<@{str(i)}>(in)
        @end
        @elif len(m.in_args) == 1:
        in
        @end
        );
        
        auto reply = core::dbus::Message::make_method_return(msg);
        @if len(m.out_args) > 0:
        reply->writer() << out;
        @end
        
        access_bus()->send(reply);
    }
    @end
   
    core::dbus::Object::Ptr object;
};


@{interface.package().namespaces_close()}

#endif   // @{interface.fqn('_').upper()}SKELETON_HPP
