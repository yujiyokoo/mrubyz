# demo04 Packed 9-bit Text Encoding Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Raise demo04's glyph ceiling from 255 to 442 by packing 8 characters into 9-byte mask blocks, decoded by a new `puts_packed` VM builtin — with the current deck rendering identically (except the removed VMRUN feature).

**Architecture:** Build-time: `converter.rb` encodes items to 9-bit tile codes and packs them (1 mask byte + 8 low bytes per block, PAD=0x1FF); `font_builder.rb`'s index ceiling rises to 445 with a glyph-budget report. Runtime: `mrbz_val` gains an explicit string length (union `{ptr,len}`, filled by `op_string` from the constant pool), and `puts_packed` walks `len/9` blocks through a per-character emit helper extracted from `puts` (so rendering behavior — including its quirks — stays byte-identical). The VMRUN live-VM feature is deleted (it was the only runtime string comparison, incompatible with packed data).

**Tech Stack:** Ruby (build scripts + minitest), C (mrubyz VM, z88dk via Docker), GNU Make.

**Authoritative spec:** `docs/superpowers/specs/2026-07-15-packed-text-design.md` — read it if any step here seems ambiguous.

---

## Context for a zero-context engineer

- **Build:** `make demo04` (Docker-based zcc; the Makefile is non-TTY safe). `make test` runs the host VM test suite (z88dk `+test` target, all groups must print `N run, N passed, 0 failed`). Ruby script tests run directly: `ruby tests/scripts/test_converter.rb` etc.
- **Text pipeline today:** `demo/demo04_data.txt` (deck text) → `scripts/font_builder.rb` (scans chars, assigns tile indices ≤255, emits `font_data.h` + `encoding_map.rb`) → `scripts/converter.rb` (encodes each item string to 1 byte/char, emits `pres_data` Ruby literal) → concatenated with `demo/demo04.src.rb` → `mrbc` → bytecode in ROM. At runtime `puts` walks NUL-terminated bytes writing tilemap words.
- **Key encoding facts:** highlight toggle = 0x01 (`{hl}` markup), newline = 0x0D, tile word bit 11 (0x0800) = highlight palette. ASCII chars are FIXED at their `ord` positions. `RESERVED_INDICES = [0x00, 0x01, 0x0C, 0x0D]` (keep all four reserved).
- **New 9-bit facts:** usable indices 1–445 (sprites own 446/447) → 442 assignable glyphs; PAD code = 0x1FF (cannot collide with any tile ≤445); block = `[mask][low0..low7]`, `code_i = low_i | ((mask >> i & 1) << 8)`; pack unit = each item string `pres_data[i][j]`, padded to whole blocks.
- **The working tree may contain unrelated uncommitted files** — commit ONLY the files each task names. Never `git add -A`.

---

### Task 1: Converter — 9-bit codes and block packing

**Files:**
- Modify: `scripts/converter.rb`
- Test: `tests/scripts/test_converter.rb`

- [ ] **Step 1.1: Update/extend the tests (they will fail until Step 1.3)**

Open `tests/scripts/test_converter.rb`. Keep `TestCommandDetection` and any `format_string` tests as-is. REPLACE the existing `TestStringEncoding` class (which asserts 1-byte-per-char output of `encode_string`) with:

```ruby
class TestCodeEncoding < Minitest::Test
  def test_encodes_ascii_to_codes
    assert_equal [2, 3, 4, 4, 5, 0x00D], Converter.encode_codes("Hello\r")
  end

  def test_encodes_highlight_marker
    assert_equal [0x001, 2, 0x001], Converter.encode_codes("{hl}H{hl}")
  end

  def test_high_codes_pass_through
    # stubbed ENCODING_MAP (top of this test file) must gain a 9-bit entry:
    # "光" => 0x1BB   — add it to the stub hash
    assert_equal [0x1BB], Converter.encode_codes("光")
  end
end

class TestPacking < Minitest::Test
  def test_one_full_block
    codes = [0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48]
    packed = Converter.pack_blocks(codes)
    assert_equal 9, packed.bytesize
    assert_equal 0x00, packed.bytes[0]                    # no high bits
    assert_equal [0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48], packed.bytes[1, 8]
  end

  def test_mask_bit_i_is_char_i_high_bit
    codes = [0x1BB, 0x41, 0x100, 0x42, 0x43, 0x44, 0x45, 0x46]
    packed = Converter.pack_blocks(codes)
    assert_equal 0b00000101, packed.bytes[0]              # chars 0 and 2 have bit 8
    assert_equal 0xBB, packed.bytes[1]
    assert_equal 0x00, packed.bytes[3]                    # low byte of 0x100
  end

  def test_pads_final_block_with_pad_code
    packed = Converter.pack_blocks([0x41, 0x42])
    assert_equal 9, packed.bytesize
    # PAD = 0x1FF: high bit set for chars 2..7 -> mask 0b11111100
    assert_equal 0b11111100, packed.bytes[0]
    assert_equal [0x41, 0x42, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF], packed.bytes[1, 8]
  end

  def test_multi_block
    codes = Array.new(9, 0x41)                            # 9 chars -> 2 blocks
    packed = Converter.pack_blocks(codes)
    assert_equal 18, packed.bytesize
    assert_equal 0b11111110, packed.bytes[9]              # 7 PADs in block 2
  end

  def test_missing_char_aborts
    assert_raises(SystemExit) do
      capture_io { Converter.encode_codes("№") } # not in the stub map
    end
  end

  def test_pack_string_composes_encode_and_pack
    packed = Converter.pack_string("Hello\r")
    assert_equal 9, packed.bytesize
    assert_equal [2, 3, 4, 4, 5, 0x0D], packed.bytes[1, 6]
    assert_equal 0b11000000, packed.bytes[0]              # 2 PADs
  end

  def test_packed_output_is_binary_encoding
    assert_equal Encoding::BINARY, Converter.pack_string("H").encoding
  end
end
```

Also update the stub `ENCODING_MAP` at the top of the test file to include `"光" => 0x1BB`.

- [ ] **Step 1.2: Run tests, verify the new ones fail**

Run: `ruby tests/scripts/test_converter.rb`
Expected: `NoMethodError: undefined method 'encode_codes'` (and `pack_blocks`/`pack_string`). Pre-existing tests still pass.

- [ ] **Step 1.3: Implement in `scripts/converter.rb`**

Replace `HIGHLIGHT_BYTE = "\x01"` and `encode_string`/callers with:

```ruby
  HIGHLIGHT_CODE = 0x001
  NEWLINE_CODE   = 0x00D
  PAD_CODE       = 0x1FF

  # Encode a display string to an array of 9-bit tile codes using ENCODING_MAP.
  # \r becomes NEWLINE_CODE; {hl} becomes HIGHLIGHT_CODE. Missing chars abort.
  def self.encode_codes(str)
    result = []
    i = 0
    while i < str.length
      if str[i, HIGHLIGHT_MARKER.length] == HIGHLIGHT_MARKER
        result << HIGHLIGHT_CODE
        i += HIGHLIGHT_MARKER.length
      elsif str[i] == "\r"
        result << NEWLINE_CODE
        i += 1
      elsif ENCODING_MAP.key?(str[i])
        result << ENCODING_MAP[str[i]]
        i += 1
      else
        abort "ERROR: No tile mapping for character #{str[i].inspect} " \
              "(U+#{str[i].codepoints.first.to_s(16).upcase}). " \
              "Add it to demo04_data.txt's character set and rebuild font_data.h."
      end
    end
    result
  end

  # Pack 9-bit codes into 9-byte blocks: [mask][low0..low7] where mask
  # bit i carries code i's bit 8. The final block is padded with PAD_CODE.
  def self.pack_blocks(codes)
    padded = codes.dup
    padded << PAD_CODE while padded.length % 8 != 0
    out = String.new(encoding: 'BINARY')
    padded.each_slice(8) do |slice|
      mask = 0
      slice.each_with_index { |c, i| mask |= ((c >> 8) & 1) << i }
      out << mask.chr
      slice.each { |c| out << (c & 0xFF).chr }
    end
    out
  end

  def self.pack_string(str)
    pack_blocks(encode_codes(str))
  end
```

In the `__FILE__ == $0` main section, change the item encoding to:

```ruby
        if Converter.command_string?(item.strip)
          (item.strip + "\r").inspect  # unused by current deck; passthrough kept
        else
          Converter.format_string(Converter.pack_string(item))
        end
```

- [ ] **Step 1.4: Run tests, verify all pass**

Run: `ruby tests/scripts/test_converter.rb` — expect 0 failures, 0 errors.

- [ ] **Step 1.5: Commit**

```bash
git add scripts/converter.rb tests/scripts/test_converter.rb
git commit -m "Encode demo04 text as packed 9-bit mask blocks"
```

---

### Task 2: font_builder — 445 index ceiling and glyph-budget report

**Files:**
- Modify: `scripts/font_builder.rb`
- Test: `tests/scripts/test_font_builder.rb`

- [ ] **Step 2.1: Write the failing tests**

Add to `tests/scripts/test_font_builder.rb` (keep existing tests; read the file first and follow its stubbing conventions):

```ruby
class TestNineBitBudget < Minitest::Test
  def test_assigns_indices_past_255
    # 351 distinct non-fixed chars: past the 188 free slots below 256
    # (spills over 255) but within the 378 free slots of the 445 ceiling
    chars = (0x3041..0x319F).map { |cp| [cp].pack('U') }
    map = FontBuilder.assign_indices(chars)
    assert map.values.max > 255, "expected assignment past the old 255 cap"
    assert map.values.max <= 445
    refute map.values.include?(0x0C), "0x0C must stay reserved"
  end

  def test_aborts_past_445
    chars = (0x3041..0x3400).map { |cp| [cp].pack('U') }  # way over budget
    assert_raises(SystemExit) do
      capture_io { FontBuilder.assign_indices(chars) }
    end
  end

  def test_budget_report
    report = FontBuilder.budget_report({ 'あ' => 100, '照' => 101, 'A' => 65 })
    assert_includes report, '3'          # total glyphs
    assert_includes report, '1'          # kanji count (照; あ is kana, A ascii)
    assert_includes report, '442'        # budget
  end
end
```

- [ ] **Step 2.2: Run tests, verify the new ones fail**

Run: `ruby tests/scripts/test_font_builder.rb`
Expected: `test_assigns_indices_past_255` fails on the current abort at >255; `budget_report` fails with NoMethodError.

- [ ] **Step 2.3: Implement**

In `scripts/font_builder.rb` `assign_indices`, replace the cap line:

```ruby
      abort "ERROR: Too many unique characters (#{map.size + 1}); tile index #{index} exceeds 445 (442-glyph budget). Reduce the character set. Remaining unassigned: #{characters.drop(characters.index(char)).inspect}" if index > 445
```

Add a report helper:

```ruby
  MAX_TILE_INDEX = 445
  GLYPH_BUDGET = 442  # indices 1..445 minus reserved 0x01, 0x0C, 0x0D

  # One-line build report: total glyphs, kanji count, remaining budget.
  def self.budget_report(map)
    kanji = map.keys.count { |c| (0x4E00..0x9FFF).cover?(c.codepoints.first) }
    "font: #{map.size} glyphs (#{kanji} kanji), budget #{map.size}/#{GLYPH_BUDGET}, #{GLYPH_BUDGET - map.size} free"
  end
```

In the `__FILE__ == $0` main section, print `FontBuilder.budget_report(map)` (via `warn` or `puts`, matching however the script currently reports progress — read the main section and place it right after `assign_indices` is called).

- [ ] **Step 2.4: Run tests, verify all pass** (`ruby tests/scripts/test_font_builder.rb`)

- [ ] **Step 2.5: Commit**

```bash
git add scripts/font_builder.rb tests/scripts/test_font_builder.rb
git commit -m "Raise font index ceiling to 445 with a glyph-budget report"
```

---

### Task 3: VM — string length in mrbz_val

**Files:**
- Modify: `mrubyz.h` (the `mrbz_val` union), `mrubyz.c` (`op_string`, ~line 640)
- Test: `tests/test_string.c`, new fixture `tests/ruby/str_len.rb`

- [ ] **Step 3.1: Write the failing test**

Create `tests/ruby/str_len.rb`:

```ruby
"abcde"
```

In `tests/test_string.c` add (follow the file's existing extern/test/suite pattern):

```c
extern const uint8_t str_len[];

// op_string must record the pool string's length in the value
void test_string_value_carries_length() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, str_len);

  Assert(T_STRING == v.type, "str_len fixture should return a string");
  Assert(5 == v.u.str.len, "u.str.len should be the pool length (5)");
  Assert(0 == strcmp("abcde", v.u.str.ptr), "ptr should alias strval");
}
```

Register it with `suite_add_test` in that file's suite function.

- [ ] **Step 3.2: Run, verify it fails to COMPILE** (`make test`)

Expected: compile error — `u.str` does not exist yet. (A compile-time RED is the expected failure mode here.)

- [ ] **Step 3.3: Implement**

`mrubyz.h` — inside the `mrbz_val` union, after `char* strval;` add:

```c
    struct { char* ptr; uint16_t len; } str;  // ptr aliases strval; len set by op_string
```

`mrubyz.c` `op_string` — replace the dead `str_len` local and set both fields:

```c
  vm->regs[reg_index].type = T_STRING;
  // Pool entry layout: [type][len hi][len lo][data...][NUL]
  const uint8_t *entry = (const uint8_t *)pool_entry_base;
  vm->regs[reg_index].u.str.ptr = (char *)(entry + 3);
  vm->regs[reg_index].u.str.len = ((uint16_t)entry[1] << 8) | entry[2];
```

(Remove the old `u.strval = pool_entry_base + 3;` line and the unused `uint16_t str_len` line; keep the surrounding comments that still apply. All other `u.strval` readers keep working because `str.ptr` occupies the same offset.)

- [ ] **Step 3.4: Run `make test`, verify ALL groups pass** (the new test plus every pre-existing group — this proves the union change is alias-safe).

- [ ] **Step 3.5: Commit**

```bash
git add mrubyz.h mrubyz.c tests/test_string.c tests/ruby/str_len.rb
git commit -m "Record string length in mrbz_val, filled by op_string"
```

---

### Task 4: VM — emit helper extraction and puts_packed builtin

**Files:**
- Modify: `mrubyz.c` (`puts` builtin ~line 888, statics ~line 322)
- Test: `tests/test_output.c`, new fixture `tests/ruby/packed_text.rb`

- [ ] **Step 4.1: Write the failing test**

Create `tests/ruby/packed_text.rb` — two blocks packing `A B \r C` + PADs, then a 9-bit code (0x1BB) + PADs:

```ruby
puts_packed "\xF0\x41\x42\x0D\x43\xFF\xFF\xFF\xFF\xFF\xBB\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
```

In `tests/test_output.c` add:

```c
extern const uint8_t packed_text[];
extern uint8_t cursor_x;
extern uint8_t cursor_y;

// Decode two mask blocks: A,B,\r,C,PAD*4 then 0x1BB,PAD*7.
// Cursor trace (replicating puts semantics exactly, including the
// post-newline x=1 quirk): A(0,0) B(1,0) CR C(1,1) [pads] 0x1BB(2,1)
void test_puts_packed_decodes_blocks() {
  mrbz_val v;
  mrbz_vm vm;
  cursor_x = 0;
  cursor_y = 0;
  mrbz_vm_run(&vm, &v, packed_text);

  Assert(3 == cursor_x, "cursor_x should be 3 after A,B,CR,C,0x1BB");
  Assert(1 == cursor_y, "cursor_y should be 1 after one newline");
}
```

Register with `suite_add_test`.

- [ ] **Step 4.2: Run `make test`, verify it fails**

Expected: link error on `cursor_x`/`cursor_y` (still static) — then after making them non-static in the next step, the runtime failure is `method not found in class Object: puts_packed`. Either failure counts as RED; note which you observed.

- [ ] **Step 4.3: Implement**

(a) `mrubyz.c` ~line 322: drop `static` from `cursor_x` and `cursor_y` (same treatment `scroll_y` got for the bg_scroll tests).

(b) Extract the per-character emit logic. Above `call_builtin`, add:

```c
// Emit one 9-bit text code at the cursor. Extracted from puts so that
// puts (byte stream) and puts_packed (mask blocks) render identically —
// including the historical quirks: the caller advances cursor_x after
// every code (so highlight/PAD compensate with cursor_x--, and the
// column after a newline is 1, exactly as puts has always behaved).
static void emit_text_code(uint16_t code, uint8_t *highlight) {
  if(cursor_x == 32) {
    cursor_x = 0;
    cursor_y++;
  }
  if(cursor_y >= 23) {
    text_scroll();
    cursor_y = 22;
  }
  if(code == 0x01) {        // highlight toggle
    *highlight = !*highlight;
    cursor_x--;
  } else if(code == 0x1FF) { // PAD (packed text only): no-op
    cursor_x--;
  } else if(code == 0x0D) {  // newline
    cursor_x = 0;
    cursor_y += 1;
    if(cursor_y >= 23) {
      text_scroll();
      cursor_y = 22;
    }
  } else {
    uint16_t tile = code;
    if(*highlight) {
      tile = tile | 0x0800; // 0x0800 for using the sprite palette
    }
    SMS_setTileatXY(cursor_x, nametable_row(cursor_y), tile);
  }
}
```

(c) Rewrite the `puts` T_STRING branch body to use it — the loop becomes:

```c
      uint8_t highlight = 0;
      char* str = vm->regs[reg_index+1].u.strval;
      for (; *str; str++, cursor_x++) {
        emit_text_code((uint16_t)(uint8_t)*str, &highlight);
      }
      gotoxy(0, nametable_row(cursor_y + 1));
```

(The wrap/scroll/highlight/newline/draw lines it replaces must have NO other behavioral residue — diff carefully against the original; the helper above is a line-for-line transplant.) Leave `print` untouched.

(d) Add the builtin, next to `puts` in `call_builtin`:

```c
  } else if (!strcmp(sym, "puts_packed")) {
    if(vm->regs[reg_index+1].type != T_STRING) {
      printf("Unexpected argument type for puts_packed\r");
      exit(-1);
    }
    uint8_t highlight = 0;
    const uint8_t *p = (const uint8_t *)vm->regs[reg_index+1].u.str.ptr;
    uint16_t nblocks = vm->regs[reg_index+1].u.str.len / 9;
    for(uint16_t b = 0; b < nblocks; b++) {
      uint8_t mask = p[0];
      for(uint8_t i = 0; i < 8; i++) {
        uint16_t code = (uint16_t)p[1 + i] | (uint16_t)((mask >> i) & 1) << 8;
        emit_text_code(code, &highlight);
        cursor_x++;
      }
      p += 9;
    }
    gotoxy(0, nametable_row(cursor_y + 1));
    vm->regs[reg_index].type = T_NIL;
```

- [ ] **Step 4.4: Run `make test`, verify ALL groups pass** — the new packed test AND every pre-existing group (especially the output tests, which prove the `puts` refactor changed nothing).

- [ ] **Step 4.5: Commit**

```bash
git add mrubyz.c tests/test_output.c tests/ruby/packed_text.rb
git commit -m "Add puts_packed builtin decoding 9-bit mask blocks"
```

---

### Task 5: Pipeline integration — VMRUN removal and demo04 switchover

**Files:**
- Modify: `demo/demo04.src.rb`, `demo/demo04_data.txt`, `demo/demo04.c`, `Makefile`

- [ ] **Step 5.1: Remove the VMRUN feature**

- `demo/demo04.src.rb`: delete the whole `def vmrun ... end` function INCLUDING its preceding `# A super simple "VM" ...` comment (lines ~34–57). Replace BOTH conditional display sites —

```ruby
        if pres_data[i][j] == "VMRUN\r"
          print "        "
          print vmrun
        else
          puts pres_data[i][j]
        end
```

— with the unconditional:

```ruby
        puts_packed pres_data[i][j]
```

- Also change the first-paragraph draw (`puts pres_data[i][j] if pres_data[i].size > j`) to `puts_packed pres_data[i][j] if pres_data[i].size > j`. (Total: 3 call sites become `puts_packed`.)
- `demo/demo04_data.txt`: delete the single `:VMRUN` line (~line 427). Leave the surrounding "The minimal VM" page text alone — the empty "value:" is a known, accepted residual per the spec.
- `demo/demo04.c`: change `bytecode_at` to a stub and drop the extern:

```c
uint8_t bytecode_at(uint8_t i) { return 0; }
```

  (delete the `extern const uint8_t add_optimised[];` line).
- `Makefile`: in the `demo04:` rule remove `demo/add_optimised.ruby.c` from both the prerequisites and the compile line; delete the `demo/add_optimised.ruby.c:` rule (2 lines).

- [ ] **Step 5.2: Build demo04**

Run: `make demo04`
Expected: font_builder prints the new budget report; converter regenerates `demo/demo04.rb` with packed literals; clean build producing `build/demo04.sms`. If mrbc chokes on any `\xNN` escape, STOP and report BLOCKED with the exact literal (do not hand-patch generated files).

- [ ] **Step 5.3: Run `make test`** — all groups 0 failed.

- [ ] **Step 5.4: Sanity-check the generated artifacts**

- `grep -c 'puts_packed' demo/demo04.rb` — expect ≥ the template's 3 call sites.
- Every packed literal's escaped byte count is a multiple of 9: spot-check one `pres_data` entry (each `\xNN` is one byte).
- `grep VMRUN demo/demo04.rb demo/demo04_data.txt demo/demo04.src.rb` — expect: no hits.

- [ ] **Step 5.5: Human visual checkpoint — SKIP (controller batches it).** The acceptance check (deck renders identically page-by-page, highlight and colour toggles included; "The minimal VM" page shows the accepted empty "value:") is done by the human in an emulator at the end.

- [ ] **Step 5.6: Commit**

```bash
git add demo/demo04.src.rb demo/demo04_data.txt demo/demo04.c Makefile
git commit -m "Switch demo04 to packed text; remove VMRUN feature"
```

(Note: `demo/demo04.rb` is generated and NOT tracked in git — do not stage it.)

---

### Task 6: Wrap-up

- [ ] **Step 6.1: Full verification** — `make clean && make && make test`: all seven ROMs build, all test groups pass, and the font budget report prints during the demo04 build.

- [ ] **Step 6.2: Report** — summarize: glyph budget status from the report; confirmation that demos 01–07 build unchanged; remind the human of the emulator acceptance pass (deck identical; VMRUN page residual accepted) and that the deck rewrite for Kansai RubyKaigi 9 is the natural next step.
