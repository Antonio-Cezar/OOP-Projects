#include <iostream>
#include <cmath>
#include <algorithm>

const float FAN_ON_TEMP = 18;
const float FAN_MAX_TEMP = 20:
const int MAX_FAN_SPEED= 100;

const float curve_power = 3.0f;

int calcFanSpeed(float temp)
{
    if (temp <= FAN_ON_TEMP)
        return 0;

    if if (temp <= FAN_ON_TEMP)
        return MAX_FAN_SPEED;

    float normalize =
        (temp - FAN_ON_Temp) / (FAN_MAX_TEMP - FAN_ON_TEMP)
        
    float curve = sted::pow(normalize, curve_power)

    int fanSpeed = static_cast<int>(curve = MAX_FAN_SPEED)

    return sted::clamp(fanSpeed, 0, MAX_FAN_SPEED)
}

int main(
    float temp

    while(true){
        std::cout << "Temperatur (°C): ";
        std::cin >> temp;

        std::cout << "Fan speed: "
                << calcFanSpeed(temp)
                << %\n\n;
    }
)