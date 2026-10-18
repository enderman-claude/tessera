# Driver classes

Every driver declares one access class in its manifest. The class decides how the
driver reaches hardware and which capabilities the loader may hand it. A driver
cannot ask for a stronger class at runtime.

| Class | Name | Owns | Reaches the bus by | Typical drivers |
|---|---|---|---|---|
| D0 | Mediated | nothing directly | kernel calls, validated per call | timer, interrupt controller, serial, PCI config, power, platform devices |
| D1 | Direct | one device function | its own BAR MMIO, its own queues, its own MSI vectors, DMA through an IOMMU domain | NVMe, network card, GPU command rings, USB host controller queues |
| D2 | Bus controller | one bus controller and everything below it | D1 rights for the controller, plus enumerating and delegating child devices | USB host controller hub, SATA/AHCI controller with port children, a PCI bridge's downstream bus |

## The rule that separates the classes
- Data-path registers belong to the device. A D1 or D2 driver may write them directly.
- Configuration registers belong to the kernel. PCI config space, bridge windows, BAR
  placement, bus-master enable of other functions, and interrupt controller routing are
  never written by a driver. Only L4 (device manager) asks the kernel to change them, and
  the kernel does the write after checking the request.

This is why direct drivers can be fast without being able to move another device's
memory window or steer interrupts elsewhere.

## Class D0: mediated
- No BAR, DMA, or MSI capabilities. Every hardware action is a kernel call.
- Calls: `dev_cfg_read(fn, off)`, `dev_cfg_write(fn, off, val)` (offsets on an allowlist
  per device class), `irq_claim(vector)`, `port_read/write(port)` (only if granted),
  `kcall` groups (timer, power, platform).
- Slowest, but each access is checked and audited. Default class for anything new.

## Class D1: direct
Granted at load time by L2 from L4's manifest:
- `bar(fn, n)`: read/write mapping of BAR n of exactly one function. Mapped by the kernel,
  then used by the driver with plain loads and stores.
- `msi(fn, count)`: the kernel programs this function's MSI or MSI-X table to deliver up to
  `count` vectors to the driver's endpoint. The driver cannot program the table itself.
- `dma(domain)`: DMA window inside one IOMMU domain. Must be backed by an IOMMU.
- `endpoint`: IPC to its supervisor and clients.

Forbidden for D1 (not granted, cannot be requested):
- any config-space write, including BAR placement and command register bits other than
  the ones the kernel exposes through `dev_cfg_write`
- port I/O (x86 ports are legacy and not allowed for direct drivers)
- any BAR of another function, any bridge window, interrupt controller registers

## Class D2: bus controller
Everything in D1 for the controller function, plus:
- `enum(bus)`: may ask the kernel to enumerate children under its bus. The kernel reads
  config space and reports children. The controller driver never reads config space
  itself.
- `delegate(child)`: may create a child capability (D1 or D0) for a child it discovered,
  as a weaker copy only. The child driver is then loaded by L4, not by the controller.

A bus controller cannot change its own upstream bridge, and cannot grant itself more
than its controller's BAR and DMA domain.

## Requirements the loader checks
1. A D1 or D2 driver requiring `dma` is refused when the machine has no IOMMU.
2. D1 and D2 drivers are refused when interrupt remapping is absent, unless the MSI
   address range is verified to point only at the driver's own vectors. Without this,
   a device can forge interrupts for another device. Default: refuse.
3. A D0 manifest that lists `bar`, `msi`, or `dma` is rejected. The class is checked, not
   trusted from the manifest's wording.
4. A class change requires a new manifest signed by L4 and a restart of the driver.

## What happens on violation
- An attempted forbidden access (wrong BAR, config write, port I/O from D1) returns an
  error and is written to the audit log with the driver's class.
- Repeated violations past a limit set by L4 stop the driver. L4 may restart it in
  class D0 only, if the manifest provides a D0 fallback.

## Open
- Whether the IOMMU is required on all targets or only on targets that have one. Current
  answer: required for D1 and D2 with DMA; optional for D1 without DMA.
- PCI config offsets allowed through `dev_cfg_write` for D0. Needs a list per device class.
