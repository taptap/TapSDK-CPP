#pragma once

#include <vector>

namespace std {

template<typename T, typename Alloc, typename Predicate>
inline typename vector<T, Alloc>::size_type
erase_if_v2(vector <T, Alloc> &cont, Predicate pred) {
    const auto osz = cont.size();
    cont.erase(std::remove_if(cont.begin(), cont.end(), pred),
               cont.end());
    return osz - cont.size();
}

}