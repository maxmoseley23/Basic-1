#include <pebble.h>

static Window *s_main_window;
static TextLayer *hour_layer, *min_layer, *s_date_layer, *s_month_layer;
static char date_buffer[12];
static char wd_buffer[8];
static char hour_buffer[4];
static char min_buffer[4];



static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  
 if (MINUTE_UNIT & units_changed) {
     strftime(hour_buffer, sizeof(hour_buffer), clock_is_24h_style() ?
                                          "%H" : "%I", tick_time);
    strftime(min_buffer, sizeof(min_buffer), "%M", tick_time);
    text_layer_set_text(hour_layer, hour_buffer);
    text_layer_set_text(min_layer, min_buffer);
 }
  
 if (DAY_UNIT & units_changed) {
  strftime(date_buffer, sizeof(date_buffer), "%d", tick_time);
  strftime(wd_buffer, sizeof(wd_buffer), "%a", tick_time);


  text_layer_set_text(s_date_layer, date_buffer);
  text_layer_set_text(s_month_layer, wd_buffer);
 }
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);

  hour_layer = text_layer_create(
      GRect(center.x - 39, PBL_IF_ROUND_ELSE((center.y - 50) - 19, (center.y - 50) - 10), 144, 72));

  text_layer_set_background_color(hour_layer, GColorClear);
  text_layer_set_text_color(hour_layer, GColorWhite);
  text_layer_set_text(hour_layer, "59");
  text_layer_set_font(hour_layer,  fonts_load_custom_font(
    resource_get_handle(PBL_IF_ROUND_ELSE(RESOURCE_ID_TIME_64, RESOURCE_ID_TIME_52))));
  text_layer_set_text_alignment(hour_layer, GTextAlignmentCenter);
  
  min_layer = text_layer_create(
      GRect(center.x - 39, PBL_IF_ROUND_ELSE((center.y + 5) - 19, (center.y + 1) - 10), 144, 72));

  text_layer_set_background_color(min_layer, GColorClear);
  text_layer_set_text_color(min_layer, GColorWhite);
  text_layer_set_text(min_layer, "59");
  text_layer_set_font(min_layer,  fonts_load_custom_font(
    resource_get_handle(PBL_IF_ROUND_ELSE(RESOURCE_ID_TIME_64, RESOURCE_ID_TIME_52))));
  text_layer_set_text_alignment(min_layer, GTextAlignmentCenter);
  
  s_date_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(center.x - 76, center.x - 68), PBL_IF_ROUND_ELSE((center.y + 5) - 18, (center.y + 5) - 10), 50, PBL_IF_ROUND_ELSE(40, 34)));

  text_layer_set_background_color(s_date_layer, GColorWhite);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text(s_date_layer, "31");
  text_layer_set_font(s_date_layer, fonts_load_custom_font(
    resource_get_handle(PBL_IF_ROUND_ELSE(RESOURCE_ID_DATE_32, RESOURCE_ID_DATE_28))));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  s_month_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(center.x - 76, center.x - 68), PBL_IF_ROUND_ELSE((center.y - 18) - 15, (center.y - 15) - 10), 50, PBL_IF_ROUND_ELSE(24, 20)));

  text_layer_set_background_color(s_month_layer, PBL_IF_COLOR_ELSE(GColorRed, GColorWhite));
  text_layer_set_text_color(s_month_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
  text_layer_set_text(s_month_layer, "Fri");
  text_layer_set_font(s_month_layer, fonts_load_custom_font(
    resource_get_handle(PBL_IF_ROUND_ELSE(RESOURCE_ID_DATE_20, RESOURCE_ID_DATE_18))));
  text_layer_set_text_alignment(s_month_layer, GTextAlignmentCenter);


  layer_add_child(window_layer, text_layer_get_layer(hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(min_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_month_layer));
  
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  tick_handler(tick_time, MINUTE_UNIT | DAY_UNIT);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(hour_layer);
  text_layer_destroy(min_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_month_layer);
}

static void app_focus_changing(bool focusing) {
    if (focusing) {
      Layer *window_layer = window_get_root_layer(s_main_window);
      layer_set_hidden(window_layer, true);
      layer_mark_dirty(window_layer);
    }
}

static void app_focus_changed(bool focused) {
    if (focused) {
      Layer *window_layer = window_get_root_layer(s_main_window);
      layer_set_hidden(window_layer, false);  
      layer_mark_dirty(window_layer);
    }
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_set_background_color(s_main_window, GColorBlack);
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  app_focus_service_subscribe_handlers((AppFocusHandlers){
    .did_focus = app_focus_changed,
    .will_focus = app_focus_changing
  });

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT | DAY_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}