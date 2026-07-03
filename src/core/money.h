#pragma once

#include <compare>
#include <cstdint>

// Strong value type for money, stored in minor units (piastres/cents).
// Replaces raw qint64 so an amount can never be mixed up with an id or a
// count at a call site, and arithmetic that is meaningless for money
// (e.g. multiplying two amounts) does not compile.
//
// Money is a regular value type: cheap to copy, totally ordered,
// default-constructed to zero — it behaves like int in containers and
// signatures, and is deliberately Qt-free.
class Money {
public:
    constexpr Money() = default;

    // Named constructor keeps the unit explicit at every call site
    [[nodiscard]] static constexpr Money fromMinorUnits(std::int64_t units) noexcept
    {
        return Money(units);
    }

    [[nodiscard]] constexpr std::int64_t minorUnits() const noexcept { return m_minorUnits; }
    [[nodiscard]] constexpr bool isZero() const noexcept { return m_minorUnits == 0; }
    [[nodiscard]] constexpr bool isNegative() const noexcept { return m_minorUnits < 0; }
    [[nodiscard]] constexpr bool isPositive() const noexcept { return m_minorUnits > 0; }

    constexpr auto operator<=>(const Money&) const noexcept = default;

    constexpr Money& operator+=(Money other) noexcept
    {
        m_minorUnits += other.m_minorUnits;
        return *this;
    }
    constexpr Money& operator-=(Money other) noexcept
    {
        m_minorUnits -= other.m_minorUnits;
        return *this;
    }
    [[nodiscard]] friend constexpr Money operator+(Money lhs, Money rhs) noexcept
    {
        return lhs += rhs;
    }
    [[nodiscard]] friend constexpr Money operator-(Money lhs, Money rhs) noexcept
    {
        return lhs -= rhs;
    }
    [[nodiscard]] friend constexpr Money operator-(Money amount) noexcept
    {
        return Money(-amount.m_minorUnits);
    }

private:
    explicit constexpr Money(std::int64_t units) noexcept
        : m_minorUnits(units)
    {
    }

    std::int64_t m_minorUnits = 0;
};
