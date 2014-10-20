#include "contiki.h"

#if IOTLAB_M3
#include "dev/light-sensor.h"
#include "dev/pressure-sensor.h"
#endif
#include "dev/acc-mag-sensor.h"
#include "dev/gyr-sensor.h"

#include "sensors-poller.h"

/*---------------------------------------------------------------------------*/
struct sensor sensor;
/*---------------------------------------------------------------------------*/
#ifdef IOTLAB_M3
static void config_light()
{
  light_sensor.configure(LIGHT_SENSOR_SOURCE, ISL29020_LIGHT__AMBIENT);
  light_sensor.configure(LIGHT_SENSOR_RESOLUTION, ISL29020_RESOLUTION__16bit);
  light_sensor.configure(LIGHT_SENSOR_RANGE, ISL29020_RANGE__1000lux);
  SENSORS_ACTIVATE(light_sensor);
}

static void process_light()
{
  int light_val = light_sensor.value(0);
  float light = (float)light_val / LIGHT_SENSOR_VALUE_SCALE;
  sensor.light = light;
}
/*---------------------------------------------------------------------------*/
static void config_pressure()
{
  pressure_sensor.configure(PRESSURE_SENSOR_DATARATE, LPS331AP_P_12_5HZ_T_1HZ);
  SENSORS_ACTIVATE(pressure_sensor);
}

static void process_pressure()
{
  int pressure = pressure_sensor.value(0);
  sensor.pressure = (float)pressure / PRESSURE_SENSOR_VALUE_SCALE;
}
#endif /* IOTLAB_M3 */
/*---------------------------------------------------------------------------*/
static unsigned acc_freq = 0;
static void config_acc()
{
  acc_sensor.configure(ACC_MAG_SENSOR_DATARATE,
      LSM303DLHC_ACC_RATE_1344HZ_N_5376HZ_LP);
  acc_freq = 1344;
  acc_sensor.configure(ACC_MAG_SENSOR_SCALE, LSM303DLHC_ACC_SCALE_2G);
  acc_sensor.configure(ACC_MAG_SENSOR_MODE, LSM303DLHC_ACC_UPDATE_ON_READ);
  SENSORS_ACTIVATE(acc_sensor);
}

static void process_acc()
{
  static unsigned count = 0;
  if ((++count % acc_freq) == 0) {
    sensor.acc.x = acc_sensor.value(ACC_MAG_SENSOR_X);
    sensor.acc.y = acc_sensor.value(ACC_MAG_SENSOR_Y);
    sensor.acc.z = acc_sensor.value(ACC_MAG_SENSOR_Z);
  }
}
/*---------------------------------------------------------------------------*/
static unsigned mag_freq = 0;
static void config_mag()
{
  mag_sensor.configure(ACC_MAG_SENSOR_DATARATE, LSM303DLHC_MAG_RATE_220HZ);
  mag_freq = 220;
  mag_sensor.configure(ACC_MAG_SENSOR_SCALE, LSM303DLHC_MAG_SCALE_1_3GAUSS);
  mag_sensor.configure(ACC_MAG_SENSOR_MODE, LSM303DLHC_MAG_MODE_CONTINUOUS);
  SENSORS_ACTIVATE(mag_sensor);
}

static void process_mag()
{
  static unsigned count = 0;
  if ((++count % mag_freq) == 0) {
    sensor.mag.x = mag_sensor.value(ACC_MAG_SENSOR_X);
    sensor.mag.y = mag_sensor.value(ACC_MAG_SENSOR_Y);
    sensor.mag.z = mag_sensor.value(ACC_MAG_SENSOR_Z);
  }
}
/*---------------------------------------------------------------------------*/
static unsigned gyr_freq = 0;
static void config_gyr()
{
  gyr_sensor.configure(GYR_SENSOR_DATARATE, L3G4200D_800HZ);
  gyr_freq = 800;
  gyr_sensor.configure(GYR_SENSOR_SCALE, L3G4200D_250DPS);
  SENSORS_ACTIVATE(gyr_sensor);
}

static void process_gyr()
{
  static unsigned count = 0;
  if ((++count % gyr_freq) == 0) {
    sensor.gyr.x = gyr_sensor.value(GYR_SENSOR_X);
    sensor.gyr.y = gyr_sensor.value(GYR_SENSOR_Y);
    sensor.gyr.z = gyr_sensor.value(GYR_SENSOR_Z);
  }
}
/*---------------------------------------------------------------------------*/
static void
configure_sensors()
{
#if IOTLAB_M3
  config_light();
  config_pressure();
#endif
  config_acc();
  config_mag();
  config_gyr();
}
/*---------------------------------------------------------------------------*/
static struct etimer *timer;
static void
process_sensor_events(process_event_t ev, process_data_t data)
{
  if (ev == PROCESS_EVENT_TIMER) {
#if IOTLAB_M3
    process_light();
    process_pressure();
#endif
    etimer_restart(timer);
  } else if (ev == sensors_event && data == &acc_sensor) {
    process_acc();
  } else if (ev == sensors_event && data == &mag_sensor) {
    process_mag();
  } else if (ev == sensors_event && data == &gyr_sensor) {
    process_gyr();
  }
}
/*---------------------------------------------------------------------------*/
void
sensors_poller_init(struct etimer *t)
{
  configure_sensors();
  timer = t;
}
/*---------------------------------------------------------------------------*/
void
sensors_poller_process_events(process_event_t ev, process_data_t data)
{
  process_sensor_events(ev, data);
}
/*---------------------------------------------------------------------------*/
