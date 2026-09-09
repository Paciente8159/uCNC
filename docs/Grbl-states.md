# Grbl states table

|CurrentState|Trigger/Event             |NextState |Notes                                      |
|------------|--------------------------|----------|-------------------------------------------|
|Idle        |~ (Cycle Start)           |Run       |Begin executing motion                     |
|Idle        |$H                        |Home      |Start homing cycle                         |
|Idle        |$C                        |Check     |Enter parser check mode                    |
|Idle        |$SLP                      |Sleep     |Enter sleep mode                           |
|Idle        |Door Open                 |Door      |Enter safety door suspend                  |
|Idle        |Hard/Soft Limit           |Alarm     |Limit triggered                            |
|Idle        |Reset                     |Idle/Alarm|Depends on config                          |
|Idle        |0x84 Safety Door          |Door      |Immediate suspend; spindle/coolant disabled|
|Idle        |0xA0 Flood Toggle         |Idle      |Coolant modal toggled                      |
|Idle        |0xA1 Mist Toggle          |Idle      |Mist toggle (if enabled)                   |
|Run         |!                         |Hold      |Feed hold (decelerate)                     |
|Run         |Door Open                 |Door      |Door suspend (parking if enabled)          |
|Run         |M2/M30                    |Idle      |Program end                                |
|Run         |Hard/Soft Limit           |Alarm     |Immediate stop                             |
|Run         |Reset                     |Alarm     |Reset while in motion                      |
|Run         |0x84 Safety Door          |Door      |Decelerate then suspend; parking if enabled|
|Run         |0x90–0x94 Feed Override   |Run       |Adjust feed override                       |
|Run         |0x95–0x97 Rapid Override  |Run       |Adjust rapid override                      |
|Run         |0x99–0x9D Spindle Override|Run       |Adjust spindle override                    |
|Run         |0xA0 Flood Toggle         |Run       |Coolant modal toggled                      |
|Run         |0xA1 Mist Toggle          |Run       |Mist toggle (if enabled)                   |
|Hold        |Decel Complete            |Hold:0    |Hold finished                              |
|Hold        |~ (Resume)                |Run       |Resume motion                              |
|Hold        |Door Open                 |Door      |Door suspend                               |
|Hold        |Reset                     |Alarm     |Reset during hold                          |
|Hold        |0x84 Safety Door          |Door      |Door supersedes hold                       |
|Hold        |0x9E Toggle Spindle Stop  |Hold      |Spindle stop override                      |
|Hold        |0xA0 Flood Toggle         |Hold      |Coolant modal toggled                      |
|Hold        |0xA1 Mist Toggle          |Hold      |Mist toggle (if enabled)                   |
|Door        |Door Open During Run      |Door:2    |Parking/hold in progress                   |
|Door        |Door Still Open           |Door:1    |Stopped but cannot resume                  |
|Door        |Door Closed               |Door:0    |Ready to resume                            |
|Door        |~ (Resume)                |Run       |Resume from door state                     |
|Door        |Reset                     |Alarm     |Reset during door state                    |
|Check       |$C again                  |Idle/Alarm|Exit check mode (soft reset)               |
|Check       |Reset                     |Idle/Alarm|Reset behavior                             |
|Check       |G-code Input              |Check     |Parse only                                 |
|Home        |Success                   |Idle      |Homing complete                            |
|Home        |Failure                   |Alarm     |Homing error                               |
|Home        |Door Open                 |Alarm     |Door during homing                         |
|Home        |Reset                     |Alarm     |Reset during homing                        |
|Home        |0x84 Safety Door          |Alarm     |Safety door aborts homing                  |
|Home        |!                         |Home      |Ignores                                    |
|Jog         |$J=…                      |Jog       |Jog motion                                 |
|Jog         |Jog Complete              |Idle      |Return to idle                             |
|Jog         |Jog Cancel                |Idle      |Planner purge                              |
|Jog         |0x85 Jog Cancel           |Idle/Door |Cancels jog; Door if ajar                  |
|Jog         |Hard/Soft Limit           |Alarm     |Jog safety fault                           |
|Jog         |Door Open                 |Door      |Door suspend                               |
|Jog         |Reset                     |Alarm     |Reset during jog                           |
|Jog         |0x84 Safety Door          |Door      |Jog canceled; planner flushed              |
|Alarm       |$X Unlock                 |Idle      |Unlock alarm                               |
|Alarm       |$H                        |Home      |Re-home after alarm                        |
|Alarm       |$SLP                      |Sleep     |Sleep from alarm                           |
|Alarm       |Reset                     |Alarm/Idle|Depends on cause                           |
|Sleep       |Alarm                     |Alarm     |Wakes in alarm                             |
|Sleep       |Any Other Command         |Sleep     |Ignored                                    |
