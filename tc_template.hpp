@require(typecollection)


#ifndef @{typecollection.fqn('_').upper()}_HPP
#define @{typecollection.fqn('_').upper()}_HPP


#include "dbus/core/codec.h"
@{typecollection.dependent_includes()}


@{typecollection.package().namespaces_open()}

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
typedef @{t.real_type().fqn("::")} @{t.name()};
   @end
   
@end

@{typecollection.package().namespaces_close()}


namespace dbus
{
namespace core
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
       out.push_int32(reinterpret_cast<int32_t>(value));
    }

    inline static 
    void decode_argument(Message::Reader& in, @{t.cpp_type()}& value)
    {
       value = reinterpret_cast<@{t.cpp_type()}>(in.pop_int32());
    }
};
@end

@end
@end

}   // namespace core
}   // namespace dbus


#endif   // @{typecollection.fqn('_').upper()}_HPP
