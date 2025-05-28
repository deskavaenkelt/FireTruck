#ifndef STEERING_SERVICE_H
#define STEERING_SERVICE_H

class SteeringService {
private:
    static constexpr int CENTER_DEADZONE_MIN = 120;
    static constexpr int CENTER_DEADZONE_MAX = 134;
    static constexpr int CENTER_JITTER_MIN = 92;
    static constexpr int CENTER_JITTER_MAX = 99;
    static constexpr int CENTER_POSITION = 95;
    static constexpr int MIN_ANGLE = 30;
    static constexpr int MAX_ANGLE = 160;
    
    bool isSteeringPulseActive = false;

public:
    void control(int angle);
    int filterJitter(int angle);
};

#endif 