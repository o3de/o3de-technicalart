# GeomNodes

# Supports: Tested on Blender 3.4, Windows 10/11

This gem enables the user to use Blender's Geometry Nodes in O3DE.

![GN-Readme-1](Docs/GN-Readme-1.gif)
## Test files
https://paveloliva.gumroad.com/l/buildify

https://www.blender.org/download/demo-files/#geometry-nodes

### **Using the Gem**
- Add the gem to your project.
- Make sure that you have Blender 3.4 installed in your machine.
    ![GN-Settings](Docs/GN-Settings.png)

    If you installed Blender in another location make sure to set it here.
    ![GN-Settings-2](Docs/GN-Settings-2.png)
- Add the **Geometry Node Component**
    ![GN-Component](Docs/GN-Component.png)
- Choose your Blender file. Make sure it has a working Geometry Node modifier.
    ![GN-Component-1](Docs/GN-Component-1.png)
- After loading the file. The component will show the Geometry Nodes Parameters that you can modify. You can switch objects as well if available.
    ![GN-Component-Parameters](Docs/GN-Component-Parameters.png)

### **Exporting the object to a Static Mesh**
Clicking the export button will start the exporting process. It might take a while depending on the size of the model but you can work on other things while waiting for it and it will be automatically setup for you.

### **Notes**
- You can create up to 10 multiple entities with Geometry Node component.
