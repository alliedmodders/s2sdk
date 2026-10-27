# AGENTS.md

alliedmodders/s2sdk holds the Source 2 SDKs, one branch per game: `cs2`, `dota` and `deadlock`. A wrong vtable slot or offset crashes consumers, so every declaration must match the game.

## Scope

- The SDK declares what the engine has, mainly for server plugins; client-only interfaces generally aren't added.
- Added classes and structs must match the game's layout and behaviour. Convenience methods, operators, iteration helpers and macros are welcome only where they fit, still match the engine and help users.
- Write production-grade, simple and readable code with users in mind. Added complexity needs a case in the game that requires it, and making users type more needs a clear gain for them.
- Reuse the SDK's helpers instead of writing new ones or using `std` equivalents, and apply a change to every sibling it fits.
- Don't declare what users can't use: a class users construct or derive from needs its virtuals defined, and a listener interface needs a way to register it.
- Game classes, enums and flags the schema exposes come from schema dumps, not by hand.
- Don't reference game DLL classes the SDK doesn't define (like `CBaseEntity`) in signatures; use `CEntityInstance` or a forward declaration.
- Correctness comes before downstream builds: don't fake a removed API to keep Metamod or plugins compiling; let the compiler point at what changed.
- Fix the cause of a bug, not its symptom. Before changing behaviour, read the history of those lines: an earlier commit may have changed them on purpose.
- Don't use leaked code, or code whose license forbids it. Names, types and logic from the games' debug builds are fine once verified against current builds. Code taken from elsewhere (preferably whole classes from the public Source SDK 2013) keeps its license, and its source goes in the PR description or commit body.

## Branches and cherry-picking

- Changes land first on the branch whose game gets them first, usually `cs2`; the other branches follow by cherry-pick, never by merge. Sync PRs only cherry-pick; game-specific changes go in a separate PR.
- The games can be on different engine versions, so check each game instead of assuming it matches another.
- Pick the commit unchanged; the game's differences go in a follow-up commit in the game-specific PR. If a pick conflicts because the branch already has its own version of the change, keep the branch's side. No empty picks and no `-x` trailers.
- Prefix the subject with `cs2:`, `dota:` or `deadlock:` when a change applies to that game only; proto updates count as game-specific. Before committing an engine change, check the same declaration in the other games' binaries:
  - If they match the change, it's shared: leave it unprefixed and fix any branch whose header still differs, in that branch's own PR.
  - If they match the old declaration, only this game's engine differs, whether behind or ahead: prefix it.
  - If they match neither, each game needs its own version: prefix it.
- To list the `cs2` commits a branch is missing (swap the branches to go the other way):
  ```bash
  base=$(git merge-base origin/cs2 origin/deadlock)
  git range-diff --no-color $base..origin/cs2 $base..origin/deadlock | grep -E '^ *[0-9]+: +[0-9a-f]+ < ' | grep -v ' cs2: '
  ```
- A missing commit may have been skipped on purpose, or replaced by the branch's own version, because the game doesn't have the change. Check the game's binary before picking it.
- A pick can apply cleanly and still redo a change the branch made in its own commit, undoing the branch's follow-up fixes, or replace a game-specific declaration with cs2's. Look for the branch's own version by subject and content before picking, and after a sync check every game-specific declaration the picks touched against the game's binary.

## Game binaries

Download them with [SteamFileDownloader](https://github.com/SteamTracking/SteamFileDownloader) into a folder outside any repo:

```bash
# CS2
SteamFileDownloader get 730 all --output <dir> -- "regex:\.(dll|so)$"
# Dota 2
SteamFileDownloader get 570 all --output <dir> -- "regex:\.(dll|so)$"
# Deadlock
SteamFileDownloader get 1422450 all --output <dir> -- "regex:\.(dll|so)$"
```

## Schema dumps

Each game's schema (classes with sizes and field offsets, enums, convars, entities) is in [s2v.app/SchemaExplorer/llms.txt](https://s2v.app/SchemaExplorer/llms.txt), which links the current gzipped JSON per game and shows how to query it with a script. Offsets and sizes are Windows layouts; still confirm them in the binary.

## Verifying

- Check vtables and layouts in each game's binaries, finding them through strings, asserts, RTTI and exports, never addresses. Schema dumps don't cover vtables or raw structs.
- Check both the Windows and Linux binaries of CS2 and Dota; Deadlock ships Windows only. Slot order and layouts can differ between platforms, so matching one doesn't prove the other; wrap only the declarations that really differ in `#ifdef PLATFORM_LINUX`.
- Existing declarations and comments can be wrong; verify them before building on them.
- An interface's version string leads through its `InterfaceReg` static initializer to the create function, the object and its vtable.
- Compare the length of the implementing class's vtable, found through its RTTI, across games or builds first: a difference shows where to look, equal counts don't prove the order. Match slots by body (string anchors, function size), not by index, and decompile the ones that differ.
- Decompiler argument counts can be wrong; confirm them from the registers the function reads, or from its call sites.
- Take a struct's size from its allocation or constructor, and each member's offset and type from the code that reads or writes it. Bytes nothing touches in a gap natural alignment leaves are padding, not an unknown member.
- After adding or removing a virtual, check that the slots after it still match. An interface version bump means rechecking the whole vtable.
- Count a header's slots the way the compiler lays them out: `#if 0` blocks and commented-out declarations don't count. `g++ -fdump-lang-class` and MSVC's `/d1reportSingleClassLayout` give the exact slots and offsets.
- MSVC gives a virtual destructor one slot (Itanium two), groups overloaded virtuals in reverse declaration order (Itanium keeps declaration order), and gives an override of a secondary base's virtual no slot of its own (GCC does). Declare the engine's overloads as overloads of one name, so each compiler orders them as the engine does.
- A method that forwards to another interface's slot has that slot's meaning and type, so the same name on two interfaces is expected, not a duplicate.
- A type passed or returned by value must be as trivial as the engine's: a user-declared copy constructor makes both compilers pass it through memory instead of registers, and on Linux a user-declared destructor or move constructor does too.
- Imported tier0 functions must exist with the same mangled name in each game's tier0 on both platforms (check Linux with `nm -D` and `c++filt`). Types that differ between platforms change Linux names: use `uchar16`/`uchar32` rather than `wchar_t`, and match the types of non-type template parameters, like `size_t`.
- On Linux a pointer to member function is 16 bytes, so struct offsets after one differ from Windows. Alignment can differ too: declare layouts so natural alignment gives the engine's offsets on both, with `ALIGN8` and similar where needed, rather than `#ifdef`ed padding fields.

## Naming and code

- Use the engine's name when strings or symbols give it (VProf scopes, assert and log text, convar names, MSVC lambda RTTI names, which embed the enclosing method), even when it breaks the SDK's style. Names from other builds and games count once the body matches: their symbols and debug info, including Valve's getter and setter names for a member (a `SetHostClient` that stores it names `m_pHostClient`). So does a name the engine gives the same value elsewhere: the schema or protobuf field it is copied to or from, or the log text that prints it. A name proven wrong becomes unknown; don't rename without such evidence.
- Without such a source, name a member, method or type after what it does once its logic is certain: every reader and writer found, including those reaching it through other pointers to the object and the functions it is passed to, and confirmed at runtime where possible. Don't mark the name as not from the engine, and be ready to show the readers and writers it rests on. Otherwise it stays unknown, even with its full signature known. Name a field only when its type is certain too. Match the surrounding style, else use `m_` and CamelCase without type letters (`m_SomeVar`).
- A member the engine initializes or destroys but never uses stays an unknown member, not padding.
- Unknown virtuals are `unkXYZ` (`unk_XYZ` where the file already uses that), unknown members `m_unkXYZ`, bitfields included (`bool m_unk101 : 1`). `X` is the group within the class: 0 before its first named member, then counting up per group; `Y` is the inheritance depth, from 0 at the first class with an `unk`; `Z` is the index within the group, from 1 (`Base1::unk001`, `Base1::Foo`, `Base1::unk101`, `Base1::unk102`, `Base2::Bar`, `Base2::unk111`). When an unknown disappears, renumber the ones after it.
- Use the SDK's containers (`CUtlVector`, `CUtlMap`, `CUtlLeanVector`, ...) inside structs where they match the layout.
- For a flag-like field, find every flag, its meaning and value. Make an enum when its name and size are certain, with a 1-5 word comment on a flag where it adds meaning; otherwise list the found flags and their meaning in a short comment where the field is used. Values a field or argument takes, like states, get an enum the same way; don't reuse another type's enum because its values look alike.
- When creating new enum, give it a default or invalid entry when the context has one: the value the engine initializes it to, returns on failure or checks against (like `-1` or `0`). When its values are sequential or bit flags, add the ones missing in between as unknowns named after their value or bit, `<PREFIX>_UNK<value>` or `<PREFIX>_UNK<bit>` for `(1 << bit)`, in the digit width the file already uses (`PARSING_FLAG_UNK006`); rename them once their meaning is found.
- Use Source 2's typed wrappers where the engine does: `CPlayerSlot`, `CEntityIndex`, `CSplitScreenSlot`, `CEntityHandle`.
- For primitives use the SDK's types (`uint8`, `uint16`, `uint32`, ... rather than `std::uint8_t`) or plain `int`, `float` and `double`; not `long` or `uint`.
- Put header files where Valve most likely has them, found through asserts, `__FILE__` strings and debug info: under `public/` in the owning module's folder (like `public/engine2/`), or `game/shared/` for game code, with Valve's lowercase file names. A path only debug info shows is enough when nothing newer contradicts it, if the types in it still match current builds. Without such evidence, leave a declaration where it is, and don't create a folder for a single header.
- Include the header that defines a type the SDK has instead of forward declaring it, unless that creates an include cycle or a long include chain.
- Match the file's style, line endings and encoding, and don't restyle code you're only touching.
- A method's name and arguments should say what it does. Comments are a plain caveat users need to know, or, starting with `AMNOTE: `, what something does or likely does, to help others work it out later (`// AMNOTE: Called only when gpGlobals->maxplayer == 1 on player_connect_full`), or a stubbed or incomplete class; keep `AMNOTE`s until they're verified. Never say where something is called from or how it was found; no vtable indices, IDA names or addresses.
- Keep comments while they're still true; when changing code, update or remove those around it that no longer hold.
- When the engine removes an API, use what replaced it instead of a compatibility wrapper, and delete removed virtuals rather than commenting them out or wrapping them in `#if 0`. A renamed type may keep a `using` alias with `// AMNOTE: Deprecated, use X instead`. Prefer plain getters to reimplementing removed virtuals.
- Update Source 1 leftovers that a game still has (found by RTTI, exports or strings) for Source 2 in place, keeping what they did where it still applies; delete only what no game has. Strings and debug info list only polymorphic classes and the types a module uses, so a plain struct, enum or typedef missing from them can still be live.
- Keep simple, one-line method bodies inline; put complex ones at the bottom of the header, or in the class's .cpp file if one already exists. New classes the game doesn't export are header-only, and code already in a .cpp stays there.
- Wrap raw `malloc`/`free`/`realloc` calls in `memdbgon.h`/`memdbgoff.h` includes where the game confirms they go through `g_pMemAlloc`; don't call `g_pMemAlloc`'s `Alloc`/`Free`/`Realloc` directly.
- Use typedefs for function pointers, and bitfields where the engine packs bits. No `static_assert`s for sizes or offsets, and no explicit padding the compiler adds anyway.
- Use the SDK's platform and compiler macros (`PLATFORM_LINUX`, `PLATFORM_64BITS`, `COMPILER_MSVC64`, `COMPILER_GCC`).
- Protobufs and the libs in `lib/` are per game; use the branch's own. Don't commit updated `lib/` binaries (like tier0.lib and libtier0.so); maintainers update them by hand. Leave `public/steam/` as the Steamworks SDK ships it.

## Checking

- Generate the protos with the protoc in `devtools/bin` from `common/*.proto`, and compile touched headers with MSVC and GCC or Clang as C++20, without new warnings.
- Templates only compile when instantiated, so instantiate template code you change.
- Every change needs a concrete reason: a fix for a build error or undefined behavior names the compiler and the error in the PR description.
- Changes to logic the SDK implements itself, like containers or KeyValues3, should come with a runtime test in `tests/run/` that fails without the change, registered like the existing ones. Declaration and layout fixes need none, and a test that only includes a header or calls a getter checks nothing.

## Commits and PRs

- One concern per commit, with a short subject that names the class or function and says what changed, not why; mention a platform or compiler only when the change is about the build. Use a body only to say where code came from, and credit others' work with `Co-authored-by:`, but not AI tools. Keep PR descriptions to a sentence or two.
- Commits read as if the work was done in one go: one commit per header or interface, with follow-up and review fixes folded into the commit they fix.
- Complete a change or leave the old code alone: no partial interfaces or half-done containers.
- Keep PRs narrow and based on the current branch tip, dropping what another open PR or a merged commit already covers.
- Be ready to back engine claims with byte signatures of the functions that show them, in each binary checked.
