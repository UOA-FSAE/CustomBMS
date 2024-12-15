import math

# ADC and voltage configuration
VADC_MAX = 3.3        # Max ADC voltage
ADC_MAX = 4095        # 12-bit ADC
R_REFERENCE = 10000   # 10k reference resistor
NTC_R25 = 10000      # NTC resistance at 25°C
NTC_BETA = 3950      # Beta value from datasheet

# Temperature configuration (all in deci-celsius)
TEMP_MIN = -200      # -20.0°C
TEMP_MAX = 1000      # 100.0°C
TEMP_HIGH_RES_MIN = 150   # 10.0°C
TEMP_HIGH_RES_MAX = 700   # 70.0°C

# Table size configuration
HIGH_RES_ENTRIES = 76    # More points in 15-70°C range
LOW_RES_ENTRIES = 4      # Fewer points outside that range
LOW_RES_ENTRIES_HALF = LOW_RES_ENTRIES // 2  # Split between low and high temps

def temp_to_adc(temp_deci):
    """Convert temperature (in deci-°C) to ADC value."""
    temp_celsius = temp_deci / 10.0
    temp_kelvin = temp_celsius + 273.15
    
    # Calculate thermistor resistance
    r_thermistor = NTC_R25 * math.exp(NTC_BETA * (1/temp_kelvin - 1/298.15))
    
    # Calculate voltage at ADC pin (voltage divider)
    v_adc = VADC_MAX * r_thermistor / (r_thermistor + R_REFERENCE)
    
    # Convert to ADC value
    adc_value = round(v_adc * ADC_MAX / VADC_MAX)
    return max(0, min(ADC_MAX, adc_value))

def generate_table():
    table = []
    
    # Generate low-res points below TEMP_HIGH_RES_MIN
    temp_step = (TEMP_HIGH_RES_MIN - TEMP_MIN) / (LOW_RES_ENTRIES_HALF - 1)
    for i in range(LOW_RES_ENTRIES_HALF):
        temp = TEMP_MIN + (i * temp_step)
        table.append((temp_to_adc(temp), int(temp)))
    
    # Generate high-res points between TEMP_HIGH_RES_MIN and TEMP_HIGH_RES_MAX
    temp_step = (TEMP_HIGH_RES_MAX - TEMP_HIGH_RES_MIN) / (HIGH_RES_ENTRIES - 1)
    for i in range(HIGH_RES_ENTRIES):
        temp = TEMP_HIGH_RES_MIN + (i * temp_step)
        table.append((temp_to_adc(temp), int(temp)))
    
    # Generate low-res points above TEMP_HIGH_RES_MAX
    temp_step = (TEMP_MAX - TEMP_HIGH_RES_MAX) / (LOW_RES_ENTRIES_HALF - 1)
    for i in range(LOW_RES_ENTRIES_HALF):
        temp = TEMP_HIGH_RES_MAX + (i * temp_step)
        table.append((temp_to_adc(temp), int(temp)))
    
    return table

def main():
    table = generate_table()
    
    # Print table in C format
    print("// Generated lookup table")
    print("static const struct {")
    print("    uint16_t adc;")
    print("    int16_t temp;")
    print("} temp_table[TEMP_TABLE_SIZE] = {")
    
    # Print entries 4 per line
    for i in range(0, len(table), 4):
        entries = table[i:i+4]
        entries_str = []
        for adc, temp in entries:
            entries_str.append(f"{{0x{adc:04X}, {temp:4d}}}")
        line = "    " + ", ".join(entries_str) + ","
        print(line)
    
    print("};")

if __name__ == "__main__":
    main()