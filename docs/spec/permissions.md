# Permissions and process posture (draft)

Status: draft for review. Proposes a compact representation for the coarse identity layer
and for each process's sandbox posture. It sits alongside the capability model in
`capabilities.md`; it does not replace it.

## godmedspy: the 12-bit permission word
The permission word is called **godmedspy**. Its layout follows the Linux mode field,
so the meaning carries over directly.
Objects and processes carry a 12-bit godmedspy word:

| Bits | Octal digit | Meaning |
|---|---|---|
| 11 | 4 | setuid-style: a program runs as its owner's uid |
| 10 | 4 | setgid-style: a program runs with its party's gid |
| 9 | 4 | sticky: only the owner may remove or rename an entry |
| 8-6 | 3 | owner: read, write, execute |
| 5-3 | 2 | party: read, write, execute |
| 2-0 | 1 | other: read, write, execute |

Each rwx triplet is 3 bits, which is one octal digit. Stored in a `uint16_t`, with the top
4 bits reserved and zero. Ownership, party, and the three rwx triplets are the same as
`rwxrwxrwx`; godmedspy is the name for this word as a whole.

## Process posture: 3 bits, 8 levels
Each process carries a 3-bit posture. Each bit is one independent choice:

| Bit | 0 | 1 |
|---|---|---|
| 0 (value 1) | Room | Town (`kernel.md`) |
| 1 (value 2) | Pure | Filesystem (`isolation.md`) |
| 2 (value 4) | not root | root |

So posture 0 is a Room, Pure, non-root process. Posture 7 is a Town, Filesystem, root
process. The three choices already exist in the specs; this only packs them.

## How the two layers combine
- godmedspy decides, by uid and party, whether a caller may touch an object at all.
- The posture decides what the process may do with what it has. Town enforcement
  (`kernel.md`) applies on top of godmedspy, and a Town process cannot widen its godmedspy
  by any means.
- Capabilities (`capabilities.md`) remain the fine-grained grants. godmedspy is the
  coarse check that comes first.

## Open questions
1. **Is the posture the intended meaning of "8 levels"?** This draft reads it as three
   independent choices. If the intent was an ordered scale, say so and the bits change.
2. **Does godmedspy replace uid checks?** It can be the only identity layer, or it can
   sit under capabilities. This is the open question from the last discussion.
3. **Where does root sit?** Here root is a bit in the posture, and a uid 0 check in godmedspy
   word. The spec currently treats the root operator as unbounded, so the bit only marks
   programs run by root, not the operator.
