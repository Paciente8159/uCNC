<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>

# µCNC

µCNC - Universal CNC firmware for microcontrollers

# µCNC data

Insert your files to be written to the board file system here. For example the index.html.gz file for web plugins

# Web pendant module

Add your index.html.gz file here. This contains the web pendant page to be served by the controller

# ATC module

The automatic tool changer adds the capability of running custom gcode files (.nc) that perform the tasks required to unmount and mount new tools.
The atc files should be placed on an atc directory inside the root of the drive (C for internal flash drive or D for SD card module).

Each tool should have a mount file:
  - tool<tool_number>mnt.nc for the tool mounting operation
    - Example /C/atc/tool1mnt.nc (for tool 1) or /atc/tool12mnt.nc (for tool 12)
  - tool<tool_number>umnt.nc for the tool unmounting operation
    - Example /C/atc/tool1umnt.nc (for tool 1) or /atc/tool12umnt.nc (for tool 12)
