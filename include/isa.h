#ifndef ISA_H
#define ISA_H

//  International Standard Atmosphere (ISA)
namespace isa
{
// get temperture in kelvin
inline float get_air_temperature(float altitude)
{
  assert(0.0f <= altitude && altitude <= 11000.0f);
  return 288.15f - 0.0065f * altitude;
}

// only accurate for altitudes < 11km
float get_air_density(float altitude)
{
  assert(0.0f <= altitude && altitude <= 11000.0f);
  float temperature = get_air_temperature(altitude);
  float pressure = 101325.0f * std::pow(1 - 0.0065f * (altitude / 288.15f), 5.25f);
  return 0.00348f * (pressure / temperature);
}

const float sea_level_air_density = get_air_density(0.0f);
};  // namespace isa

#endif