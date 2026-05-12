package forestfiresim.sensor;

//
// SensorApp: LoRa sensor node application.
// Generates Exp-distributed telemetry and Bernoulli fire alarms.
//
simple SensorApp
{
    parameters:
        int    nodeId = default(0);
        int    zoneId = default(0);

        double meanTelemetryInterval @unit(s) = default(60s);

        double fireDetectionProb  = default(0.90);
        double falseAlarmProb     = default(0.01);

        double baseTempMu    = default(25.0);
        double baseTempSigma = default(3.0);
        double baseHumMu     = default(60.0);
        double baseHumSigma  = default(5.0);
        double tempThreshold  = default(60.0);
        double smokeThreshold = default(0.7);

        @display("i=misc/sensor2");

        @signal[alarmsSent](type=long);
        @signal[telemetrySent](type=long);

        @statistic[alarmsSent](title="alarms sent"; source=alarmsSent; record=sum,vector; checkSignals=false);
        @statistic[telemetrySent](title="telemetry sent"; source=telemetrySent; record=sum; checkSignals=false);

    gates:
        input  socketIn;
        output socketOut;
        input  directIn @directIn;
}
