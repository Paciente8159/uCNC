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

System menu works by calling two main funtions inside the main loop. These are `system_menu_action` and `system_menu_render` (the calling of these functions must be done by the module that makes use of the system menu, like for example the display menu).

`system_menu_action` - receives a value that indicates a user action and sets the current navigation internal logic (page, selected item, etc...). Currently the system module handles 4 different actions:
 - No action (when there is no user activity)
 - User selected and item/or enter edit mode/or execute action
 - User moved to the next item/or increment in edit mode
 - User moved to the previous item/or decrement in edit mode
Custom actions can also be taylored to menu items, extending the range of user actions depending on the type of feedback system that is used (rotary knobs, keyboards, touch screens, etc..).

`system_menu_render` - renders the screen (if it's on a custom screen), according to the current menu page, type, position, etc. Menu items can navigate to other menu pages or also have fully custom rendered screens.

There are several built in simple actions that are available via system menu module like calling serial commands, calling real time commands, navigate through the system menu, and utility functions like converting numbers and number arrays to strings to be rendered.

### Static and dynamic menus

It's possible to generate both static and dynamic menus.
Static menus are declarated in a static way. Menus are composed by a menu page that can contain multiple menu items. Menu items can perform several actions like navigate to other pages, execute code, edit variables, etc..
In some cases static menus are not possible. For example when navigating though an SD card file system. In this case it's possible to generate dynamic menus. Dynamic menu pages can have custom action and render actions, to allow the content to be rendered/generated live.

## General overview of the system_menu components

All menus and screens support translation through the language subsystem.

The system menu manages:

  - Display rendering for all system screens
  - Navigation between screens
  - User input handling (navigation, selection, editing)
  - Menu item filtering and visibility
  - Modal popups
  - Idle and startup screen behavior
  - Integration points for extension modules to append custom menus

The module is implemented in system_menu.c and configured through system_menu.h.

### Screen Types
The system menu supports two major categories of screens:

#### Custom Screens
These are fully user‑defined and manually rendered. Examples include:

  - Startup screen
  - Idle screen
  - Alarm screen
  - Modal popups

Custom screens allow complete control over layout and rendering, making them suitable for displays with unique constraints or for presenting non‑menu information.

Relevant functions (from system_menu.c)

  - `system_menu_startup()` – renders the startup screen
  - `system_menu_idle()` – renders the idle screen
  - `system_menu_show_modal_popup()` – displays modal popups with timeout handling

#### Structured Menu Screens
These screens follow a standardized layout:

  - Header section
  - Navigation “back” element
  - The body section that includes
    - List of (visible depends on context like screen size/current menu position) menu items (or a single item in edit mode)
  - Footer section

Menu screens support:

  - Automatic rendering
  - Item filtering (visibility based on context)
  - Edit modes (modify, edit, simple edit, select) defined in system_menu.h
  - Navigation between parent/child menus

Relevant functions

  - `system_menu_render()` – main rendering entry point for structured screens. This function is the heart of the system_menu API for handling rendering
  - `system_menu_get_item_count()` – counts visible items based on filters
  - `system_menu_get_current_item()` – retrieves the active item
  - `system_menu_action_nav_back()` – handles back navigation

Menu Modes

__Defined in system_menu.h:__

  - SYSTEM_MENU_MODE_NONE – no active mode
  - SYSTEM_MENU_MODE_REDRAW – forces a redraw
  - SYSTEM_MENU_MODE_SELECT – selection mode
  - SYSTEM_MENU_MODE_SIMPLE_EDIT – simple edit mode
  - SYSTEM_MENU_MODE_EDIT – full edit mode
  - SYSTEM_MENU_MODE_MODIFY – modify existing value
  - SYSTEM_MENU_MODE_DELAYED_REDRAW – redraw after a delay
  - SYSTEM_MENU_MODE_MODAL_POPUP – modal popup active

These modes determine how user input is interpreted and how the screen is rendered.

Menu Identifiers

Also defined in system_menu.h:

  - SYSTEM_MENU_ID_STARTUP
  - SYSTEM_MENU_ID_IDLE
  - SYSTEM_MENU_ID_MAIN_MENU
  - SYSTEM_MENU_ID_SETTINGS
  - SYSTEM_MENU_ID_HOMING

These IDs allow the system to switch between screens using system_menu_goto().

Navigation and Actions

The system menu processes user actions through:

  - `system_menu_action()` – central dispatcher for user input events. This function is the heart of the system_menu API for handling events
  - `system_menu_action_settings_cmd()` – handles settings menu actions
  - `system_menu_action_jog()` – handles jog‑related actions
  - `system_menu_action_overrides()` – handles override adjustments (feed, spindle, etc.)
  - `system_menu_action_timeout()` – handles idle timeouts and popup expiration

Navigation between screens is performed using:

  - `system_menu_goto()` – switches to a new menu or screen ID
  - `system_menu_main_open()` – opens the main menu from idle or startup

Menu Items

Menu items are dynamically appended using:

  - `system_menu_append_item()` – adds a single menu entry
  - `system_menu_append()` – adds multiple items or a menu block

Items can include:

  - Commands
  - Submenus
  - Editable values
  - Selectable options
  - Visibility filtering ensures only relevant items appear depending on the current context or machine state.

Rendering Details

Rendering is performed through:

  - `system_menu_render()` – orchestrates the entire screen drawing process
  - `system_menu_render_axis_position()` – helper for displaying axis positions on certain screens

Only overridden callbacks are rendered; non‑overridden screens produce no output.

Time‑Based Behavior
Timing constants defined in system_menu.h include:

  - SYSTEM_MENU_GO_IDLE_MS – delay before returning to idle screen
  - SYSTEM_MENU_REDRAW_STARTUP_MS – startup screen redraw interval
  - SYSTEM_MENU_REDRAW_IDLE_MS – idle screen redraw interval
  - SYSTEM_MENU_MODAL_POPUP_MS – popup display duration

These values control automatic transitions and redraw behavior.

Modal Popups

Modal popups interrupt normal menu flow and are displayed using:

  - `system_menu_show_modal_popup()`

Popups automatically dismiss after SYSTEM_MENU_MODAL_POPUP_MS milliseconds unless overridden.

Extension Module Integration

Modules can register their own menus by calling:

  - `system_menu_append()`
  - `system_menu_append_item()`

This allows:

  - Custom configuration menus
  - Diagnostics screens
  - Feature‑specific UI elements

All added menus automatically support translation and structured rendering.


