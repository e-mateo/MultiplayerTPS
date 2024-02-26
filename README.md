# Mulitplayer TPS

![png](./Images/Game1.png)<br>
Portfolio Link: https://merbistigp.editorx.io/portfolio-mateo/blank-3-3-1-1 <br>

## Introduction
Third Person Multi is a game project made on Unreal Engine 5, focused on Unreal's
Network system. <br>
The objective was to transition an offline TPS shooter to a Client/Server multiplayer one.<br>
This project has been made by RABINNE Lenny and ERBISTI Matéo, programming
students at Isart Digital Paris.<br>
It started on November 2nd 2023, and finished on November 28th 2023.<br>

## How to open the project
To launch a server open StartDedicatedServer.bat. You can open the .bat file with the
modify option and make sure that the Unreal Engine path matches your own.<br>
If it’s the first time you're launching the project, you will need to build it (a pop asking you to
do it will appear).<br>
You can then start adding clients by opening StartClient.bat. Be careful about the
Unreal Engine path in the .bat as for the dedicated server.<br>
We have made LAN sessions so you must be on the same network as the dedicated server.<br>

## Game:
The game is a Team Deathmatch that lasts for 5 minutes or stops if one team goes to 20
kills. You either get points by killing a player from another team or if an opponent dies no
matter how.<br>
There are also AI spawners which you should pay attention to because AI will try to kill you. <br>
You can interact with AI spawners to spawn friendly AI during a short time that will only hunt players of the other team. <br>

## Controls:
• WASD: Move<br>
• Space: Jump<br>
• LMB: Shoot<br>
• RMB: Aim<br>
• E: Interact with buttons<br>
• F: Punch<br>
• R: Reload<br>
• Shift: Sprint<br>
• TAB: Display Scoreboard<br>

## Tasks I've worked on:

• Updated player and weapon functionalities to work in multiplayer.<br>
• Replicated AIs and AI spawners to work on each client.<br>
• Replicated Health pack and munition spawners.<br>
• Created lobby and sessions with a dedicated server.<br>
• Created Kill feed and Leaderboard.<br>
• Updated the HUD to work on each client.<be>

![png](./Images/Kill.png)<br>
![png](./Images/AIs.png)<br>
![png](./Images/Leaderboard.png)<br>
![png](./Images/Sessions.png)<br>
