// #include "etienneClock.h"

// Clock::Clock(long setRefreshTime, NTPClient setTimeClient, TM1637Display setDisplayObj) : refreshTime(setRefreshTime), testTimeClient(setTimeClient), testDisplayObj(setDisplayObj)
// {
//     previousMillis = 0;
// }

// void Clock::UpdateTime(void)
// {
//     unsigned long currentMillis = millis();
//     if (currentMillis - previousMillis >= refreshTime)
//     {
//         testTimeClient.update();
//         int timeNow = testTimeClient.getHours() * 100 + testTimeClient.getMinutes();
//         testDisplayObj.showNumberDecEx(timeNow, 0b01000000, false, 4, 0);
//         previousMillis = currentMillis; // Remember the time
//     }
// }