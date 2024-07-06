#pragma once

#include <memory>
#include <optional>
#include <span>
#include <vector>
#include <string_view>

#include "memory_scanner/mb_kmp.h"

namespace blook {
    class Process;

    class Function;

    class Pointer {

    protected:
        size_t offset;
        std::shared_ptr<Process> proc;
    public:
        static void *malloc_rwx(size_t size);

        static void protect_rwx(void *p, size_t size);

        static void *malloc_near_rwx(void *near, size_t size);

        enum class MemoryProtection {
            Read = 0x0001,
            Write = 0x0010,
            Execute = 0x0100,
            ReadWrite = Read | Write,
            ReadWriteExecute = Read | Write | Execute,
            ReadExecute = Read | Execute,
            rw = ReadWrite,
            rwx = ReadWriteExecute,
            rx = ReadExecute
        };

        void *malloc(size_t size, void *near,
                     MemoryProtection protection = MemoryProtection::rw);

        void *malloc(size_t size, MemoryProtection protection = MemoryProtection::rw);

        std::vector<uint8_t> read(void *ptr, size_t size);

        std::span<uint8_t> read_leaked(void *ptr, size_t size);

        template<typename Struct>
        inline std::optional<Struct> read(void *ptr) {
            const auto val = read_leaked(ptr, sizeof(Struct));
            return reinterpret_cast<Struct>(val.data());
        }

        std::optional<std::vector<uint8_t>> try_read(void *ptr, size_t size);

        explicit Pointer(std::shared_ptr<Process> proc);

        Pointer(std::shared_ptr<Process> proc, void *offset);

        Pointer(std::shared_ptr<Process> proc, size_t offset);

        Function as_function();

        [[nodiscard]] void *data() const;

        // Pointer operations
        inline Pointer add(const auto &t) {
            return {proc, (void *) (offset + (size_t) t)};
        }

        inline Pointer sub(const auto &t) {
            return {proc, (void *) (offset - (size_t) t)};
        }

        inline auto operator+(const auto &t) { return this->add(t); }

        inline auto operator-(const auto &t) { return this->sub(t); }

        inline auto operator<=>(const Pointer &o) const {
            return this->offset <=> o.offset;
        }
    };

    class MemoryRange : public Pointer {
        size_t _size;
    public:
        MemoryRange(std::shared_ptr<Process> proc, void *offset, size_t size);

        [[nodiscard]] size_t size() const {
            return _size;
        }

        template<class Scanner = memory_scanner::mb_kmp>
        inline std::optional<Pointer> find_one(const std::vector<uint8_t> &pattern) {
            const auto span = std::span<uint8_t>((uint8_t *) offset, _size);
            const auto aa = span.data();
            std::optional<size_t> res = Scanner::searchOne(span, pattern);
            return res.and_then([this](const auto val) {
                return std::optional<Pointer>(Pointer(this->proc, val));
            });
        }

        inline auto find_one(std::string_view sv) {
            return find_one(std::vector<uint8_t>(sv.begin(), sv.end()));
        }
    };
} // namespace blook