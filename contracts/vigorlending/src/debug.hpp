#pragma once

#include "common.hpp"
#include <string>
#include <eosio/check.hpp>
#include <eosio/time.hpp>
#include <eosio/print.hpp>
#include <eosio/system.hpp>

namespace debug
{
    class context {
        const char *_function;
        unsigned short _line;
#ifdef USE_DEBUG_TIMERS        
        eosio::time_point _begin;
#endif
        static unsigned char _stackPosition;
    public:
        context(const char *function_name, unsigned short line);
        ~context();

        void update(unsigned short line);

        constexpr void default_log(std::string &output) const{
        #ifdef USE_SSTREAM    
            std::ostringstream result;
            result << std::string(_stackPosition * 2, ' ') << _function << ":" << _line - 1 << "=> ";
            output += result.str();
        #else
            output += std::string(_stackPosition * 2, ' ') + _function +  ":" + std::to_string(_line - 1) + "=> ";
        #endif
        }
        constexpr void default_extended_log(std::string &output) const{
        #ifdef USE_SSTREAM    
            std::ostringstream result;
            result << std::endl << std::endl
                << "========VIGOR CHECK FAILED========" << std::endl
                << "Function: " << _function << std::endl
                << "Line    : " << _line - 2 << std::endl
                ;
            output += result.str();
        #else
            output += std::string("\n\n========VIGOR CHECK FAILED========\nFunction: ") + _function +  "\nLine    : " + std::to_string(_line - 2) + "\nMessage : ";
        #endif
        }




        constexpr void stringify_data(std::string &output) const {  }
        constexpr void stringify_data(std::string &output, const char *data) const { output += data; }
        template<typename T, typename decayed = std::decay_t<T>>
        constexpr void stringify_data(std::string &output, T &&data) const {
            if constexpr (std::is_same_v<decayed, std::string>) {
                output += data;
            } else if constexpr (std::is_same_v<decayed, char *>) {
                output += data;
            } else if constexpr (std::is_class_v<decayed>) {
                output += data.to_string();
            } else if constexpr (std::is_same_v<decayed, bool>) {
                output += data ? "true" : "false";
            } else if constexpr (std::is_arithmetic_v<decayed>) {
                output += std::to_string(data);
            } else {
                throw_invalid_type(T);
            }
        }
        template<typename Arg, typename... Args>
        constexpr void stringify(std::string &output, Arg &&a, Args && ... args) const {
            stringify_data(output, std::forward<Arg>(a));
            if constexpr (sizeof...(Args) > 0) {
                output += " ";
                stringify(output, std::forward<Args>(args)...);
            }
        }




        template<typename Arg, typename... Args>
        void trace(Arg&& a, Args&&... args) const {
            std::string message;
            default_log(message);
            stringify(message, std::forward<Arg>(a), std::forward<Args>(args)..., "\n");

            eosio::print(message);
        }
        template<typename Arg, typename... Args>
        void check(bool pred, Arg&& a, Args&&... args) const {
            if(pred)    
                return;
            
            std::string message;
            default_extended_log(message);
            stringify(message, std::forward<Arg>(a), std::forward<Args>(args)..., "\n");

            eosio::check(false, message);
        }

        void noop() const;
    };
}

#define METHOD_CONTEXT debug::context __context(__PRETTY_FUNCTION__, __LINE__)
#define CHECK(pred, ...) {__context.update(__LINE__); __context.check(pred, __VA_ARGS__);}

#ifdef DEBUG_TRACE
#define TRACE(...) {__context.update(__LINE__); __context.trace(__VA_ARGS__);}
#define TRACE_STEP(step) TRACE("Step ", step)
#define MULTI_INDEX_INVALID_ITERATOR_BEGIN_WORKAROUND {__context.update(__LINE__); __context.noop();}
#else
#define TRACE(...) 
#define TRACE_STEP(step) 
#define MULTI_INDEX_INVALID_ITERATOR_BEGIN_WORKAROUND eosio::print("")
#endif