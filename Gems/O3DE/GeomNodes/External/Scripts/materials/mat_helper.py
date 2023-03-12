import bpy

def get_material_node_attributes(node, target_properties: dict, use_o3de_mat_names=False):
    attribute_dictionary = {}
    for material_input in node.inputs:
        if material_input.name in target_properties.keys():
            try:
                material_input_name = get_material_input_name(material_input.name, target_properties, use_o3de_mat_names)
                attribute_dictionary[material_input_name] = get_dict_safe_value(material_input.default_value)
            except AttributeError:
                pass
    return attribute_dictionary

def get_material_input_name(material_input_name: str, target_properties: dict, use_o3de_mat_names=False):
    return target_properties[material_input_name] if use_o3de_mat_names else material_input_name

def get_dict_safe_value(value):
    if type(value) == bpy.types.bpy_prop_array:
        return list(value)
    else:
        return value