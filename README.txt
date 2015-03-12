                      TeamPlanets engine and bots README
                      ==================================

1. Introduction
---------------
   This package contains a distribution of the TeamPlanets game engine and a
set of bots. The set of bots consist in a complete bot "sage" and a series of 
bots implementing some simple strategies. "Sage" was written as a job offer test
exercise by MachineZone RUS. General solo game description can be found at 
http://planetwars.aichallenge.org/specification.php and this particular exercise
specification at http://nsk.machinezone.ru.

2. Package contents
-------------------
   This package contains the following files and directories:
      |- README.txt         - This file.
      |- INSTALL.txt        - Compilation and installation instructions.
      |- COPYING.txt        - This package distribution terms and conditions.
      |- technical_note.txt - High level description of the "sage" bot code and 
      |                       of the choices that was made during its
      |                       development.
      |- CMakeLists.txt     - General package building script.
      |- libs               - Common shared libraries
      |  |-libteamplanets   - The library containing the code common to the bots 
      |                       and the engine.
      |
      |- engine             - The game engine.
      |
      |- bots               - A set of bots
      |  |-bully            - A simple mildly aggressive strategy.
      |  |-bully_team       - A simply mildly aggressive strategy that don't 
      |  |                    attack its teammates.
      |  |-dual             - A somewhat balanced strategy.
      |  |-dual_team        - A somewhat balanced strategy that don't attack its
      |  |                    teammates.
      |  |-rage             - An aggressive strategy.
      |  |-rage_team        - An aggressive strategy that don't attack its 
      |  |                    teammates.
      |  |-sage             - The bot that was written for MachineZone RUS. 
      |  |                    See technical_note.txt for details.
      |  |-sage0            - An early version of "sage" without prediction.
      |
      |-maps                - A set of testing maps borrowed from 
                              http://nsk.machinezone.ru

3. Installation instructions
----------------------------
   Please refer yourself to the file INSTALL.txt for the list of this package 
external dependencies, the compilation and the installation instructions.

4. Usage
--------
   After the successful compilation, the bots and the engine executables will be
placed in the "bin" sub-directory of the installation path. The bots executables 
can directly be launched and used by the engine following the specifications at 
http://nsk.machinezone.ru.

   To launch the engine, execute teamplanets_engine. To begin a battle, select
File->Start battle... In the popped dialog box, select the map, a bot executable 
for each team and the number of teammates for each team. Don't select more 
teammates than the map allows, for the maps shipped with this package the max 
number of bots is the second number in the map name. The log of the battle is 
written to the standard output.

   Alternatively, you can launch a battle directly from the command line. Simply
execute:
   $teamplanets_engine <MAP> <TEAM_1_BOT> <TEAM_1_NUM_PLAYERS> \
                       <TEAM_2_BOT> <TEAM_2_NUM_PLAYERS>

Have fun!
 
                                    Vadim Litvinov
                                    12 mars 2015
