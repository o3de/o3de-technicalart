# GeomNodes

# Supports: Tested on Blender 3.4, Windows 10/11

This gem enables the user to use Blender's Geometry Nodes in O3DE.

![GN-Readme-1-d](https://user-images.githubusercontent.com/874957/231731997-11ee16f0-676a-431a-991e-eeceb75f26da.gif)

## Test files
https://paveloliva.gumroad.com/l/buildify

https://www.blender.org/download/demo-files/#geometry-nodes

### **Using the Gem**
- Add the gem to your project.
- Make sure that you have Blender 3.4 installed in your machine.
    ![GN-Settings](https://user-images.githubusercontent.com/874957/231714280-00171ca1-bf46-4054-8bce-1af73b2b98b0.png)
    If you installed Blender in another location make sure to set it here.
    ![GN-Settings-2](https://user-images.githubusercontent.com/874957/231714384-cc1ba6f6-07f2-4ed9-b96a-ec00f1a45d5c.png)
- Add the **Geometry Node Component**

    ![GN-Component](https://user-images.githubusercontent.com/874957/231714527-a408dce5-e063-45d3-aebf-3965b99205de.png)
- Choose your Blender file. Make sure it has a working Geometry Node modifier.
    ![GN-Component-1](https://user-images.githubusercontent.com/874957/231714577-8219d43d-d158-41ba-b39d-cd5fed3fcad9.png)
- After loading the file. The component will show the Geometry Nodes Parameters that you can modify. You can switch objects as well if available.
    ![GN-Component-Parameters](https://user-images.githubusercontent.com/874957/231714641-24f5af1c-5aa3-46e4-ae2d-77d1d99dc7a0.png)
### **Exporting the object to a Static Mesh**
Clicking the export button will start the exporting process. It might take a while depending on the size of the model but you can work on other things while waiting for it and it will be automatically setup for you.

### **Notes**
- You can create up to 10 multiple entities with Geometry Node component.
