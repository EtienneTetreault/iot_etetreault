// https://xstate.js.org/

const XState = require('xstate');
// const XState = global.get('xstate');
const Machine = XState.Machine;


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


// const delayList = [2000, 2000, 2000, "LastAlarm"];
const delayList = [10 * 60000, 10 * 60000, 5 * 60000, "LastAlarm"];

const actionList = [{ "led": "Off", "mp3": "http://192.168.2.100:8001/toShare/ambianceMusic/NatureTherapyRelaxingFullMotionForestrywithNaturalSounds.mp3" },
{ "led": "Rainbow", "delay": 500, "mp3": "http://cbcmp3.ic.llnwd.net/stream/cbcmp3_P-2QMTL0_MTL" },
{ "led": "Rainbow", "delay": 20, "mp3": "http://192.168.2.100:8001/toShare/trashMusic/TheSiegeofDunkeld.mp3" },
{ "led": "Disco", "mp3": "http://192.168.2.100:8001/toShare/LoudAlarmClockSoundEffectsAllSounds.mp3" }];


const alarmClockMachine = Machine(
    {
        id: 'alarmClock',
        initial: 'boot',
        context: {
            iterator: 0,
        },
        states: {
            boot: { after: { 5000: 'idle' } }, // Let time to NodeRed to boot!!
            idle: {
                entry: ["reset_states", XState.assign({ iterator: 0 })],
                on: { ARMING: 'armed' }
            },
            armed: { on: { STOP: 'idle', LAUNCHING: 'ringing' } },
            ringing: {
                id: "testID",
                on: {
                    STOP: 'lightedOn',
                    MP3_STOPPED: { target: undefined, actions: 'send_mqtt_from_list' }
                },
                initial: "check_if_last",
                states: {
                    check_if_last: {
                        entry: ['send_mqtt_from_list'],
                        // TODO : Use "always" transition instead of deprecated "" transition. Note: Try but was hard
                        on: {
                            '': [
                                { target: "ring_last", cond: "is_it_last_alarm" },
                                { target: "ring_from_list" }
                            ]
                        },
                    },
                    ring_from_list: {
                        after: {
                            ALARM_DELAY: [{ target: "check_if_last" },]
                        },
                        exit: [XState.assign({ iterator: (context) => context.iterator + 1 })],
                    },
                    ring_last: {}
                },
            },
            lightedOn: { entry: ['music_stop_light_on'], after: { 600000: 'idle' } },
        }
    },
    {
        delays: {
            ALARM_DELAY: (context, event) => {
                return delayList[context.iterator];
            },
        },
        actions: {
            send_mqtt_from_list: (context, event) => {
                node.send({
                    topic: "esparkle/in",
                    payload: actionList[context.iterator],
                });
            },
            music_stop_light_on: (context, event) => {
                node.send({
                    topic: "esparkle/in",
                    payload: { "led": "Solid", "color": "0xFF0000", "rtttl": "starwars:d=4" },
                });
            },
            reset_states: (context, event) => {
                node.send({
                    topic: "esparkle/in",
                    payload: { "led": "Off", "cmd": "break" },
                });
                node.send({
                    topic: "disableArmingAlarm",
                    payload: true,
                });
            },
        },
        guards: {
            is_it_last_alarm: (context, event) => {
                return context.iterator + 1 >= delayList.length;
            }
        },
    },
);
// The Interpreter : Manage machine's event and transition
const alarmClockService = XState.interpret(alarmClockMachine).start();

// Listener service of Interpreter : print State on transition (to Node status)
alarmClockService.onTransition(
    (state) => node.status({ text: JSON.stringify(alarmClockService.state.value) })
);
// context.set("alarmClockService",alarmClockService); // to store a variable

module.exports = alarmClockService;
