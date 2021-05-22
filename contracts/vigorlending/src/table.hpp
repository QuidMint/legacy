#pragma once

#include <string>
#include <eosio/name.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/variadic_seq_to_seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/enum.hpp>
#include <boost/preprocessor/seq/rest_n.hpp>
#include <boost/preprocessor/seq/pop_front.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/subseq.hpp>

#define FIELD_STORAGE_NAME(tupple)          BOOST_PP_TUPLE_ELEM(0, tupple)
#define FIELD_VISUAL_NAME(tupple)           BOOST_PP_TUPLE_ELEM(1, tupple)
#define FIELD_DATA(tupple)                  BOOST_PP_TUPLE_ELEM(2, tupple)
#define FIELD_DATA_TRAITS(tupple)           BOOST_PP_SEQ_ELEM(0, FIELD_DATA(tupple))
#define FIELD_DATA_DEFAULT_VALUE(tupple)    BOOST_PP_SEQ_ELEM(1, FIELD_DATA(tupple))
#define FIELD_DATA_STORAGE_TYPE(tupple)     BOOST_PP_SEQ_ELEM(2, FIELD_DATA(tupple))
// #define FIELD_DATA_FACTOR(tupple)           BOOST_PP_SEQ_ELEM(3, FIELD_DATA(tupple))
// #define FIELD_DATA_MINIMUM(tupple)          BOOST_PP_SEQ_ELEM(4, FIELD_DATA(tupple))
// #define FIELD_DATA_MAXIMUM(tupple)          BOOST_PP_SEQ_ELEM(5, FIELD_DATA(tupple))

#define FIELD(storageName, visualName, data) \
    (storageName, visualName, data)

#define FIELD_DATA_DOUBLE_TO_INT(type, factor, minimum, maximum, defaultValue) \
    (field_type_traits_double_to_int)(defaultValue)(type)(factor)(minimum)(maximum)

#define FIELD_DATA_NUMBER(type, minimum, maximum, defaultValue) \
    (field_type_traits_number)(defaultValue)(type)(minimum)(maximum)

#define FIELD_DATA_NAME(type, defaultValue) \
    (field_type_traits_name)(defaultValue)(type)


#define FIELD_TYPEDEF_HANDLER(r, data, ELEM) \
    typedef FIELD_DATA_TRAITS(ELEM)<BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_REST_N(2, FIELD_DATA(ELEM)))> \
    FIELD_VISUAL_NAME(ELEM);

#define FIELD_DECLARATION_HANDLER(r, data, ELEM) \
    FIELD_DATA_STORAGE_TYPE(ELEM) FIELD_STORAGE_NAME(ELEM) = FIELD_VISUAL_NAME(ELEM)::converter_t::from(FIELD_DATA_DEFAULT_VALUE(ELEM));

#define FIELD_ACCESSORS_HANDLER(r, data, ELEM) \
    FIELD_VISUAL_NAME(ELEM)::visual_t BOOST_PP_CAT(get,FIELD_VISUAL_NAME(ELEM))() const { return FIELD_VISUAL_NAME(ELEM)::converter_t::to(FIELD_STORAGE_NAME(ELEM)); } \
    void BOOST_PP_CAT(set,FIELD_VISUAL_NAME(ELEM))(const FIELD_VISUAL_NAME(ELEM)::visual_t &value) { FIELD_STORAGE_NAME(ELEM) = FIELD_VISUAL_NAME(ELEM)::converter_t::to(value); }

#define SETVALUE_SWITCH_HANDLER(r, data, ELEM) \
    case eosio::name(BOOST_PP_STRINGIZE(FIELD_STORAGE_NAME(ELEM))).value: \
        FIELD_VISUAL_NAME(ELEM)::set(FIELD_STORAGE_NAME(ELEM), value); \
        break;

#define SERIALIZER_HANDLER(r, data, ELEM) \
    (FIELD_STORAGE_NAME(ELEM))
    

#define TABLE_HANDLER(NAME, SEQ) \
    BOOST_PP_SEQ_FOR_EACH(FIELD_TYPEDEF_HANDLER, (1), BOOST_PP_VARIADIC_SEQ_TO_SEQ(SEQ)) \
    BOOST_PP_SEQ_FOR_EACH(FIELD_DECLARATION_HANDLER, (1), BOOST_PP_VARIADIC_SEQ_TO_SEQ(SEQ)) \
    BOOST_PP_SEQ_FOR_EACH(FIELD_ACCESSORS_HANDLER, (1), BOOST_PP_VARIADIC_SEQ_TO_SEQ(SEQ)) \
    void set_value(const eosio::name &key, const std::string &value) { \
        switch(key.value) { \
            BOOST_PP_SEQ_FOR_EACH(SETVALUE_SWITCH_HANDLER, (1), BOOST_PP_VARIADIC_SEQ_TO_SEQ(SEQ)) \
        } \
    } \
    EOSLIB_SERIALIZE( NAME, \
        BOOST_PP_SEQ_FOR_EACH(SERIALIZER_HANDLER, NAME, BOOST_PP_VARIADIC_SEQ_TO_SEQ(SEQ)) \
    )
