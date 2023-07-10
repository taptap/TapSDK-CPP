#pragma once

#include <type_traits>  //  std::remove_const, std::is_base_of, std::is_same

#include "functional/cxx_universal.h"
#include "functional/cxx_type_traits_polyfill.h"
#include "type_traits.h"

namespace sqlite_orm {

    /** @short Base class for a custom table alias, column alias or expression alias.
     */
    struct alias_tag {};

    namespace internal {

        template<class A>
        SQLITE_ORM_INLINE_VAR constexpr bool is_alias_v = std::is_base_of<alias_tag, A>::value;

        template<class A>
        using is_alias = polyfill::bool_constant<is_alias_v<A>>;

        /** @short Alias of a column in a record set, see `orm_column_alias`.
         */
        template<class A>
        SQLITE_ORM_INLINE_VAR constexpr bool is_column_alias_v =
            polyfill::conjunction_v<is_alias<A>, polyfill::negation<polyfill::is_detected<type_t, A>>>;

        template<class A>
        using is_column_alias = is_alias<A>;

        /** @short Alias of any type of record set, see `orm_recordset_alias`.
         */
        template<class A>
        SQLITE_ORM_INLINE_VAR constexpr bool is_recordset_alias_v =
            polyfill::conjunction_v<is_alias<A>, polyfill::is_detected<type_t, A>>;

        template<class A>
        using is_recordset_alias = polyfill::bool_constant<is_recordset_alias_v<A>>;

        /** @short Alias of a concrete table, see `orm_table_alias`.
         */
        template<class A>
        SQLITE_ORM_INLINE_VAR constexpr bool is_table_alias_v = polyfill::conjunction_v<
            is_recordset_alias<A>,
            polyfill::negation<std::is_same<polyfill::detected_t<type_t, A>, std::remove_const_t<A>>>>;

        template<class A>
        using is_table_alias = polyfill::bool_constant<is_table_alias_v<A>>;
    }
}
