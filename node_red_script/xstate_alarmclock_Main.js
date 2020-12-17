// ==== EXTRA =====
const node = {
    send: function (msgOut) {
        console.log(msgOut);
    },
    status: function (msgOut) {
        console.log(msgOut);

    },
}
// ================

// ==== TEST SECTION ===
node_xstate_clock({ topic: 'disabled', payload: false });
node_xstate_clock({ topic: 'alarmClock' });
setTimeout(() => { node_xstate_clock({ topic: "remoteScale/fromESP", payload: "STOP" }); }, 10000);
// node_xstate_clock({ topic: "remoteScale/fromESP", payload: "STOP" });


// ==== NODE SECTION ===
function node_xstate_clock(msg) {
    // alarmClockService =context.get("alarmClockService"); //to retrieve a variable 
    const alarmClockService = require('./xstate_alarmclock_Setup.js')


    function throw_error() {
        node.status({ text: "ERROR!" });
        node.send({ payload: "ERROR!" });

    }

    // parsing input!!
    switch (msg.topic) {
        case "disabled": // from the Node Red UI toggling / switch button
            if (msg.payload == false) { event_input = 'ARMING' }
            else if (msg.payload == true) { event_input = 'STOP' }
            else { throw_error() }
            break;
        case "alarmClock": // from the Node Red alarm clock / timer
            event_input = "LAUNCHING";
            break;
        case "remoteScale/fromESP": // from the MCU on the remote scale
            event_input = "STOP"
            break;
        case "esparkle/out": // from the MCU on the alarmclock
            if (msg.payload == "Reconnected to MQTT") { node.status({ text: "esp_reboot" }) } // TODO : Use these watchdog!
            else if (msg.payload == "Mp3 done watchdog : is it normal?") { node.status({ text: "esp_mp3_done" }) }
            break;
        default:
            throw_error();
            break;

    }

    alarmClockService.send(event_input);
}
