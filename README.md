
# Stepper Motor Controller

This is a single C++ class that provides full control of a stepper motor.

This class is designed to be included in the firmware of MCU's such as Arduino and ESP32.  It will interface with most digital stepper drivers that have Enable, Direction and Step pins, such as the [BTT TMC2209](https://biqu.equipment/products/bigtreetech-tmc2209-stepper-motor-driver-for-3d-printer-board-vs-tmc2208).

## Features:
- Can be used by directly calling class methods -OR- sending text commands
- Uses soft limits with lower and upper values
- Optionally uses hard limits by specifying one or two limit switch pins
- Includes Enable (engage) and Disable (disengage) of the motor
- Allows for any step speed from 1 to 9999 steps per second
- Allows for both Absolute and Relative motion
- Includes adjustable Velocity Ramping for soft start and stop motion
- Includes physical Homing using limit switch
- Includes Emergency Stop (E-Stop)
- Includes query methods/commands

---
### If you use this code, please <a href="https://buymeacoffee.com/dstechlabs" target="_blank">Buy Me A Coffee ☕</a>


## Some Background
A "digital" stepper motor driver requires three(3) GPIO pins to operate: Enable, Direction
and Step.  The default GPIO pins used by this class are the following, but can be set using Constructor params:

~~~
Enable    = digital pin 2 (D2)
Direction = digital pin 3 (D3)
Step      = digital pin 4 (D4)
~~~

(GND) should be used for common logic ground.<br>
Two extra pins may also be specified in the Constructor for optional upper and lower limit switches.

---
This class keeps track of the motor's step count from its HOME position, which is zero(0).
- The Motor's ABSOLUTE position is the number of steps away from HOME.  This number can be
positive (clockwise) or negative (counter-clockwise).
- The motor's RELATIVE position is the number of steps away from its last position.

~~~
                         🠈 -   + 🠊
 ▐── ··· ──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼── ··· ──▌
 ▐               │           │           │               ▌
Lower         -10000         0         10000           Upper
Limit                       HOME                       Limit
~~~

LOWER and UPPER range limit values can be specified.
The default LOWER and UPPER range limits are set to +2 billion and -2 billion at start up.
It is assumed the user will set these "soft" limits from a configuration file or other means.

If a range limit has been reached, then the motor's motion is stopped and a Range Error
is returned from the Run() method.

Two GPIO pins can also be optionally specified for checking upper and lower limit switches.

---
At startup, this class initializes the motor as DISABLED (the driver is not engaged) and
not homed (the motor is at an unknown position).  Before motion can begin, the motor must
be Enabled with Enable() method or "EN" command.  This will also "Home" to motor by setting
the current ABSOLUTE position to zero(0).

## Velocity Ramping
Motor rotation velocity follows a trapezoidal shape.
A linear ramp-up/ramp-down rate is set with the SetRamp() method or "SR" command.

                                     ┌────────────────────────────────┐
    A ramp value of 0                │                                │
    specifies no ramping:            │                                │
    (not recommended !!!)            │                                │
                                   ──┴────────────────────────────────┴── time 🠊<br>
                                         .────────────────────────.
    A ramp value of 5                   /                          \.
    specifies moderate ramping:        /                            \.
    This is the default at startup    /                              \.
                                   ──┴────────────────────────────────┴── time 🠊<br>
                                             .────────────────.
    A ramp value of 9                      /                    \.
    specifies gradual ramping            /                        \.
    for heavy loads                    /                            \.
                                   ───┴──────────────────────────────┴─── time 🠊<br>

Use low values (2, 3, ..) for fast accelerations with light loads.  Use high values (.., 8, 9)
for slow accelerations with heavy loads.  It is highly recommended to use slow acceleration
when moving high inertial loads.

                                              ──────────    <── full velocity
    If there is not enough time to achieve
    full velocity, then rotation velocity         /\.
    follows a "stunted" triangle path:           /  \.
                                              ──┴────┴──

Once a ramp value is set, all rotate commands will use that ramp value.
The default ramp value at startup is 5.

## How To Use

Include the StepperMotorController .cpp and .h files in your firmware.
Decide which GPIO pins to use for Enable, Direction and Step signals to your driver board.
The defaults are 2, 3 and 4 respectively.

Instantiate a StepperMotorController as a global so the rest of your firmware can access it:

    StepperMotorController MyMotor = StepperMotorController (enablePin, directionPin, stepPin);

Optionally, you can also specify one or two GPIO pins to use for Limit Switches:

    StepperMotorController MyMotor = StepperMotorController (enablePin, directionPin, stepPin, lowerLimitSwitch, upperLimitSwitch);

Once you have a StepperMotorController object, you will need to Enable the driver before
motion can begin:

    setup()
    {
      ...
      MyMotor.Enable();
      ...
    }

If your motor is attached to equipment then "Homing" is usually necessary to place the motor
in a known beginning position.  You can do this with the `FindHome()` method or the `FH` command.
`FindHome()` Enables the driver and uses the physical lower limit switch specified in the constructor.

    setup()
    {
      ...
      MyMotor.FindHome();
      ...
    }

Once Enabled and ready, you __MUST__ continually call the `Run()` method in your `loop()` function:

    loop()
    {
      ...
      RunReturn rr = MyMotor.Run();
      ...
    }

Don't worry about any timings, its handled for you. If it's not time to take a step then
Run() immediately returns without delay.

The Run() method returns one of the following `RunReturn` enum results:
~~~
OKAY                - Idle or still running
RUN_COMPLETE        - Motion complete, reached target position normally
RANGE_ERROR_LOWER   - Reached lower range soft limit
RANGE_ERROR_UPPER   - Reached upper range soft limit
LIMIT_SWITCH_LOWER  - Lower limit switch triggered
LIMIT_SWITCH_UPPER  - Upper limit switch triggered
~~~
<br>

---
All control can be performed either by directly calling methods or by executing
a command string - from a UI for example.

Your firmware should normally wait until the motor is finished with a previous Rotate method/command
before issuing a new Rotate command.  If a Rotate command is called while the motor is already
running, then the current rotation is interrupted and the new Rotate command is executed from
the motor's current position.

## Class Methods
See the `StepperMotorController.h` file for all methods.

This class also has an `ExecuteCommand()` method to allow external interfaces to run the motor,
say from a browser app or other UI.

`ExecuteCommand()` takes a short 2-char string command.  Some commands require additional parameters.

<table>
  <tr><td>EN   </td><td>ENABLE               </td><td>Enables the motor driver (energizes the motor) and also sets the HOME position</td></tr>
  <tr><td>DI   </td><td>DISABLE              </td><td>Disables the motor driver (releases the motor)</td></tr>
  <tr><td>FH   </td><td>FIND HOME            </td><td>Seeks counter-clockwise until lower limit switch is triggered, backs off a bit and sets HOME position</td></tr>
  <tr><td>SH   </td><td>SET HOME POSITION    </td><td>Sets the current position of the motor as its HOME position (Sets Absolute position to zero)</td></tr>
  <tr><td>SL...</td><td>SET LOWER LIMIT      </td><td>Sets the LOWER LIMIT (minimum Absolute Position) of the motor's range</td></tr>
  <tr><td>SU...</td><td>SET UPPER LIMIT      </td><td>Sets the UPPER LIMIT (maximum Absolute Position) of the motor's range</td></tr>
  <tr><td>SRr  </td><td>SET RAMP             </td><td>Sets the trapezoidal velocity RAMP (up/down) for smooth motor start and stop</td></tr>
  <tr><td>RA...</td><td>ROTATE ABSOLUTE      </td><td>Rotates motor to an Absolute target position from its HOME position</td></tr>
  <tr><td>RR...</td><td>ROTATE RELATIVE      </td><td>Rotates motor clockwise or counter-clockwise any number of steps from its current position</td></tr>
  <tr><td>RH   </td><td>ROTATE HOME          </td><td>Rotates motor to its HOME position</td></tr>
  <tr><td>RL   </td><td>ROTATE TO LOWER LIMIT</td><td>Rotates motor to its LOWER LIMIT position</td></tr>
  <tr><td>RU   </td><td>ROTATE TO UPPER LIMIT</td><td>Rotates motor to its UPPER LIMIT position</td></tr>
  <tr><td>ES   </td><td>E-STOP               </td><td>Stops motion immediately (emergency stop) and DIsables the motor. It must be re-enabled with EN</td></tr>
  <tr><td>GA   </td><td>GET ABSOLUTE position</td><td>Returns the motor's current step position relative to its HOME position</td></tr>
  <tr><td>GR   </td><td>GET RELATIVE position</td><td>Returns the motor's current step position relative to its last targeted position</td></tr>
  <tr><td>GL   </td><td>GET LOWER LIMIT      </td><td>Returns the motor's Absolute LOWER LIMIT position</td></tr>
  <tr><td>GU   </td><td>GET UPPER LIMIT      </td><td>Returns the motor's Absolute UPPER LIMIT position</td></tr>
  <tr><td>GT   </td><td>GET TIME             </td><td>Returns the remaining time in ms for motion to complete</td></tr>
  <tr><td>GV   </td><td>GET VERSION          </td><td>Returns this firmware's current version</td></tr>
  <tr><td>BLp  </td><td>BLINK LED            </td><td>Blink the specified LED to indicate identification</td></tr>
</table>

where r is the velocity ramp rate (0-9), p is the pin number of an LED

~~~
───────────────────────────────────────────────────
 Command String Format: (no spaces between fields)
───────────────────────────────────────────────────
                           cc vvvv sssssssssss...<lf>
                           │   │        │
  Command/Query ───────────┘   │        │
    [2-chars]                  │        │
      EN = ENABLE              │        │
      DI = DISABLE             │        │
      FH = FIND HOME           │        │
      SH = SET HOME            │        │
      SL = SET LOWER LIMIT     │        │
      SU = SET UPPER LIMIT     │        │
      SR = SET RAMP            │        │
      RA = ROTATE ABSOLUTE     │        │
      RR = ROTATE RELATIVE     │        │
      RH = ROTATE HOME         │        │
      RL = ROTATE LOWER LIMIT  │        │
      RU = ROTATE UPPER LIMIT  │        │
      ES = E-STOP              │        │
      GA = GET ABS POSITION    │        │
      GR = GET REL POSITION    │        │
      GL = GET LOWER LIMIT     │        │
      GU = GET UPPER LIMIT     │        │
      GT = GET TIME            │        │
      GV = GET VERSION         │        │
      BL = BLINK ────pin#──────┤        │
                               │        │
  Velocity (steps per sec) ────┤        │
    [4-digits] 1___ - 9999     │        │
    Right-padded with spaces   │        │
                               │        │
  Ramp Slope ──────────────────┘        │
    [1-digit] 0 - 9                     │
    (For SR command only)               │
                                        │
  Absolute or Relative Step Position ───┘ ───┐
    [1 to 10-digits plus sign]               │
    No padding necessary                     ├─── For ROTATE commands only
    -2147483648 to 2147483647 (long)         │
    Positive values = Clockwise              │
    Negative values = Counter-Clockwise  ────┘
~~~

### Example String Commands: (quotes are not included in string)

<table>
  <tr><td>"EN"          </td><td>ENABLE               </td><td>Enable the stepper motor driver</td></tr>
  <tr><td>"DI"          </td><td>DISABLE              </td><td>Disable the stepper motor driver</td></tr>
  <tr><td>"SR6"         </td><td>SET RAMP             </td><td>Set the velocity ramp-up/ramp-down rate to 6</td></tr>
  <tr><td>"RA500 2000"  </td><td>ROTATE ABSOLUTE      </td><td>Rotate the motor at 500 steps per second, to Absolute position of +2000 steps clockwise from HOME</td></tr>
  <tr><td>"RR3210-12000"</td><td>ROTATE RELATIVE      </td><td>Rotate the motor at 3210 steps per second, -12000 steps counter-clockwise from its current position</td></tr>
  <tr><td>"RH"          </td><td>ROTATE HOME          </td><td>Rotate motor back to its HOME position (0)</td></tr>
  <tr><td>"RL"          </td><td>ROTATE to LOWER LIMIT</td><td>Rotate motor to its LOWER LIMIT position</td></tr>
  <tr><td>"ES"          </td><td>EMERGENCY STOP       </td><td>Immediately stop the motor and cancel rotation command</td></tr>
  <tr><td>"GR"          </td><td>GET RELATIVE POSITION</td><td>Get the current relative step position of the motor</td></tr>
</table>

This class may also be queried for position, range limits, remaining motion time and firmware version with the following commands:

<table>
  <tr><td>"GA"</td><td>GET ABSOLUTE position</td><td>Returns the motor's current step position relative to its HOME position</td></tr>
  <tr><td>"GR"</td><td>GET RELATIVE position</td><td>Returns the motor's current step position relative to its last targeted position</td></tr>
  <tr><td>"GL"</td><td>GET LOWER LIMIT      </td><td>Returns the motor's Absolute LOWER LIMIT position</td></tr>
  <tr><td>"GU"</td><td>GET UPPER LIMIT      </td><td>Returns the motor's Absolute UPPER LIMIT position</td></tr>
  <tr><td>"GT"</td><td>GET TIME             </td><td>Returns the remaining time in ms for motion to complete</td></tr>
  <tr><td>"GV"</td><td>GET VERSION          </td><td>Returns this firmware's current version</td></tr>
</table>
<br>

The returned result is a string with the following format:
~~~
APs... = Absolute Position of motor is s steps from its HOME position
RPs... = Relative Position of motor is s steps from its last targeted position
LLs... = LOWER LIMIT of motion
ULs... = UPPER LIMIT of motion
~~~
where s is a positive or negative long integer (depending on clockwise or counter-clockwise position)
and represents a number of steps which may be 1 to 10 digits plus a possible sign.

---
### If you use this code, please <a href="https://buymeacoffee.com/dstechlabs" target="_blank">Buy Me A Coffee ☕</a>

