#include <pebble.h>

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;

static int s_battery_level;

static GFont s_time_font;
static GFont s_date_font;
static GFont s_battery_font;

static void update_time() {
  time_t time_temp = time(NULL); 
  struct tm *time_tick = localtime(&time_temp);
  static char time_buffer[] = "00:00";
  char *time_format = "%I:%M"; 
  if (clock_is_24h_style()) { time_format = "%H:%M"; }
  strftime(time_buffer, sizeof(time_buffer), time_format, time_tick);
  text_layer_set_text(s_time_layer, time_buffer + (('0' == time_buffer[0]) ? 1 : 0));
}

static void update_date() {
  time_t date_temp = time(NULL); 
  struct tm *tick_date = localtime(&date_temp);
  static char date_buffer[] = "Sep 25";
  char date_format[] = "%d %b";
  strftime(date_buffer, sizeof(date_buffer), date_format, tick_date);
  text_layer_set_text(s_date_layer, date_buffer + (('0' == date_buffer[0]) ? 1 : 0));
}

static void update_battery() {
  static char battery_buffer[16];
  snprintf(battery_buffer, sizeof(battery_buffer), "%d%%", s_battery_level);
  text_layer_set_text(s_battery_layer, battery_buffer);
}

static void update_display() {
  update_time();
  update_date();
  update_battery();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_display();
}

static void battery_callback(BatteryChargeState battery_charge_state) {
  s_battery_level = battery_charge_state.charge_percent;
  update_display();
}

static void main_window_load(Window *window) {  
  // Fonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HELVETICANEUE_BOLD_48));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HELVETICANEUE_REGULAR_28));
  s_battery_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  
  // Layer dimensions: left, top, width [max=144], height[max=168]
  
  // Time layer 
  s_time_layer = text_layer_create(GRect(0, 100, 138, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Date layer
  s_date_layer = text_layer_create(GRect(0, 71, 138, 40));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  // Battery layer
  s_battery_layer = text_layer_create(GRect(6, 6, 138, 40));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_font(s_battery_layer, s_battery_font);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  
  // Update the display
  update_display();
}

static void main_window_unload(Window *window) {
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_battery_font);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_battery_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Set window background color
  window_set_background_color(s_main_window, GColorBlack);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true); 
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  s_battery_level = battery_state_service_peek().charge_percent;
  
  // Start with an updated display
  update_display();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}
