package forestfiresim;

//
// SensorApp.ned
// The application layer running on each LoRa sensor node.
// Handles periodic telemetry, fire detection, false alarms,
// and retransmission logic.
//
simple SensorApp
{
    parameters:
        int    nodeId = default(0);
        int    zoneId = default(0);          // which forest zone this sensor is in

        // Telemetry scheduling
        double meanTelemetryInterval @unit(s) = default(60s);

        // Detection probabilities
        double fireDetectionProb  = default(0.90);  // pd: Bernoulli trial on real fire
        double falseAlarmProb     = default(0.01);  // pf: Bernoulli trial each cycle

        // Normal environmental baselines (used for Normal variate generation)
        double baseTempMu    = default(25.0);   // degrees C
        double baseTempSigma = default(3.0);
        double baseHumMu     = default(60.0);   // percent
        double baseHumSigma  = default(5.0);

        // Thresholds above which a reading is considered fire-risk
        double tempThreshold  = default(60.0);
        double smokeThreshold = default(0.7);

        @display("i=block/app");

        // Statistics collected automatically by OMNeT++
        @statistic[e2eDelay](title="end-to-end delay"; unit=s; record=mean,max,vector);
        @statistic[alarmsSent](title="alarms sent"; record=sum,vector);
        @statistic[telemetrySent](title="telemetry sent"; record=sum);

    gates:
        // Gate for receiving fire notifications directly from FireGen
        input directIn @directIn;

        // Gates to/from the LoRa radio layer below
        input  lowerLayerIn;
        output lowerLayerOut;
}
