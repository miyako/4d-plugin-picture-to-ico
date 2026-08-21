# 4d-plugin-picture-to-ico

`PICTURE TO ICO` converts a `Picture` into a Windows `.ico`-format `BLOB` containing that image at six standard sizes (16, 32, 48, 64, 128, and 256 pixels square). Internally, the plugin first converts the source picture to PNG, then asks 4D's own picture-scaling command to produce each size, extracts each result's PNG data, and assembles a standards-compliant ICO container (per the [MS ICO format spec](https://msdn.microsoft.com/en-us/library/ms997538.aspx)) around them.

| Command | Returns | Purpose |
|---|---|---|
| [PICTURE TO ICO](#picture-to-ico) | (writes to `BLOB` parameter) | Build a multi-size `.ico` `BLOB` from a source `Picture` |

**Platforms:** macOS (Carbon and Cocoa) and Windows (32-bit and 64-bit).

---

## Requirements & platform notes

- **Both parameters are mandatory.** `picture` is read unconditionally as parameter 1; there is no optional/omitted form.
- **Failure is silent, not a 4D error.** If the internal PNG conversion or any of the six resize steps fails, the command does not raise a 4D error and does not set `$0` (the manifest declares no return-value token, so there's nothing for 4D to report on). As of this review's fix, it instead leaves `ico` as an **empty (0-byte) `BLOB`** — check `BLOB size` after the call rather than assuming success. *(This empty-BLOB behavior is forward-looking: it's true of the source as patched in this review, not necessarily of whatever binary you currently have built/installed — see the note at the end of this section.)*
- **Alpha channel may be lost.** If the source picture isn't already PNG, the plugin converts it to PNG internally first; per the project's own README, this conversion can drop the alpha channel depending on the source format.
- **96×96 is intentionally not generated.** The source notes `96 is automatically rendered by windows` — Windows synthesizes that size itself from the other entries, so it's not one of the six sizes this plugin produces.
- **Two internal 4D command IDs are load-bearing and unconfirmed by name.** The plugin drives 4D's own picture-scaling and picture-format-conversion commands via `PA_ExecuteCommandByID` with raw numeric IDs (`679`, `1002`). Their behavior is exactly as inferred from the call sites (scale to width×height; convert to a named format) — treat the specific 4D command names as unverified if you're cross-referencing against a 4D Language Reference.
- **No specific minimum OS version can be confidently stated.** The scaling and format conversion are delegated to 4D's own internal commands rather than to directly-visible platform APIs (no raw `Gdiplus`/`CoreGraphics` calls appear in this plugin's own source), so there's no version-specific API call in this file to derive a minimum from.

> **Note on the fix above:** this review patched three issues in the plugin's C++ source — a handle leak on a failure path, an unchecked resize/convert path that could previously reach undefined behavior (and, before that, potentially a corrupt or truncated `.ico`) on any transient failure, and a fragile hand-built string literal. If you're documenting or troubleshooting against a binary built from the *original*, unpatched source, the "returns an empty BLOB on failure" guarantee above does not apply — the original code could instead produce a malformed `.ico` or crash in that situation.

---

## PICTURE TO ICO

### Syntax

```4d
PICTURE TO ICO ( picture ; ico )
```

| Parameter | Type | Description |
|---|---|---|
| `picture` | Picture | Source image. Mandatory. Any format 4D can hold in a `Picture` variable; internally converted to PNG first (see alpha-channel note above). |
| `ico` | BLOB | Output parameter. Declare a `BLOB` variable and pass it in; the command overwrites it with the raw bytes of a complete `.ico` file containing the 16/32/48/64/128/256 px versions of `picture`. Left as an empty (0-byte) `BLOB` if any internal step fails. |

### Description

The command runs a fixed pipeline every call:

1. Duplicates `picture` and converts the duplicate to PNG.
2. For each of the six target sizes (16, 32, 48, 64, 128, 256), asks 4D's internal scaling command to produce a resized duplicate, then extracts that duplicate's PNG-format bytes.
3. Assembles a single ICO container: a 6-byte `ICONDIR` header, six 16-byte `ICONDIRENTRY` records (one per size, each declaring 32 bits/pixel), followed by the six PNG payloads back-to-back. The 256 px entry uses the ICO spec's convention of encoding both width and height as `0` (meaning "256") rather than the literal value.
4. Writes the assembled bytes into `ico` as a `BLOB`.

There's no partial-success mode — either all six sizes are produced and `ico` comes back as a complete, well-formed `.ico`, or (per the fix in this review) none are, and `ico` comes back empty.

### Example

From the plugin's own test method (`Method1.4dm`):

```4d
//%attributes = {}
$path:=Get 4D folder:C485(Current resources folder:K5:16)+"4D.png"

READ PICTURE FILE:C678($path; $icon)

PICTURE TO ICO($icon; $ico)

BLOB TO DOCUMENT:C526(System folder:C487(Desktop:K41:16)+"test.ico"; $ico)
```

A generalized version of the same pattern, with an explicit success check using the empty-`BLOB`-on-failure behavior documented above:

```4d
var $icon : Picture
var $ico : Blob

$icon:=Read picture file("/RESOURCES/AppIcon.png")

PICTURE TO ICO($icon; $ico)

If (BLOB size($ico)=0)
	ALERT("Icon conversion failed.")
Else
	WRITE BLOB($ico; "/RESOURCES/AppIcon.ico")
End if
```

---

## Error handling & troubleshooting

- **Empty `ico` after the call.** The command failed somewhere in the pipeline (format conversion or one of the six resizes) and did not raise a 4D error — always check `BLOB size(ico)=0` rather than assuming a non-crash means success.
- **No type-checking beyond 4D's own parameter coercion.** Passing something other than a valid `Picture` for `picture` relies entirely on 4D's own type system to catch the mismatch before the plugin runs; the plugin itself does no additional validation.
- **Unexpected `.ico` corruption on an unpatched build.** If you're running a binary built from the source *before* this review's fixes, a transient failure partway through the pipeline could produce a truncated/malformed `.ico` (or, in the worst case, crash) instead of a clean empty result — rebuild from the patched source if you hit this.
- **Alpha transparency missing in the output.** Expected if the source `picture` wasn't already PNG — the internal PNG conversion step can drop the alpha channel depending on the original format.
- **No 96×96 entry in the `.ico`.** By design — Windows renders that size itself from the other entries in the file.

---

## Quick reference

```4d
var $icon : Picture
var $ico : Blob

$icon:=Read picture file("/path/to/source.png")
PICTURE TO ICO($icon; $ico)
If (BLOB size($ico)#0)
	WRITE BLOB($ico; "/path/to/output.ico")
End if
```
