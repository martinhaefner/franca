@require(typecollection)


#ifndef @{typecollection.fqn('_').upper()}_HPP
#define @{typecollection.fqn('_').upper()}_HPP


#include "core/dbus/codec.h"
@{typecollection.dependent_includes()}


@{typecollection.package().namespaces_open()}
namespace @{typecollection.name()} {
   

@for t in typecollection.types:
   @if isinstance(t, franca.struct):
struct @{t.name()}
{
   @for m in t.members():
      @{m.first.cpp_type()} @{m.second};
   @end
}; 
   @elif isinstance(t, franca.enumeration):
enum class @{t.name()} : int32_t
{
@{t.print_enumerators()}
};
   @elif isinstance(t, franca.typedef):
typedef @{t.real_type().cpp_type()} @{t.name()};
   @end
   
@end

}   // namespace
@{typecollection.package().namespaces_close()}


namespace core
{
namespace dbus
{

@for t in typecollection.types:
@if isinstance(t, franca.struct):
template<>
struct Codec<@{t.fqn("::")}>
{
    inline static 
    void encode_argument(Message::Writer& out, const @{t.cpp_type()}& value)
    {
       @for m in t.members():
       Codec<@{m.first.cpp_type()}>::encode_argument(out, value.@{m.second});
       @end
    }

    inline static 
    void decode_argument(Message::Reader& in, @{t.cpp_type()}& value)
    {
       @for m in t.members():
       Codec<@{m.first.cpp_type()}>::decode_argument(in, value.@{m.second});
       @end
    }
};
@elif isinstance(t, franca.enumeration):
template<>
struct Codec<@{t.cpp_type()}>
{
    inline static 
    void encode_argument(Message::Writer& out, const @{t.cpp_type()}& value)
    {
       out.push_int32(int32_t(value));
    }

    inline static 
    void decode_argument(Message::Reader& in, @{t.cpp_type()}& value)
    {
       value = @{t.cpp_type()}(in.pop_int32());
    }
};
@end

@end

}   // namespace dbus
}   // namespace core


#endif   // @{typecollection.fqn('_').upper()}_HPP
