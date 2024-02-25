<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>


# µCNC
µCNC - Universal CNC firmware for microcontrollers

## Kinematics
Currently µCNC support the following kinematics

### Cartesian
Cartesian kinematics is the standard supported kinematics and one of the most common type of kinematics used.
The settings to configure this type of kinematics are similar to Grbl and can be checked at the [Configuration page](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#%C2%B5CNC-configurations)

Additionaly this type of kinematics support skew compensation feature that uses the following settings:

| Setting | Description |
| --- | --- |
| $37         | [XY skew compensation](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#37-to-39---axis-skew-compensation)     |
| $38         | [XZ skew compensation](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#37-to-39---axis-skew-compensation)     |
| $39         | [YZ skew compensation](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#37-to-39---axis-skew-compensation)     |

### CoreXY
CoreXY kinematics is supported.
The settings to configure this type of kinematics are similar to Grbl and can be checked at the [Configuration page](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#%C2%B5CNC-configurations)

### Linear Delta
Linear Delta robot kinematics is supported.
The settings to configure this type of kinematics can be checked at the [Configuration page](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#%C2%B5CNC-configurations)

In addition to the standard configurations you need to set 2 extra settings:

| Setting | Description |
| --- | --- |
| $106        | [Linear delta kinematic arm length, mm](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#106---linear-delta-kinematic-arm-length-mm)             |
| $107        | [Linear delta kinematic radial difference between the arm's tower joints and the effector joints, mm](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#107---linear-delta-kinematic-radial-difference-between-the-arms-tower-joints-and-the-effector-joints-mm)             |

### Delta
Delta robot (rotary) kinematics is supported.
The settings to configure this type of kinematics can be checked at the [Configuration page](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#%C2%B5CNC-configurations)

In addition to the standard configurations you need to set 5 extra settings:

| Setting | Description |
| --- | --- |
| $28        | [Delta homing bicep angle, degrees](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#27---delta-homing-biceps-angle-degrees)             |
| $106        | [Delta base radius, mm](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#106---delta-base-radius-mm)             |
| $107        | [Delta effector radius, mm](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#107---delta-effector-radius-mm)             |
| $108        | [Delta bicep length, mm](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#108---delta-bicep-length-mm)             |
| $109        | [Delta forearm length, mm](https://github.com/Paciente8159/uCNC/wiki/Basic-user-guide#109---delta-forearm-length-mm)             |

For more information please head to the [µCNC wiki page](https://github.com/Paciente8159/uCNC/wiki)

### SCARA

SCARA kinematic is supported. µCNC supports 2 implementations of SCARA kinematic:
* standard serial SCARA. The shoulder motor is fixed to the base and rotates
  the arm. The elbow motor is fixed to the arm and rotates the forearm. When the
	shoulder motor rotates and the elbow motor is stopped, the angle between the
	arm and the forearm is fixed, and the angle of the arm changes in relation
	to the base.
* serial SCARA implementation with fixed motors, known as MP SCARA (mostly
  printed SCARA). Both shoulder and  elbow motors are fixed to the base. A
	system of pulleys and closed belts is used to transfer motion from the elbow
	motor to elbow joint. When the shoulder motor rotates and the elbow motor is
	stopped, the angle of the arm changes in relation	to the base, and the angle
	between the forearm and the base is fixed.

To use MP SCARA kinematic, define MP_SCARA.

In addition to the standard configurations you need to set the following extra settings:

| Setting | Description |
| --- | --- |
| $28        | SCARA arm homing angle, degrees
| $29        | SCARA forearm homing angle, degrees
| $106       | SCARA arm length, mm
| $107       | SCARA forearm length, mm

Settings $100, $101, $110, $111, $120 and $121 have a different meaning, they
use revolution instead of mm.


## The kinematics HAL
This HAL is manages the way the linear actuators and the 3D Cartesian space axis relate to each other. 
   * Converts linear actuators (steppers) in to X,Y,Z,A,B,C coordinates and back.
   * Converts any X,Y,Z transformation to compensate for non perpendicular axis (un-skew) and back.
   * Defines the order of the axis movements when homing.

### Creating the HAL for a custom kinematics
   **2 steps are needed:**

   **1. Implement all functions defined in the kinematics.h**

   All functions defined by the ```kinematics.h``` must be implemented. These are: 

   ```
    /**
	 * @brief Initializes the kinematic system.
	 * This can be for example read the non volatile memory parameters for that kinematics and store them in RAM
     * Any other kind of initialization procedure can be done
	 */
   	void kinematics_init(void);

	/**
	 * @brief Converts from machine absolute coordinates to step position.
	 * This is done after computing position relative to the active coordinate system
	 *
	 * @param axis Position in world coordinates
	 * @param steps Position in steps
	 */

	void kinematics_apply_inverse(float *axis, int32_t *steps);

	/**
	 * @brief Converts from step position to machine absolute coordinates.
	 * This is done after computing position relative to the active coordinate system
	 *
	 * @param steps Position in steps
	 * @param axis Position in world coordinates
	 */
	void kinematics_apply_forward(int32_t *steps, float *axis);

	/**
	 * @brief Executes the homing motion for the machine.
	 * The homing motion for each axis is defined in the motion control.
	 * In the kinematics home function the axis order of the homing motion and other custom motions can be defined
	 *
	 * @return uint8_t Error status
	 */
	uint8_t kinematics_home(void);

	/**
	 * @brief Aplies a transformation to the position sent to planner.
	 * This is aplied only on normal and jog moves. Homing motions go directly to planner.
	 *
	 * @param axis Target in absolute coordinates
	 */
	void kinematics_apply_transform(float *axis);

	/**
	 * @brief Aplies a reverse transformation to the position returned from the planner.
	 * This is aplied only on normal and jog moves. Homing motions go directly to planner.
	 *
	 * @param axis Target in absolute coordinates
	 */
	void kinematics_apply_reverse_transform(float *axis);

	/**
	 * @brief Checks if the desired target is inside sofware boundries
	 *
	 * @param axis Target in absolute coordinates
	 * @return true If inside boundries
	 * @return false If outside boundries
	 */
	bool kinematics_check_boundaries(float *axis);

   ```

**2. Add the new kinematic library to µCNC**

   * Add the new kinematic to `kinematics.h` file and give it an ID. Add the needed libraries to load in the `kinematicdefs.h` file. 

Now just edit the config file to specify the kinemtics file you want to use and recompile µCNC for your board.
