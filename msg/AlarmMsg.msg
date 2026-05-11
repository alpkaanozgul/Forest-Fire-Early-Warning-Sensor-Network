namespace forestfiresim;

message AlarmMsg
{
    int    sensorId;
    double timestamp;
    double sensorReading;
    bool   isFire;
    bool   isFalseAlarm;
    int    zoneId;
}