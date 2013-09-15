/**
 * Battery management system, Walking Machine, Ecole de Technologie Superieure
 * Copyright (C) 2012 Benjamin Vanheuverzwijn
 *
 * Battery
 */
#include <stdint.h>
#include "battery.h"
#include "ad72.h"

void battery_init(battery_t *bat, uint8_t cell_id) {
    bat->status = BATTERY_STATUS_UNKNOWN;
    bat->health = BATTERY_HEALTH_UNKNOWN;
    bat->power = BATTERY_POWER_UNKNOWN;
    bat->present = 0;
    bat->temperature = 0;
    bat->voltage = 0;
    bat->cell_id= cell_id;
}

uint8_t battery_check_voltage(ad7280a_t *a,battery_t cell[]) {
  uint8_t i;

  for(i=0; i<6; i++){
    cell[i].voltage = ad7280a_read_cell(i,a);
  }

  return 1;
}



battery_status_t battery_get_status(battery_t *bat) {
    return bat->status;
}

battery_health_t battery_get_health(battery_t *bat) {
    return bat->health;
}

battery_power_t battery_get_power(battery_t *bat) {
    return bat->power;
}

uint32_t battery_is_present(battery_t *bat) {
    return bat->present;
}

uint32_t battery_get_temperature(battery_t *bat) {
    return bat->temperature;
}

uint32_t battery_get_voltage(battery_t *bat) {
    return bat->voltage;
}


/*
 * Callbacks
 */

void battery_on_status_update(battery_t *bat) {
}

void battery_on_health_update(battery_t *bat) {
}

void battery_on_power_update(battery_t *bat) {
}
