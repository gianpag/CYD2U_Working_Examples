#include <vars.h>
bool light_state = false;

bool get_light_state() {
  return light_state;
}

bool set_light_state(bool value) {
  light_state = value;
}