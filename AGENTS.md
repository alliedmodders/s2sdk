# AGENTS.md

alliedmodders/s2sdk holds the Source 2 SDKs, one branch per game: `cs2`, `dota` and `deadlock`. A wrong vtable slot or offset crashes consumers, so every declaration must match the game.

## Scope

- The SDK declares what the engine has, mainly for server plugins. Client-only interfaces generally aren't added.
- Validate added classes/struct to match the game in terms of layout & its functionality, you can add convenience methods, operators, iteration helpers, macros but only if they make sense in a given context, are still matching the engine and actually bring value to the end user.
- Write production-grade code, be mindful about end user usage of the code you write.
- Keep code simple and readable. Added complexity needs a case in the game that requires it, and a change that makes users type more needs a clear gain for them.
- Reuse the SDK's existing helpers instead of writing new ones, and apply a change to every sibling it fits, not just one.
- Don't declare what users can't use: a class users are meant to construct or derive from needs its virtuals defined, and a listener interface needs a way to register it.
- Game classes, enums and flags the schema exposes come from schema dumps; don't copy them in by hand.
- Don't reference game DLL classes the SDK doesn't define (like `CBaseEntity`) in signatures; use `CEntityInstance` or a forward declaration.
- Correctness comes before downstream builds: don't fake a removed API to keep Metamod or plugins compiling; let the compiler point at what changed.
- Don't use leaked code in any form, or code whose license forbids this; You can use debug builds of the games, and use names/types/logic from it only if it actually matches the up to date game builds and you have verified so. Code taken from elsewhere (preferably whole classes from the public Source SDK 2013) keeps its license, and its source goes in the PR description or commit body.

## Branches

- Changes land first on the branch whose game gets them first, usually `cs2`, and the other branches follow by cherry-pick, never by merge.
- The games can be on different engine versions, so check each game instead of assuming it matches another game.
- Sync PRs only cherry-pick; game-specific changes go in a separate PR.

## Cherry-picking

- Pick the commit unchanged and put the game's differences in a follow-up commit.
- If a pick conflicts because the branch already has its own version of the change, keep the branch's side. Don't add empty picks.
- Prefix the subject with `cs2:`, `dota:` or `deadlock:` when a change applies to that game only. Leave an engine change unprefixed only if the other branches have or need it too.
- Before committing an engine change, check the same declaration in the other games' binaries:
  - If they match the change, it's shared: leave it unprefixed and fix any branch whose header still differs.
  - If they match the old declaration, only this game's engine differs, whether behind or ahead: prefix it.
  - If they match neither, each game needs its own version: prefix it.
- Don't add `-x` trailers. To list the `cs2` commits a branch is missing (swap the branches to go the other way):
  ```bash
  base=$(git merge-base origin/cs2 origin/deadlock)
  git range-diff --no-color $base..origin/cs2 $base..origin/deadlock | grep -E '^ *[0-9]+: +[0-9a-f]+ < ' | grep -v ' cs2: '
  ```
- A missing commit may have been skipped on purpose, or replaced by the branch's own version, because the game doesn't have the change. Check the game's binary before picking it.
- A pick can apply cleanly and still redo a change the branch already made in its own commit, undoing the branch's follow-up fixes. Look for the branch's own version by subject and content before picking.

## Verifying

- Check vtables and layouts in each game's binaries, finding them through strings, asserts, RTTI and exports, never addresses. Schema dumps don't cover vtables or raw structs.
- Check both the Windows and Linux binaries of games that ship both, downloading them if they aren't installed. Slot order and layouts can differ between platforms, so matching one doesn't prove the other.
- Existing declarations and comments can be wrong; verify them before building on them.
- After adding or removing a virtual, check that the slots after it still match the binary. An interface version bump means rechecking the whole vtable.
- Count a header's slots the way the compiler lays them out: `#if 0` blocks and commented-out declarations don't count. An override of a secondary base's virtual gets no slot of its own under MSVC, but does under GCC. `g++ -fdump-lang-class` and MSVC's `/d1reportSingleClassLayout` give the exact slots and offsets.
- A method that forwards to another interface's slot has that slot's meaning and type, so the same name on two interfaces is expected, not a duplicate.
- MSVC gives a virtual destructor one slot (Itanium two) and groups overloaded virtuals in reverse declaration order (Itanium keeps declaration order). Declare the engine's overloads as overloads of one name, so each compiler orders them as the engine does.
- A type passed or returned by value must be as trivial as the engine's: a user-declared copy constructor or destructor makes both compilers pass it through memory instead of registers.
- Imported tier0 functions must exist with the same mangled name in each game's tier0 on both platforms (check Linux with `nm -D` and `c++filt`). Types that differ between platforms change Linux names: use `uchar16`/`uchar32` rather than `wchar_t`, and match the types of non-type template parameters, like `size_t`.
- On Linux a pointer to member function is 16 bytes, so struct offsets after one differ from Windows.
- Alignment can differ between platforms too. Declare layouts so natural alignment gives the engine's offsets on both, with `ALIGN8` and similar where needed, rather than `#ifdef`ed padding fields.

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

## Naming and code

- Use the engine's name when strings or symbols give it, even when it breaks the SDK's style. Otherwise name something only when the binary leaves no doubt about what it does; if in doubt it stays unknown, even with its full signature known. Don't rename without such evidence; a name proven wrong becomes unknown.
- An unknown whose behaviour you can describe with certainty, like in its `AMNOTE`, gets a name after that behaviour instead of `unkXYZ`. Don't mark such names as not coming from the engine.
- Put header files where Valve most likely has them, found through asserts, `__FILE__` strings and debug info, under `public/` in the owning module's folder (like `public/engine2/`). A path only debug info shows is enough when nothing newer contradicts it, but check that the types in it still match current builds.
- Unknown virtuals are `unkXYZ` (`unk_XYZ` where the file already uses that), each `X` in them is a group index (starting with 0), `Y` is inheritance depth (starting from 0 at a first `unk`), `Z` is virtual index within the group of `unk`'s (starting from 1) (Example: `Base1::unk001`, `Base1::Foo`, `Base1::unk101`, `Base1::unk102`, `Base2::Bar`, `Base2::unk111`). When an `unk` virtual disappears, renumber `unk`'s after it.
- Unknown members follow the same as virtuals scheme, like `m_unkXYZ`.
- Name only the fields you are certain are there and of the correct type, try to deduce its name by its usage/engine strings/rtti info and come up with the name if you are certain with its meaning, else leave as unknown.
- Look for common hl2sdk utl class structures (`CUtlVector`, `CUtlMap`, `CUtlLeanVector`, etc) within other structs/classes, try to use these over plain types if applicable and is matching target struct/class layout.
- For any unknown flag like type try to find all possible flags, their name/meaning and a value it corresponds to. Create an enum if you are certain in it's name and type size, and provide short 1-5 word comment above the flag if it adds any description to its meaning, else leave a short comment above the place where it's used at with a list of found flags and their expected meaning in 1-5 words.
- When naming members try to match its style with surrounding code, else use Hungarian notation without type prefix (`m_SomeVar`), replacing placeholder names from reverse engineering.
- Use Source 2's typed wrappers where the engine does: `CPlayerSlot`, `CEntityIndex`, `CSplitScreenSlot`, `CEntityHandle`.
- For primitives prefer hl2sdk defined types, `uint8` instead of `std::uint8_t`, `uint16`, `uint32`, etc. You can use plain `int`, `float`, `double` where applicable. Don't use `long`, `uint`.
- Match the file's style, line endings and encoding, and don't restyle code you're only touching.
- A method's name and arguments should say what it does. Comment only a catch or caveat users need to know, never where something is called from or how it was found; no vtable indices, IDA names or addresses.
- When implementing a method body for a class/struct keep simple/one-line methods inline. Put complex bodies at the bottom of the header or in an existing .cpp file (only if it already exists else use header file).
- Comments that describe or guess what something does, to help others work it out later, start with `AMNOTE: `, like `// AMNOTE: Called only when gpGlobals->maxplayer == 1 on player_connect_full`. Mark stubbed or incomplete classes the same way. Don't remove such notes until they're verified.
- Keep comments while they're still true. When changing code, recheck the comments around it and update or remove those that no longer hold.
- When the engine removes an API, use what replaced it instead of keeping a compatibility wrapper. Delete removed virtuals rather than commenting them out or wrapping them in `#if 0`. A renamed type may keep a `using` alias with `// AMNOTE: Deprecated, use X instead`.
- Update Source 1 leftovers that a game still has (found by RTTI, exports or strings) for Source 2 in place instead of deleting them; Attempt to preserve all the functionality it previously had where applicable. delete only what no game has.
- New classes the game doesn't export are header-only, without extra .cpp files. Don't move code that's already in a .cpp into a header.
- Do wrap raw `malloc`/`free` calls in `memdbgon.h`/`memdbgoff.h` includes if the game confirms the usage of `g_pMemAlloc` in these places. Don't use `g_pMemAlloc`'s `Free`/`Alloc` manually.
- Prefer plain getters to reimplementing removed virtuals. Use typedefs for function pointers, and bitfields where the engine packs bits.
- No `static_assert`s for sizes or offsets, and no explicit padding the compiler adds anyway.
- Use the SDK's platform and compiler macros (`PLATFORM_LINUX`, `PLATFORM_64BITS`, `COMPILER_MSVC64`, `COMPILER_GCC`).
- Protobufs and the libs in `lib/` are per game; use the branch's own. Don't commit updated `lib/` binaries (like tier0.lib and libtier0.so); maintainers update them by hand.

## Checking

- Generate the protos with the protoc in `devtools/bin` from `common/*.proto`, and compile touched headers with MSVC and GCC or Clang as C++17, without new warnings.
- Templates only compile when instantiated, so compiling a header doesn't check them; instantiate template code you change.
- Every change needs a concrete reason: a fix for a build error or undefined behavior names the compiler and the error.
- Add tests only for logic the SDK implements itself; a test that only includes a header or calls a getter checks nothing.

## Commits and PRs

- One concern per commit, with a short subject. Use a body only to say where code came from, and credit others' work with `Co-authored-by:`. Keep PR descriptions to a sentence or two.
- Commits read as if the work was done in one go: one commit per header or interface, with follow-up fixes squashed in before review.
- Complete a change or leave the old code alone: no partial interfaces or half-done containers.
- Keep PRs narrow and based on the current branch tip, dropping what another open PR or a merged commit already covers.
- Be ready to back engine claims with byte signatures of the functions that show them, in each binary checked.
