---
key: Renderer
summary: Which graphics interface the Windows build drew through; macOS always uses Metal.
when_omitted:
  kind: value
  value: "0"
  note: Every value selects Metal on macOS.
---

The number selected the graphics interface the Windows build asked for when it started. On macOS the game always draws through Metal, whatever the value, so the setting has no effect. It is still read and kept, so a settings file shared with the Windows build is unchanged.

| Value | Interface on Windows |
| --- | --- |
| `0` | Chosen automatically |
| `1` | Direct3D 11 |
| `2` | Direct3D 12 |
| `3` | Vulkan |
| `4` | OpenGL |

The [debug log](/using/debug-logging/) names the interface that started.
