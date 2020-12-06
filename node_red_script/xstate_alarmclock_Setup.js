const XState = require('xstate');
// const XState = global.get('xstate');

const alarmClockMachine = XState.Machine(
    {
        id: 'alarmClock',
        initial: 'idle',
        states: {
            idle: { on: { ARMING: 'armed' } },
            armed: { on: { DISARMING: 'idle', LAUNCHING: 'alarm_a' } },
            alarm_a: {
                entry: ['set_alarm_a_msg'],
                after: {
                    5000: { target: 'alarm_b', actions: "refresh" }
                },
                on: { STOP: 'clean_up', REPLAY_ALARM: { actions: 'set_alarm_a_msg' } }
            },
            alarm_b: {
                entry: ['set_alarm_b_msg'],
                on: { STOP: 'clean_up', REPLAY_ALARM: { actions: 'set_alarm_b_msg' } }
            },
            clean_up: {
                entry: ['set_clean_up_msg', XState.send('FINISHING')],
                on: { FINISHING: 'idle' }
            }
        }
    },
    {
        actions: {
            set_alarm_a_msg: (context, event) => {
                node.send({
                    topic: "mrdiynotifier/play",
                    payload: 'http://192.168.2.100:8001/toShare/radiohead.mp3',
                    state: alarmClockService.state.value
                });
                node.send({
                    topic: "mrdiynotifier/mqttLedState",
                    payload: '1,10000',
                    state: alarmClockService.state.value
                });
            },
            set_alarm_b_msg: (context, event) => {
                node.send({
                    topic: "mrdiynotifier/stream",
                    payload: 'http://ais-edge16-jbmedia-nyc04.cdnstream.com/hot108',
                    state: alarmClockService.state.value
                });
                node.send({
                    topic: "mrdiynotifier/mqttLedState",
                    payload: '2,1000',
                    state: alarmClockService.state.value
                });
            },
            set_clean_up_msg: (context, event) => {
                node.send({
                    topic: "mrdiynotifier/stop",
                    payload: '',
                    state: alarmClockService.state.value
                });
                node.send({
                    topic: "mrdiynotifier/mqttLedState",
                    payload: '3,5000',
                    state: alarmClockService.state.value
                });
                node.send({
                    topic: "disableArmingAlarm",
                    payload: 'true',
                    state: alarmClockService.state.value
                });
            },
            refresh: (context, event) => {
                node.status({ text: alarmClockService.state.value });
            },
        }
    },
    {
        guards: {
            // guardTest
        }
    }
);

const alarmClockService = XState.interpret(alarmClockMachine).start();
// context.set("alarmClockService",alarmClockService); // to store a variable

// ==== EXTRA =====
const node = {
    send: function (msgOut) {
        console.log(msgOut);
    },
    status: function (msgOut) {
        console.log(msgOut);

    },
}

module.exports = alarmClockService;
