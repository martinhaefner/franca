@require(interface)


#ifndef @{interface.fqn('_').upper()}_HPP
#define @{interface.fqn('_').upper()}_HPP


@{interface.dependent_includes()}


@{interface.package().namespaces_open()}

class @{interface.name()}Client : public Client
{
public:
   @{interface.name()}Client()
   {
      // NOOP
   }
   
   ~@{interface.name()}Client()
   {
      // NOOP
   }

   @for m in interface.methods :
   void @{m.name()}(@{m.in_args.for_method_decl_in()}@{ (len(m.in_args) > 0 and len(m.out_args) > 0) and ', ' or '' }@{m.out_args.for_method_decl_out()})
   {
      // TODO do anything
   }
   
   @end
   
   
   
private:
   
};


@{interface.package().namespaces_close()}

#endif   // @{interface.fqn('_').upper()}_HPP
