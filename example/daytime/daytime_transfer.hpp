// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#include <neev/buffer/prefixed_buffer.hpp>

template <class TransferType>
using daytime_buffer = neev::prefixed16_buffer<TransferType>;
