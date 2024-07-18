//==========================================================
//
//     FILE : StepperMotorController.h
//
//  PROJECT : Stepper Motor Controller (Digital Only)
//
//    NOTES : Stepper Motor Controller, Version 2024-06-10
//            This firmware is used to operate a single digital bipolar stepper driver/motor
//            from an MCU development board such as an Arduino Nano.  It incorporates both
//            soft and hard limits.
//
//            To upload to an old Arduino Nano Clone:
//              ∙ Arduino Board Config
//                · Selected Board : Arduino Nano (Arduino AVR Boards)
//                · Processor      : ATmega328P (Old Bootloader)
//
//   AUTHOR : Bill Daniels (bill@dstechlabs.com)
//            See LICENSE.md
//
//==========================================================
//
//  This class communicates (by GPIO pins) with a bipolar micro-stepping motor driver that take
//  Enable, Direction and Step (pulse) signals.
//
//  A "digital" stepper motor driver requires three(3) GPIO pins to operate: Enable, Direction
//  and Step.  The default GPIO pins are the following, but can be set using Constructor params:
//
//    Enable    = digital pin 2 (D2)
//    Direction = digital pin 3 (D3)
//    Pulse     = digital pin 4 (D4)
//
//  (GND) should be used for common logic ground.
//
//  Two extra pins may also be specified for upper and lower limit switches.
//
//─────────────────────────────────────────────────────────────────────────────────────────────────
//
//  This class keeps track of the motor's step count from its HOME position, which is zero(0).
//  The Motor's ABSOLUTE position is the number of steps away from HOME.  This number can be
//  positive (clockwise) or negative (counter-clockwise).
//
//     ▐── ····· ──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼── ····· ──▌
//     ▐           │           │           │           │           │                 ▌
//    Lower      -2000       -1000         0          1000        2000             Upper
//    Limit                               HOME                                     Limit
//
//  The motor's RELATIVE position is the number of steps away from its last position.
//
//  LOWER and UPPER range limit values can be specified.
//  The default LOWER and UPPER range limits are set to +2 billion and -2 billion at start up.
//  It is assumed the user will set these limits from a configuration file or other means.
//
//  If a range limit has been reached, then the motor's motion is stopped and a Range Error
//  is returned from the Run() method.
//
//  Two GPIO pins can also be optionally specified for checking upper and lower limit switches.
//
//─────────────────────────────────────────────────────────────────────────────────────────────────
//
//  At startup, this class initializes the motor as DISABLED (the driver is not engaged) and
//  not homed (the motor is at an unknown position).  Before motion can begin, the motor must
//  be Enabled with Enable() or "EN" command and Homed with SetHomePosition() or "SH" command.
//
//─────────────────────────────────────────────────────────────────────────────────────────────────
//
//  Motor rotation velocity follows a trapezoidal shape.
//  A linear ramp-up/ramp-down rate is set with the SetRamp() method or "SR" command.
//
//                                   ┌────────────────────────────────┐    <── full velocity
//  A ramp value of 0                │                                │
//  specifies no ramping:            │                                │
//  (not recommended !!!)            │                                │
//                                 ──┴────────────────────────────────┴──
//
//                                       .────────────────────────.        <── full velocity
//  A ramp value of 5                   /                          \.
//  specifies moderate ramping:        /                            \.
//  This is the default at startup    /                              \.
//                                 ──┴────────────────────────────────┴──
//
//                                           .────────────────.            <── full velocity
//  A ramp value of 9                      /                    \.
//  specifies gradual ramping:           /                        \.
//                                     /                            \.
//                                 ───┴──────────────────────────────┴───
//
//  Use low values (2, 3, ..) for fast accelerations with light loads and high values (.., 8, 9) for slow accelerations
//  with heavy loads.  It is highly recommended to use slow acceleration when moving high inertial loads.
//
//                                            ──────────    <── full velocity
//  If there is not enough time to achieve
//  full velocity, then rotation velocity         /\.
//  follows a "stunted" triangle path:           /  \.
//                                            ──┴────┴──
//
//  Once a ramp value is set, all rotate commands will use that ramp value.
//  The default ramp value at start-up is 5.
//
//─────────────────────────────────────────────────────────────────────────────────────────────────
//
//  All control can be performed either by directly calling methods or by executing a command
//  string, from a UI for example.  To keep the motor running, the Run() method must be called
//  continuously from your loop() method.
//
//  The Run() method returns one the following enum results:
//
//    OKAY                - Idle or still running
//    RUN_COMPLETE        - Motion complete, reached target position normally
//    RANGE_ERROR_LOWER   - Reached lower range limit
//    RANGE_ERROR_UPPER   - Reached upper range limit
//    LIMIT_SWITCH_LOWER  - Lower limit switch triggered
//    LIMIT_SWITCH_UPPER  - Upper limit switch triggered
//
//  Your app should normally wait until the motor is finished with a previous Rotate method/command
//  before issuing a new Rotate command.  If a Rotate command is called while the motor is already
//  running, then the current rotation is interrupted and the new Rotate command is executed from
//  the motor's current position.
//
//─────────────────────────────────────────────────────────────────────────────────────────────────
//
//  This class also has a method for operating the stepper motor by executing String commands.
//  Use the ExecuteCommand() method passing the following 2-char commands:
//
//    EN    = ENABLE                - Enables the motor driver (energizes the motor) and also sets the HOME position
//    DI    = DISABLE               - Disables the motor driver (releases the motor)
//    FH    = FIND HOME             - Seeks counter-clockwise until lower limit switch is triggered, backs off a bit and sets HOME position
//    SH    = SET HOME POSITION     - Sets the current position of the motor as its HOME position (Sets Absolute position to zero)
//    SL... = SET LOWER LIMIT       - Sets the LOWER LIMIT (minimum Absolute Position) of the motor's range
//    SU... = SET UPPER LIMIT       - Sets the UPPER LIMIT (maximum Absolute Position) of the motor's range
//    SRr   = SET RAMP              - Sets the trapezoidal velocity RAMP (up/down) for smooth motor start and stop
//    RA... = ROTATE ABSOLUTE       - Rotates motor to an Absolute target position from its HOME position
//    RR... = ROTATE RELATIVE       - Rotates motor clockwise or counter-clockwise any number of steps from its current position
//    RH    = ROTATE HOME           - Rotates motor to its HOME position
//    RL    = ROTATE TO LOWER LIMIT - Rotates motor to its LOWER LIMIT position
//    RU    = ROTATE TO UPPER LIMIT - Rotates motor to its UPPER LIMIT position
//    ES    = E-STOP                - Stops the motor immediately and releases motor (emergency stop)
//    GA    = GET ABSOLUTE position - Returns the motor's current step position relative to its HOME position
//    GR    = GET RELATIVE position - Returns the motor's current step position relative to its last targeted position
//    GL    = GET LOWER LIMIT       - Returns the motor's Absolute LOWER LIMIT position
//    GU    = GET UPPER LIMIT       - Returns the motor's Absolute UPPER LIMIT position
//    GT    = GET TIME              - Returns the remaining time in ms for motion to complete
//    GV    = GET VERSION           - Returns this firmware's current version
//    BLp   = BLINK LED             - Blink the specified LED to indicate identification
//
//    where r is the velocity ramp rate (0-9)
//          p is the pin number of the built-in LED
//
//    ───────────────────────────────────────────────────
//     Command String Format: (no spaces between fields)
//    ───────────────────────────────────────────────────
//                               cc vvvv sssssssssss...<lf>
//                               │   │        │
//      Command/Query ───────────┘   │        │
//        [2-chars]                  │        │
//          EN = ENABLE              │        │
//          DI = DISABLE             │        │
//          FH = FIND HOME           │        │
//          SH = SET HOME            │        │
//          SL = SET LOWER LIMIT     │        │
//          SU = SET UPPER LIMIT     │        │
//          SR = SET RAMP            │        │
//          RA = ROTATE ABSOLUTE     │        │
//          RR = ROTATE RELATIVE     │        │
//          RH = ROTATE HOME         │        │
//          RL = ROTATE LOWER LIMIT  │        │
//          RU = ROTATE UPPER LIMIT  │        │
//          ES = E-STOP              │        │
//          GA = GET ABS POSITION    │        │
//          GR = GET REL POSITION    │        │
//          GL = GET LOWER LIMIT     │        │
//          GU = GET UPPER LIMIT     │        │
//          GT = GET TIME            │        │
//          GV = GET VERSION         │        │
//          BL = BLINK ────pin#──────┤        │
//                                   │        │
//      Velocity (steps per sec) ────┤        │
//        [4-digits] 1___ - 9999     │        │
//        Right-padded with spaces   │        │
//                                   │        │
//      Ramp Slope ──────────────────┘        │
//        [1-digit] 0 - 9                     │
//        (For SR command only)               │
//                                            │
//      Absolute or Relative Step Position ───┘ ───┐
//        [1 to 10-digits plus sign]               │
//        No padding necessary                     ├─── For ROTATE commands only
//        -2147483648 to 2147483647 (long)         │
//        Positive values = Clockwise              │
//        Negative values = Counter-Clockwise  ────┘
//
//    ───────────────────────────────────────────────
//     Examples: (quotes are not included in string)
//    ───────────────────────────────────────────────
//     "EN"            - ENABLE                 - Enable the stepper motor driver
//     "DI"            - DISABLE                - Disable the stepper motor driver
//     "SR6"           - SET RAMP               - Set the velocity ramp-up/ramp-down rate to 6
//     "SH"            - SET HOME               - Set the current position of the motor as its HOME position (which is zero)
//     "RA500 2000"    - ROTATE ABSOLUTE        - Rotate the motor at 500 steps per second, to Absolute position of +2000 steps clockwise from HOME
//     "RR3210-12000"  - ROTATE RELATIVE        - Rotate the motor at 3210 steps per second, -12000 steps counter-clockwise from its current position
//     "RH"            - ROTATE HOME            - Rotate motor back to its HOME position (0)
//     "RL"            - ROTATE to LOWER LIMIT  - Rotate motor to its LOWER LIMIT position
//     "ES"            - EMERGENCY STOP         - Immediately stop the motor and cancel rotation command
//     "GR"            - GET RELATIVE POSITION  - Get the current relative step position of the motor
//
//  This class may also be queried for position, range limits, remaining motion time and firmware version with the following commands:
//
//     GA = GET ABSOLUTE position - Returns the motor's current step position relative to its HOME position
//     GR = GET RELATIVE position - Returns the motor's current step position relative to its last targeted position
//     GL = GET LOWER LIMIT       - Returns the motor's Absolute LOWER LIMIT position
//     GU = GET UPPER LIMIT       - Returns the motor's Absolute UPPER LIMIT position
//     GT = GET TIME              - Returns the remaining time in ms for motion to complete
//     GV = GET VERSION           - Returns this firmware's current version
//
//  The returned result is a string with the following format:
//     APs... = Absolute Position of motor is s steps from its HOME position
//     RPs... = Relative Position of motor is s steps from its last targeted position
//     LLs... = LOWER LIMIT of motor relative to its HOME position
//     ULs... = UPPER LIMIT of motor relative to its HOME position
//  where s is a positive or negative long integer (depending on clockwise or counter-clockwise position)
//  and represents a number of steps which may be 1 to 10 digits plus a possible sign.
//
//
//─────────────────────────────────────────────────────────────────────────────────────────────────

#ifndef SMC_H
#define SMC_H

#define HOMING_SPEED      3000L
#define PULSE_WIDTH       5     // 5-microseconds (check your driver's pulse width requirement)
#define EC_RETURN_LENGTH  30

enum MotorState
{
  ENABLED,   // Motor driver is enabled, this is the normal idle/holding state
  DISABLED,  // Motor driver is disable, motor can be manually rotated
  RUNNING,   // Motor is running (rotating) and is currently executing a Rotate command
  ESTOPPED   // Motor is in an E-STOP condition (Emergency Stop), it must be Enabled to resume motion
};

enum RunReturn
{
  OKAY,                // Idle or still running
  RUN_COMPLETE,        // Motion complete, reached target position normally
  RANGE_ERROR_LOWER,   // Reached lower range limit
  RANGE_ERROR_UPPER,   // Reached upper range limit
  LIMIT_SWITCH_LOWER,  // Lower limit switch triggered
  LIMIT_SWITCH_UPPER   // Upper limit switch triggered
};


//=========================================================
//  class StepperMotorController
//=========================================================

class StepperMotorController
{
  private:
    const char  version[40] = "Stepper Motor Controller 2024-06-10";
    char        ecReturnString[EC_RETURN_LENGTH];

    MotorState  State = DISABLED;  // Default is disabled (unlocked)

    // GPIO Pins For Digital Stepper Driver and Limit Switches
    int  EnablePin, DirectionPin, StepPin, LLSwitchPin, ULSwitchPin;

    bool           Homed = false;      // The motor must be "Homed" before use
    const long     RampScale = 5L;
    long           MaxVelocity;        // Highest velocity while running
    long           TargetOrSteps;
    long           TotalSteps;
    long           StepCount;
    long           StepIncrement;      // 1 for clockwise rotations, -1 for counter-clockwise
    long           AbsolutePosition;   // Number of steps from HOME position
    long           DeltaPosition;      // Number of steps from last position
    long           TargetPosition;     // Target position for end of rotation
    long           LowerLimit;         // Minimum step position
    long           UpperLimit;         // Maximum step position
    long           RampSteps;          // Total number of steps during ramping
    long           RampDownStep;       // Step at which to start ramping down
    long           Velocity;           // Current velocity of motor
    long           VelocityIncrement;  // Velocity adjustment for ramping (determined by ramp factor)
    long           NextPosition;       // Position after next step
    unsigned long  NextStepMicros;     // Target micros for next step

    void startRotation ();
    void doStep ();

  public:
    StepperMotorController (int enablePin=2, int directionPin=3, int stepPin=4, int llSwitchPin=0, int ulSwitchPin=0);

    RunReturn      Run                 ();  // Keeps the motor running (must be called from your loop() function with no delay)

    void           Enable              ();                                      // Enables the motor driver (energizes the motor)
    void           Disable             ();                                      // Disables the motor driver (releases the motor)

    void           FindHome            ();                                      // "Auto Home": Seek to lower limit switch and set home position just off it
    void           SetHomePosition     ();                                      // Sets the current position of the motor as its HOME position (Sets Absolute position to zero)
    void           SetLowerLimit       (long lowerLimit);                       // Sets the LOWER LIMIT (minimum Absolute Position) of the motor's range
    void           SetUpperLimit       (long upperLimit);                       // Sets the UPPER LIMIT (maximum Absolute Position) of the motor's range
    void           SetRamp             (int ramp);                              // Sets the trapezoidal velocity RAMP (up/down) for smooth motor start and stop

    void           RotateAbsolute      (long absPosition, int stepsPerSecond);  // Rotates motor to an Absolute target position from its HOME position
    void           RotateRelative      (long numSteps, int stepsPerSecond);     // Rotates motor clockwise(+) or counter-clockwise(-) any number of steps from its current position
    void           RotateToHome        ();                                      // Rotates motor to its HOME position
    void           RotateToLowerLimit  ();                                      // Rotates motor to its LOWER LIMIT position
    void           RotateToUpperLimit  ();                                      // Rotates motor to its UPPER LIMIT position
    void           EStop               ();                                      // Stops the motor immediately (emergency stop)

    bool           IsHomed             ();                                      // Returns true or false
    MotorState     GetState            ();                                      // Returns current state of motor
    long           GetAbsolutePosition ();                                      // Returns the motor's current step position relative to its HOME position
    long           GetRelativePosition ();                                      // Returns the motor's current step position relative to its last targeted position
    long           GetLowerLimit       ();                                      // Returns the motor's Absolute LOWER LIMIT position
    long           GetUpperLimit       ();                                      // Returns the motor's Absolute UPPER LIMIT position
    unsigned long  GetRemainingTime    ();                                      // Return the remaining time in ms when rotation will complete
    const char *   GetVersion          ();                                      // Returns this firmware's current version
    void           BlinkLED            (int LEDpin);                            // Blink the specified LED to indicate identification

    const char *   ExecuteCommand      (const char *packet);                    // Execute a stepper motor function by string command (see notes above)
};

#endif
