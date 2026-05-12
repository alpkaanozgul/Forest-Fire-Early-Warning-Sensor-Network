package forestfiresim.server;

//
// FireServer: central fire monitoring server.
// Receives all packets from gateway queues; computes D_alarm, E2E delay, PDR.
//
simple FireServer
{
    parameters:
        int numNodes = default(40);

        @display("i=block/server2;p=500,500");

        @signal[dAlarm](type=double);
        @signal[e2eDelay](type=double);
        @signal[telemetryDelay](type=double);
        @signal[packetsReceived](type=long);
        @signal[alarmsReceived](type=long);
        @signal[falseAlarmsReceived](type=long);

        // D_alarm: fire event occurrence --> server reception (primary KPI)
        @statistic[dAlarm](title="D_alarm end-to-end alarm delay"; source=dAlarm; unit=s; record=mean,max,min,vector,histogram; checkSignals=false);
        @statistic[e2eDelay](title="E2E packet delay"; source=e2eDelay; unit=s; record=mean,max,vector; checkSignals=false);
        @statistic[telemetryDelay](title="telemetry E2E delay"; source=telemetryDelay; unit=s; record=mean,vector; checkSignals=false);
        @statistic[packetsReceived](title="packets received"; source=packetsReceived; record=sum,vector; checkSignals=false);
        @statistic[alarmsReceived](title="alarms received"; source=alarmsReceived; record=sum,vector; checkSignals=false);
        @statistic[falseAlarmsReceived](title="false alarms received"; source=falseAlarmsReceived; record=sum; checkSignals=false);

    gates:
        input in[];   // vector gate: G gateways connect here
}
