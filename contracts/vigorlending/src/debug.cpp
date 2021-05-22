#include "debug.hpp"

#include <eosio/print.hpp>
#include <eosio/check.hpp>
#ifdef USE_SSTREAM
    #include <sstream>
#endif

namespace debug
{

unsigned char context::_stackPosition = 0;

context::context(const char *function_name, unsigned short line)
    : _function(function_name)
    , _line(line)
#ifdef USE_DEBUG_TIMERS
    , _begin(eosio::current_time_point()) 
#endif
{
#ifdef DEBUG_OUTPUT_STACKCALLS
    eosio::print(std::string(_stackPosition * 2, ' '), "->", _function, "\n");
#endif
    ++_stackPosition;
}

context::~context()
{
    --_stackPosition;

#ifdef DEBUG_OUTPUT_STACKCALLS
    eosio::time_point end = eosio::current_time_point();
    std::string message;
#ifdef USE_SSTREAM    
    std::ostringstream result;
    result << std::string(_stackPosition * 2, ' ') << "<-" << _function << " - ";
#ifdef USE_DEBUG_TIMERS
    eosio::microseconds diff = end - _begin;
    result << diff.count() << "usec [" << _begin.time_since_epoch().count() << "/" << end.time_since_epoch().count();
#endif
    result << std::endl;
    message = result.str();
#else
    message = std::string(_stackPosition * 2, ' ') + "<-" + _function + "\n";
#ifdef USE_DEBUG_TIMERS
    eosio::microseconds diff = end - _begin;
    message +=  + " - " + std::to_string(diff.count()) + "usec [" + std::to_string(_begin.time_since_epoch().count()) + "/" + std::to_string(end.time_since_epoch().count()) + "]\n";
#endif
#endif

    eosio::print(message);
#endif
}

void context::update(unsigned short line)
{ 
    _line = line; 
}


void context::noop() const {
    eosio::print("");
}

} // namespace debug