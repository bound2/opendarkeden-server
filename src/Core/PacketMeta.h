//////////////////////////////////////////////////////////////////////////////
// Filename    : PacketMeta.h
// Description : Compile-time packet factory metadata (docs/TOOLCHAIN.md §3,
//               "Compile-time packet metadata"). Every packet factory states
//               its packet id, name and maximum body size as static
//               constexpr members; PacketFactoryType names that contract,
//               and FactoryList folds a pack of factories into a constexpr
//               table whose duplicate or out-of-range ids fail compilation.
//               Runtime creation stays virtual: PacketFactoryManager still
//               owns one heap factory per id and PacketFactory::createPacket
//               still allocates the packet.
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_PACKET_META_H
#define DARKEDEN_PACKET_META_H

#include <array>
#include <concepts>
#include <cstddef>

#include <string_view>
#include <type_traits>

#include "Packet.h"
#include "PacketFactory.h"

namespace de {

// A packet factory whose static facts are constant expressions. The
// integral_constant / bool_constant instantiations are what force the
// members to be usable at compile time: a factory that still computed its
// id or size in a virtual body would satisfy a plain `F::kPacketID` check.
template <typename F>
concept PacketFactoryType = std::derived_from<F, PacketFactory> && std::default_initializable<F> && requires {
    typename std::integral_constant<PacketID_t, F::kPacketID>;
    typename std::integral_constant<PacketSize_t, F::kMaxSize>;
    requires std::bool_constant<!F::kName.empty()>::value;
    requires std::same_as<std::remove_cvref_t<decltype(F::kName)>, std::string_view>;
};

namespace packet {

struct Meta {
    PacketID_t id;
    PacketSize_t maxSize;
    std::string_view name;
};

template <PacketFactoryType F> consteval Meta metaOf() {
    return Meta{F::kPacketID, F::kMaxSize, F::kName};
}

// Why a registration table is rejected. validateRegistry returns the first
// violation together with the offending id rather than asserting, so the
// tests pin each rule with a hand-built table and RegistryCheck can carry
// the id into the compiler diagnostic.
enum class RegistryError { None, IdOutOfRange, DuplicateId, EmptyName };

struct RegistryVerdict {
    RegistryError error = RegistryError::None;
    PacketID_t id = 0;
};

template <std::size_t N> consteval RegistryVerdict validateRegistry(const std::array<Meta, N>& metas) {
    std::array<bool, Packet::PACKET_MAX> seen{};
    for (const Meta& meta : metas) {
        if (meta.id >= Packet::PACKET_MAX)
            return {RegistryError::IdOutOfRange, meta.id};
        if (meta.name.empty())
            return {RegistryError::EmptyName, meta.id};
        if (seen[meta.id])
            return {RegistryError::DuplicateId, meta.id};
        seen[meta.id] = true;
    }
    return {};
}

// Instantiated with the verdict so a failure names the rule and the packet
// id in the "in instantiation of template class" note that follows the
// static_assert message.
template <RegistryError Error, PacketID_t Id> struct RegistryCheck {
    static_assert(Error == RegistryError::None,
                  "packet factory registration rejected: the RegistryError and the packet id are the "
                  "template arguments of this RegistryCheck");
    static constexpr bool ok = true;
};

// An ordered pack of factories with its metadata table validated while
// compiling. forEach visits each factory type in pack order through a
// std::type_identity tag, which is how PacketFactoryManager instantiates
// them without this header knowing about the manager.
template <PacketFactoryType... Factories> struct FactoryList {
    static constexpr std::size_t kCount = sizeof...(Factories);
    static constexpr std::array<Meta, kCount> kMeta{metaOf<Factories>()...};
    static constexpr RegistryVerdict kVerdict = validateRegistry(kMeta);
    // Naming ::ok forces the instantiation, so the check runs with the class.
    static_assert(RegistryCheck<kVerdict.error, kVerdict.id>::ok);

    template <typename Fn> static void forEach(Fn&& fn) {
        (fn(std::type_identity<Factories>{}), ...);
    }
};

template <typename... Lists> struct ConcatImpl;

template <PacketFactoryType... A> struct ConcatImpl<FactoryList<A...>> {
    using type = FactoryList<A...>;
};

template <PacketFactoryType... A, PacketFactoryType... B, typename... Rest>
struct ConcatImpl<FactoryList<A...>, FactoryList<B...>, Rest...> {
    using type = typename ConcatImpl<FactoryList<A..., B...>, Rest...>::type;
};

// FactoryList<A...> ++ FactoryList<B...> ++ ... ; the result is validated as
// one table, so ids duplicated across the parts are rejected too.
template <typename... Lists> using Concat = typename ConcatImpl<Lists...>::type;

} // namespace packet
} // namespace de

#endif // DARKEDEN_PACKET_META_H
