#ifndef INCLUDE_YAMAIL_DATA_REFLECTION_ADT_H_
#define INCLUDE_YAMAIL_DATA_REFLECTION_ADT_H_

#include <boost/preprocessor/tuple/remove.hpp>
#include <boost/fusion/adapted.hpp>

#define YR_INT_ADT_SET_MEMBER_NAME(R, CLASSNAME, IDX, MEMBERNAME)\
        template <> struct struct_member_name<CLASSNAME, IDX> {\
            typedef char const *type; \
            static type call() { return BOOST_PP_STRINGIZE(MEMBERNAME); } \
        };

#define YR_ADT_MEMBER_NAMES(CLASSNAME, NAMES)\
    namespace boost { namespace fusion { namespace extension {\
    BOOST_PP_SEQ_FOR_EACH_I_R(1,YR_INT_ADT_SET_MEMBER_NAME, CLASSNAME, NAMES) \
    } } }

#define YR_INT_REMOVE_FIRST_ATTR(R, DATA, ATTRS) BOOST_PP_TUPLE_REMOVE(ATTRS, 0)

#define YR_ADAPT_ADT_FILLER_0(A, B, C, D, E)\
    ((A, B, C, D, E)) YR_ADAPT_ADT_FILLER_1
#define YR_ADAPT_ADT_FILLER_1(A, B, C, D, E)\
    ((A, B, C, D, E)) YR_ADAPT_ADT_FILLER_0
#define YR_ADAPT_ADT_FILLER_0_END
#define YR_ADAPT_ADT_FILLER_1_END

#define YR_INT_GET_NAME(R, DATA, ATTRS) (BOOST_PP_TUPLE_ELEM(5, 0, ATTRS))

#define YR_INT_FOR_SEQUENCE(SEQ, MACRO)                                 \
    BOOST_PP_IF(                                                        \
        BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(SEQ)),                           \
        BOOST_PP_SEQ_FOR_EACH,                                          \
        BOOST_PP_TUPLE_EAT(5))(                                         \
            MACRO,                                                      \
            0,                                                          \
            BOOST_PP_SEQ_TAIL(SEQ))                                     \

#define YR_INT_FOR_ATTRS(ATTRIBUTES, MACRO)\
        YR_INT_FOR_SEQUENCE(\
                BOOST_PP_CAT(YR_ADAPT_ADT_FILLER_0(0,0,0,0,0)ATTRIBUTES,_END),\
                MACRO)

#define YR_INT_GET_NAMES(ATTRIBUTES)\
        YR_INT_FOR_ATTRS(ATTRIBUTES, YR_INT_GET_NAME)

#define YR_INT_GET_ADT_ATTRS(ATTRIBUTES)\
        YR_INT_FOR_ATTRS(ATTRIBUTES, YR_INT_REMOVE_FIRST_ATTR)




#define YREFLECTION_ADAPT_ADT(NAME, ATTRIBUTES)\
        BOOST_FUSION_ADAPT_ADT(NAME, YR_INT_GET_ADT_ATTRS(ATTRIBUTES))\
        YR_ADT_MEMBER_NAMES(NAME, YR_INT_GET_NAMES(ATTRIBUTES))

#define YREFLECTION_PROPERTY_SETTER(NAME) obj.NAME(val)
#define YREFLECTION_PROPERTY_GETTER(NAME) obj.NAME()

#define YREFLECTION_PROPERTY(TYPE, NAME)\
    (NAME, TYPE, TYPE, YREFLECTION_PROPERTY_GETTER(NAME), YREFLECTION_PROPERTY_SETTER(NAME))

#define YREFLECTION_ROPROPERTY(TYPE, NAME)\
    (NAME, TYPE, TYPE, YREFLECTION_PROPERTY_GETTER(NAME), (void)obj; (void)val )

#define YREFLECTION_SETTER(NAME) obj.set##NAME(val)
#define YREFLECTION_GETTER(NAME) obj.get##NAME()

#define YREFLECTION_GETSET(TYPE, NAME)\
    (NAME, TYPE, TYPE, YREFLECTION_GETTER(NAME), YREFLECTION_SETTER(NAME))

#define YREFLECTION_GET(TYPE, NAME)\
    (NAME, TYPE, TYPE, YREFLECTION_GETTER(NAME), (void)obj; (void)val )

#define YREFLECTION_SETTER_(NAME) obj.set_##NAME(val)
#define YREFLECTION_GETTER_(NAME) obj.get_##NAME()

#define YREFLECTION_GETSET_(TYPE, NAME)\
    (NAME, TYPE, TYPE, YREFLECTION_GETTER_(NAME), YREFLECTION_SETTER_(NAME))

#define YREFLECTION_GET_(TYPE, NAME)\
    (NAME, TYPE, TYPE, YREFLECTION_GETTER_(NAME), (void)obj; (void)val )

#endif /* INCLUDE_YAMAIL_DATA_REFLECTION_ADT_H_ */
