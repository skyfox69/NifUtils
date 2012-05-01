Utility for creating correct model data NIF file (Skyrim version) from
NIF file (Morrowind? version) and 2nd NIF (Skyrim version) used as
template.

Uses modified niflib from git://niftools.git.sourceforge.net/gitroot/niftools/niflib

Code comes without any warranty and 'as is'. 

Comments:
- Code is 'quick and very! dirty' ;-)
- didn't got niflib compiled as lib, so files are included in project
- compiled with MSVC 2005
- many changes/add-ons to niflib matching actual NIF structure

Usage:
1. Choose NIF-file (Morrowind version?) containing model.
2. Choose NIF-file (Skyrim version) as template.
3. Choose NIF-file for output.
4. Manually change of texture file names and path with NifSkope
5. Use ChunkMerge to generate correct collision model