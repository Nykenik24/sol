## Roadmap

__This roadmap is read top-to-bottom, separated by priority.__

### Pre-v1.0.0
1. [ ] __Full Lua__: I first want to implement Lua 5.5/5.4/5.1 (not _EVERYTHING_, but most of it), then expand on it.
2. [ ] __Proper compiler__: as of writing this I am about to make an _interpreter_ for Sol, in the future I want to make either a compiler that generates native assembly per-platform or bytecode + JIT.
3. [ ] __Standard library__: a nice, bigger standard library than Lua's.
4. [ ] __Typing system__: static structural typing, to be specific.
5. [ ] __Object/structure definitions__: think of them as Luau's `type A = { x: int, y: int }`, or C's `struct`.
6. [ ] __Enhanced module system__: `export/import` over Lua's table returning and `local mod = require('...')`.


### Top priority
- [ ] __Namespaces__: `namespace` declaration for cleaner codebases.
- [ ] __FFI/Direct C interop__: even if developed in C, Sol won't come with C interoperability in the standard library by default, instead being added later. For now, I'll keep it in the roadmap.
- [ ] __Better errors/warnings__: right now errors are too simple, nicer errors (clang-style?) will be added.

### Medium priority
- [ ] __Optimization__: a language that derives from Lua can't be slow or heavy: I will add a lot of optimizations along the way.
  - [ ] __Lexer optimizations__.
  - [ ] __Parser optimizations__.

### Small priority or late development
- [ ] __GitHub org, website, documentation...__
- [ ] __Self-host__: write Sol in Sol.
- [ ] __Package manager__: something Lua seriously needs: a good way to add dependencies. Will fetch from GitHub.
- [ ] __Tooling__: includes:
  - [ ] __Treesitter__ (fast syntax highlighting for most editors).
  - [ ] __Language Server Protocol (LSP)__ to make coding in Sol nicer.
