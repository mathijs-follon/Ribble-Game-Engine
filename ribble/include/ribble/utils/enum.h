//
// Created by Mathijs Follon on 2/17/26.
//

#ifndef RIBBLE_ENUM_H
#define RIBBLE_ENUM_H
#include <cstddef>

struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

#endif //RIBBLE_ENUM_H