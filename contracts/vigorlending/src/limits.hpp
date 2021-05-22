#pragma once

#include "common.hpp"
//#include "debug.hpp"
#include <string>

typedef double percentage;


//----------------------- limit definitions -----------------------
template<typename T>
struct limits_numeric {
    typedef T type;
    static constexpr T min() { return std::numeric_limits<T>::min(); }
    static constexpr T max() { return std::numeric_limits<T>::max(); }
};

template<typename T, T min_value, T max_value>
struct limits_integer {
    typedef T type;
    static constexpr T min() { return static_cast<T>(min_value); }
    static constexpr T max() { return static_cast<T>(max_value); }
};

template<typename T>
struct limits_percent : limits_integer<T, 0, 100> {};

//----------------------- converters -----------------------
template<typename T, uint16_t F >
struct converter_factor {
    static constexpr double to(T value) { return (double)value / F; }
    static constexpr T from(double value) { return value * F; }
    static constexpr T from_string(const std::string &value) { 
        double result = 0;
        std::size_t index = value.find(".");
        if(index == std::string::npos)
            result = std::strtoull(value.c_str(), nullptr, 10) * F;
        else {
            int digits = 0;
            digits = value.length() - index - 1;
            result = (double)std::strtoull((value.substr(0, index) + value.substr(index + 1)).c_str(), nullptr, 10) * F;
            result /= pow(10, digits);        
        }

        return result;
    }
};

template<typename T>
struct converter_name {
    static constexpr eosio::name to(T value) { return value; }
    static constexpr T from(const std::string &value) { return eosio::name(value); }
    static constexpr T from_string(const std::string &value) { return eosio::name(value); }
};

template<typename T>
struct converter_numeric_integer : converter_factor<T, 1> {};

template<typename T>
struct converter_numeric_float : converter_factor<T, 1> {};



//----------------------- validators -----------------------

template<typename T>
struct validator_base {
    typedef T type;
    static constexpr void validate(T value) {}
};
template<typename T, uint64_t max_len = 256>
struct validator_string : validator_base<T> {
    static void validate(const std::string &value) {
        //METHOD_CONTEXT;
        eosio::check(value.size() <= max_len, "String length out of range [size:" + std::to_string(value.size()) + "]");
    }
};
template<typename limits, typename T = typename limits::type>
struct validator_numeric_base : validator_base<T> {
    static void validate(T value) {
        //METHOD_CONTEXT;
        eosio::check(limits::min() <= value && value <= limits::max(), "Numeric value out of range [" + std::to_string(limits::min()) + " <= " + std::to_string(value) + " <= " + std::to_string(limits::max()) + "]");
    }
};

template<typename T>
struct validator_null : validator_base<T> {};

template<typename T>
struct validator_generic : validator_numeric_base<limits_numeric<T>> {};

template<typename T, T min, T max>
struct validator_integer : validator_numeric_base<limits_integer<T, min, max>> {};

template<typename T>
struct validator_percentage : validator_numeric_base<limits_percent<T>> {};




//----------------------- field traits -----------------------

template<typename storageType, typename visualType, typename converter, typename validator>
struct field_type_traits {
    typedef storageType storage_t;
    typedef visualType visual_t;
    typedef converter converter_t;
    typedef validator validator_t;

    static constexpr void set(storageType &storage, const std::string &value) {
        // METHOD_CONTEXT;
        storage = converter_t::from_string(value);
        validator_t::validate(storage);
    }
};

template<typename T, T min_value, T max_value>
struct field_type_traits_number : field_type_traits<T, T, converter_numeric_integer<T>, validator_integer<T, min_value, max_value>> {};

template<typename T, uint16_t F, T min_value, T max_value>
struct field_type_traits_double_to_int : field_type_traits<T, double, converter_factor<T, F>, validator_integer<T, min_value, max_value>> {};

template<typename T>
struct field_type_traits_name : field_type_traits<T, eosio::name, converter_name<T>, validator_null<T>> {};
