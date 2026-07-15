#include "test.h"
#include "../mrubyz.h"

extern const uint8_t str_built_ins[];
extern const uint8_t random_builtin[];
extern const uint8_t bg_scroll_once[];
extern const uint8_t bg_scroll_reset[];
extern uint8_t scroll_y;

void test_string_built_ins() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, str_built_ins);

  Assert(T_TRUE == v.type, "string built-in returned unexpected type");
}

void test_random_built_in() {
  mrbz_val v;
  mrbz_vm vm;
  mrbz_vm_run(&vm, &v, random_builtin);

  Assert(T_INT == v.type, "random built-in returned unexpected type");
  Assert(v.u.intval >= 0 && v.u.intval < 10, "random(10) out of 0..9 range");
}

// The VDP's Y scroll wraps at 224 (28-row BG plane), so scrolling up from 0
// must land on 223 — not the uint8_t's 255, which glitches the display.
void test_bg_scroll_wraps_at_plane_height() {
  mrbz_val v;
  mrbz_vm vm;
  scroll_y = 0;
  mrbz_vm_run(&vm, &v, bg_scroll_once);

  Assert(223 == scroll_y, "bg_scroll from 0 should wrap to 223 (plane height - 1)");
}

void test_bg_reset_clears_scroll_state() {
  mrbz_val v;
  mrbz_vm vm;
  scroll_y = 0;
  mrbz_vm_run(&vm, &v, bg_scroll_reset);

  Assert(0 == scroll_y, "bg_reset should zero scroll_y, not just the hardware register");
}

int test_built_ins() {
  suite_setup("built-in tests");
  suite_add_test(test_string_built_ins);
  suite_add_test(test_random_built_in);
  suite_add_test(test_bg_scroll_wraps_at_plane_height);
  suite_add_test(test_bg_reset_clears_scroll_state);

  return suite_run();
}
