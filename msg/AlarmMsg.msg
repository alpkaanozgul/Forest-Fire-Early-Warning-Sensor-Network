namespace forestfiresim;

// AlarmMsg: alarm packet sent by a sensor when it detects (or falsely detects) a fire.
// fireEventTimestamp is set by SensorApp at the moment the fire notification arrives,
// so that FireServer can compute D_alarm = simTime() - fireEventTimestamp (end-to-end
// alarm delay from fire occurrence to reception at the monitoring server).
message AlarmMsg
{
    int    sensorId;
    double timestamp;           // time the alarm packet was created at the sensor
    double fireEventTimestamp;  // time the fire ignition event occurred (for D_alarm)
    double sensorReading;
    bool   isFire;
    bool   isFalseAlarm;
    int    zoneId;
}
