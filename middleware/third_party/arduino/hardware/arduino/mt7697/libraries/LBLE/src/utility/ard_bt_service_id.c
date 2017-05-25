// This Bluetooth Manufacturer ID Table is pulled from
// https://github.com/dannycabrera/Bluetooth-Company-Identifiers

#include "ard_bt_company_id.h"

struct BTServiceEntry {
    uint16_t id;
    const char* name;
};

static struct BTServiceEntry service_list[] = {
    {0x1811, "alert_notification"},
    {0x1815, "automation_io"},
    {0x180F, "battery_service"},
    {0x1810, "blood_pressure"},
    {0x181B, "body_composition"},
    {0x181E, "bond_management"},
    {0x181F, "continuous_glucose_monitoring"},
    {0x1805, "current_time"},
    {0x1818, "cycling_power"},
    {0x1816, "cycling_speed_and_cadence"},
    {0x180A, "device_information"},
    {0x181A, "environmental_sensing"},
    {0x1826, "fitness_machine"},
    {0x1800, "generic_access"},
    {0x1801, "generic_attribute"},
    {0x1808, "glucose"},
    {0x1809, "health_thermometer"},
    {0x180D, "heart_rate"},
    {0x1823, "http_proxy"},
    {0x1812, "human_interface_device"},
    {0x1802, "immediate_alert"},
    {0x1821, "indoor_positioning"},
    {0x1820, "internet_protocol_support"},
    {0x1803, "link_loss"},
    {0x1819, "location_and_navigation"},
    {0x1807, "next_dst_change"},
    {0x1825, "object_transfer"},
    {0x180E, "phone_alert_status"},
    {0x1822, "pulse_oximeter"},
    {0x1806, "reference_time_update"},
    {0x1814, "running_speed_and_cadence"},
    {0x1813, "scan_parameters"},
    {0x1824, "transport_discovery"},
    {0x1804, "tx_power"},
    {0x181C, "user_data"},
    {0x181D, "weight_scale"},
};

const char* getBluetoothServiceName(uint16_t serviceId)
{
    // To prevent holes in id list in the future,
    // we still linear search here.
    for(unsigned int i = 0; i < sizeof(service_list) / sizeof(struct BTServiceEntry); ++i)
    {
        if(service_list[i].id == serviceId)
        {
            return service_list[i].name;
        }
    }
    return 0;
}