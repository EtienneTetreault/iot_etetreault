
// ==== EXTRA =====
const node = {
    status: function (msgOut) {
        console.log(msgOut);
    }
};

// ==== TEST SECTION ===
node_xstate_clock({ topic: 'disabled', payload: false });
node_xstate_clock({ topic: 'alarmClock' });
setTimeout(() => { node_xstate_clock({ topic: "remoteScale/fromESP", payload: "STOP" }); }, 10000);
// node_xstate_clock({ topic: "remoteScale/fromESP", payload: "STOP" });


// ==== NODE SECTION ===
function node_xstate_clock(msg) {
    // alarmClockService =context.get("alarmClockService"); //to retrieve a variable 
    const alarmClockService = require('./xstate_alarmclock_Setup.js')

    // parsing input!!
    switch (msg.topic) {
        case "disabled": // from the Node Red UI toggling / switch button
            if (msg.payload == false) { event_input = 'ARMING' }
            else { event_input = alarmClockService.state.value == "armed" ? 'DISARMING' : 'STOP' }
            break;
        case "alarmClock": // from the Node Red alarm clock / timer
            event_input = "LAUNCHING";
            break;
        case "remoteScale/fromESP": // from the remote scale PCB
            event_input = "STOP"
            break;
        case "mrdiynotifier/status": // from the alarm clock PCB
            if (msg.payload == "idle") { event_input = "REPLAY_ALARM" }
            break;
        default:
            event_input = "BUG";
            console.log('BUG!!!!');
            break;

    }

    alarmClockService.send(event_input);
    node.status({ text: alarmClockService.state.value });
}
