# Dark Souls 2: SOTFS BabyJump & No-logo Mod
## Description
When Dark Souls II is first opened, this dll is automatically loaded. As long as the Dinput8.dll is in
your game folder when you start the game, the "NoBabyJ (full jump length)" script from the sotfs bbj mode ce table is injected and automatically enabled. It remains active until the game closes. 
This has been written to make it simpler for newer speedrunners to use, to save the hassle 
of opening and enabling CE each time you open the game, and to clear up confusion on which version of the ce table should be used.

The No-logo mod from r3sus has been merged into this repository and is included in this dll.

## Install (one-time)
 - [For BBJ mod only]: Download the Dinput8.dll file from the Releases, put it into the sotfs game folder (often found at C:\Program Files (x86)\Steam\steamapps\common\Dark Souls II Scholar of the First Sin\Game). Done.
 - [For BBJ mod with nologo]: Download the Dinput8_nologo.dll file from the Releases, rename it to Dinput8.dll, put it into the sotfs game folder (often found at C:\Program Files (x86)\Steam\steamapps\common\Dark Souls II Scholar of the First Sin\Game). Done.
 
Note: If a Dinput8.dll file already exists in the game folder, either rename that file to something else, or overwrite it.

## Uninstall
Rename or delete Dinput8.dll from the game folder.

## FAQ
* Will this work with the new Online update for Sotfs?
Yes this will work both if you have patched your game with the new update, or if you have remain unpatched.

* Does this dll work with vanilla DS2?
No, please see the vanilla version at https://github.com/pseudostripy/bbj_mod/releases/latest.

* Why does my Cheat Engine table not work when this mod is installed?
The CE tables should now work with the latest version of the bbj in Releases. The bbj mod inject location has now been moved to avoid overwriting the ArrayOfBytes that was used for initialising many of the CE tables.

* Why does this code look so much more complicated that the equivalent vanilla version?
Basically because sotfs is 64-bit. It has reduced in complexity with the most recent update and should be more maintainable now.

* Should I use the nologo mod?
    * If you are speedrunning it is advisable to use only the ordinary bbj mod (named Dinput8.dll in Releases) as it has minor impacts if you need to Alt F4 during a run.
    * If not, you may as well use the nologo mod (named Dinput8_nologo.dll in Releases) as it saves a couple of seconds when loading the game. See #Install section above.

* I want the no-logo mod but not the bbj dll, is this possible?
Yes, please find the no-logo only dll from r3sus here: https://github.com/r3sus/Resouls/tree/main/ds2s/mods/nologo.

* Why is the dll with the inclusion of the no-logo mod smaller?
This is now N/A, the files are the same size. Previously it was due to using different compiler settings.

* I am receiving the following error message when building: "cc1.exe: sorry, unimplemented: 64-bit mode not compiled"?
You either don't have mingw64 installed, or you have the 32-bit version installed and with higher %PATH% environment variable precedence.

* Why is there a new flag in the build command?
This is to tell the linker where to find the GetModuleInformation header function that is used for differentiating versions of DS2, and thus making the mod more versatile. Previously this method was not called and thus specific dlls did not need to be explicitly linked.

## Credit
* NEZ64 for a working .dll example found at https://github.com/NEZ64/DarkSoulsOfflineLogoSkip.

* B3LYP for the original baby jump mod code written as a Cheat Engine table and from which this dll is simply a conversion.

* r3sus for no-logo mod investigation and addition.
