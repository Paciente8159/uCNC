<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>


# µCNC
µCNC - Universal CNC firmware for microcontrollers

## System menu

System menu is a module that acts an integrated manager to control displays and handle user actions for the several screens that are presented to the user.
Extension modules can make use of the system menu to add custom menus for the user.
All generated menus can be translated.

### Screens and overridable functions

System menus have a set of overridable functions that can be customized to fit the screen format and display capabilities that can be broken in to two types:
 - Custom screens (like the startup screen, the idle screen and the alarm screen, and popups)
 - Structured screens (like menu screens)

Each custom screen can be taylored to best fit a particular display format.
Menu screens are rendered in a different way. A menu screen is composed of a head section, a nav back element, then each menu entry item or a single item in edit mode and in the end a footer section. Menu items can be filtered so that only visible elements are rendered depending on were the user is inside the current menu.

Only the the overrided screens and render callbacks will be displayed. All others will not generate any display feedback.

### User actions and menu interactions

System menu works by calling two main funtions inside the main loop. These are ´system_menu_action´ and ´system_menu_render´ (the calling of these functions must be done by the module that makes use of the system menu, like for example the display menu).

´system_menu_action´ - receives a value that indicates a user action and sets the current navigation internal logic (page, selected item, etc...). Currently the system module handles 4 different actions:
 - No action (when there is no user activity)
 - User selected and item/or enter edit mode/or execute action
 - User moved to the next item/or increment in edit mode
 - User moved to the previous item/or decrement in edit mode
Custom actions can also be taylored to menu items, extending the range of user actions depending on the type of feedback system that is used (rotary knobs, keyboards, touch screens, etc..).

´system_menu_render´ - renders the screen (if it's on a custom screen), according to the current menu page, type, position, etc. Menu items can navigate to other menu pages or also have fully custom rendered screens.

There are several built in simple actions that are available via system menu module like calling serial commands, calling real time commands, navigate through the system menu, and utility functions like converting numbers and number arrays to strings to be rendered.

### Static and dynamic menus

It's possible to generate both static and dynamic menus.
Static menus are declarated in a static way. Menus are composed by a menu page that can contain multiple menu items. Menu items can perform several actions like navigate to other pages, execute code, edit variables, etc..
In some cases static menus are not possible. For example when navigating though an SD card file system. In this case it's possible to generate dynamic menus. Dynamic menu pages can have custom action and render actions, to allow the content to be rendered/generated live.

