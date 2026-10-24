# Permissions and process posture (draft)

Status: draft for review. Proposes a compact representation for the coarse identity layer
and for each process's sandbox posture. It sits alongside the capability model in
`capabilities.md`; it does not replace it.

## godcivspy: rwxrwxrwx
godcivspy is the official name for the nine permission bits `rwxrwxrwx`: read, write, and
execute for the owner, the party, and everyone else.

Name origins, as the author gave them:
- **God:** no comment.
- **Civ:** not spy. The game Civilization.
- **Spy:** no access. Team Fortress 2.

godcivspy is the name of the whole field, so the names do not map to the owner, party, and
other triplets. It is the kernel's name for `rwxrwxrwx`.

### Representation
- **The OS form:** a godcivspy value is nine bits in a `uint16_t`, with the top seven bits
  zero. The kernel stores it, checks it, and returns it from syscalls. Nothing else.
- **Text and octal are userland.** `rwxrwxrwx` and three-digit octal are produced by
  commands such as `ls` and `stat`, which turn a godcivspy value into text for people. The
  OS never produces either form, and the syscall interface never carries them.

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
- godcivspy decides, by uid and party, whether a caller may touch an object at all.
- The posture decides what the process may do with what it has. Town enforcement
  (`kernel.md`) applies on top of godcivspy, and a Town process cannot widen its godcivspy
  by any means.
- Capabilities (`capabilities.md`) remain the fine-grained grants. godcivspy is the
  coarse check that comes first.

## Open questions
1. **Is the posture the intended meaning of "8 levels"?** This draft reads it as three
   independent choices. If the intent was an ordered scale, say so and the bits change.
2. **Does godcivspy replace uid checks?** It can be the only identity layer, or it can
   sit under capabilities. This is the open question from the last discussion.
3. **Where does root sit?** Here root is a bit in the posture, and a uid 0 check in godcivspy
   word. The spec currently treats the root operator as unbounded, so the bit only marks
   programs run by root, not the operator.
4. **Special bits.** Should setuid, setgid, and sticky exist at all? Setuid and setgid let a
   program run as its owner, which conflicts with the no-ambient-authority rule in Town. If they
   are dropped, godcivspy is the whole permission field.
