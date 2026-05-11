package forestfiresim;

//
// FireServer.ned
// The central fire monitoring server.
// Receives all packets from gateways via INET backend.
// Computes and records end-to-end delay and PDR.
//
simple FireServer
{
    parameters:
        int numNodes = default(40);

        @display("i=block/server2;p=500,500");

        // Statistics
        @statistic[e2eDelay](title="end-to-end alarm delay"; unit=s; record=mean,max,min,vector);
        @statistic[telemetryDelay](title="end-to-end telemetry delay"; unit=s; record=mean,vector);
        @statistic[packetsReceived](title="packets received"; record=sum,vector);
        @statistic[alarmsReceived](title="alarms received"; record=sum,vector);
        @statistic[falseAlarmsReceived](title="false alarms received"; record=sum);

    gates:
        input in;
}
