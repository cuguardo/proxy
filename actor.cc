#include "actor.hh"


// Now this function used only for dump the response header values only,
// but can be used to refactoring the answer content (rewrite absolute links in the body for instance)  
void homologate(std::shared_ptr<context_t> context, const std::string& id, msg_t& res, beast::error_code& ec)
{
    if(context->swear > 0)
    std::clog << "RESULT: " << res.result() << std::endl;
    if(context->swear > 1)
    {
        msg_t::header_type h = res.base();
        msg_t::fields_type f = h;

        for(auto i = f.begin(); i != f.end(); ++i)
        {
            std::string a(i->name_string()), v(i->value());
            std::clog << "\t" << a << " = " << v << std::endl;
        }
    }
}
