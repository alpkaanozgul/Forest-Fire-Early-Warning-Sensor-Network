namespace forestfiresim;

message TelemetryMsg
{
    int    sensorId;
    double timestamp;
    double temperature;
    double humidity;
    double smokeLevel;
    double windSpeed;
    bool   nearThreshold;
}