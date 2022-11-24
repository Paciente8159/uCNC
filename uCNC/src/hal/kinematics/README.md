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
