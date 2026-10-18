# Lib

Freestanding helpers with no dependencies.

## u64_to_dec
`size_t u64_to_dec(uint64_t value, char *out)`
Writes the decimal form of `value` and a NUL terminator into `out`. `out` must hold
at least 21 bytes. Returns the length excluding the NUL.
