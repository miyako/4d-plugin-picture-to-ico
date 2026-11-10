# Native string and picture ownership audit

## Immutable scope and provenance

Baseline: `e84c05a510fefc4f6ecb040a8cabfb9456611359` (default branch).
Canonical guidance: `miyako/skills`, `4d-skills/skills/4dplugin/SKILL.md`,
revision `e6b492fcb81e5e1a7f7db723594b420e63bf6eb9`, read through the
immutable GitHub contents API. No additional checkout or project was created.

The actual bundled engine implementation says `rev : v14`; its public header
says `rev : v13`. These labels are not a verified upstream SDK release.
The complete SDK is compiled unchanged, not extracted functions or a substitute
implementation. The harness pins these Git blob IDs before compilation:

| File in `4D Plugin API/` | Git blob |
|---|---|
| `4DPluginAPI.c` | `29788ed041effaf67269ccd93f971e8aee2ce5f8` |
| `4DPluginAPI.h` | `9054b45ffc7ff6f5b8d57529a3788a55d977b314` |
| `EntryPoints.h` | `730f2f77ff2713fb2fe4b7d6562baeedf5f664cb` |
| `Flags.h` | `10a83185333dac06a62dda61b9f9428243d1e2ee` |
| `PrivateTypes.h` | `77053dc0c0a3718d2d3b6f158371a6af78c6ae66` |
| `PublicTypes.h` | `2bb392e73e9c556c274fe0a75b3a6b83e310b77c` |
| `4DPluginAPI.def` | `996ecc4ab11ad165de9192eba2e7478db141c757` |

Reference harness structure was read through immutable files in
`miyako/4d-plugin-packages` at `87d363be9fa95789e3a344c07e7fcc683ba7849b`
(`.github/tests/ownership_host.mm`, `ownership_platform.h`,
`test_ownership.py`). Its engine and architecture flag blobs differ from this
repository. Neither its SDK nor its test results were copied as evidence.

## Complete tracked native/build inventory

Inventory starts with `git ls-files`, including dormant helpers and bundled
headers, rather than filtering on command signatures or indexed search hits.
Both tracked `manifest.json` files have blob
`68cba364a3edc9c62cc862d0aa3b94163327b6bf`: the root copy and
`PICTURE TO ICO.4dbase/Plugins/PICTURE TO ICO.bundle/Contents/Resources/manifest.json`.
They declare one command, `PICTURE TO ICO(&P;&O)`: picture input and BLOB
output, no return value and no exposed scalar/text-array parameters.

| Native sources and matching headers | Membership and ownership surface |
|---|---|
| `4DPlugin.cpp`, `4DPlugin.h` | Sole dispatcher, selector 1; PNG-format scalar string argument, picture arguments/outputs, metadata strings and temporary data handles. `getBMP` is an internal alias of `getPNG`; `scaleOneIcon` and `getIcons` are included. |
| `4D Plugin API/4DPluginAPI.c`, `.h`, `EntryPoints.h`, `PrivateTypes.h`, `PublicTypes.h`, `Flags.h`, `4DPluginAPI.def` | Complete engine forwarding, ABI/types and export list; protected and unchanged. Windows exports only `FourDPackex`; engine entry delegates to `PluginMain`, then the dispatcher. SDK-only scalar/array/field/method APIs were distinguished from plugin call sites, not rewritten. |
| `Classes/ARRAY_TEXT.cpp`, `.h` | Compiled but not called by the public command. Copies borrowed elements into C++ strings, creates one fresh owner per output element, shallow-transfers to the array without post-transfer disposal. Includes UTF-8/UTF-16, explicit-length, Objective-C and path helpers. |
| `Classes/C_TEXT.cpp`, `.h` | Compiled; path and scalar helpers copy borrowed input into C++ storage. Parameter/return writes use `PA_SetUnistring`, not scalar struct assignment. Includes UTF conversion, replacement and Objective-C/path methods. |
| `Classes/C_PICTURE.cpp`, `.h` | Compiled; `CPicture` and forwarding `C_PICTURE` include dormant file, byte, copy PNG/JPG/TIF, grayscale, properties, thumbnail, native image, parameter and return paths. Ownership defects fixed below. |
| `Classes/C_POINTER.cpp`, `.h` | Compiled, public-command dormant; owning pointer lookup, borrowed pointer descriptor, compiled/interpreted parameter branches, field/reference copying. Successful lookup now cleared; failure checked before reading output. Reference-descriptor semantics are not certified by the ownership test. |
| `Classes/C_BLOB.cpp`, `.h` | Compiled; `CBytes` and `C_BLOB` copy BLOB/hex/base64 content through C++ storage and `C_TEXT`, never transfer a `PA_Unistring`. Temporary output handles transfer to parameters/returns. OOM boundary remains a limitation. |
| `Classes/ARRAY_BOOLEAN.cpp`, `.h`; `ARRAY_DATE.cpp`, `.h`; `ARRAY_INTEGER.cpp`, `.h`; `ARRAY_LONGINT.cpp`, `.h`; `ARRAY_REAL.cpp`, `.h`; `ARRAY_TIME.cpp`, `.h` | All six compiled, dormant from this command. Numeric/date/bool buffers, borrowed array descriptor copies and resize/write-back. No hidden string ownership. They share the SDK resize limitation, not a proof of OOM safety. |
| `Classes/C_DATE.cpp`, `.h`; `C_INTEGER.cpp`, `.h`; `C_LONGINT.cpp`, `.h`; `C_REAL.cpp`, `.h`; `C_TIME.cpp`, `.h` | All five compiled, scalar numeric/date/time storage only; no string allocations, aliases or owning getter returns. |
| `Classes/C_TYPES.h`, `4D Plugin_Prefix.pch` | Shared C++ text/vector/parameter types and Carbon prefix import. No alternate engine implementation. |
| `include/libpng16/png.h`, `pngconf.h`, `pnglibconf.h`; `include/qrencode.h` | Tracked dormant third-party declarations, not compiled implementation units or active linked dependencies; not modified. |
| New `Classes/CommandArguments.h` | Header-only plugin helper, included directly by dispatcher and picture wrapper; no project membership changes needed. Initializes empty descriptors as Undefined, clears argument owners once and cleans successful independent command results only. |
| New `.github/tests/ownership_host.mm`, `ownership_platform.h`, `test_ownership.py` | Synthetic host, pre-compile platform redirection and isolated runner. Never included in the shipped plugin build. |

`PICTURE TO ICO.xcodeproj/project.pbxproj` and `PICTURE TO ICO.vcxproj`
both compile the same 18 native implementation units: dispatcher, complete
engine, and all 16 wrapper `.cpp` files above. Xcode explicitly treats these
as Objective-C++; MSBuild's engine/wrapper entries use `CompileAsCpp` in all
four Debug/Release Win32/x64 configurations. The `.sln` and `.vcxproj.filters`
introduce no additional implementations. Xcode has Headers, Sources,
Frameworks, Resources, empty Rez, and CopyFiles (manifest/constants) phases,
no shell-script phase; code-sign identities and bundle outputs make project
builds unsuitable for this audit.

All tracked native extension paths were inventoried; there is no other
tracked `.c`, `.cpp`, `.m`, `.mm` or `.hpp` implementation. Prebuilt binaries,
4D project/database/wizard files, example method, resources and localization
are not substitute native source and were neither executed nor modified.
There are no plugin object/collection getter/setter calls and no corresponding
object/collection variable kinds in this bundled ABI. Object/collection
reference balances are **not applicable**, not simulated passing coverage.
Indexed absence alone is not the safety argument: the actual dispatcher,
wrappers, engine kinds and both build memberships were checked.

## Source-derived contracts and surgical changes

Line references here refer to the immutable baseline engine.

| Exact engine implementation | Consequence |
|---|---|
| `PA_CreateUnistring`, `PA_SetUnistring`, `PA_DisposeUnistring` (244-270) | Create owns, content-setter replaces/copies, disposal resets only the supplied descriptor. Struct aliases do not become independent owners. Engine string functions do not reliably propagate allocation failures through `PA_GetLastError`. |
| `PA_CreateVariable` (3524-3656) | Typed strings and pictures can allocate an empty owner. Undefined initialization followed by shallow assignment avoids overwriting that owner. |
| `PA_SetStringVariable` (3826-3831), `PA_SetPictureVariable` (3852-3857) | Shallow assignment, no disposal of old content and no independent retain. Argument descriptor must own exactly one assigned allocation/reference. |
| `PA_GetStringInArray` (4266-4286), `PA_SetStringInArray` (4444-4460) | Getter borrows. Accepted setter disposes previous content then transfers the incoming owner. Accepts index zero through logical count; rejects wrong kind, missing data, negative/too-large index or null descriptor without transfer. |
| `PA_CreatePicture`, `PA_DisposePicture`, `PA_DuplicatePicture` (380-455) | Creation owns; duplicate with `retainOnly=1` retains. Retain the output before clearing an owning argument. Release temporary creation references after wrapper adoption. |
| `PA_GetPointerValue` (2035-2053) and public header declaration | Successful result is owning and explicitly requires `PA_ClearVariable`. Failed output is unspecified: check error first, do not inspect or clear it. |
| `PA_ConvertCharsetToCharset` (4776-4794) | `fParam1`/capacity/returned `fParam3` are bytes. UTF-16 C++ constructor requires units: divide returned bytes by `sizeof(PA_Unichar)`. |
| `PA_ExecuteCommandByID` (5257-5291) | Separate returned variable; initialize Undefined only after command-vector handle allocation. Failure may leave raw kind zero (normalized to string by `PA_GetVariableKind`) or arbitrary result bytes. Check error before inspection/adoption/cleanup. Successful owning results have a lifetime independent from synchronous argument cleanup; Undefined results own nothing. |

`fromFileAtPath` previously disposed `_path` and then cleared its scalar
alias, overwrote a newly allocated empty scalar owner, and left the owning
output picture argument uncleared. It now owns each argument exactly once;
`setPicture` retains the output before argument destruction.

`setBytes` now holds the new picture in a temporary owning descriptor:
`setPicture` retains it and scope exit releases the creation reference,
including C++ unwinding. `createCopy` corrects byte-to-unit conversion and
checks conversion failure before interpreting output. Copy, grayscale,
properties and thumbnail operations no longer overwrite empty picture/string
owners. Returned pictures are retained before argument cleanup; properties
and thumbnail inputs get their own retained reference rather than a borrowed
alias that must be detached manually.

The actual dispatcher similarly uses Undefined slots for shallow-assigned
input owners, captures command errors before cleanup, and adopts output only
on success. Existing empty-BLOB failure behavior and all six ICO sizes are
preserved. The dormant picture helpers now throw `std::runtime_error` on
command-handle/command/conversion failure instead of reading failed results.
`C_POINTER::getVariable` checks error before copying successful metadata and
clears its owning temporary afterward; its existing reference layout is
otherwise unchanged.

No blanket disposal removal was applied: accepted array transfers still have
no caller disposal, scalar owners are cleared once, independently copied
content-setter results survive their wrappers, and borrowed values are never
made owning by struct assignment. No SDK, ABI header, manifest, project,
release workflow, dependency or shipped binary was changed.

## Executed validation

Run on macOS 26.7 (25G229), arm64, Apple clang 21.0.0
(`clang-2100.1.1.101`):

```sh
python3 .github/tests/test_ownership.py -v
OWNERSHIP_SOURCE_ROOT=/path/to/immutable-baseline \
  python3 .github/tests/test_ownership.py -v
python3 .github/tests/test_lifecycle.py
```

The baseline source root was materialized with `git archive` of the exact
baseline above, limited to `4D Plugin API`, `4DPlugin.cpp`, and `4DPlugin.h`.
Both runs used the **same final harness**, same unchanged complete bundled
SDK and same sanitizers. Per-leaf child processes isolate each intentional
baseline failure; no edited baseline or SDK fragments were used.

| Evidence | Immutable baseline | Fixed actual sources |
|---|---|---|
| 9 named test methods, 67 leaf runs | 11 pass, 56 fail | 67 pass, 0 fail |
| Array transfers/copies/churn/replacement/shrink/clear/rejection (2 leaves) | 2 pass | 2 pass |
| Scalar borrowed copy/content-setter lifetime (8 leaves) | 8 pass | 8 pass |
| SDK failure-predicate characterization (1 leaf) | Pass | Pass |
| Picture path/copy exact text and empty owners (16 leaves) | Fail | Pass |
| Picture creation and owned returns (4 leaves) | Fail | Pass |
| Command failures (10) and conversion failure (1) | Fail | Pass |
| Pointer owning lookup and poisoned failure (2 leaves) | Fail | Pass |
| Actual dispatcher success, failures at each of 7 commands, missing/failed metadata (20 leaves) | Fail | Pass |
| Separate successful Undefined/string/picture result lifetime probes (3 leaves) | Fail | Pass |
| Existing isolated lifecycle tests | Not changed by audit | 43 pass |

Baseline diagnostics: 12 duplicate/unowned string disposals, 28 string-owner
imbalances, 2 picture-reference imbalances, 2 retained-return count failures,
8 unsurfaced command failures, 1 unsurfaced pointer failure, and 3 ASan
heap-buffer-overflows in `CPicture::createCopy` at baseline line 249.
The latter are the byte/unit overread on the ASCII fixture and a failed
converter output read. The fixed run has no sanitizer diagnostics.

Fixtures cover exact ASCII, BMP U+65E5, supplementary U+1F600 (two UTF-16
units), and empty strings, with allocated-empty and null-empty engines.
The synthetic converter checks source bytes, destination byte capacity and
terminator capacity and returns bytes, never units. The host tracks all
string allocations, picture retain counts and handles to explicit zero at
each passing leaf. ASan leak detection is disabled on macOS; these explicit
owner balances, not a claim about LeakSanitizer, detect leaks. UBSan is
enabled except alignment, because the unchanged SDK packs its ABI structs
to two-byte alignment. This does not certify packed ABI portability.

The harness compiles the **actual dispatcher, all 16 actual wrappers, and the
entire unchanged SDK**. The preincluded platform header redirects URL/path
and native image classes before any production translation unit is compiled;
`nm -u` rejects real file/image/workspace/process boundary imports.
All engine selectors are deny-by-default; only synthetic memory/string/
picture/pointer/command operations are allowed. Commands 678, 1002, 988, 457,
679 execute no actual 4D file/image/UI command. Failed return values are
deliberately poisoned. Dispatcher success asserts all 7 command calls, all
18 data callbacks, exact 120-byte synthetic ICO, header and six synthetic
payloads, and preservation of the caller's input reference.

## Limits and explicit deferrals

- **Allocation failure/OOM is not certified.** The SDK array resize updates
  logical element count after physical resize failure. The harness proves
  that predicate and restores only its test descriptor before cleanup; it
  does not invoke unsafe wrapper writes against an undersized allocation.
  The existing void array/BLOB/string wrapper boundaries lack a documented
  transactional failure-preservation contract. No speculative SDK rewrite
  or silent empty-success OOM fallback was introduced. Picture/string
  creation/retain allocation failures also require a verified engine contract.
- **Picture metadata ownership remains ambiguous.** `PA_GetPictureData`
  forwards an engine string without an ownership comment. Successful
  metadata is modeled as stable borrowed synthetic `.png`; failed metadata
  is poisoned and never inspected. This checks error ordering and data
  handle cleanup, not whether a real engine requires metadata disposal.
  No guessed metadata disposal was added.
- **Command return probes are lifetime controls.** They exercise independent
  owning string/picture and Undefined results; they do not assert that the
  real procedure command IDs return those synthetic kinds. Failure return
  bytes are never classified or cleaned up. Argument ownership is independent.
- **No actual Windows/4D execution.** No minimum host/version, Windows ABI,
  internal command-ID name, actual image codec fidelity, platform URL/native
  image implementation, delayed callback, abort/host shutdown, or pointer
  reference-metadata semantics is certified by this synthetic harness.
  Helpers are synchronous here; no asynchronous callbacks are registered.
- **No operational validation side effects.** Workflows, build phases,
  release scripts and pre-push hook were read before validation. Existing
  `Bump version` and `Build` workflows are manual-only. No project build,
  signing, notarization, packaging, releases/tags, binary upload, installation,
  private user data, real image/file/UI command or manual dispatch was used.
  Lifecycle tests use temporary repositories and mocked publication APIs.
  The new relevant-path `Native ownership` CI is unsigned and read-only.
