/**
 * @internal
 * @brief Internal cuStinger types
 * @details Lowest level layer of the cuStinger programming model
 *          (hidden for the users)
 * @author Federico Busato                                                  <br>
 *         Univerity of Verona, Dept. of Computer Science                   <br>
 *         federico.busato@univr.it
 * @date April, 2017
 * @version v2
 *
 * @copyright Copyright © 2017 cuStinger. All rights reserved.
 *
 * @license{<blockquote>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * </blockquote>}
 *
 * @file
 */
#pragma once

#include <tuple>
#include "Support/Metaprogramming.hpp"  //xlib::TupleToTypeSize
#include "Support/Basic.hpp"            //xlib::byte_t
#include "Support/Numeric.hpp"          //xlib::roundup_pow2

template<typename... TArgs>
using TypeList = std::tuple<TArgs...>;

namespace cu_stinger {
//------------------------------------------------------------------------------

//User configuration include
#include "../config.inc"

//------------------------------------------------------------------------------

using xlib::byte_t;

using degree_t = int;
using   edge_t = typename xlib::TupleConcat<TypeList<id_t>, EdgeTypes>::type;

struct ALIGN(16) VertexBasicData {
    byte_t* __restrict__ edge_ptr;
    degree_t degree;
};

using vertex_t = typename xlib::TupleConcat<TypeList<VertexBasicData>,
                                            VertexTypes>::type;

//------------------------------------------------------------------------------

static_assert(xlib::IsPower2<MIN_EDGES_PER_BLOCK>::value  &&
              xlib::IsPower2<EDGES_PER_BLOCKARRAY>::value &&
              MIN_EDGES_PER_BLOCK <= EDGES_PER_BLOCKARRAY,
              "Memory Management Constrains");

static_assert(std::is_integral<id_t>::value, "id_t must be integral");
static_assert(std::is_integral<off_t>::value, "off_t must be integral");

static_assert(std::is_same<degree_t, int>::value ||
              std::is_same<degree_t, unsigned>::value ||
              std::is_same<degree_t, unsigned long long>::value,
              "degree_t type must allows atomicAdd operation");

//------------------------------------------------------------------------------

using      VTypeSize = typename xlib::TupleToTypeSize<vertex_t>::type;
using      ETypeSize = typename xlib::TupleToTypeSize<edge_t>::type;
using ExtraVTypeSize = typename xlib::TupleToTypeSize<VertexTypes>::type;
using ExtraETypeSize = typename xlib::TupleToTypeSize<EdgeTypes>::type;
using    VTypeSizePS = typename xlib::ExcPrefixSum<VTypeSize>::type;
using    ETypeSizePS = typename xlib::ExcPrefixSum<ETypeSize>::type;

extern const VTypeSize      VTYPE_SIZE;
extern const ETypeSize      ETYPE_SIZE;
extern const ExtraVTypeSize EXTRA_VTYPE_SIZE;
extern const ExtraETypeSize EXTRA_ETYPE_SIZE;

extern const VTypeSizePS    VTYPE_SIZE_PS;
extern const ETypeSizePS    ETYPE_SIZE_PS;

const unsigned NUM_EXTRA_VTYPES = std::tuple_size<VertexTypes>::value;
const unsigned NUM_EXTRA_ETYPES = std::tuple_size<EdgeTypes>::value;
const unsigned       NUM_VTYPES = std::tuple_size<vertex_t>::value;
const unsigned       NUM_ETYPES = std::tuple_size<edge_t>::value;

namespace detail {

HOST_DEVICE degree_t limit(degree_t degree) noexcept {
    #if defined(__CUDACC__)
        return ::max(static_cast<degree_t>(MIN_EDGES_PER_BLOCK),
                     xlib::roundup_pow2(degree + 1));
    #else
        return std::max(static_cast<degree_t>(MIN_EDGES_PER_BLOCK),
                        xlib::roundup_pow2(degree + 1));
    #endif
}

} //namespace detail

} // namespace cu_stinger
