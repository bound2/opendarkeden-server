//--------------------------------------------------------------------------------
//
// Filename    : DescriptorTable.h
// Description : Bounds for the player tables the connection managers index
//               by socket descriptor.
//
//--------------------------------------------------------------------------------

#ifndef __DESCRIPTOR_TABLE_H__
#define __DESCRIPTOR_TABLE_H__

namespace de {

//--------------------------------------------------------------------------------
//
// The connection managers keep their players in a fixed array indexed by
// socket descriptor and remember the smallest and largest descriptor in it,
// so every sweep is a walk over the closed range [minFD, maxFD]. The range
// is seeded from the listening socket and grown by accepted ones, so it can
// name a descriptor the array does not have.
//
// DescriptorRange is that walk clamped to the array: `first` is never
// negative and `last` never reaches the array's size, and the range is empty
// when nothing between minFD and maxFD is an index the array holds. A walk
// that runs it therefore reads the table only where the table exists.
//
//--------------------------------------------------------------------------------

struct DescriptorRange {
    int first;
    int last;

    constexpr bool empty() const {
        return first > last;
    }
};

// An empty range, written so that `for (int i = first; i <= last; i++)` runs
// no iteration.
constexpr DescriptorRange kEmptyDescriptorRange{0, -1};

constexpr DescriptorRange descriptorRange(int minFD, int maxFD, int tableSize) {
    if (tableSize <= 0)
        return kEmptyDescriptorRange;

    // A negative minimum is the "no connection" marker the managers write,
    // and no descriptor below zero is an index.
    const int first = minFD < 0 ? 0 : minFD;
    const int last = maxFD < tableSize ? maxFD : tableSize - 1;

    if (last < first)
        return kEmptyDescriptorRange;

    return DescriptorRange{first, last};
}

// True when the listening socket's descriptor is one the table can hold. A
// listener outside it would be skipped by every clamped walk, so its own
// manager could never see a connection arrive.
constexpr bool fitsDescriptorTable(int fd, int tableSize) {
    return fd >= 0 && fd < tableSize;
}

} // namespace de

#endif
