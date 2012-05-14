Utility for adding collision data to NIF file (Skyrim version) from NIF file (Morrowind?
version / NiTriShape data) or OBJ file using 2nd NIF file (Skyrim version) as template.

--------------------------------------------------------------------------------------------
SOFTWARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.

AUTHOR DISCLAIMS ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, WITH
RESPECT TO THE PROGRAM. THE END-USER ASSUMES ALL RISK AS TO THE SUITABILITY, QUALITY,
AND PERFORMANCE OF THE PROGRAM. IN NO EVENT WILL AUTHOR, OR ITS DIRECTORS, OFFICERS,
EMPLOYEES OR AFFILIATES, BE LIABLE TO THE END-USER FOR ANY CONSEQUENTIAL, INCIDENTAL,
INDIRECT, SPECIAL OR EXEMPLARY DAMAGES (INCLUDING DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF DATA OR BUSINESS INFORMATION, AND THE LIKE) ARISING OUT
OF THE USE OF OR INABILITY TO USE THE PROGRAM OR ACCOMPANYING WRITTEN MATERIALS, EVEN
IF AUTHOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
--------------------------------------------------------------------------------------------


THANKS TO:

- Piotr Pluta: HavokUtilities (http://piotrpluta.opol.pl/programming/havok-physics-tutorial-update-havok-2010-1/)
- neomonkeus from NifTools forum for his global support
- ttl269 from NifTools forum for his support decoding bhkCompressedMeshShpae
- Macoron3 from NifTools forum for his tipp creating collision data from other shpaes than RootCollisionObject
- and many others :-)


PREREQUISITES:

- Havok SDK (http://software.intel.com/sites/havok/en/)
  Havok_Physics_Animation_2010-2-0_PC_XS_win32_VS2005_keycode_perpetual_20101115.zip
- MS DirectX9 SDK (http://www.microsoft.com/en-us/download/details.aspx?id=6812)
  DXSDK_Jun10.exe
- MS Visual Studio 2005
- NifUtils (https://github.com/skyfox69/NifUtils)
  git clone --recursive https://skyfox69@github.com/skyfox69/NifUtils.git


USAGE:

NifConvert <path to Skyrim> <path to templates>

1. Choose NIF-File (Skyrim version) collision data should be injected
2. Choose NIF-file for collision source
3. Select template from ComboBox
4. Select handling of collission source (collision data or shape meshes)
6. Press Convert-button

