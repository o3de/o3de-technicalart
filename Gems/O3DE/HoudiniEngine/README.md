# HoudiniEngine

This gem enables the use to use Houdini in O3DE. This is a port from Houdini's HoudiniEngine Unreal plugin. 

# Supports: Tested on Houdini 19.5.0.605, Windows 10/11

### **Notes**
- Make sure to have the necessary Houdini license to be able to run the gem. i.e. Houdini FX license.
- Currently supports simple loading of HDA files and not all functionalities are ported yet. This is supported via the HoudiniDigitalAsset component.
- HDAs with Spline/Curve are detected and presented as a Spline Component that user can modify.
- Has basic Material support.
- Baking to FBX is supported. An entity with a Mesh component is created upon baking.

### Loading HDAs
- All HDAs needed to be in a specific folder under HoudiniEngine/DigitalAssets. There are initial work to support loading via the File Browser.
- Upon gem startup all HDAs are loaded initially. 
- Create an entity and add a HoudiniDigitalAsset component.
- Under the operator field choose the HDA object that you want to load.
- After loading, a model should be rendering on your editor.

