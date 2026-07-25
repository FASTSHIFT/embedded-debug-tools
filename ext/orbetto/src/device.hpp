// Copyright (c) 2024, Auterion AG
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <unordered_map>
#include <string_view>
#include <string>
#include <memory>
#include <cstdint>

// enum for skynode device id
enum DeviceId
{
    SYKNODE_V5X = 0,
    SYKNODE_V6X = 1,
    SKYNODE_V6S = 2,
    GENERIC_ELF = 3,
};

class Device
{
public:
    using IrqTable = std::unordered_map<int16_t, std::string>;
    using RegisterTable = std::unordered_map<uint32_t, std::string_view>;
    Device() = default;

    /// Build the IRQ table from the ELF itself: on Cortex-M the exception
    /// vector table is an array of handler pointers, so entry N names
    /// exception N once the address is resolved through the symbol table.
    /// Vendor-agnostic -- works for any Cortex-M ELF, no device database.
    /// @param elfPath path to the ELF being decoded.
    /// @param err     optional; receives a human-readable reason on failure.
    /// @return a valid Device, or an invalid one if the vectors could not be
    ///         located (caller may then fall back to a built-in table).
    static Device
    fromElf(const std::string &elfPath, std::string *err = nullptr);

    /// Built-in device table, selected by explicit device name, e.g.
    /// "stm32h743" / "stm32h753" / "stm32f765" (the skynode aliases
    /// v6s/v6x/v5x and "nuttx" are also accepted). Only needed when the ELF
    /// has no usable vector table. The ELF *filename* is NOT consulted.
    explicit Device(std::string_view name);

    /// @return true if the device is valid.
    bool
    valid() const
    { return (bool)_irq_table; }

    /// Provides the IRQ table for a given device.
    const std::string_view
    irq(int16_t irq) const
    {
        if (_irq_table and _irq_table->contains(irq))
            return _irq_table->at(irq);
        return "unknown";
    }

    /// Provides largest IRQ number.
    int16_t
    max_irq() const
    { return _max_irq; }

    const std::string_view
    register_name(uint32_t addr) const
    {
        if (_register_table and _register_table->contains(addr))
            return _register_table->at(addr);
        return "";
    }

    /// Provides the clock frequency for a given device.
    /// @return 0 if unknown (ELF-derived devices cannot know it; pass -C).
    uint32_t
    clock() const
    { return _clock; }

    /// Provides the device id.
    int
    id() const
    { return (int)_id; }

    /// @return where this device description came from, for logging.
    const std::string &
    origin() const
    { return _origin; }

    /// @return comma-separated list of names accepted by the ctor.
    static const char *
    supported();

    /// Peripheral register names are the only genuinely vendor-specific data
    /// (used to annotate DMA slices); the ELF cannot provide them. Optional:
    /// without it register_name() returns "" and the annotation degrades.
    void
    attachRegisterTable(std::string_view name);


private:
    /* shared_ptr, not a raw pointer: fromElf() owns its table, and Device is
     * copy-assigned (`device = Device(...)`) -- a pointer into our own members
     * would dangle after the copy. Built-in tables are wrapped non-owning. */
    std::shared_ptr<const IrqTable> _irq_table{};
    const RegisterTable *_register_table{nullptr};
    uint32_t _clock{0};
    int16_t _max_irq{0};
    DeviceId _id{DeviceId::GENERIC_ELF};
    std::string _origin;
};
