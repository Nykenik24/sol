## Roadmap

__This roadmap is read top-to-bottom, separated by priority.__

### Top priority
- [ ] __Typing system__: static structural typing, to be specific.
- [ ] __Object/structure definitions__: think of them as Luau's `type A = { x: int, y: int }`, or C's `struct`.
- [ ] __Proper compiler__: as of writing this I am about to make an _interpreter_ for Sol, in the future I want to make either a compiler that generates native assembly per-platform or bytecode + JIT.
- [ ] __Standard library__: a nice, bigger standard library than Lua's. _Post-compiler!_.

### Medium priority
- [ ] __Namespaces__: `namespace` declaration for cleaner codebases.
- [ ] __FFI/Direct C interop__: even if developed in C, Sol won't come with C interoperability in the standard library by default, instead being added later. For now, I'll keep it in the roadmap.
- [ ] __Optimization__: a language that derives from Lua can't be slow: I will add a lot of optimizations along the way.
  - [ ] __Lexer optimizations__.
  - [ ] __Parser optimizations__.
