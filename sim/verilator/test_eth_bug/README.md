# Description
Found an issue where back to back ringbus messages cause a later one to be sent earlier

# Edit
bjm: disabling this as a guess, this and possibly more is broken since 767537f18f13802d134a070cfd7e711e5d9d8844 or earlier

# Bug
send 6 rb packets back to back. counting from zero, packet 1 will be overwritten by packet 5


# Results
turns out we can accept input DMA faster than we can deliver ringbus. an internal buffer in vmem builds up. when head = tail we set a value, FIXME we need to add this as a blink code.

# Recent changes
After writing this, I made LOTS of changes to how eth buffers work, I re-connected the ready signals which might cause chaos, test this.