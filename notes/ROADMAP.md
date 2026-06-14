## Roadmap

__This roadmap is read top-to-bottom, separated by priority.__

### Top priority
- [ ] __Typing system__: static structural typing, to be specific.
- [ ] __Object/structure definitions__: think of them as Luau's `type A = { x: int, y: int }`, or C's `struct`.
- [ ] __Proper compiler__: as of writing this I am about to make an _interpreter_ for Sol, in the future I want to make either a compiler that generates native assembly per-platform or bytecode + JIT.
- [ ] __Standard library__: a nice, bigger standard library than Lua's. _Post-compiler!_.
- [ ] __Enhanced module system__: `export/import` over Lua's table returning and `local mod = require('...')`.

### Medium priority
- [ ] __Namespaces__: `namespace` declaration for cleaner codebases.
- [ ] __FFI/Direct C interop__: even if developed in C, Sol won't come with C interoperability in the standard library by default, instead being added later. For now, I'll keep it in the roadmap.
- [ ] __Optimization__: a language that derives from Lua can't be slow: I will add a lot of optimizations along the way.
  - [ ] __Lexer optimizations__.
  - [ ] __Parser optimizations__.
- [ ] __Better errors/warnings__: right now errors are too simple, nicer errors (C-style?) will be added.

### Small priority or late development
- [ ] __GitHub org, website, documentation...__
- [ ] __Self-host__: write Sol in Sol.
- [ ] __Package manager__: something Lua seriously needs: a good way to add dependencies. Will fetch from GitHub.
- [ ] __Tooling__: includes:
  - [ ] __Treesitter__ (fast syntax highlighting for most editors).
  - [ ] __Language Server Protocol (LSP)__ to make coding in Sol nicer.
