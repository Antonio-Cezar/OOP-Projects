#include <iostream>
#include <cmath>
#include <algorithm>

const float FAN_ON_TEMP = 18.0f;
const float FAN_MAX_TEMP = 20.0f;
const int MAX_FAN_SPEED = 100;
const float curve_power = 3.0f;

int calcFanSpeed(float temp)
{
    // Fan off below threshold
    if (temp <= FAN_ON_TEMP)
        return 0;

    // Fan maxed out above max temp
    if (temp >= FAN_MAX_TEMP)
        return MAX_FAN_SPEED;

    // Normalize temperature to 0..1
    float normalized =
        (temp - FAN_ON_TEMP) / (FAN_MAX_TEMP - FAN_ON_TEMP);

    // Apply curve
    float curve = std::pow(normalized, curve_power);

    int fanSpeed = static_cast<int>(curve * MAX_FAN_SPEED);

    return std::clamp(fanSpeed, 0, MAX_FAN_SPEED);
}

int main()
{
    float temp;

    while (true)
    {
        std::cout << "Temperature (°C) (Ctrl+C to quit): ";
        std::cin >> temp;

        if (!std::cin)
            break;

        std::cout << "Fan speed: "
                  << calcFanSpeed(temp)
                  << "%\n\n";
    }

    return 0;
}
