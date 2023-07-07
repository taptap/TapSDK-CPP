//
// Created by 甘尧 on 2022/6/16.
//

#pragma once

 namespace std_compat {

     template <class _Container, class _Predicate>
     inline typename _Container::size_type
     erase_if(_Container& __c, _Predicate __pred) {
         typename _Container::size_type __old_size = __c.size();

         const typename _Container::iterator __last = __c.end();
         for (typename _Container::iterator __iter = __c.begin(); __iter != __last;) {
             if (__pred(*__iter))
                 __iter = __c.erase(__iter);
             else
                 ++__iter;
         }

         return __old_size - __c.size();
     }

}
