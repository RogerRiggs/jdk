/*
 * Copyright (c) 2003, 2024, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_GC_PARALLEL_PSVIRTUALSPACE_HPP
#define SHARE_GC_PARALLEL_PSVIRTUALSPACE_HPP

#include "memory/allocation.hpp"
#include "memory/reservedSpace.hpp"
#include "memory/virtualspace.hpp"

// VirtualSpace for the parallel scavenge collector.
//
// VirtualSpace is data structure for committing a previously reserved address
// range in smaller chunks.

class PSVirtualSpace : public CHeapObj<mtGC> {
  friend class VMStructs;

  // The space is committed/uncommitted in chunks of size _alignment.  The
  // ReservedSpace passed to initialize() must be aligned to this value.
  const size_t _alignment;

  // Reserved area
  char* _reserved_low_addr;
  char* _reserved_high_addr;

  // Committed area
  char* _committed_low_addr;
  char* _committed_high_addr;

  // The entire space has been committed and pinned in memory, no
  // os::commit_memory() or os::uncommit_memory().
  bool _special;

 public:
  PSVirtualSpace(ReservedSpace rs, size_t alignment);

  ~PSVirtualSpace();

  bool is_in_committed(const void* p) const {
    return (p >= committed_low_addr()) && (p < committed_high_addr());
  }

  bool is_in_reserved(const void* p) const {
    return (p >= reserved_low_addr()) && (p < reserved_high_addr());
  }

  // Accessors (all sizes are bytes).
  size_t alignment()          const { return _alignment; }
  char* reserved_low_addr()   const { return _reserved_low_addr; }
  char* reserved_high_addr()  const { return _reserved_high_addr; }
  char* committed_low_addr()  const { return _committed_low_addr; }
  char* committed_high_addr() const { return _committed_high_addr; }
  bool  special()             const { return _special; }

  // Return size in bytes
  inline size_t committed_size()   const;
  inline size_t reserved_size()    const;
  inline size_t uncommitted_size() const;

  // Operations.
  inline  void   set_reserved(char* low_addr, char* high_addr, bool special);
  inline  void   set_reserved(ReservedSpace rs);
  inline  void   set_committed(char* low_addr, char* high_addr);
  virtual bool   expand_by(size_t bytes);
  virtual bool   shrink_by(size_t bytes);
  void           release();

#ifndef PRODUCT
  // Debugging
  void verify() const;

  // Helper class to verify a space when entering/leaving a block.
  class PSVirtualSpaceVerifier: public StackObj {
   private:
    const PSVirtualSpace* const _space;
   public:
    PSVirtualSpaceVerifier(PSVirtualSpace* space): _space(space) {
      _space->verify();
    }
    ~PSVirtualSpaceVerifier() { _space->verify(); }
  };
#endif

  virtual void print_space_boundaries_on(outputStream* st) const;

 // Included for compatibility with the original VirtualSpace.
 public:
  // Committed area
  char* low()  const { return committed_low_addr(); }
  char* high() const { return committed_high_addr(); }

  // Reserved area
  char* low_boundary()  const { return reserved_low_addr(); }
  char* high_boundary() const { return reserved_high_addr(); }
};

//
// PSVirtualSpace inlines.
//

inline size_t PSVirtualSpace::committed_size() const {
  return pointer_delta(committed_high_addr(), committed_low_addr(), sizeof(char));
}

inline size_t PSVirtualSpace::reserved_size() const {
  return pointer_delta(reserved_high_addr(), reserved_low_addr(), sizeof(char));
}

inline size_t PSVirtualSpace::uncommitted_size() const {
  return reserved_size() - committed_size();
}

inline void PSVirtualSpace::set_reserved(char* low_addr, char* high_addr, bool special) {
  _reserved_low_addr = low_addr;
  _reserved_high_addr = high_addr;
  _special = special;
}

inline void PSVirtualSpace::set_reserved(ReservedSpace rs) {
  set_reserved(rs.base(), rs.base() + rs.size(), rs.special());
}

inline void PSVirtualSpace::set_committed(char* low_addr, char* high_addr) {
  _committed_low_addr = low_addr;
  _committed_high_addr = high_addr;
}

#endif // SHARE_GC_PARALLEL_PSVIRTUALSPACE_HPP
