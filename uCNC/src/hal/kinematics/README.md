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

Settings $100 and $101 have a different meaning, they
use revolution instead of mm.


## The kinematics HAL
This HAL is manages the way the linear actuators and the 3D Cartesian space axis relate to each other. 
   * Converts linear actuators (steppers) in to X,Y,Z,A,B,C coordinates and back.
   * Converts any X,Y,Z transformation to compensate for non perpendicular axis (un-skew) and back.
   * Defines the order of the axis movements when homing.

### Creating the HAL for a custom kinematics
   **2/3 steps are needed:**

   **1. Implement all functions defined in the kinematics.h**

   All functions defined by the ```kinematics.h``` must be implemented except for ```kinematics_apply_transform``` and ```kinematics_apply_reverse_transform```. These are: 

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

**3. Custom kinematic settings integration to µCNC**

Your kinematic may have custom variables that are configurable by the user via $ command and storable in non volatile memory
To integrate these variables with the µCNC global settings system you need to declare 3 macros:
These are `KINEMATICS_VARS_DECL`, `KINEMATICS_VARS_DEFAULTS_INIT` and `KINEMATICS_VARS_SETTINGS_INIT`:
The settings API uses 4 types of variables (bool, uint8_t, uint16_t and float)
If you need uint32_t you should store it as a float. You can also use arrays of these types of variables

```
	/**
	 * this declares the variables to be used
	 */
	#define KINEMATICS_VARS_DECL   \
		float my_kynematic_var1;     \
		uint8_t my_kynematic_var2;   \
		uint16_t my_kynematic_var3[3];
```

```
	/**
	 * this initializes each variable with the default values
	 */
	#define KINEMATICS_VARS_DEFAULTS_INIT   \
		.my_kynematic_var1 = 1.5f,             \
		.my_kynematic_var2 = 3,     \
		.my_kynematic_var3 = {0, 1, 2},
```

```
	/**
	 * this integrates the variables in the global settings (global g_settings struct) API and assigned each setting an ID and the appropriate type
	 * for example to access my_kynematic_var1 you assign id 28 then you can use command $28= to change it's value
	 * in the case of an array you assign the id of the first index and all subsequent values allow to access the other array positions
	 * for example my_kynematic_var3 id is 106. To access my_kynematic_var3[1] you access id 106 + 1 that is $107=
	 *
	 * check the types [here](https://github.com/Paciente8159/uCNC/blob/2366e954879c59935c8d9c40f73a8b78f06f2e2c/uCNC/src/interface/grbl_settings.h#L178)
	 * Also note that the global g_settings struct expects the ID's to be in the range of 0-255
	 * All ID's from id 256 and above are managed via the [extended settings API](https://github.com/Paciente8159/uCNC/blob/8d94b616589c538a663df2833017272f4e5ee7ec/uCNC/src/interface/grbl_settings.h#L226)
   *
	 */
	#define KINEMATICS_VARS_SETTINGS_INIT   \
		{.id = 28, .memptr = &g_settings.my_kynematic_var1, .type = SETTING_TYPE_FLOAT},																			\
		{.id = 29, .memptr = &g_settings.my_kynematic_var2, .type = SETTING_TYPE_UINT8},																			\
		{.id = 106, .memptr = &g_settings.my_kynematic_var3, .type = SETTING_TYPE_UINT16 | SETTING_ARRAY | SETTING_ARRCNT(3)},
```

You can also integrate this into the system menu API to allow these variables to be edited via displays that work over the system menu module
For this you just create the entries in the menu screen you want to add the settings. The menu screens ID's can be found [here](https://github.com/Paciente8159/uCNC/blob/2366e954879c59935c8d9c40f73a8b78f06f2e2c/uCNC/src/modules/system_menu.h#L68)
Menu and menu items can be added via the system menu macros available [here](https://github.com/Paciente8159/uCNC/blob/2366e954879c59935c8d9c40f73a8b78f06f2e2c/uCNC/src/modules/system_menu.h#L160)

Here is an example for adding the settings in the kinematics menu

```
	/**
	 * this integrates the variables in the global settings (g_settings struct) API also in the system menu do allow edit in a display
	 * for arrays you need to declare them explicitly
	 */
	#define KINEMATICS_VARS_SYSTEM_MENU_INIT   \
		DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s28, "kynematic var1", &g_settings.my_kynematic_var1, VAR_TYPE_FLOAT); \
		DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s29, "kynematic var2", &g_settings.my_kynematic_var2, VAR_TYPE_UINT8); \
		DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s106, "kynematic var3[0]", &g_settings.my_kynematic_var3[0], VAR_TYPE_UINT16); \
		DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s107, "kynematic var3[1]", &g_settings.my_kynematic_var3[1], VAR_TYPE_UINT16); \
		DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s108, "kynematic var3[2]", &g_settings.my_kynematic_var3[2], VAR_TYPE_UINT16);
```
		